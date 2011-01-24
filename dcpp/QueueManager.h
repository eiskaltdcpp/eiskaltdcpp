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

#ifndef DCPLUSPLUS_DCPP_QUEUE_MANAGER_H
#define DCPLUSPLUS_DCPP_QUEUE_MANAGER_H

#include "TimerManager.h"

#include "CriticalSection.h"
#include "Exception.h"
#include "User.h"
#include "File.h"
#include "QueueItem.h"
#include "Singleton.h"
#include "DirectoryListing.h"
#include "MerkleTree.h"

#include "QueueManagerListener.h"
#include "SearchManagerListener.h"
#include "ClientManagerListener.h"

namespace dcpp {

STANDARD_EXCEPTION(QueueException);

class UserConnection;

class DirectoryItem {
public:
    typedef DirectoryItem* Ptr;
    typedef unordered_multimap<UserPtr, Ptr, User::Hash> DirectoryMap;
    typedef DirectoryMap::iterator DirectoryIter;
    typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;

    typedef vector<Ptr> List;
    typedef List::iterator Iter;

    DirectoryItem() : priority(QueueItem::DEFAULT) { }
    DirectoryItem(const UserPtr& aUser, const string& aName, const string& aTarget,
        QueueItem::Priority p) : name(aName), target(aTarget), priority(p), user(aUser) { }
    ~DirectoryItem() { }

    UserPtr& getUser() { return user; }
    void setUser(const UserPtr& aUser) { user = aUser; }

    GETSET(string, name, Name);
    GETSET(string, target, Target);
    GETSET(QueueItem::Priority, priority, Priority);
private:
    UserPtr user;
};

class ConnectionQueueItem;
class QueueLoader;

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener,
    private SearchManagerListener, private ClientManagerListener
{
public:
    //NOTE: freedcpp
    void add(const string& aTarget, int64_t aSize, const TTHValue& root) throw(QueueException, FileException);

    /** Add a file to the queue. */
    void add(const string& aTarget, int64_t aSize, const TTHValue& root, const HintedUser& aUser,
    int aFlags = 0, bool addBad = true) throw(QueueException, FileException);
    /** Add a user's filelist to the queue. */
    void addList(const HintedUser& HintedUser, int aFlags, const string& aInitialDir = Util::emptyString) throw(QueueException, FileException);
    /** Readd a source that was removed */
    void readd(const string& target, const HintedUser& aUser) throw(QueueException);
    /** Add a directory to the queue (downloads filelist and matches the directory). */
    void addDirectory(const string& aDir, const HintedUser& aUser, const string& aTarget,
            QueueItem::Priority p = QueueItem::DEFAULT) throw();

    int matchListing(const DirectoryListing& dl) throw();

    bool getTTH(const string& name, TTHValue& tth) throw();

    int64_t getSize(const string& target) throw();
    int64_t getPos(const string& target) throw();

    /** Move the target location of a queued item. Running items are silently ignored */
    void move(const string& aSource, const string& aTarget) throw();

    void remove(const string& aTarget) throw();
    void removeSource(const string& aTarget, const UserPtr& aUser, int reason, bool removeConn = true) throw();
    void removeSource(const UserPtr& aUser, int reason) throw();

    void recheck(const string& aTarget);

    void setPriority(const string& aTarget, QueueItem::Priority p) throw();

    void getTargets(const TTHValue& tth, StringList& sl);
    QueueItem::StringMap& lockQueue() throw() { cs.enter(); return fileQueue.getQueue(); } ;
    void unlockQueue() throw() { cs.leave(); }

    Download* getDownload(UserConnection& aSource, bool supportsTrees) throw();
    void putDownload(Download* aDownload, bool finished) throw();
    void setFile(Download* download);

    int64_t getQueued(const UserPtr& aUser) const;

    /** @return The highest priority download the user has, PAUSED may also mean no downloads */
    QueueItem::Priority hasDownload(const UserPtr& aUser) throw();

    bool getQueueInfo(const UserPtr& aUser, string& aTarget, int64_t& aSize, int& aFlags) throw();

    int countOnlineSources(const string& aTarget);

    void loadQueue() throw();
    void saveQueue(bool force = false) throw();

    void noDeleteFileList(const string& path);

    bool handlePartialSearch(const TTHValue& tth, PartsInfo& _outPartsInfo);
    bool handlePartialResult(const UserPtr& aUser, const string& hubHint, const TTHValue& tth, const QueueItem::PartialSource& partialSource, PartsInfo& outPartialInfo);

    bool isChunkDownloaded(const TTHValue& tth, int64_t startPos, int64_t& bytes, string& tempTarget, int64_t& size) {
        Lock l(cs);
        QueueItem::List ql;
        fileQueue.find(ql, tth);

        if(ql.empty()) return false;

        QueueItem* qi = ql.front();

        tempTarget = qi->getTempTarget();
        size = qi->getSize();

        return qi->isChunkDownloaded(startPos, bytes);
    }

    GETSET(uint64_t, lastSave, LastSave);
    GETSET(string, queueFile, QueueFile);
private:
    enum { MOVER_LIMIT = 10*1024*1024 };
    class FileMover : public Thread {
    public:
        FileMover() : active(false) { }
        virtual ~FileMover() { join(); }

        void moveFile(const string& source, const string& target);
        virtual int run();
    private:
        typedef pair<string, string> FilePair;
        typedef vector<FilePair> FileList;
        typedef FileList::iterator FileIter;

        bool active;

        FileList files;
        CriticalSection cs;
    } mover;

    typedef vector<pair<QueueItem::SourceConstIter, const QueueItem*> > PFSSourceList;

    class Rechecker : public Thread {
        struct DummyOutputStream : OutputStream {
            virtual size_t write(const void*, size_t n) throw(Exception) { return n; }
            virtual size_t flush() throw(Exception) { return 0; }
        };

    public:
        explicit Rechecker(QueueManager* qm_) : qm(qm_), active(false) { }
        virtual ~Rechecker() { join(); }

        void add(const string& file);
        virtual int run();

    private:
        QueueManager* qm;
        bool active;

        StringList files;
        CriticalSection cs;
    } rechecker;

    /** All queue items by target */
    class FileQueue {
    public:
        FileQueue() : lastInsert(queue.end()) { }
        ~FileQueue() {
            for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i)
                delete i->second;
        }
        void add(QueueItem* qi);
        QueueItem* add(const string& aTarget, int64_t aSize, int aFlags, QueueItem::Priority p,
            const string& aTempTarget, time_t aAdded, const TTHValue& root)
            throw(QueueException, FileException);

        QueueItem* find(const string& target);
        void find(QueueItem::List& sl, int64_t aSize, const string& ext);
        void find(QueueItem::List& ql, const TTHValue& tth);
        // find some PFS sources to exchange parts info
        void findPFSSources(PFSSourceList&);
        bool exists(const TTHValue& tth) const;
        // return a PFS tth to DHT publish
        TTHValue* findPFSPubTTH();
        QueueItem* findAutoSearch(StringList& recent);
        size_t getSize() { return queue.size(); }
        QueueItem::StringMap& getQueue() { return queue; }
        void move(QueueItem* qi, const string& aTarget);
        void remove(QueueItem* qi);
    private:
        QueueItem::StringMap queue;
        /** A hint where to insert an item... */
        QueueItem::StringIter lastInsert;
    };

