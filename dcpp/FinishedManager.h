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

#ifndef DCPLUSPLUS_DCPP_FINISHED_MANAGER_H
#define DCPLUSPLUS_DCPP_FINISHED_MANAGER_H

#include "DownloadManagerListener.h"
#include "UploadManagerListener.h"

#include "Speaker.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "FinishedManagerListener.h"
#include "Util.h"
#include "User.h"

namespace dcpp {

class FinishedManager : public Singleton<FinishedManager>,
    public Speaker<FinishedManagerListener>, private DownloadManagerListener, private UploadManagerListener
{
public:
    typedef unordered_map<string, FinishedFileItemPtr> MapByFile;
    typedef unordered_map<UserPtr, FinishedUserItemPtr, User::Hash> MapByUser;

    void lockLists();
    const MapByFile& getMapByFile(bool upload) const;
    const MapByUser& getMapByUser(bool upload) const;
    void unLockLists();

    void remove(bool upload, const string& file);
    void remove(bool upload, const UserPtr& user);
    void removeAll(bool upload);

private:
    friend class Singleton<FinishedManager>;

    CriticalSection cs;
    MapByFile DLByFile, ULByFile;
    MapByUser DLByUser, ULByUser;

    FinishedManager();
    virtual ~FinishedManager() throw();

    void clearDLs();
    void clearULs();

    void onComplete(Transfer* t, bool upload, bool crc32Checked = false);

    virtual void on(DownloadManagerListener::Complete, Download* d) throw();
    virtual void on(DownloadManagerListener::Failed, Download* d, const string&) throw();

    virtual void on(UploadManagerListener::Complete, Upload* u) throw();
    virtual void on(UploadManagerListener::Failed, Upload* u, const string&) throw();
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_FINISHED_MANAGER_H)
