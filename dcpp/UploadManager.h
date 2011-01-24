/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_UPLOAD_MANAGER_H
#define DCPLUSPLUS_DCPP_UPLOAD_MANAGER_H

#include "forward.h"
#include "UserConnectionListener.h"
#include "Singleton.h"
#include "UploadManagerListener.h"
#include "Client.h"
#include "ClientManagerListener.h"
#include "MerkleTree.h"
#include "User.h"
#include "Util.h"
#include "TimerManager.h"
#include "Speaker.h"
#include "PerFolderLimit.h"

namespace dcpp {

class UploadManager : private ClientManagerListener, private UserConnectionListener, public Speaker<UploadManagerListener>, private TimerManagerListener, public Singleton<UploadManager>
{
public:
    /** @return Number of uploads. */
    size_t getUploadCount() { Lock l(cs); return uploads.size(); }

    /**
     * @remarks This is only used in the tray icons. Could be used in
     * MainFrame too.
     *
     * @return Running average download speed in Bytes/s
     */
    int64_t getRunningAverage();
    uint8_t getSlots() const { return (uint8_t) (SETTING(SLOTS)* Client::getTotalCounts());}
    /** @return Number of free slots. */
    int getFreeSlots() { return max((SETTING(SLOTS) - running), 0); }

    /** @internal */
    int getFreeExtraSlots() { return max(3 - getExtra(), 0); }

    /** @param aUser Reserve an upload slot for this user and connect. */
    void reserveSlot(const HintedUser& aUser);

    /** */
    void reloadRestrictions();

    typedef set<string> FileSet;
    typedef unordered_map<UserPtr, FileSet, User::Hash> FilesMap;
    void clearUserFiles(const UserPtr&);
    HintedUserList getWaitingUsers() const;
    const FileSet& getWaitingUserFiles(const UserPtr&);

    /** @internal */
    void addConnection(UserConnectionPtr conn);

    void notifyQueuedUsers();
    void setRunning(int _running) { running = _running; notifyQueuedUsers(); }

    //GETSET(int, running, Running);
    GETSET(uint8_t, extraPartial, ExtraPartial);
    GETSET(uint8_t, extra, Extra);
    GETSET(uint64_t, lastGrant, LastGrant);

    void updateLimits() {limits.RenewList(NULL);}
private:
    int running;
    UploadList uploads;
    mutable CriticalSection cs;

    typedef unordered_set<UserPtr, User::Hash> SlotSet;
    typedef SlotSet::iterator SlotIter;
    SlotSet reservedSlots;
    CPerfolderLimit limits;
    int lastFreeSlots; /// amount of free slots at the previous minute

    typedef pair<HintedUser, uint64_t> WaitingUser;
    typedef list<WaitingUser> WaitingUserList;

    struct WaitingUserFresh {
        bool operator()(const WaitingUser& wu) { return wu.second > GET_TICK() - 5*60*1000; }
    };

    //functions for manipulating waitingFiles and waitingUsers
    WaitingUserList waitingUsers;       //this one merely lists the users waiting for slots
    FilesMap waitingFiles;      //set of files which this user has asked for
    void addFailedUpload(const UserConnection& source, string filename);

    friend class Singleton<UploadManager>;
    UploadManager() throw();
    virtual ~UploadManager() throw();

    bool getAutoSlot();
    void removeConnection(UserConnection* aConn);
    void removeUpload(Upload* aUpload);

    // ClientManagerListener
    virtual void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw();

    // TimerManagerListener
    virtual void on(Second, uint64_t aTick) throw();
    virtual void on(Minute, uint64_t aTick) throw();

    // UserConnectionListener
    virtual void on(BytesSent, UserConnection*, size_t, size_t) throw();
    virtual void on(Failed, UserConnection*, const string&) throw();
    virtual void on(Get, UserConnection*, const string&, int64_t) throw();
    virtual void on(Send, UserConnection*) throw();
    virtual void on(GetListLength, UserConnection* conn) throw();
    virtual void on(TransmitDone, UserConnection*) throw();

    virtual void on(AdcCommand::GET, UserConnection*, const AdcCommand&) throw();
    virtual void on(AdcCommand::GFI, UserConnection*, const AdcCommand&) throw();

    bool prepareFile(UserConnection& aSource, const string& aType, const string& aFile, int64_t aResume, int64_t aBytes, bool listRecursive = false);
};

} // namespace dcpp

#endif // !defined(UPLOAD_MANAGER_H)