    /** All queue items indexed by user (this is a cache for the FileQueue really...) */
    class UserQueue {
    public:
        void add(QueueItem* qi);
        void add(QueueItem* qi, const UserPtr& aUser);
        QueueItem* getNext(const UserPtr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST, int64_t wantedSize = 0,int64_t lastSpeed =0 ,bool allowRemove = true);
        QueueItem* getRunning(const UserPtr& aUser);
        void addDownload(QueueItem* qi, Download* d);
        void removeDownload(QueueItem* qi, const UserPtr& d);
        QueueItem::UserListMap& getList(int p) { return userQueue[p]; }
        void remove(QueueItem* qi, bool removeRunning = true);
        void remove(QueueItem* qi, const UserPtr& aUser, bool removeRunning = true);
        void setPriority(QueueItem* qi, QueueItem::Priority p);

        QueueItem::UserMap& getRunning() { return running; }
        bool isRunning(const UserPtr& aUser) const {
            return (running.find(aUser) != running.end());
        }
        int64_t getQueued(const UserPtr& aUser) const;
    private:
        /** QueueItems by priority and user (this is where the download order is determined) */
        QueueItem::UserListMap userQueue[QueueItem::LAST];
        /** Currently running downloads, a QueueItem is always either here or in the userQueue */
        QueueItem::UserMap running;
    };

    friend class QueueLoader;
    friend class Singleton<QueueManager>;

    QueueManager();
    virtual ~QueueManager() throw();

    mutable CriticalSection cs;

    /** QueueItems by target */
    FileQueue fileQueue;
    /** QueueItems by user */
    UserQueue userQueue;
    /** Directories queued for downloading */
    DirectoryItem::DirectoryMap directories;
    /** Recent searches list, to avoid searching for the same thing too often */
    StringList recent;
    /** The queue needs to be saved */
    bool dirty;
    /** Next search */
    uint64_t nextSearch;
    /** File lists not to delete */
    StringList protectedFileLists;
    /** Sanity check for the target filename */
    static string checkTarget(const string& aTarget, bool checkExsistence) throw(QueueException, FileException);
    /** Add a source to an existing queue item */
    bool addSource(QueueItem* qi, const HintedUser& aUser, Flags::MaskType addBad) throw(QueueException, FileException);

    void processList(const string& name, const HintedUser& user, int flags);

    void load(const SimpleXML& aXml);
    void moveFile(const string& source, const string& target);
    static void moveFile_(const string& source, const string& target);
    void moveStuckFile(QueueItem* qi);
    void rechecked(QueueItem* qi);

    void setDirty();

    string getListPath(const HintedUser& user);

    void checkSfv(QueueItem* qi, Download* d);
    uint32_t calcCrc32(const string& file) throw(FileException);

    // TimerManagerListener
    virtual void on(TimerManagerListener::Second, uint64_t aTick) throw();
    virtual void on(TimerManagerListener::Minute, uint64_t aTick) throw();

    // SearchManagerListener
    virtual void on(SearchManagerListener::SR, const SearchResultPtr&) throw();

    // ClientManagerListener
    virtual void on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw();
    virtual void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw();
};

} // namespace dcpp

#endif // !defined(QUEUE_MANAGER_H)
