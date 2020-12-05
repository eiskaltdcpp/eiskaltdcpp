/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2019 EiskaltDC++ developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdinc.h"

#include "FinishedManager.h"

#include "FinishedItem.h"
#include "FinishedManagerListener.h"
#include "Download.h"
#include "Upload.h"
#include "DownloadManager.h"
#include "QueueManager.h"
#include "UploadManager.h"
#include "ClientManager.h"

namespace dcpp {

FinishedManager::FinishedManager() {
    DownloadManager::getInstance()->addListener(this);
    UploadManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);
}

FinishedManager::~FinishedManager() {
    DownloadManager::getInstance()->removeListener(this);
    UploadManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);

    clearDLs();
    clearULs();
}

Lock FinishedManager::lock() {
    return Lock(cs);
}

const FinishedManager::MapByFile& FinishedManager::getMapByFile(bool upload) const {
    return upload ? ULByFile : DLByFile;
}

const FinishedManager::MapByUser& FinishedManager::getMapByUser(bool upload) const {
    return upload ? ULByUser : DLByUser;
}

void FinishedManager::remove(bool upload, const string& file) {
    {
        Lock l(cs);
        MapByFile& map = upload ? ULByFile : DLByFile;
        auto it = map.find(file);
        if(it != map.end())
            map.erase(it);
        else
            return;
    }
    fire(FinishedManagerListener::RemovedFile(), upload, file);
}

void FinishedManager::remove(bool upload, const HintedUser& user) {
    {
        Lock l(cs);
        MapByUser& map = upload ? ULByUser : DLByUser;
        auto it = map.find(user);
        if(it != map.end())
            map.erase(it);
        else
            return;
    }
    fire(FinishedManagerListener::RemovedUser(), upload, user);
}

void FinishedManager::removeAll(bool upload) {
    if(upload)
        clearULs();
    else
        clearDLs();
    fire(FinishedManagerListener::RemovedAll(), upload);
}

void FinishedManager::clearDLs() {
    Lock l(cs);
    DLByFile.clear();
    DLByUser.clear();
}

void FinishedManager::clearULs() {
    Lock l(cs);
    ULByFile.clear();
    ULByUser.clear();
}

void FinishedManager::getParams(const string & target, ParamMap& params) {
    Lock l(cs);

    auto it = DLByFile.find(target);
    if(it == DLByFile.end()) {
        return;
    }

    auto entry = it->second;
    if (!entry->getUsers().empty()) {
        StringList nicks, cids, ips, hubNames, hubUrls, temp;
        string ip;
        for(auto& i: entry->getUsers()) {

            nicks.push_back(Util::toString(ClientManager::getInstance()->getNicks(i)));
            cids.push_back(i.user->getCID().toBase32());

            ip.clear();
            if (i.user->isOnline()) {
                OnlineUser* u = ClientManager::getInstance()->findOnlineUser(i, false);
                if (u) {
                    ip = u->getIdentity().getIp();
                }
            }
            if (ip.empty()) {
                ip = _("Offline");
            }
            ips.push_back(ip);

            temp = ClientManager::getInstance()->getHubNames(i);
            if(temp.empty()) {
                temp.push_back(_("Offline"));
            }
            hubNames.push_back(Util::toString(temp));

            temp = ClientManager::getInstance()->getHubUrls(i);
            if(temp.empty()) {
                temp.push_back(_("Offline"));
            }
            hubUrls.push_back(Util::toString(temp));
        }

        params["userNI"] = Util::toString(nicks);
        params["userCID"] = Util::toString(cids);
        params["userI4"] = Util::toString(ips);
        params["hubNI"] = Util::toString(hubNames);
        params["hubURL"] = Util::toString(hubUrls);
    }

    params["fileSIsession"] = Util::toString(entry->getTransferred());
    params["fileSIsessionshort"] = Util::formatBytes(entry->getTransferred());
    params["fileSIactual"] = Util::toString(entry->getActual());
    params["fileSIactualshort"] = Util::formatBytes(entry->getActual());

    params["speed"] = str(F_("%1%/s") % Util::formatBytes(entry->getAverageSpeed()));
    params["time"] = Util::formatSeconds(entry->getMilliSeconds() / 1000);
}

