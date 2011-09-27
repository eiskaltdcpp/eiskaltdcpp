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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_DOWNLOAD_MANAGER_H
#define DCPLUSPLUS_DCPP_DOWNLOAD_MANAGER_H

#include "forward.h"

#include "DownloadManagerListener.h"
#include "UserConnectionListener.h"
#include "QueueItem.h"
#include "TimerManager.h"
#include "Singleton.h"
#include "MerkleTree.h"
#include "Speaker.h"

namespace dcpp {

/**
 * Singleton. Use its listener interface to update the download list
 * in the user interface.
 */
class DownloadManager : public Speaker<DownloadManagerListener>,
    private UserConnectionListener, private TimerManagerListener,
    public Singleton<DownloadManager>
{
public:

    /** @internal */
    void addConnection(UserConnectionPtr conn);
    void checkIdle(const UserPtr& user);

    /** @return Running average download speed in Bytes/s */
    int64_t getRunningAverage();

    /** @return Number of downloads. */
    size_t getDownloadCount() {
        Lock l(cs);
        return downloads.size();
    }

    bool startDownload(QueueItem::Priority prio);
private:

    CriticalSection cs;
    DownloadList downloads;
    UserConnectionList idlers;

    void removeConnection(UserConnectionPtr aConn);
    void removeDownload(Download* aDown);
    void fileNotAvailable(UserConnection* aSource);
    void noSlots(UserConnection* aSource);

    void logDownload(UserConnection* aSource, Download* d);
    int64_t getResumePos(const string& file, const TigerTree& tt, int64_t startPos);

    void failDownload(UserConnection* aSource, const string& reason);

    friend class Singleton<DownloadManager>;

    DownloadManager();
    virtual ~DownloadManager();

    void checkDownloads(UserConnection* aConn);
    void startData(UserConnection* aSource, int64_t start, int64_t newSize, bool z);
    void endData(UserConnection* aSource);

    void onFailed(UserConnection* aSource, const string& aError);

    // UserConnectionListener
    virtual void on(Data, UserConnection*, const uint8_t*, size_t) noexcept;
    virtual void on(Failed, UserConnection* aSource, const string& aError) noexcept { onFailed(aSource, aError); }
    virtual void on(ProtocolError, UserConnection* aSource, const string& aError) noexcept { onFailed(aSource, aError); }
    virtual void on(MaxedOut, UserConnection*) noexcept;
    virtual void on(FileNotAvailable, UserConnection*) noexcept;
    virtual void on(Updated, UserConnection*) noexcept;

    virtual void on(AdcCommand::SND, UserConnection*, const AdcCommand&) noexcept;
    virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) noexcept;

    // TimerManagerListener
    virtual void on(TimerManagerListener::Second, uint64_t aTick) noexcept;
};

} // namespace dcpp

#endif // !defined(DOWNLOAD_MANAGER_H)
