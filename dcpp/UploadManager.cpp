/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

#include "UploadManager.h"

#include <cmath>

#include "ConnectionManager.h"
#include "LogManager.h"
#include "ShareManager.h"
#include "ClientManager.h"
#include "FilteredFile.h"
#include "ZUtils.h"
#include "HashManager.h"
#include "AdcCommand.h"
#include "FavoriteManager.h"
#include "CryptoManager.h"
#include "Upload.h"
#include "UserConnection.h"
#include "QueueManager.h"
#include "FinishedManager.h"
#include "File.h"
#include "extra/ipfilter.h"
#include "DCPlusPlus.h"

namespace dcpp {

static const string UPLOAD_AREA = "Uploads";


UploadManager::UploadManager(DCContext& ctx) : ContextAware(ctx), extra(0), lastGrant(0), running(0), limits(NULL), lastFreeSlots(-1) {
    this->ctx().getClientManager()->addListener(this);
    this->ctx().getTimerManager()->addListener(this);
}

UploadManager::~UploadManager() {
    ctx().getTimerManager()->removeListener(this);
    ctx().getClientManager()->removeListener(this);
    while(true) {
        {
            Lock l(cs);
            if(uploads.empty())
                break;
        }
        Thread::sleep(100);
    }
}

bool UploadManager::hasUpload ( UserConnection& aSource ) {
    Lock l(cs);
    if (!aSource.getSocket() || CTX_SETTING(ALLOW_SIM_UPLOADS))
        return false;

    for ( UploadList::const_iterator i = uploads.begin(); i != uploads.end(); ++i ) {
        Upload* u = *i;
        const string l_srcip = aSource.getSocket()->getIp();
        const int64_t l_share = ctx().getClientManager()->getBytesShared(aSource.getUser());

        if (u && u->getUserConnection().getSocket() &&
                l_srcip == u->getUserConnection().getSocket()->getIp() &&
                u->getUser() && l_share == ctx().getClientManager()->getBytesShared(u->getUser())
                )
        {
            return true;
        }
    }

    return false;
}

bool UploadManager::prepareFile(UserConnection& aSource, const string& aType, const string& aFile, int64_t aStartPos, int64_t aBytes, bool listRecursive) {
    dcdebug("Preparing %s %s " I64_FMT " " I64_FMT " %d\n", aType.c_str(), aFile.c_str(),
            static_cast<long long int>(aStartPos), static_cast<long long int>(aBytes), listRecursive);

    if(aFile.empty() || aStartPos < 0 || aBytes < -1 || aBytes == 0) {
        aSource.fileNotAvail("Invalid request");
        return false;
    }

    InputStream* is = 0;
    int64_t start = 0;
    int64_t size = 0;
    int64_t fileSize = 0;

    bool userlist = (aFile == Transfer::USER_LIST_NAME_BZ || aFile == Transfer::USER_LIST_NAME);
    bool free = userlist;

    string sourceFile;
    Transfer::Type type;

    try {
        if(aType == Transfer::names[Transfer::TYPE_FILE]) {
            sourceFile = ctx().getShareManager()->toReal(aFile);

            if(aFile == Transfer::USER_LIST_NAME) {
                // Unpack before sending...
                string bz2 = File(sourceFile, File::READ, File::OPEN).read();
                string xml;
                ctx().getCryptoManager()->decodeBZ2(reinterpret_cast<const uint8_t*>(bz2.data()), bz2.size(), xml);
                // Clear to save some memory...
                string().swap(bz2);
                is = new MemoryInputStream(xml);
                start = 0;
                fileSize = size = xml.size();
            } else {
                {
                    ShareManager *SM = ctx().getShareManager();
                    string msg;
                    if ( aFile != Transfer::USER_LIST_NAME_BZ && aFile != Transfer::USER_LIST_NAME &&
                         !limits.IsUserAllowed(ctx(), SM->toVirtual(SM->getTTH(aFile)), aSource.getUser(), &msg)
                         )
                    {
                        throw ShareException(msg);
                    }
                    else if ( aFile != Transfer::USER_LIST_NAME_BZ && aFile != Transfer::USER_LIST_NAME &&
                              hasUpload(aSource)
                              )
                    {
                        msg = _("Connection already exists.");

                        throw ShareException(msg);
                    }
                }
                File* f = new File(sourceFile, File::READ, File::OPEN);

                start = aStartPos;
                int64_t sz = f->getSize();
                size = (aBytes == -1) ? sz - start : aBytes;
                fileSize = sz;

                if((start + size) > sz) {
                    aSource.fileNotAvail();
                    delete f;
                    return false;
                }

                free = free || (sz <= (int64_t)(CTX_SETTING(SET_MINISLOT_SIZE) * 1024) );

                f->setPos(start);
                is = f;
                if((start + size) < sz) {
                    is = new LimitedInputStream<true>(is, size);
                }
            }
            type = userlist ? Transfer::TYPE_FULL_LIST : Transfer::TYPE_FILE;
        } else if(aType == Transfer::names[Transfer::TYPE_TREE]) {
            sourceFile = ctx().getShareManager()->toReal(aFile);
            MemoryInputStream* mis = ctx().getShareManager()->getTree(aFile);
            if(!mis) {
                aSource.fileNotAvail();
                return false;
            }

            start = 0;
            fileSize = size = mis->getSize();
            is = mis;
            free = true;
            type = Transfer::TYPE_TREE;
        } else if(aType == Transfer::names[Transfer::TYPE_PARTIAL_LIST]) {
            // Partial file list
            MemoryInputStream* mis = ctx().getShareManager()->generatePartialList(aFile, listRecursive);
            if(mis == NULL) {
                aSource.fileNotAvail();
                return false;
            }

            start = 0;
            fileSize = size = mis->getSize();
            is = mis;
            free = true;
            type = Transfer::TYPE_PARTIAL_LIST;
        } else {
            aSource.fileNotAvail("Unknown file type");
            return false;
        }
    } catch(const ShareException& e) {
        // -- Added by RevConnect : Partial file sharing upload
        if(aType == Transfer::names[Transfer::TYPE_FILE] && aFile.compare(0, 4, "TTH/") == 0) {

            TTHValue fileHash(aFile.substr(4));
            // find in download queue
            string target;

            if(ctx().getQueueManager()->isChunkDownloaded(fileHash, aStartPos, aBytes, target, fileSize)){
                sourceFile = target;

                try {
                    File* f = new File(sourceFile, File::READ, File::OPEN | File::SHARED);

                    start = aStartPos;
                    fileSize = f->getSize();
                    size = (aBytes == -1) ? fileSize - start : aBytes;

                    if((start + size) > fileSize) {
                        aSource.fileNotAvail();
                        delete f;
                        return false;
                    }

                    f->setPos(start);
                    is = f;

                    if((start + size) < fileSize) {
                        is = new LimitedInputStream<true>(is, size);
                    }

                    type = Transfer::TYPE_FILE;
                    goto ok;
                } catch(const Exception&) {
                    aSource.fileNotAvail();
                    //aSource.disconnect();
                    delete is;
                    return false;
                }
            } else {
                // Share finished file
                target = ctx().getFinishedManager()->getTarget(fileHash.toBase32());

                if(!target.empty() && Util::fileExists(target)){
                    sourceFile = target;
                    try {
                        File* f = new File(sourceFile, File::READ, File::OPEN | File::SHARED);

                        start = aStartPos;
                        int64_t sz = f->getSize();
                        size = (aBytes == -1) ? sz - start : aBytes;
                        fileSize = sz;

                        if((start + size) > sz) {
                            aSource.fileNotAvail();
                            delete f;
                            return false;
                        }

                        f->setPos(start);
                        is = f;
                        if((start + size) < sz) {
                            is = new LimitedInputStream<true>(is, size);
                        }

                        type = Transfer::TYPE_FILE;
                        goto ok;
                    }catch(const Exception&){
                        aSource.fileNotAvail();
                        delete is;
                        return false;
                    }
                }
            }
        }
        aSource.fileNotAvail(e.getError());
        return false;
    } catch(const Exception& e) {
        ctx().getLogManager()->message(str(F_("Unable to send file %1%: %2%") % Util::addBrackets(sourceFile) % e.getError()));
        aSource.fileNotAvail();
        return false;
    } catch(const std::exception& e) {
        ctx().getLogManager()->message(str(F_("Unable to send file %1%: %2%") % Util::addBrackets(sourceFile) % string(e.what())));
        aSource.fileNotAvail();
        return false;
    }

ok:

    try {
    Lock l(cs);

    bool extraSlot = false;

    if(!aSource.isSet(UserConnection::FLAG_HASSLOT)) {
        bool hasReserved = (reservedSlots.find(aSource.getUser()) != reservedSlots.end());
        bool isFavorite = ctx().getFavoriteManager()->hasSlot(aSource.getUser());

        if(!(hasReserved || isFavorite || getFreeSlots() > 0 || getAutoSlot())) {
            bool supportsFree = aSource.isSet(UserConnection::FLAG_SUPPORTS_MINISLOTS);
            bool allowedFree = aSource.isSet(UserConnection::FLAG_HASEXTRASLOT) || aSource.isSet(UserConnection::FLAG_OP) || getFreeExtraSlots() > 0;
            if(free && supportsFree && allowedFree) {
                extraSlot = true;
            } else {
                delete is;
                aSource.maxedOut();

                // Check for tth root identifier
                string tFile = aFile;
                if (tFile.compare(0, 4, "TTH/") == 0)
                    tFile = ctx().getShareManager()->toVirtual(TTHValue(aFile.substr(4)));

                addFailedUpload(aSource, tFile +
                                " (" +  Util::formatBytes(aStartPos) + " - " + Util::formatBytes(aStartPos + aBytes) + ")");
                aSource.disconnect();
                return false;
            }
        } else {
            clearUserFiles(aSource.getUser());  // this user is using a full slot, nix them.
        }

        setLastGrant(GET_TICK());
    }

    Upload* u = new Upload(aSource, sourceFile, TTHValue());
    u->setStream(is);
    u->setSegment(Segment(start, size));

    u->setType(type);

    uploads.push_back(u);

    if(!aSource.isSet(UserConnection::FLAG_HASSLOT)) {
        if(extraSlot) {
            if(!aSource.isSet(UserConnection::FLAG_HASEXTRASLOT)) {
                aSource.setFlag(UserConnection::FLAG_HASEXTRASLOT);
                extra++;
            }
        } else {
            if(aSource.isSet(UserConnection::FLAG_HASEXTRASLOT)) {
                aSource.unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
                extra--;
            }
            aSource.setFlag(UserConnection::FLAG_HASSLOT);
            running++;
        }

        reservedSlots.erase(aSource.getUser());
    }

    return true;

    } catch(const std::exception& e) {
        delete is;
        ctx().getLogManager()->message(str(F_("Unable to send file %1%: %2%") % Util::addBrackets(sourceFile) % string(e.what())));
        aSource.fileNotAvail();
        return false;
    }
}

int64_t UploadManager::getRunningAverage() {
    Lock l(cs);
    int64_t avg = 0;
    for(auto u: uploads) {
        avg += u->getAverageSpeed();
    }
    return avg;
}

bool UploadManager::getAutoSlot() {
    /** A 0 in settings means disable */
    if(CTX_SETTING(MIN_UPLOAD_SPEED) == 0)
        return false;
    /** Only grant one slot per 30 sec */
    if(GET_TICK() < getLastGrant() + 30*1000)
        return false;
    /** Grant if upload speed is less than the threshold speed */
    return getRunningAverage() < (CTX_SETTING(MIN_UPLOAD_SPEED)*1024);
}

void UploadManager::removeUpload(Upload* aUpload) {
    Lock l(cs);
    dcassert(find(uploads.begin(), uploads.end(), aUpload) != uploads.end());
    uploads.erase(remove(uploads.begin(), uploads.end(), aUpload), uploads.end());
    delete aUpload;
}

void UploadManager::reserveSlot(const HintedUser& aUser) {
    {
        Lock l(cs);
        reservedSlots.insert(aUser);
    }
    if(aUser.user->isOnline())
        ctx().getClientManager()->connect(aUser, Util::toString(Util::rand()));
}

void UploadManager::on(UserConnectionListener::Get, UserConnection* aSource, const string& aFile, int64_t aResume) {
    try {
    if(aSource->getState() != UserConnection::STATE_GET) {
        dcdebug("UM::onGet Bad state, ignoring\n");
        return;
    }

    if(prepareFile(*aSource, Transfer::names[Transfer::TYPE_FILE], Util::toAdcFile(aFile), aResume, -1)) {
        aSource->setState(UserConnection::STATE_SEND);
        aSource->fileLength(Util::toString(aSource->getUpload()->getSize()));
    }
    } catch(const std::exception& e) {
        fprintf(stderr, "[UploadManager::on(Get)] exception: %s\n", e.what());
        aSource->fileNotAvail();
    } catch(...) {
        fprintf(stderr, "[UploadManager::on(Get)] unknown exception\n");
        aSource->fileNotAvail();
    }
}

void UploadManager::on(UserConnectionListener::Send, UserConnection* aSource) {
    if(aSource->getState() != UserConnection::STATE_SEND) {
        dcdebug("UM::onSend Bad state, ignoring\n");
        return;
    }

    Upload* u = aSource->getUpload();
    dcassert(u != NULL);

    u->setStart(GET_TICK());
    u->tick();
    aSource->setState(UserConnection::STATE_RUNNING);
    aSource->transmitFile(u->getStream());
    fire(UploadManagerListener::Starting(), u);
}

void UploadManager::on(AdcCommand::GET, UserConnection* aSource, const AdcCommand& c) {
    try {
    if(aSource->getState() != UserConnection::STATE_GET) {
        dcdebug("UM::onGET Bad state, ignoring\n");
        return;
    }

    const string& type = c.getParam(0);
    const string& fname = c.getParam(1);
    int64_t aStartPos = Util::toInt64(c.getParam(2));
    int64_t aBytes = Util::toInt64(c.getParam(3));

    if(prepareFile(*aSource, type, fname, aStartPos, aBytes, c.hasFlag("RE", 4))) {
        Upload* u = aSource->getUpload();
        dcassert(u != NULL);

        AdcCommand cmd(AdcCommand::CMD_SND);
        cmd.addParam(type).addParam(fname)
                .addParam(Util::toString(u->getStartPos()))
                .addParam(Util::toString(u->getSize()));

        if(c.hasFlag("ZL", 4)) {
            u->setStream(new FilteredInputStream<ZFilter, true>(u->getStream()));
            u->setFlag(Upload::FLAG_ZUPLOAD);
            cmd.addParam("ZL1");
        }

        aSource->send(cmd);

        u->setStart(GET_TICK());
        u->tick();
        aSource->setState(UserConnection::STATE_RUNNING);
        aSource->transmitFile(u->getStream());
        fire(UploadManagerListener::Starting(), u);
    }
    } catch(const std::exception& e) {
        fprintf(stderr, "[UploadManager::on(ADC GET)] exception: %s\n", e.what());
        aSource->fileNotAvail();
    } catch(...) {
        fprintf(stderr, "[UploadManager::on(ADC GET)] unknown exception\n");
        aSource->fileNotAvail();
    }
}

void UploadManager::on(UserConnectionListener::BytesSent, UserConnection* aSource, size_t aBytes, size_t aActual) {
    dcassert(aSource->getState() == UserConnection::STATE_RUNNING);
    Upload* u = aSource->getUpload();
    dcassert(u != NULL);
    u->addPos(aBytes, aActual);
    u->tick();
}

void UploadManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) {
    Upload* u = aSource->getUpload();

    if(u) {
        fire(UploadManagerListener::Failed(), u, aError);

        dcdebug("UM::onFailed (%s): Removing upload\n", aError.c_str());
        removeUpload(u);
    }

    removeConnection(aSource);
}

