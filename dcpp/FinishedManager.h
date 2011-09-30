/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DCPLUSPLUS_DCPP_FINISHED_MANAGER_H
#define DCPLUSPLUS_DCPP_FINISHED_MANAGER_H

#include "DownloadManagerListener.h"
#include "UploadManagerListener.h"
#include "QueueManagerListener.h"

#include "Speaker.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "FinishedManagerListener.h"
#include "Util.h"
#include "User.h"

#include "MerkleTree.h"
#include "ClientManager.h"

namespace dcpp {

/*RTF*/
class FinishedItem
{
public:
    typedef vector<FinishedItem> FinishedItemList;
//      typedef FinishedItemList::const_iterator;

    FinishedItem(string const& aTarget, const UserPtr& aUser, string const& aHub,
            int64_t aSize, int64_t aSpeed, time_t aTime,
            const string& aTTH = Util::emptyString) :
            target(aTarget), user(aUser), hub(aHub), size(aSize), avgSpeed(aSpeed),
            time(aTime), tth(aTTH)
    {
    }

    int imageIndex() const;

    GETSET(string, target, Target);
    GETSET(string, hub, Hub);
    GETSET(string, tth, TTH);

    GETSET(int64_t, size, Size);
    GETSET(int64_t, avgSpeed, AvgSpeed);
    GETSET(time_t, time, Time);
    GETSET(UserPtr, user, User);

private:
    friend class FinishedManager;

};
/**/
class FinishedManager : public Singleton<FinishedManager>,
    public Speaker<FinishedManagerListener>, private DownloadManagerListener, private UploadManagerListener, private QueueManagerListener
{
public:
    typedef unordered_map<string, FinishedFileItemPtr> MapByFile;
    typedef unordered_map<HintedUser, FinishedUserItemPtr, User::Hash> MapByUser;

    void lockLists();
    //const FinishedItem::FinishedItemList& lockList(bool upload = false) { cs.lock(); return upload ? uploads : downloads; }
    const MapByFile& getMapByFile(bool upload) const;
    const MapByUser& getMapByUser(bool upload) const;
    void unlockLists();

    void remove(bool upload, const string& file);
    void remove(bool upload, const HintedUser& user);
    void removeAll(bool upload);
    //Partial
    /** Get file full path by tth to share */
    string getTarget(const string& aTTH);

    bool handlePartialRequest(const TTHValue& tth, vector<uint16_t>& outPartialInfo);
    //end
private:
    friend class Singleton<FinishedManager>;

    CriticalSection cs;
    MapByFile DLByFile, ULByFile;
    MapByUser DLByUser, ULByUser;
    //Partial
    FinishedItem::FinishedItemList downloads, uploads;

    FinishedManager();
    virtual ~FinishedManager();

    void clearDLs();
    void clearULs();

    void onComplete(Transfer* t, bool upload, bool crc32Checked = false);

    virtual void on(DownloadManagerListener::Complete, Download* d) noexcept;
    virtual void on(DownloadManagerListener::Failed, Download* d, const string&) noexcept;

    virtual void on(UploadManagerListener::Complete, Upload* u) noexcept;
    virtual void on(UploadManagerListener::Failed, Upload* u, const string&) noexcept;

    virtual void on(QueueManagerListener::CRCChecked, Download* d) noexcept;
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_FINISHED_MANAGER_H)