void FinishedManager::onComplete(Transfer* t, bool upload, bool crc32Checked) {
    if(t->getType() == Transfer::TYPE_FILE || (t->getType() == Transfer::TYPE_FULL_LIST && BOOLSETTING(LOG_FILELIST_TRANSFERS))) {
        string file = t->getPath();
        const HintedUser& user = t->getHintedUser();

        uint64_t milliSeconds = GET_TICK() - t->getStart();
        time_t time = GET_TIME();

        int64_t size = 0, pos = 0;
        // get downloads' file size here to avoid deadlocks
        if(!upload) {
            if(t->getType() == Transfer::TYPE_FULL_LIST) {
                // find the correct extension of the downloaded file list
                file += ".xml";
                if(File::getSize(file) == -1) {
                    file += ".bz2";
                    if(File::getSize(file) == -1) {
                        // no file list?
                        return;
                    }
                }
                size = t->getSize();
            } else {
                QueueManager::getInstance()->getSizeInfo(size, pos, file);
                if (size == -1) {
                    // not in the queue anymore?
                    return;
                }
            }
        }

        Lock l(cs);

        {
            MapByFile& map = upload ? ULByFile : DLByFile;
            auto it = map.find(file);
            if(it == map.end()) {
                FinishedFileItemPtr p = new FinishedFileItem(
                            pos + t->getPos(),
                            milliSeconds,
                            time,
                            upload ? File::getSize(file) : size,
                            t->getActual(),
                            crc32Checked,
                            user
                            );
                map[file] = p;
                fire(FinishedManagerListener::AddedFile(), upload, file, p);
            } else {
                it->second->update(
                            crc32Checked ? 0 : t->getPos(), // in case of a successful crc check at the end we want to update the status only
                            milliSeconds,
                            time,
                            t->getActual(),
                            crc32Checked,
                            user
                            );
                // we still dispatch a FinishedFileItem pointer in case previous ones were ignored
                fire(FinishedManagerListener::UpdatedFile(), upload, file, it->second);
            }
        }

        {
            MapByUser& map = upload ? ULByUser : DLByUser;
            auto it = map.find(user);
            if(it == map.end()) {
                FinishedUserItemPtr p = new FinishedUserItem(
                            t->getPos(),
                            milliSeconds,
                            time,
                            file
                            );
                map[user] = p;
                fire(FinishedManagerListener::AddedUser(), upload, user, p);
            } else {
                it->second->update(
                            t->getPos(),
                            milliSeconds,
                            time,
                            file
                            );
                fire(FinishedManagerListener::UpdatedUser(), upload, user);
            }
        }
    }
}

void FinishedManager::on(QueueManagerListener::CRCChecked, Download* d) noexcept {
    onComplete(d, false, /*crc32Checked*/true);
}

void FinishedManager::on(DownloadManagerListener::Complete, Download* d) noexcept {
    onComplete(d, false);
}

void FinishedManager::on(DownloadManagerListener::Failed, Download* d, const string&) noexcept {
    if(d->getPos() > 0)
        onComplete(d, false);
}

void FinishedManager::on(UploadManagerListener::Complete, Upload* u) noexcept {
    onComplete(u, true);
}

void FinishedManager::on(UploadManagerListener::Failed, Upload* u, const string&) noexcept {
    if(u->getPos() > 0)
        onComplete(u, true);
}

string FinishedManager::getTarget(const string& aTTH){
    if(aTTH.empty()) return Util::emptyString;

    {
        Lock l(cs);

        for(FinishedItem::FinishedItemList::const_iterator i = downloads.begin(); i != downloads.end(); ++i)
        {
            if((*i).getTTH() == aTTH)
                return (*i).getTarget();
        }
    }

    return Util::emptyString;
}


bool FinishedManager::handlePartialRequest(const TTHValue& tth, vector<uint16_t>& outPartialInfo)
{

    string target = getTarget(tth.toBase32());

    if(target.empty()) return false;

    int64_t fileSize = File::getSize(target);

    if(fileSize < PARTIAL_SHARE_MIN_SIZE)
        return false;

    uint16_t len = TigerTree::calcBlocks(fileSize,(int)100);
    outPartialInfo.push_back(0);
    outPartialInfo.push_back(len);

    return true;
}

} // namespace dcpp