void UploadManager::on(UserConnectionListener::TransmitDone, UserConnection* aSource) {
    dcassert(aSource->getState() == UserConnection::STATE_RUNNING);
    Upload* u = aSource->getUpload();
    dcassert(u != NULL);

    aSource->setState(UserConnection::STATE_GET);

    if(CTX_BOOLSETTING(LOG_UPLOADS) && u->getType() != Transfer::TYPE_TREE && (CTX_BOOLSETTING(LOG_FILELIST_TRANSFERS) || u->getType() != Transfer::TYPE_FULL_LIST)) {
        StringMap params;
        u->getParams(*aSource, params);
        CTX_LOG(LogManager::UPLOAD, params);
    }

    fire(UploadManagerListener::Complete(), u);
    removeUpload(u);
}

void UploadManager::notifyQueuedUsers() {
    Lock l(cs);
    int freeSlots = getFreeSlots()*2;               //because there will be non-connecting users

    //while all contacted users may not connect, many probably will; it's fine that the rest are filled with randomly allocated slots
    while (freeSlots) {
        //get rid of offline users
        while (!waitingUsers.empty() && !waitingUsers.front().first.user->isOnline()) waitingUsers.pop_front();
        if (waitingUsers.empty()) break;                //no users to notify

        // FIXME: record and replay a client url hint URL
        ctx().getClientManager()->connect(waitingUsers.front().first, Util::toString(Util::rand()));
        --freeSlots;

        waitingUsers.pop_front();
    }
}

void UploadManager::addFailedUpload(const UserConnection& source, string filename) {
    {
        Lock l(cs);
        auto it = find_if(waitingUsers.begin(), waitingUsers.end(), CompareFirst<UserPtr, uint32_t>(source.getUser()));
        if (it==waitingUsers.end()) {
            waitingUsers.emplace_back(source.getHintedUser(), GET_TICK());
        } else {
            it->second = GET_TICK();
        }
        waitingFiles[source.getUser()].insert(filename);        //files for which user's asked
    }

    fire(UploadManagerListener::WaitingAddFile(), source.getHintedUser(), filename);
}

void UploadManager::clearUserFiles(const UserPtr& source) {
    Lock l(cs);
    //run this when a user's got a slot or goes offline.
    auto sit = find_if(waitingUsers.begin(), waitingUsers.end(), CompareFirst<UserPtr, uint32_t>(source));
    if (sit == waitingUsers.end()) return;

    FilesMap::iterator fit = waitingFiles.find(sit->first);
    if (fit != waitingFiles.end()) waitingFiles.erase(fit);
    fire(UploadManagerListener::WaitingRemoveUser(), sit->first);

    waitingUsers.erase(sit);
}

HintedUserList UploadManager::getWaitingUsers() const {
    Lock l(cs);
    HintedUserList u;
    for(auto i = waitingUsers.begin(), iend = waitingUsers.end(); i != iend; ++i) {
        u.push_back(i->first);
    }
    return u;
}

const UploadManager::FileSet& UploadManager::getWaitingUserFiles(const UserPtr& u) {
    Lock l(cs);
    return waitingFiles.find(u)->second;
}

void UploadManager::addConnection(UserConnectionPtr conn) {
    Lock l(cs);
    if (!CTX_BOOLSETTING(ALLOW_UPLOAD_MULTI_HUB)) {
        for(auto u: uploads) {
            if (u->getUserConnection().getRemoteIp() == conn->getRemoteIp()) {
                conn->disconnect();
                return;
            }
        }
    }
    if (CTX_BOOLSETTING(IPFILTER) && !ctx().getIPFilter()->OK(conn->getRemoteIp(),eDIRECTION_OUT)) {
        conn->error("Your IP is Blocked!");// TODO translate
        ctx().getLogManager()->message(_("IPFilter: Blocked incoming connection to ") + conn->getRemoteIp()); // TODO translate
        //ctx().getQueueManager()->removeSource(conn->getUser(), QueueItem::Source::FLAG_REMOVED);
        conn->disconnect();
        return;
    }

    if(CTX_SETTING(REQUIRE_TLS) && !conn->isSecure()) {
        conn->error("Secure connection required!");
        conn->disconnect();
        return;
    }

    conn->addListener(this);
    conn->setState(UserConnection::STATE_GET);
}

void UploadManager::removeConnection(UserConnection* aSource) {
    aSource->removeListener(this);
    if(aSource->isSet(UserConnection::FLAG_HASSLOT)) {
        running--;
        aSource->unsetFlag(UserConnection::FLAG_HASSLOT);
    }
    if(aSource->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
        extra--;
        aSource->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
    }
}

void UploadManager::reloadRestrictions(){
    limits.RenewList(NULL);
}

void UploadManager::on(TimerManagerListener::Minute, uint64_t /* aTick */) {
    UserList disconnects;
    {
        Lock l(cs);

        auto i = stable_partition(waitingUsers.begin(), waitingUsers.end(), WaitingUserFresh());
        for (auto j = i; j != waitingUsers.end(); ++j) {
            auto fit = waitingFiles.find(j->first);
            if (fit != waitingFiles.end()) waitingFiles.erase(fit);
            fire(UploadManagerListener::WaitingRemoveUser(), j->first);
        }

        waitingUsers.erase(i, waitingUsers.end());

        if( CTX_BOOLSETTING(AUTO_KICK) ) {
            for(auto u: uploads) {
                if(u->getUser()->isOnline()) {
                    u->unsetFlag(Upload::FLAG_PENDING_KICK);
                    continue;
                }

                if(u->isSet(Upload::FLAG_PENDING_KICK)) {
                    disconnects.push_back(u->getUser());
                    continue;
                }

                if(CTX_BOOLSETTING(AUTO_KICK_NO_FAVS) && ctx().getFavoriteManager()->isFavoriteUser(u->getUser())) {
                    continue;
                }

                u->setFlag(Upload::FLAG_PENDING_KICK);
            }
        }
    }

    for(auto i = disconnects.begin(); i != disconnects.end(); ++i) {
        ctx().getLogManager()->message(str(F_("Disconnected user leaving the hub: %1%") %
                                               Util::toString(ctx().getClientManager()->getNicks((*i)->getCID(), Util::emptyString))));
        ctx().getConnectionManager()->disconnect(*i, false);
    }

    int freeSlots = getFreeSlots();
    if(freeSlots != lastFreeSlots) {
        lastFreeSlots = freeSlots;
        ctx().getClientManager()->infoUpdated();
    }
}

void UploadManager::on(GetListLength, UserConnection* conn) {
    conn->error("GetListLength not supported");
    conn->disconnect(false);
}

void UploadManager::on(AdcCommand::GFI, UserConnection* aSource, const AdcCommand& c) {
    if(aSource->getState() != UserConnection::STATE_GET) {
        dcdebug("UM::onSend Bad state, ignoring\n");
        return;
    }

    if(c.getParameters().size() < 2) {
        aSource->send(AdcCommand(AdcCommand::SEV_RECOVERABLE, AdcCommand::ERROR_PROTOCOL_GENERIC, "Missing parameters"));
        return;
    }

    const string& type = c.getParam(0);
    const string& ident = c.getParam(1);

    if(type == Transfer::names[Transfer::TYPE_FILE]) {
        try {
            aSource->send(ctx().getShareManager()->getFileInfo(ident));
        } catch(const ShareException&) {
            aSource->fileNotAvail();
        }
    } else {
        aSource->fileNotAvail();
    }
}

// TimerManagerListener
void UploadManager::on(TimerManagerListener::Second, uint64_t) {
    Lock l(cs);
    UploadList ticks;

    for(auto u: uploads) {
        if(u->getPos() > 0) {
            ticks.push_back(u);
            u->tick();
        }
    }

    if(!uploads.empty())
        fire(UploadManagerListener::Tick(), UploadList(uploads));

    //notifyQueuedUsers();
}

void UploadManager::on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) {
    if(!aUser->isOnline()) {
        clearUserFiles(aUser);
    }
}

} // namespace dcpp
