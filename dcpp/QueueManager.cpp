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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "QueueManager.h"
#include "format.h"

#include "ClientManager.h"
#include "ConnectionManager.h"
#include "DirectoryListing.h"
#include "Download.h"
#include "DownloadManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "SearchManager.h"
#include "ShareManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"
#include "Transfer.h"
#include "UserConnection.h"
#include "version.h"
#include "SearchResult.h"
#include "MerkleCheckOutputStream.h"
#include "SFVReader.h"
#include "FilteredFile.h"
#ifdef DHT
#include "dht/IndexManager.h"
#endif
#include <climits>

#if !defined(_WIN32) && !defined(PATH_MAX) // Extra PATH_MAX check for Mac OS X
#include <sys/syslimits.h>
#endif

#ifdef ff
#undef ff
#endif

namespace dcpp {

QueueItem* QueueManager::FileQueue::add(const string& aTarget, int64_t aSize,
                          int aFlags, QueueItem::Priority p, const string& aTempTarget,
                          time_t aAdded, const TTHValue& root) throw(QueueException, FileException)
{
    if(p == QueueItem::DEFAULT) {
        p = QueueItem::NORMAL;
        if(aSize <= SETTING(PRIO_HIGHEST_SIZE)*1024) {
            p = QueueItem::HIGHEST;
        } else if(aSize <= SETTING(PRIO_HIGH_SIZE)*1024) {
            p = QueueItem::HIGH;
        } else if(aSize <= SETTING(PRIO_NORMAL_SIZE)*1024) {
            p = QueueItem::NORMAL;
        } else if(aSize <= SETTING(PRIO_LOW_SIZE)*1024) {
            p = QueueItem::LOW;
        } else if(SETTING(PRIO_LOWEST)) {
            p = QueueItem::LOWEST;
        }
    }

    QueueItem* qi = new QueueItem(aTarget, aSize, p, aFlags, aAdded, root);

    if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
        qi->setPriority(QueueItem::HIGHEST);
    }

    qi->setTempTarget(aTempTarget);

    dcassert(find(aTarget) == NULL);
    add(qi);
    return qi;
}

void QueueManager::FileQueue::add(QueueItem* qi) {
    if(lastInsert == queue.end())
        lastInsert = queue.insert(make_pair(const_cast<string*>(&qi->getTarget()), qi)).first;
    else
        lastInsert = queue.insert(lastInsert, make_pair(const_cast<string*>(&qi->getTarget()), qi));
}

void QueueManager::FileQueue::remove(QueueItem* qi) {
    if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
        ++lastInsert;
    queue.erase(const_cast<string*>(&qi->getTarget()));
    delete qi;
}

QueueItem* QueueManager::FileQueue::find(const string& target) {
    QueueItem::StringIter i = queue.find(const_cast<string*>(&target));
    return (i == queue.end()) ? NULL : i->second;
}

void QueueManager::FileQueue::find(QueueItem::List& sl, int64_t aSize, const string& suffix) {
    for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
        if(i->second->getSize() == aSize) {
            const string& t = i->second->getTarget();
            if(suffix.empty() || (suffix.length() < t.length() &&
                Util::stricmp(suffix.c_str(), t.c_str() + (t.length() - suffix.length())) == 0) )
                sl.push_back(i->second);
        }
    }
}

void QueueManager::FileQueue::find(QueueItem::List& ql, const TTHValue& tth) {
    for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
        QueueItem* qi = i->second;
        if(qi->getTTH() == tth) {
            ql.push_back(qi);
        }
    }
}

bool QueueManager::FileQueue::exists(const TTHValue& tth) const {
    for(QueueItem::StringMap::const_iterator i = queue.begin(); i != queue.end(); ++i)
        if(i->second->getTTH() == tth)
            return true;
    return false;
}

static QueueItem* findCandidate(QueueItem* cand, QueueItem::StringIter start, QueueItem::StringIter end, const StringList& recent) {
    for(QueueItem::StringIter i = start; i != end; ++i) {
        QueueItem* q = i->second;

        // We prefer to search for things that are not running...
        if((cand != NULL) && q->isRunning())
            continue;
        // No finished files
        if(q->isFinished())
            continue;
        // No user lists
        if(q->isSet(QueueItem::FLAG_USER_LIST))
            continue;
        // No paused downloads
        if(q->getPriority() == QueueItem::PAUSED)
            continue;
        // No files that already have more than AUTO_SEARCH_LIMIT online sources
        if(q->countOnlineUsers() >= SETTING(AUTO_SEARCH_LIMIT))
            continue;
        // Did we search for it recently?
        if(find(recent.begin(), recent.end(), q->getTarget()) != recent.end())
            continue;

        cand = q;

        if(cand->isWaiting())
            break;
    }
    return cand;
}

QueueItem* QueueManager::FileQueue::findAutoSearch(StringList& recent) {
    // We pick a start position at random, hoping that we will find something to search for...
    QueueItem::StringMap::size_type start = (QueueItem::StringMap::size_type)Util::rand((uint32_t)queue.size());

    QueueItem::StringIter i = queue.begin();
    advance(i, start);

    QueueItem* cand = findCandidate(NULL, i, queue.end(), recent);
    if(cand == NULL || cand->isRunning()) {
        cand = findCandidate(cand, queue.begin(), i, recent);
    }
    return cand;
}

void QueueManager::FileQueue::move(QueueItem* qi, const string& aTarget) {
    if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
        lastInsert = queue.end();
    queue.erase(const_cast<string*>(&qi->getTarget()));
    qi->setTarget(aTarget);
    add(qi);
}

bool QueueManager::getQueueInfo(const UserPtr& aUser, string& aTarget, int64_t& aSize, int& aFlags) throw() {
    Lock l(cs);
    QueueItem* qi = userQueue.getNext(aUser);
    if(qi == NULL)
        return false;

    aTarget = qi->getTarget();
    aSize = qi->getSize();
    aFlags = qi->getFlags();

    return true;
}

void QueueManager::UserQueue::add(QueueItem* qi) {
    for(QueueItem::SourceConstIter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
        add(qi, i->getUser());
    }
}

void QueueManager::UserQueue::add(QueueItem* qi, const UserPtr& aUser) {
    QueueItem::List& l = userQueue[qi->getPriority()][aUser];

    if(qi->getDownloadedBytes() > 0) {
        l.push_front(qi);
    } else {
        l.push_back(qi);
    }
}

QueueItem* QueueManager::UserQueue::getNext(const UserPtr& aUser, QueueItem::Priority minPrio, int64_t wantedSize,int64_t lastSpeed,bool allowRemove) {
    int p = QueueItem::LAST - 1;
        string lastError = Util::emptyString;

    do {
        QueueItem::UserListIter i = userQueue[p].find(aUser);
        if(i != userQueue[p].end()) {
            dcassert(!i->second.empty());
            for(QueueItem::Iter j = i->second.begin(); j != i->second.end(); ++j) {
                QueueItem* qi = *j;
                QueueItem::SourceConstIter source = qi->getSource(aUser);
                if(source->isSet(QueueItem::Source::FLAG_PARTIAL)) {
                        // check partial source
                        int64_t blockSize = HashManager::getInstance()->getBlockSize(qi->getTTH());
                        if(blockSize == 0)
                                blockSize = qi->getSize();

                        Segment segment = qi->getNextSegment(blockSize, wantedSize, lastSpeed, source->getPartialSource());
                        if(allowRemove && segment.getStart() != -1 && segment.getSize() == 0) {
                                // no other partial chunk from this user, remove him from queue
                                remove(qi, aUser);
                                qi->removeSource(aUser, QueueItem::Source::FLAG_NO_NEED_PARTS);
                                lastError = _("No needed part");
                                p++;
                                break;
                        }
                }
                if(qi->isWaiting()) {
                    return qi;
                }

                // No segmented downloading when getting the tree
                if(qi->getDownloads()[0]->getType() == Transfer::TYPE_TREE) {
                    continue;
                }
                if(!qi->isSet(QueueItem::FLAG_USER_LIST)) {
                    int64_t blockSize = HashManager::getInstance()->getBlockSize(qi->getTTH());
                    if(blockSize == 0)
                        blockSize = qi->getSize();
                    if(qi->getNextSegment(blockSize, wantedSize,lastSpeed, source->getPartialSource()).getSize() == 0) {
                        dcdebug("No segment for %s in %s, block " I64_FMT "\n", aUser->getCID().toBase32().c_str(), qi->getTarget().c_str(), blockSize);
                        continue;
                    }
                }
                return qi;
            }
        }
        p--;
    } while(p >= minPrio);

    return NULL;
}

void QueueManager::UserQueue::addDownload(QueueItem* qi, Download* d) {
    qi->getDownloads().push_back(d);

    // Only one download per user...
    dcassert(running.find(d->getUser()) == running.end());
    running[d->getUser()] = qi;
}

void QueueManager::UserQueue::removeDownload(QueueItem* qi, const UserPtr& user) {
    running.erase(user);

    for(DownloadList::iterator i = qi->getDownloads().begin(); i != qi->getDownloads().end(); ++i) {
        if((*i)->getUser() == user) {
            qi->getDownloads().erase(i);
            break;
        }
    }
}

void QueueManager::UserQueue::setPriority(QueueItem* qi, QueueItem::Priority p) {
    remove(qi, false);
    qi->setPriority(p);
    add(qi);
}

int64_t QueueManager::UserQueue::getQueued(const UserPtr& aUser) const {
    int64_t total = 0;
    for(size_t i = QueueItem::LOWEST; i < QueueItem::LAST; ++i) {
        const QueueItem::UserListMap& ulm = userQueue[i];
        QueueItem::UserListMap::const_iterator iulm = ulm.find(aUser);
        if(iulm == ulm.end()) {
            continue;
        }

        for(QueueItem::List::const_iterator j = iulm->second.begin(); j != iulm->second.end(); ++j) {
            const QueueItem::Ptr qi = *j;
            if(qi->getSize() != -1) {
                total += qi->getSize() - qi->getDownloadedBytes();
            }
        }
    }
    return total;
}

QueueItem* QueueManager::UserQueue::getRunning(const UserPtr& aUser) {
    QueueItem::UserIter i = running.find(aUser);
    return (i == running.end()) ? 0 : i->second;
}

void QueueManager::UserQueue::remove(QueueItem* qi, bool removeRunning) {
    for(QueueItem::SourceConstIter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
        remove(qi, i->getUser(), removeRunning);
    }
}

void QueueManager::UserQueue::remove(QueueItem* qi, const UserPtr& aUser, bool removeRunning) {
        if(removeRunning && qi == getRunning(aUser)) {
        removeDownload(qi, aUser);
    }

    dcassert(qi->isSource(aUser));
    QueueItem::UserListMap& ulm = userQueue[qi->getPriority()];
    QueueItem::UserListIter j = ulm.find(aUser);
    dcassert(j != ulm.end());
    QueueItem::List& l = j->second;
    QueueItem::Iter i = find(l.begin(), l.end(), qi);
    dcassert(i != l.end());
    l.erase(i);

    if(l.empty()) {
        ulm.erase(j);
    }
}

void QueueManager::FileMover::moveFile(const string& source, const string& target) {
    Lock l(cs);
    files.push_back(make_pair(source, target));
    if(!active) {
        active = true;
        start();
    }
}

int QueueManager::FileMover::run() {
    for(;;) {
        FilePair next;
        {
            Lock l(cs);
            if(files.empty()) {
                active = false;
                return 0;
            }
            next = files.back();
            files.pop_back();
        }
                moveFile_(next.first, next.second);
    }
}

void QueueManager::Rechecker::add(const string& file) {
    Lock l(cs);
    files.push_back(file);
    if(!active) {
        active = true;
        start();
    }
}

int QueueManager::Rechecker::run() {
    while(true) {
        string file;
        {
            Lock l(cs);
            StringIter i = files.begin();
            if(i == files.end()) {
                active = false;
                return 0;
            }
            file = *i;
            files.erase(i);
        }

        QueueItem* q;
        int64_t tempSize;
        TTHValue tth;

        {
            Lock l(qm->cs);

            q = qm->fileQueue.find(file);
            if(!q || q->isSet(QueueItem::FLAG_USER_LIST))
                continue;

                        qm->fire(QueueManagerListener::RecheckStarted(), q->getTarget());
            dcdebug("Rechecking %s\n", file.c_str());

            tempSize = File::getSize(q->getTempTarget());

            if(tempSize == -1) {
                                qm->fire(QueueManagerListener::RecheckNoFile(), q->getTarget());
                continue;
            }

            if(tempSize < 64*1024) {
                                qm->fire(QueueManagerListener::RecheckFileTooSmall(), q->getTarget());
                continue;
            }

            if(tempSize != q->getSize()) {
                File(q->getTempTarget(), File::WRITE, File::OPEN).setSize(q->getSize());
            }

            if(q->isRunning()) {
                                qm->fire(QueueManagerListener::RecheckDownloadsRunning(), q->getTarget());
                continue;
            }

            tth = q->getTTH();
        }

        TigerTree tt;
        bool gotTree = HashManager::getInstance()->getTree(tth, tt);

        string tempTarget;

        {
            Lock l(qm->cs);

            // get q again in case it has been (re)moved
            q = qm->fileQueue.find(file);
            if(!q)
                continue;

            if(!gotTree) {
                                qm->fire(QueueManagerListener::RecheckNoTree(), q->getTarget());
                continue;
            }

            //Clear segments
            q->resetDownloaded();

            tempTarget = q->getTempTarget();
        }

        //Merklecheck
        int64_t startPos=0;
        DummyOutputStream dummy;
        int64_t blockSize = tt.getBlockSize();
        bool hasBadBlocks = false;

        vector<uint8_t> buf((size_t)min((int64_t)1024*1024, blockSize));

        typedef pair<int64_t, int64_t> SizePair;
        typedef vector<SizePair> Sizes;
        Sizes sizes;

        {
            File inFile(tempTarget, File::READ, File::OPEN);

            while(startPos < tempSize) {
                try {
                    MerkleCheckOutputStream<TigerTree, false> check(tt, &dummy, startPos);

                    inFile.setPos(startPos);
                    int64_t bytesLeft = min((tempSize - startPos),blockSize); //Take care of the last incomplete block
                    int64_t segmentSize = bytesLeft;
                    while(bytesLeft > 0) {
                        size_t n = (size_t)min((int64_t)buf.size(), bytesLeft);
                        size_t nr = inFile.read(&buf[0], n);
                        check.write(&buf[0], nr);
                        bytesLeft -= nr;
                        if(bytesLeft > 0 && nr == 0) {
                            // Huh??
                            throw Exception();
                        }
                    }
                    check.flush();

                    sizes.push_back(make_pair(startPos, segmentSize));
                } catch(const Exception&) {
                    hasBadBlocks = true;
                    dcdebug("Found bad block at " I64_FMT "\n", startPos);
                }
                startPos += blockSize;
            }
        }

        Lock l(qm->cs);

        // get q again in case it has been (re)moved
        q = qm->fileQueue.find(file);
        if(!q)
            continue;

        //If no bad blocks then the file probably got stuck in the temp folder for some reason
        if(!hasBadBlocks) {
            qm->moveStuckFile(q);
            continue;
        }

        for(Sizes::const_iterator i = sizes.begin(); i != sizes.end(); ++i)
            q->addSegment(Segment(i->first, i->second));

        qm->rechecked(q);
    }
    return 0;
}

QueueManager::QueueManager() :
lastSave(0),
queueFile(Util::getPath(Util::PATH_USER_CONFIG) + "Queue.xml"),
rechecker(this),
dirty(true),
nextSearch(0)
{
    TimerManager::getInstance()->addListener(this);
    SearchManager::getInstance()->addListener(this);
    ClientManager::getInstance()->addListener(this);

    File::ensureDirectory(Util::getListPath());
}

QueueManager::~QueueManager() throw() {
    SearchManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    ClientManager::getInstance()->removeListener(this);

    if(!BOOLSETTING(KEEP_LISTS)) {
        string path = Util::getListPath();

                std::sort(protectedFileLists.begin(), protectedFileLists.end());

        StringList filelists = File::findFiles(path, "*.xml.bz2");
                std::sort(filelists.begin(), filelists.end());
                std::for_each(filelists.begin(), std::set_difference(filelists.begin(), filelists.end(),
                        protectedFileLists.begin(), protectedFileLists.end(), filelists.begin()), &File::deleteFile);

        filelists = File::findFiles(path, "*.DcLst");
                std::sort(filelists.begin(), filelists.end());
                std::for_each(filelists.begin(), std::set_difference(filelists.begin(), filelists.end(),
                        protectedFileLists.begin(), protectedFileLists.end(), filelists.begin()), &File::deleteFile);
    }
}

bool QueueManager::getTTH(const string& name, TTHValue& tth) throw() {
    Lock l(cs);
    QueueItem* qi = fileQueue.find(name);
    if(qi) {
        tth = qi->getTTH();
        return true;
    }
    return false;
}

struct PartsInfoReqParam{
        PartsInfo       parts;
        string          tth;
        string          myNick;
        string          hubIpPort;
        string          ip;
    uint16_t    udpPort;
};

void QueueManager::on(TimerManagerListener::Minute, uint64_t aTick) throw() {
    string fn;
    string searchString;
    bool online = false;
    vector<const PartsInfoReqParam*> params;
    TTHValue* tthPub = NULL;
    {
        Lock l(cs);
        //find max 10 pfs sources to exchange parts
        //the source basis interval is 5 minutes
        PFSSourceList sl;
        fileQueue.findPFSSources(sl);

        for(PFSSourceList::const_iterator i = sl.begin(); i != sl.end(); i++){
                QueueItem::PartialSource::Ptr source = (*i->first).getPartialSource();
                const QueueItem* qi = i->second;

                PartsInfoReqParam* param = new PartsInfoReqParam;

                int64_t blockSize = HashManager::getInstance()->getBlockSize(qi->getTTH());
                if(blockSize == 0)
                        blockSize = qi->getSize();
                qi->getPartialInfo(param->parts, blockSize);

                param->tth = qi->getTTH().toBase32();
                param->ip  = source->getIp();
                param->udpPort = source->getUdpPort();
                param->myNick = source->getMyNick();
                param->hubIpPort = source->getHubIpPort();

                params.push_back(param);

                source->setPendingQueryCount(source->getPendingQueryCount() + 1);
                source->setNextQueryTime(aTick + 300000);               // 5 minutes
        }

        if(BOOLSETTING(USE_DHT) && SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_PASSIVE)
                tthPub = fileQueue.findPFSPubTTH();

        if(BOOLSETTING(AUTO_SEARCH) && (aTick >= nextSearch) && (fileQueue.getSize() > 0)) {
            // We keep 30 recent searches to avoid duplicate searches
            while((recent.size() >= fileQueue.getSize()) || (recent.size() > 30)) {
                recent.erase(recent.begin());
            }

            QueueItem* qi = fileQueue.findAutoSearch(recent);
            if(qi) {
                searchString = qi->getTTH().toBase32();
                online = qi->hasOnlineUsers();
                recent.push_back(qi->getTarget());
                nextSearch = aTick + (SETTING(AUTO_SEARCH_TIME) * 60000);
                if (BOOLSETTING(REPORT_ALTERNATES))
                    LogManager::getInstance()->message(str(F_("Searching TTH alternates for: %1%")%Util::getFileName(qi->getTargetFileName())));
            }
        }
    }
    // Request parts info from partial file sharing sources
    for(vector<const PartsInfoReqParam*>::const_iterator i = params.begin(); i != params.end(); i++){
        const PartsInfoReqParam* param = *i;
        dcassert(param->udpPort > 0);

        try {
            AdcCommand cmd = SearchManager::getInstance()->toPSR(true, param->myNick, param->hubIpPort, param->tth, param->parts);
            Socket s;
            s.writeTo(param->ip, param->udpPort, cmd.toString(ClientManager::getInstance()->getMyCID()));
        } catch(...) {
            dcdebug("Partial search caught error\n");
        }

        delete param;
    }

     // DHT PFS announce
    if(tthPub)
    {
    #ifdef DHT
            dht::IndexManager::getInstance()->publishPartialFile(*tthPub);
    #endif
            delete tthPub;
    }

    if(!searchString.empty()) {
        SearchManager::getInstance()->search(searchString, 0, SearchManager::TYPE_TTH, SearchManager::SIZE_DONTCARE, "auto");
    }
}

void QueueManager::addList(const HintedUser& aUser, int aFlags, const string& aInitialDir /* = Util::emptyString */) throw(QueueException, FileException) {
        add(aInitialDir, -1, TTHValue(), aUser, QueueItem::FLAG_USER_LIST | aFlags);
}

string QueueManager::getListPath(const HintedUser& user) {
        StringList nicks = ClientManager::getInstance()->getNicks(user);
    string nick = nicks.empty() ? Util::emptyString : Util::cleanPathChars(nicks[0]) + ".";
        return checkTarget(Util::getListPath() + nick + user.user->getCID().toBase32(), /*checkExistence*/ false);
}
//NOTE: freedcpp
void QueueManager::add(const string& aTarget, int64_t aSize, const TTHValue& root) throw(QueueException, FileException)
{
        // Check if we're not downloading something already in our share
        if (BOOLSETTING(DONT_DL_ALREADY_SHARED))
        {
                if (ShareManager::getInstance()->isTTHShared(root))
                {
                        throw QueueException(_("A file with the same hash already exists in your share"));
                }
        }
        // Check that target contains at least one directory...we don't want headless files...
        // Check that the file doesn't already exist...
        const string target = checkTarget(aTarget, aSize);

        // Check if it's a zero-byte file, if so, create and return...
        if (aSize == 0)
        {
                if (!BOOLSETTING(SKIP_ZERO_BYTE))
                {
                        File::ensureDirectory(target);
                        File f(target, File::WRITE, File::CREATE);
                }
                return;
        }

        Lock l(cs);

        // This will be pretty slow on large queues...
        if (BOOLSETTING(DONT_DL_ALREADY_QUEUED))
        {
                QueueItem::List ql;
                fileQueue.find(ql, root);
                if (ql.size() > 0)
                        throw QueueException(_("This file is already queued"));
        }

        QueueItem* q = fileQueue.find(target);

        if(q == NULL)
        {
                q = fileQueue.add(target, aSize, 0, QueueItem::DEFAULT/*QueueItem::Priority*/, Util::emptyString/*aTempTarget*/,
                        GET_TIME()/*time_t aAdded*/, root/*TTHValue& root*/);
                fire(QueueManagerListener::Added(), q);
        }
        else
        {
                if(q->getSize() != aSize)
                {
                        throw QueueException(_("A file with a different size already exists in the queue"));
}
                if(!(root == q->getTTH()))
                {
                        throw QueueException(_("A file with different tth root already exists in the queue"));
                }
        }
}//NOTE: freedcpp

void QueueManager::add(const string& aTarget, int64_t aSize, const TTHValue& root, const HintedUser& aUser,
    int aFlags /* = 0 */, bool addBad /* = true */) throw(QueueException, FileException)
{
    bool wantConnection = true;

    // Check that we're not downloading from ourselves...
    if(aUser == ClientManager::getInstance()->getMe()) {
        throw QueueException(_("You're trying to download from yourself!"));
    }

    // Check if we're not downloading something already in our share
    if(BOOLSETTING(DONT_DL_ALREADY_SHARED)){
        if (ShareManager::getInstance()->isTTHShared(root)){
            throw QueueException(_("A file with the same hash already exists in your share"));
        }
    }

    string target;
    string tempTarget;
    if((aFlags & QueueItem::FLAG_USER_LIST) == QueueItem::FLAG_USER_LIST) {
        target = getListPath(aUser);
        tempTarget = aTarget;
    } else {
                target = checkTarget(aTarget, /*checkExistence*/ true);
    }

    // Check if it's a zero-byte file, if so, create and return...
    if(aSize == 0) {
        if(!BOOLSETTING(SKIP_ZERO_BYTE)) {
            File::ensureDirectory(target);
            File f(target, File::WRITE, File::CREATE);
        }
        return;
    }

    {
        Lock l(cs);

        // This will be pretty slow on large queues...
                if(BOOLSETTING(DONT_DL_ALREADY_QUEUED) && !(aFlags & QueueItem::FLAG_USER_LIST)) {
                        QueueItem::List ql;
                        fileQueue.find(ql, root);
                        if (ql.size() > 0) {
                                // Found one or more existing queue items, lets see if we can add the source to them
                                bool sourceAdded = false;
                                for(QueueItem::Iter i = ql.begin(); i != ql.end(); ++i) {
                                        if(!(*i)->isSource(aUser)) {
                                                try {
                                                        wantConnection = addSource(*i, aUser, addBad ? QueueItem::Source::FLAG_MASK : 0);
                                                        sourceAdded = true;
                                                } catch(...) { }
                                        }
                                }

                                if(!sourceAdded) {
            throw QueueException(_("This file is already queued"));
                                }
                                goto connect;
                        }
                }

        QueueItem* q = fileQueue.find(target);
        if(q == NULL) {
            q = fileQueue.add(target, aSize, aFlags, QueueItem::DEFAULT, tempTarget, GET_TIME(), root);
            fire(QueueManagerListener::Added(), q);
        } else {
            if(q->getSize() != aSize) {
                throw QueueException(_("A file with a different size already exists in the queue"));
            }
            if(!(root == q->getTTH())) {
                                throw QueueException(_("A file with a different TTH root already exists in the queue"));
            }

                        if(q->isFinished()) {
                                throw QueueException(_("This file has already finished downloading"));
        }

                        q->setFlag(aFlags);
    }

                wantConnection = addSource(q, aUser, addBad ? QueueItem::Source::FLAG_MASK : 0);
    }

connect:
        if(wantConnection && aUser.user->isOnline())
                ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

void QueueManager::readd(const string& target, const HintedUser& aUser) throw(QueueException) {
    bool wantConnection = false;
    {
        Lock l(cs);
        QueueItem* q = fileQueue.find(target);
        if(q && q->isBadSource(aUser)) {
            wantConnection = addSource(q, aUser, QueueItem::Source::FLAG_MASK);
        }
    }
        if(wantConnection && aUser.user->isOnline())
                ConnectionManager::getInstance()->getDownloadConnection(aUser);
}

void QueueManager::setDirty() {
    if(!dirty) {
        dirty = true;
        lastSave = GET_TICK();
    }
}

string QueueManager::checkTarget(const string& aTarget, bool checkExistence) throw(QueueException, FileException) {
#ifdef _WIN32
    if(aTarget.length() > MAX_PATH) {
        throw QueueException(_("Target filename too long"));
    }
    // Check that target starts with a drive or is an UNC path
    if( (aTarget[1] != ':' || aTarget[2] != '\\') &&
        (aTarget[0] != '\\' && aTarget[1] != '\\') ) {
        throw QueueException(_("Invalid target file (missing directory, check default download directory setting)"));
    }
#else
    if(aTarget.length() > PATH_MAX) {
        throw QueueException(_("Target filename too long"));
    }
    // Check that target contains at least one directory...we don't want headless files...
    if(aTarget[0] != '/') {
        throw QueueException(_("Invalid target file (missing directory, check default download directory setting)"));
    }
#endif

    string target = Util::validateFileName(aTarget);

    // Check that the file doesn't already exist...
        if(checkExistence && File::getSize(target) != -1) {
                throw FileException(_("File already exists at the target location"));
    }
    return target;
}

/** Add a source to an existing queue item */
bool QueueManager::addSource(QueueItem* qi, const HintedUser& aUser, Flags::MaskType addBad) throw(QueueException, FileException) {
    bool wantConnection = (qi->getPriority() != QueueItem::PAUSED) && !userQueue.getRunning(aUser);

    if(qi->isSource(aUser)) {
        if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
            return wantConnection;
        }
        throw QueueException(str(F_("Duplicate source: %1%") % Util::getFileName(qi->getTarget())));
    }

    if(qi->isBadSourceExcept(aUser, addBad)) {
        throw QueueException(str(F_("Duplicate source: %1%") % Util::getFileName(qi->getTarget())));
    }

    qi->addSource(aUser);

        if(aUser.user->isSet(User::PASSIVE) && !ClientManager::getInstance()->isActive() ) {
        qi->removeSource(aUser, QueueItem::Source::FLAG_PASSIVE);
                wantConnection = false;
        } else if(qi->isFinished()) {
        wantConnection = false;
    } else {
        userQueue.add(qi, aUser);
    }

    fire(QueueManagerListener::SourcesUpdated(), qi);
    setDirty();

    return wantConnection;
}

void QueueManager::addDirectory(const string& aDir, const HintedUser& aUser, const string& aTarget, QueueItem::Priority p /* = QueueItem::DEFAULT */) throw() {
    bool needList;
    {
        Lock l(cs);

        DirectoryItem::DirectoryPair dp = directories.equal_range(aUser);

        for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
            if(Util::stricmp(aTarget.c_str(), i->second->getName().c_str()) == 0)
                return;
        }

        // Unique directory, fine...
        directories.insert(make_pair(aUser, new DirectoryItem(aUser, aDir, aTarget, p)));
        needList = (dp.first == dp.second);
        setDirty();
    }

    if(needList) {
        try {
                        addList(aUser, QueueItem::FLAG_DIRECTORY_DOWNLOAD);
        } catch(const Exception&) {
            // Ignore, we don't really care...
        }
    }
}

QueueItem::Priority QueueManager::hasDownload(const UserPtr& aUser) throw() {
    Lock l(cs);
    QueueItem* qi = userQueue.getNext(aUser, QueueItem::LOWEST);
    if(!qi) {
        return QueueItem::PAUSED;
    }
    return qi->getPriority();
}
namespace {
typedef unordered_map<TTHValue, const DirectoryListing::File*> TTHMap;

// *** WARNING ***
// Lock(cs) makes sure that there's only one thread accessing this
static TTHMap tthMap;

void buildMap(const DirectoryListing::Directory* dir) throw() {
    for(DirectoryListing::Directory::List::const_iterator j = dir->directories.begin(); j != dir->directories.end(); ++j) {
        if(!(*j)->getAdls())
            buildMap(*j);
    }

    for(DirectoryListing::File::List::const_iterator i = dir->files.begin(); i != dir->files.end(); ++i) {
        const DirectoryListing::File* df = *i;
        tthMap.insert(make_pair(df->getTTH(), df));
    }
}
}

int QueueManager::matchListing(const DirectoryListing& dl) throw() {
    int matches = 0;
    {
        Lock l(cs);
        tthMap.clear();
        buildMap(dl.getRoot());

        for(QueueItem::StringMap::const_iterator i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
            QueueItem* qi = i->second;
                        if(qi->isFinished())
                                continue;
            if(qi->isSet(QueueItem::FLAG_USER_LIST))
                continue;
            TTHMap::iterator j = tthMap.find(qi->getTTH());
            if(j != tthMap.end() && i->second->getSize() == qi->getSize()) {
                try {
                    addSource(qi, dl.getUser(), QueueItem::Source::FLAG_FILE_NOT_AVAILABLE);
                } catch(...) {
                    // Ignore...
                }
                matches++;
            }
        }
    }
    if(matches > 0)
                ConnectionManager::getInstance()->getDownloadConnection(dl.getUser());
    return matches;
}

int64_t QueueManager::getPos(const string& target) throw() {
    Lock l(cs);
    QueueItem* qi = fileQueue.find(target);
    if(qi) {
        return qi->getDownloadedBytes();
    }
    return -1;
}

int64_t QueueManager::getSize(const string& target) throw() {
    Lock l(cs);
    QueueItem* qi = fileQueue.find(target);
    if(qi) {
        return qi->getSize();
    }
    return -1;
}


void QueueManager::move(const string& aSource, const string& aTarget) throw() {
    string target = Util::validateFileName(aTarget);
    if(aSource == target)
        return;

    bool delSource = false;

    Lock l(cs);
    QueueItem* qs = fileQueue.find(aSource);
    if(qs) {
        // Don't move running downloads
        if(qs->isRunning()) {
            return;
        }
        // Don't move file lists
        if(qs->isSet(QueueItem::FLAG_USER_LIST))
            return;

        // Let's see if the target exists...then things get complicated...
        QueueItem* qt = fileQueue.find(target);
        if(qt == NULL || Util::stricmp(aSource, target) == 0) {
            // Good, update the target and move in the queue...
            fileQueue.move(qs, target);
            fire(QueueManagerListener::Moved(), qs, aSource);
            setDirty();
        } else {
            // Don't move to target of different size
            if(qs->getSize() != qt->getSize() || qs->getTTH() != qt->getTTH())
                return;

                for(QueueItem::SourceConstIter i = qs->getSources().begin(); i != qs->getSources().end(); ++i) {
                    try {
                        addSource(qt, i->getUser(), QueueItem::Source::FLAG_MASK);
                    } catch(const Exception&) {
                    }
                }
            delSource = true;
        }
    }

    if(delSource) {
        remove(aSource);
    }
}

void QueueManager::getTargets(const TTHValue& tth, StringList& sl) {
    Lock l(cs);
    QueueItem::List ql;
    fileQueue.find(ql, tth);
    for(QueueItem::Iter i = ql.begin(); i != ql.end(); ++i) {
        sl.push_back((*i)->getTarget());
    }
}

Download* QueueManager::getDownload(UserConnection& aSource, bool supportsTrees) throw() {
    Lock l(cs);

    UserPtr& u = aSource.getUser();
    dcdebug("Getting download for %s...", u->getCID().toBase32().c_str());

    QueueItem* q = userQueue.getNext(u, QueueItem::LOWEST, aSource.getChunkSize());

    if(!q) {
        dcdebug("none\n");
        return 0;
    }

    // Check that the file we will be downloading to exists
    if(q->getDownloadedBytes() > 0) {
        int64_t tempSize = File::getSize(q->getTempTarget());
        if(tempSize != q->getSize()) {
            // <= 0.706 added ".antifrag" to temporary download files if antifrag was enabled...
            // 0.705 added ".antifrag" even if antifrag was disabled
            std::string antifrag = q->getTempTarget() + ".antifrag";
            if(File::getSize(antifrag) > 0) {
                File::renameFile(antifrag, q->getTempTarget());
                tempSize = File::getSize(q->getTempTarget());
            }
            if(tempSize != q->getSize()) {
                if(tempSize > 0 && tempSize < q->getSize()) {
                    // Probably started with <=0.699 or with 0.705 without antifrag enabled...
                    try {
                        File(q->getTempTarget(), File::WRITE, File::OPEN).setSize(q->getSize());
                    } catch(const FileException&) { }
                } else {
                    // Temp target gone?
                    q->resetDownloaded();
                }
            }
        }
    }

    Download* d = new Download(aSource, *q, q->isSet(QueueItem::FLAG_PARTIAL_LIST) ? q->getTempTarget() : q->getTarget(), supportsTrees);

    userQueue.addDownload(q, d);

    fire(QueueManagerListener::StatusUpdated(), q);
    dcdebug("found %s\n", q->getTarget().c_str());
    return d;
}

namespace {
class TreeOutputStream : public OutputStream {
public:
    TreeOutputStream(TigerTree& aTree) : tree(aTree), bufPos(0) {
    }

    virtual size_t write(const void* xbuf, size_t len) throw(Exception) {
        size_t pos = 0;
        uint8_t* b = (uint8_t*)xbuf;
        while(pos < len) {
            size_t left = len - pos;
            if(bufPos == 0 && left >= TigerTree::BYTES) {
                tree.getLeaves().push_back(TTHValue(b + pos));
                pos += TigerTree::BYTES;
            } else {
                size_t bytes = min(TigerTree::BYTES - bufPos, left);
                memcpy(buf + bufPos, b + pos, bytes);
                bufPos += bytes;
                pos += bytes;
                if(bufPos == TigerTree::BYTES) {
                    tree.getLeaves().push_back(TTHValue(buf));
                    bufPos = 0;
                }
            }
        }
        return len;
    }

    virtual size_t flush() throw(Exception) {
        return 0;
    }
private:
    TigerTree& tree;
    uint8_t buf[TigerTree::BYTES];
    size_t bufPos;
};

}

void QueueManager::setFile(Download* d) {
    if(d->getType() == Transfer::TYPE_FILE) {
        Lock l(cs);

        QueueItem* qi = fileQueue.find(d->getPath());
        if(!qi) {
            throw QueueException(_("Target removed"));
        }

        string target = d->getDownloadTarget();

        if(d->getSegment().getStart() > 0) {
            if(File::getSize(target) != qi->getSize()) {
                // When trying the download the next time, the resume pos will be reset
                throw QueueException(_("Target file is missing or wrong size"));
            }
        } else {
            File::ensureDirectory(target);
        }

        File* f = new File(target, File::WRITE, File::OPEN | File::CREATE | File::SHARED);

        if(f->getSize() != qi->getSize()) {
            f->setSize(qi->getSize());
        }

        f->setPos(d->getSegment().getStart());
        d->setFile(f);
    } else if(d->getType() == Transfer::TYPE_FULL_LIST) {
        string target = d->getPath();
        File::ensureDirectory(target);

        if(d->isSet(Download::FLAG_XML_BZ_LIST)) {
            target += ".xml.bz2";
        } else {
            target += ".xml";
        }
        d->setFile(new File(target, File::WRITE, File::OPEN | File::TRUNCATE | File::CREATE));
    } else if(d->getType() == Transfer::TYPE_PARTIAL_LIST) {
        d->setFile(new StringOutputStream(d->getPFS()));
    } else if(d->getType() == Transfer::TYPE_TREE) {
        d->setFile(new TreeOutputStream(d->getTigerTree()));
    }
}

void QueueManager::moveFile(const string& source, const string& target) {
        File::ensureDirectory(target);
        if(File::getSize(source) > MOVER_LIMIT) {
            mover.moveFile(source, target);
        } else {
                moveFile_(source, target);
        }
            }

void QueueManager::moveFile_(const string& source, const string& target) {
            try {
                File::renameFile(source, target);
                getInstance()->fire(QueueManagerListener::FileMoved(), target);
        } catch(const FileException& e1) {
                // Try to just rename it to the correct name at least
                string newTarget = Util::getFilePath(source) + Util::getFileName(target);
                try {
                        File::renameFile(source, newTarget);
                        LogManager::getInstance()->message(str(F_("Unable to move %1% to %2% (%3%); renamed to %4%") %
                                Util::addBrackets(source) % Util::addBrackets(target) % e1.getError() % Util::addBrackets(newTarget)));
                } catch(const FileException& e2) {
                        LogManager::getInstance()->message(str(F_("Unable to move %1% to %2% (%3%) nor to rename to %4% (%5%)") %
                                Util::addBrackets(source) % Util::addBrackets(target) % e1.getError() % Util::addBrackets(newTarget) % e2.getError()));
        }
    }
}

void QueueManager::moveStuckFile(QueueItem* qi) {
    moveFile(qi->getTempTarget(), qi->getTarget());

        if(qi->isFinished()) {
    userQueue.remove(qi);
        }

        string target = qi->getTarget();

        if(!BOOLSETTING(KEEP_FINISHED_FILES)) {
                fire(QueueManagerListener::Removed(), qi);
    fileQueue.remove(qi);
         } else {
                qi->addSegment(Segment(0, qi->getSize()));
                fire(QueueManagerListener::StatusUpdated(), qi);
        }

        fire(QueueManagerListener::RecheckAlreadyFinished(), target);
}

void QueueManager::rechecked(QueueItem* qi) {
        fire(QueueManagerListener::RecheckDone(), qi->getTarget());
    fire(QueueManagerListener::StatusUpdated(), qi);

    setDirty();
}

void QueueManager::putDownload(Download* aDownload, bool finished) throw() {
        HintedUserList getConn;
        string fl_fname;
        HintedUser fl_user(UserPtr(), Util::emptyString);
        int fl_flag = 0;
        bool downloadList = false;

    {
        Lock l(cs);

        delete aDownload->getFile();
        aDownload->setFile(0);

        if(aDownload->getType() == Transfer::TYPE_PARTIAL_LIST) {
            QueueItem* q = fileQueue.find(getListPath(aDownload->getHintedUser()));
            if(q) {
                if(!aDownload->getPFS().empty()) {
                    if( (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) && directories.find(aDownload->getUser()) != directories.end()) ||
                            (q->isSet(QueueItem::FLAG_MATCH_QUEUE)) )
                    {
                            dcassert(finished);

                            fl_fname = aDownload->getPFS();
                            fl_user = aDownload->getHintedUser();
                            fl_flag = (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) ? (QueueItem::FLAG_DIRECTORY_DOWNLOAD) : 0)
                                    | (q->isSet(QueueItem::FLAG_MATCH_QUEUE) ? QueueItem::FLAG_MATCH_QUEUE : 0) | QueueItem::FLAG_TEXT;
                    } else {
                        fire(QueueManagerListener::PartialList(), aDownload->getHintedUser(), aDownload->getPFS());
                    }
                } else {
                        // partial filelist probably failed, redownload full list
                        dcassert(!finished);

                        downloadList = true;
                        fl_flag = q->getFlags() & ~QueueItem::FLAG_PARTIAL_LIST;
                }
                fire(QueueManagerListener::Removed(), q);

                    userQueue.remove(q);
                    fileQueue.remove(q);
                //} else {
                    //userQueue.removeDownload(q, aDownload->getUser());
                    //fire(QueueManagerListener::StatusUpdated(), q);
                //}
            }
        } else {
            QueueItem* q = fileQueue.find(aDownload->getPath());

            if(q) {
                if(aDownload->getType() == Transfer::TYPE_FULL_LIST) {
                    if(aDownload->isSet(Download::FLAG_XML_BZ_LIST)) {
                        q->setFlag(QueueItem::FLAG_XML_BZLIST);
                    } else {
                        q->unsetFlag(QueueItem::FLAG_XML_BZLIST);
                    }
                }

                if(finished) {
                    if(aDownload->getType() == Transfer::TYPE_TREE) {
                        // Got a full tree, now add it to the HashManager
                        dcassert(aDownload->getTreeValid());
                        HashManager::getInstance()->addTree(aDownload->getTigerTree());

                        userQueue.removeDownload(q, aDownload->getUser());
                        fire(QueueManagerListener::StatusUpdated(), q);
                    } else {
                        // Now, let's see if this was a directory download filelist...
                        if( (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) && directories.find(aDownload->getUser()) != directories.end()) ||
                            (q->isSet(QueueItem::FLAG_MATCH_QUEUE)) )
                        {
                            fl_fname = q->getListName();
                            fl_user = aDownload->getHintedUser();
                            fl_flag = (q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD) ? QueueItem::FLAG_DIRECTORY_DOWNLOAD : 0)
                                | (q->isSet(QueueItem::FLAG_MATCH_QUEUE) ? QueueItem::FLAG_MATCH_QUEUE : 0);
                        }

                        string dir;
                        if(aDownload->getType() == Transfer::TYPE_FULL_LIST) {
                            dir = q->getTempTarget();
                            q->addSegment(Segment(0, q->getSize()));
                        } else if(aDownload->getType() == Transfer::TYPE_FILE) {
                            q->addSegment(aDownload->getSegment());
                        }

                        if (q->isFinished() && BOOLSETTING(SFV_CHECK)) {
                                checkSfv(q, aDownload);
                        }

                        if(aDownload->getType() != Transfer::TYPE_FILE || q->isFinished()) {
                            // Check if we need to move the file
                            if( aDownload->getType() == Transfer::TYPE_FILE && !aDownload->getTempTarget().empty() && (Util::stricmp(aDownload->getPath().c_str(), aDownload->getTempTarget().c_str()) != 0) ) {
                                moveFile(aDownload->getTempTarget(), aDownload->getPath());
                            }

                            fire(QueueManagerListener::Finished(), q, dir, aDownload->getAverageSpeed());

                            userQueue.remove(q);

                            if(!BOOLSETTING(KEEP_FINISHED_FILES) || aDownload->getType() == Transfer::TYPE_FULL_LIST) {
                                fire(QueueManagerListener::Removed(), q);
                                fileQueue.remove(q);
                            } else {
                                fire(QueueManagerListener::StatusUpdated(), q);
                            }
                        } else {
                            userQueue.removeDownload(q, aDownload->getUser());
                            fire(QueueManagerListener::StatusUpdated(), q);
                        }
                        setDirty();
                    }
                } else {
                    if(aDownload->getType() != Transfer::TYPE_TREE) {
                        if(q->getDownloadedBytes() == 0) {
                            q->setTempTarget(Util::emptyString);
                        }
                        if(q->isSet(QueueItem::FLAG_USER_LIST)) {
                            // Blah...no use keeping an unfinished file list...
                            File::deleteFile(q->getListName());
                        }
                        if(aDownload->getType() == Transfer::TYPE_FILE) {
                            // mark partially downloaded chunk, but align it to block size
                            int64_t downloaded = aDownload->getPos();
                            downloaded -= downloaded % aDownload->getTigerTree().getBlockSize();

                            if(downloaded > 0) {
                                q->addSegment(Segment(aDownload->getStartPos(), downloaded));
                                setDirty();
                            }
                        }
                    }

                    if(q->getPriority() != QueueItem::PAUSED) {
                        q->getOnlineUsers(getConn);
                    }

                    userQueue.removeDownload(q, aDownload->getUser());
                    fire(QueueManagerListener::StatusUpdated(), q);
                }
            } else if(aDownload->getType() != Transfer::TYPE_TREE) {
                if(!aDownload->getTempTarget().empty() && (aDownload->getType() == Transfer::TYPE_FULL_LIST || aDownload->getTempTarget() != aDownload->getPath())) {
                    File::deleteFile(aDownload->getTempTarget());
                }
            }
        }
        delete aDownload;
    }

        for(HintedUserList::iterator i = getConn.begin(); i != getConn.end(); ++i) {
                ConnectionManager::getInstance()->getDownloadConnection(*i);
    }

        if(!fl_fname.empty()) {
                processList(fl_fname, fl_user, fl_flag);
    }
}

void QueueManager::processList(const string& name, const HintedUser& user, int flags) {
    DirectoryListing dirList(user);
    try {
        dirList.loadFile(name);
    } catch(const Exception&) {
        LogManager::getInstance()->message(str(F_("Unable to open filelist: %1%") % Util::addBrackets(name)));
        return;
    }

    if(flags & QueueItem::FLAG_DIRECTORY_DOWNLOAD) {
        DirectoryItem::List dl;
        {
            Lock l(cs);
            DirectoryItem::DirectoryPair dp = directories.equal_range(user);
            for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
                dl.push_back(i->second);
            }
            directories.erase(user);
        }

        for(DirectoryItem::Iter i = dl.begin(); i != dl.end(); ++i) {
            DirectoryItem* di = *i;
            dirList.download(di->getName(), di->getTarget(), false);
            delete di;
        }
    }
    if(flags & QueueItem::FLAG_MATCH_QUEUE) {
                size_t files = matchListing(dirList);
        LogManager::getInstance()->message(str(FN_("%1%: Matched %2% file", "%1%: Matched %2% files", files) %
                        Util::toString(ClientManager::getInstance()->getNicks(user)) % files));
    }
}

void QueueManager::recheck(const string& aTarget) {
    rechecker.add(aTarget);
}

void QueueManager::remove(const string& aTarget) throw() {
    UserList x;

    {
        Lock l(cs);

        QueueItem* q = fileQueue.find(aTarget);
        if(!q)
            return;

        if(q->isSet(QueueItem::FLAG_DIRECTORY_DOWNLOAD)) {
            dcassert(q->getSources().size() == 1);
            DirectoryItem::DirectoryPair dp = directories.equal_range(q->getSources()[0].getUser());
            for(DirectoryItem::DirectoryIter i = dp.first; i != dp.second; ++i) {
                delete i->second;
            }
            directories.erase(q->getSources()[0].getUser());
        }

        if(q->isRunning()) {
            for(DownloadList::iterator i = q->getDownloads().begin(); i != q->getDownloads().end(); ++i) {
                x.push_back((*i)->getUser());
            }
        } else if(!q->getTempTarget().empty() && q->getTempTarget() != q->getTarget()) {
            File::deleteFile(q->getTempTarget());
        }

        fire(QueueManagerListener::Removed(), q);

                if(!q->isFinished()) {
        userQueue.remove(q);
                }
        fileQueue.remove(q);

        setDirty();
    }

    for(UserList::iterator i = x.begin(); i != x.end(); ++i) {
        ConnectionManager::getInstance()->disconnect(*i, true);
    }
}

void QueueManager::removeSource(const string& aTarget, const UserPtr& aUser, int reason, bool removeConn /* = true */) throw() {
    bool isRunning = false;
    bool removeCompletely = false;
    {
        Lock l(cs);
        QueueItem* q = fileQueue.find(aTarget);
        if(!q)
            return;

        if(!q->isSource(aUser))
            return;

        if(q->isSet(QueueItem::FLAG_USER_LIST)) {
            removeCompletely = true;
            goto endCheck;
        }

        if(reason == QueueItem::Source::FLAG_NO_TREE) {
            q->getSource(aUser)->setFlag(reason);
            return;
        }

        if(q->isRunning() && userQueue.getRunning(aUser) == q) {
            isRunning = true;
            userQueue.removeDownload(q, aUser);
            fire(QueueManagerListener::StatusUpdated(), q);
        }

        if(!q->isFinished()) {
            userQueue.remove(q, aUser);
        }
        q->removeSource(aUser, reason);

        fire(QueueManagerListener::SourcesUpdated(), q);
        setDirty();
    }
endCheck:
    if(isRunning && removeConn) {
        ConnectionManager::getInstance()->disconnect(aUser, true);
    }
    if(removeCompletely) {
        remove(aTarget);
    }
}

int64_t QueueManager::getQueued(const UserPtr& aUser) const {
    Lock l(cs);
    return userQueue.getQueued(aUser);
}

void QueueManager::removeSource(const UserPtr& aUser, int reason) throw() {
        // @todo remove from finished items
    bool isRunning = false;
    string removeRunning;
    {
        Lock l(cs);
        QueueItem* qi = NULL;
        while( (qi = userQueue.getNext(aUser, QueueItem::PAUSED)) != NULL) {
            if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
                remove(qi->getTarget());
            } else {
                userQueue.remove(qi, aUser);
                qi->removeSource(aUser, reason);
                fire(QueueManagerListener::SourcesUpdated(), qi);
                setDirty();
            }
        }

        qi = userQueue.getRunning(aUser);
        if(qi) {
            if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
                removeRunning = qi->getTarget();
            } else {
                userQueue.removeDownload(qi, aUser);
                userQueue.remove(qi, aUser);
                isRunning = true;
                qi->removeSource(aUser, reason);
                fire(QueueManagerListener::StatusUpdated(), qi);
                fire(QueueManagerListener::SourcesUpdated(), qi);
                setDirty();
            }
        }
    }

    if(isRunning) {
        ConnectionManager::getInstance()->disconnect(aUser, true);
    }
    if(!removeRunning.empty()) {
        remove(removeRunning);
    }
}

void QueueManager::setPriority(const string& aTarget, QueueItem::Priority p) throw() {
        HintedUserList getConn;

    {
        Lock l(cs);

        QueueItem* q = fileQueue.find(aTarget);
                if( (q != NULL) && (q->getPriority() != p) && !q->isFinished() ) {
            if(q->getPriority() == QueueItem::PAUSED || p == QueueItem::HIGHEST) {
                // Problem, we have to request connections to all these users...
                                q->getOnlineUsers(getConn);
            }
            userQueue.setPriority(q, p);
            setDirty();
            fire(QueueManagerListener::StatusUpdated(), q);
        }
    }

        for(HintedUserList::iterator i = getConn.begin(); i != getConn.end(); ++i) {
                ConnectionManager::getInstance()->getDownloadConnection(*i);
    }
}

void QueueManager::saveQueue(bool force) throw() {
        if(!dirty && !force)
        return;

        std::vector<CID> cids;

    try {
                Lock l(cs);

        File ff(getQueueFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
        BufferedOutputStream<false> f(&ff);

        f.write(SimpleXML::utf8Header);
        f.write(LIT("<Downloads Version=\"" VERSIONSTRING "\">\r\n"));
        string tmp;
        string b32tmp;
        for(QueueItem::StringIter i = fileQueue.getQueue().begin(); i != fileQueue.getQueue().end(); ++i) {
            QueueItem* qi = i->second;
            if(!qi->isSet(QueueItem::FLAG_USER_LIST)) {
                f.write(LIT("\t<Download Target=\""));
                f.write(SimpleXML::escape(qi->getTarget(), tmp, true));
                f.write(LIT("\" Size=\""));
                f.write(Util::toString(qi->getSize()));
                f.write(LIT("\" Priority=\""));
                f.write(Util::toString((int)qi->getPriority()));
                f.write(LIT("\" Added=\""));
                f.write(Util::toString(qi->getAdded()));
                b32tmp.clear();
                f.write(LIT("\" TTH=\""));
                f.write(qi->getTTH().toBase32(b32tmp));
                if(!qi->getDone().empty()) {
                    f.write(LIT("\" TempTarget=\""));
                    f.write(SimpleXML::escape(qi->getTempTarget(), tmp, true));
                }
                f.write(LIT("\">\r\n"));

                for(QueueItem::SegmentSet::const_iterator i = qi->getDone().begin(); i != qi->getDone().end(); ++i) {
                    f.write(LIT("\t\t<Segment Start=\""));
                    f.write(Util::toString(i->getStart()));
                    f.write(LIT("\" Size=\""));
                    f.write(Util::toString(i->getSize()));
                    f.write(LIT("\"/>\r\n"));
                }

                for(QueueItem::SourceConstIter j = qi->sources.begin(); j != qi->sources.end(); ++j) {
                                        const CID& cid = j->getUser().user->getCID();
                                        const string& hint = j->getUser().hint;

                    f.write(LIT("\t\t<Source CID=\""));
                                        f.write(cid.toBase32());
                                        if(!hint.empty()) {
                                                f.write(LIT("\" Hub=\""));
                                                f.write(hint);
                                        }
                    f.write(LIT("\"/>\r\n"));

                                        cids.push_back(cid);
                }

                f.write(LIT("\t</Download>\r\n"));
            }
        }

        f.write("</Downloads>\r\n");
        f.flush();
        ff.close();
        File::deleteFile(getQueueFile());
        File::renameFile(getQueueFile() + ".tmp", getQueueFile());

        dirty = false;
    } catch(const FileException&) {
        // ...
    }
    // Put this here to avoid very many saves tries when disk is full...
    lastSave = GET_TICK();

        //NOTE: freedcpp, save user cids and nicks to Users.xml see dcplusplus revision 1771
        ClientManager* cm = ClientManager::getInstance();
//#ifdef _WIN32
//        std::for_each(cids.begin(), cids.end(), std::tr1::bind(&ClientManager::saveUser, cm, std::tr1::placeholders::_1));
//#else
        for (vector<CID>::const_iterator it = cids.begin(); it != cids.end(); ++it)
        {
                cm->saveUser(*it);
        }
//#endif
}

class QueueLoader : public SimpleXMLReader::CallBack {
public:
    QueueLoader() : cur(NULL), inDownloads(false) { }
    virtual ~QueueLoader() { }
    virtual void startTag(const string& name, StringPairList& attribs, bool simple);
    virtual void endTag(const string& name, const string& data);
private:
    string target;

    QueueItem* cur;
    bool inDownloads;
};

void QueueManager::loadQueue() throw() {
    try {
        QueueLoader l;
        Util::migrate(getQueueFile());

                File f(getQueueFile(), File::READ, File::OPEN);
                SimpleXMLReader(&l).parse(f);
        dirty = false;
    } catch(const Exception&) {
        // ...
    }
}

int QueueManager::countOnlineSources(const string& aTarget) {
    Lock l(cs);

    QueueItem* qi = fileQueue.find(aTarget);
    if(!qi)
        return 0;
    int onlineSources = 0;
    for(QueueItem::SourceConstIter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
                if(i->getUser().user->isOnline())
            onlineSources++;
    }
    return onlineSources;
}

static const string sDownload = "Download";
static const string sTempTarget = "TempTarget";
static const string sTarget = "Target";
static const string sSize = "Size";
static const string sDownloaded = "Downloaded";
static const string sPriority = "Priority";
static const string sSource = "Source";
static const string sNick = "Nick";
static const string sDirectory = "Directory";
static const string sAdded = "Added";
static const string sTTH = "TTH";
static const string sCID = "CID";
static const string sHubHint = "Hub";
static const string sSegment = "Segment";
static const string sStart = "Start";

void QueueLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
    QueueManager* qm = QueueManager::getInstance();
    if(!inDownloads && name == "Downloads") {
        inDownloads = true;
    } else if(inDownloads) {
        if(cur == NULL && name == sDownload) {
            int64_t size = Util::toInt64(getAttrib(attribs, sSize, 1));
            if(size == 0)
                return;
            try {
                const string& tgt = getAttrib(attribs, sTarget, 0);
                                // @todo do something better about existing files
                                target = QueueManager::checkTarget(tgt,  /*checkExistence*/ false);
                if(target.empty())
                    return;
            } catch(const Exception&) {
                return;
            }
            QueueItem::Priority p = (QueueItem::Priority)Util::toInt(getAttrib(attribs, sPriority, 3));
            time_t added = static_cast<time_t>(Util::toInt(getAttrib(attribs, sAdded, 4)));
            const string& tthRoot = getAttrib(attribs, sTTH, 5);
            if(tthRoot.empty())
                return;

            string tempTarget = getAttrib(attribs, sTempTarget, 5);
            int64_t downloaded = Util::toInt64(getAttrib(attribs, sDownloaded, 5));
            if (downloaded > size || downloaded < 0)
                downloaded = 0;

            if(added == 0)
                added = GET_TIME();

            QueueItem* qi = qm->fileQueue.find(target);

            if(qi == NULL) {
                                qi = qm->fileQueue.add(target, size, 0, p, tempTarget, added, TTHValue(tthRoot));
                if(downloaded > 0) {
                    qi->addSegment(Segment(0, downloaded));
                }
                qm->fire(QueueManagerListener::Added(), qi);
            }
            if(!simple)
                cur = qi;
        } else if(cur && name == sSegment) {
            int64_t start = Util::toInt64(getAttrib(attribs, sStart, 0));
            int64_t size = Util::toInt64(getAttrib(attribs, sSize, 1));

            if(size > 0 && start >= 0 && (start + size) <= cur->getSize()) {
                cur->addSegment(Segment(start, size));
            }
        } else if(cur && name == sSource) {
            const string& cid = getAttrib(attribs, sCID, 0);
            if(cid.length() != 39) {
                // Skip loading this source - sorry old users
                return;
            }
            UserPtr user = ClientManager::getInstance()->getUser(CID(cid));

            try {
                                const string& hubHint = getAttrib(attribs, sHubHint, 1);
                                HintedUser hintedUser(user, hubHint);
                                if(qm->addSource(cur, hintedUser, 0) && user->isOnline())
                                        ConnectionManager::getInstance()->getDownloadConnection(hintedUser);
            } catch(const Exception&) {
                return;
            }
        }
    }
}

void QueueLoader::endTag(const string& name, const string&) {
    if(inDownloads) {
                if(name == sDownload) {
            cur = NULL;
                } else if(name == "Downloads") {
            inDownloads = false;
    }
}
}

void QueueManager::noDeleteFileList(const string& path) {
        if(!BOOLSETTING(KEEP_LISTS)) {
                protectedFileLists.push_back(path);
        }
}

// SearchManagerListener
void QueueManager::on(SearchManagerListener::SR, const SearchResultPtr& sr) throw() {
    bool added = false;
    bool wantConnection = false;

    {
        Lock l(cs);
        QueueItem::List matches;

        fileQueue.find(matches, sr->getTTH());

        for(QueueItem::Iter i = matches.begin(); i != matches.end(); ++i) {
            QueueItem* qi = *i;

            // Size compare to avoid popular spoof
            if(qi->getSize() == sr->getSize() && !qi->isSource(sr->getUser())) {
                try {
                    if(!BOOLSETTING(AUTO_SEARCH_AUTO_MATCH))
                                                wantConnection = addSource(qi, HintedUser(sr->getUser(), sr->getHubURL()), 0);
                    added = true;
                } catch(const Exception&) {
                    // ...
                }
                break;
            }
        }
    }

    if(added && BOOLSETTING(AUTO_SEARCH_AUTO_MATCH)) {
        try {
                        addList(HintedUser(sr->getUser(), sr->getHubURL()), QueueItem::FLAG_MATCH_QUEUE);
        } catch(const Exception&) {
            // ...
        }
    }
    if(added && sr->getUser()->isOnline() && wantConnection) {
                ConnectionManager::getInstance()->getDownloadConnection(HintedUser(sr->getUser(), sr->getHubURL()));
    }

}

// ClientManagerListener
void QueueManager::on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw() {
    bool hasDown = false;
    {
        Lock l(cs);
        for(int i = 0; i < QueueItem::LAST; ++i) {
            QueueItem::UserListIter j = userQueue.getList(i).find(aUser);
            if(j != userQueue.getList(i).end()) {
                for(QueueItem::Iter m = j->second.begin(); m != j->second.end(); ++m)
                    fire(QueueManagerListener::StatusUpdated(), *m);
                if(i != QueueItem::PAUSED)
                    hasDown = true;
            }
        }
    }

    if(hasDown) {
                // the user just came on, so there's only 1 possible hub, no need for a hint
                ConnectionManager::getInstance()->getDownloadConnection(HintedUser(aUser, Util::emptyString));
    }
}

void QueueManager::on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw() {
    Lock l(cs);
    for(int i = 0; i < QueueItem::LAST; ++i) {
        QueueItem::UserListIter j = userQueue.getList(i).find(aUser);
        if(j != userQueue.getList(i).end()) {
            for(QueueItem::Iter m = j->second.begin(); m != j->second.end(); ++m)
                fire(QueueManagerListener::StatusUpdated(), *m);
        }
    }
}

void QueueManager::on(TimerManagerListener::Second, uint64_t aTick) throw() {
    if(dirty && ((lastSave + 10000) < aTick)) {
        saveQueue();
    }
}

bool QueueManager::handlePartialResult(const UserPtr& aUser, const string& hubHint, const TTHValue& tth, const QueueItem::PartialSource& partialSource, PartsInfo& outPartialInfo) {
        bool wantConnection = false;
        dcassert(outPartialInfo.empty());

        {
                Lock l(cs);

                // Locate target QueueItem in download queue
                QueueItem::List ql;
                fileQueue.find(ql, tth);

                if(ql.empty()){
                        dcdebug("Not found in download queue\n");
                        return false;
                }

                QueueItem::Ptr qi = ql[0];
                // don't add sources to finished files
                // this could happen when "Keep finished files in queue" is enabled
                if(qi->isFinished())
                        return false;

                // Check min size
                if(qi->getSize() < PARTIAL_SHARE_MIN_SIZE){
                        dcassert(0);
                        return false;
                }

                // Get my parts info
                int64_t blockSize = HashManager::getInstance()->getBlockSize(qi->getTTH());
                if(blockSize == 0)
                        blockSize = qi->getSize();
                qi->getPartialInfo(outPartialInfo, blockSize);

                // Any parts for me?
                wantConnection = qi->isNeededPart(partialSource.getPartialInfo(), blockSize);

                // If this user isn't a source and has no parts needed, ignore it
                QueueItem::SourceIter si = qi->getSource(aUser);
                if(si == qi->getSources().end()){
                        si = qi->getBadSource(aUser);

                        if(si != qi->getBadSources().end() && (si->isSet(QueueItem::Source::FLAG_BAD_TREE) || si->isSet(QueueItem::Source::FLAG_TTH_INCONSISTENCY)))
                                return false;

                        if(!wantConnection){
                                if(si == qi->getBadSources().end())
                                        return false;
                        }else{
                                // add this user as partial file sharing source
                                qi->addSource(HintedUser(aUser,""));
                                si = qi->getSource(aUser);
                                si->setFlag(QueueItem::Source::FLAG_PARTIAL);

                                QueueItem::PartialSource* ps = new QueueItem::PartialSource(partialSource.getMyNick(),
                                        partialSource.getHubIpPort(), partialSource.getIp(), partialSource.getUdpPort());
                                si->setPartialSource(ps);

                                userQueue.add(qi, aUser);
                                dcassert(si != qi->getSources().end());
                                fire(QueueManagerListener::SourcesUpdated(), qi);
                        }
                }

                // Update source's parts info
                if(si->getPartialSource()) {
                        si->getPartialSource()->setPartialInfo(partialSource.getPartialInfo());
                }
        }

        // Connect to this user
        if(wantConnection)
                ConnectionManager::getInstance()->getDownloadConnection(HintedUser(aUser, hubHint));

        return true;
}

bool QueueManager::handlePartialSearch(const TTHValue& tth, PartsInfo& _outPartsInfo) {
        {
                Lock l(cs);

                // Locate target QueueItem in download queue
                QueueItem::List ql;
                fileQueue.find(ql, tth);

                if(ql.empty()){
                        return false;
                }

                QueueItem::Ptr qi = ql[0];
                if(qi->getSize() < PARTIAL_SHARE_MIN_SIZE){
                        return false;
                }

                int64_t blockSize = HashManager::getInstance()->getBlockSize(qi->getTTH());
                if(blockSize == 0)
                        blockSize = qi->getSize();
                qi->getPartialInfo(_outPartsInfo, blockSize);
        }

        return !_outPartsInfo.empty();
}

void QueueManager::checkSfv(QueueItem* qi, Download* d) {
	SFVReader sfv(qi->getTarget());

	if(sfv.hasCRC()) {
		bool crcMatch = false;
		try {
			crcMatch = (calcCrc32(qi->getTempTarget()) == sfv.getCRC());
		} catch(const FileException& ) {
			// Couldn't read the file to get the CRC(!!!)
		}

		if(!crcMatch) {
			/// @todo There is a slight chance that something happens with a file while it's being saved to disk
			/// maybe calculate tth along with crc and if tth is ok and crc is not flag the file as bad at once
			/// if tth mismatches (possible disk error) then repair / redownload the file

			File::deleteFile(qi->getTempTarget());
			qi->resetDownloaded();
			dcdebug("QueueManager: CRC32 mismatch for %s\n", qi->getTarget().c_str());
			LogManager::getInstance()->message(_("CRC32 inconsistency (SFV-Check)") + ' ' + Util::addBrackets(qi->getTarget()));

			setPriority(qi->getTarget(), QueueItem::PAUSED);

			QueueItem::SourceList sources = qi->getSources();
			for(QueueItem::SourceConstIter i = sources.begin(); i != sources.end(); ++i) {
				removeSource(qi->getTarget(), i->getUser(), QueueItem::Source::FLAG_CRC_FAILED, false);
			}

			fire(QueueManagerListener::CRCFailed(), d, _("CRC32 inconsistency (SFV-Check)"));
			return;
		}

		dcdebug("QueueManager: CRC32 match for %s\n", qi->getTarget().c_str());
		fire(QueueManagerListener::CRCChecked(), d);
	}
}

uint32_t QueueManager::calcCrc32(const string& file) throw(FileException) {
	File ff(file, File::READ, File::OPEN);
	CalcInputStream<CRC32Filter, false> f(&ff);

	const size_t BUF_SIZE = 1024*1024;
	boost::scoped_array<uint8_t> b(new uint8_t[BUF_SIZE]);
	size_t n = BUF_SIZE;
	while(f.read(&b[0], n) > 0)
		;		// Keep on looping...

	return f.getFilter().getValue();
}

// compare nextQueryTime, get the oldest ones
void QueueManager::FileQueue::findPFSSources(PFSSourceList& sl)
{
	typedef multimap<time_t, pair<QueueItem::SourceConstIter, const QueueItem*> > Buffer;
	Buffer buffer;
	uint64_t now = GET_TICK();

	for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i) {
		const QueueItem* q = i->second;

		if(q->getSize() < PARTIAL_SHARE_MIN_SIZE) continue;

		const QueueItem::SourceList& sources = q->getSources();
		const QueueItem::SourceList& badSources = q->getBadSources();

		for(QueueItem::SourceConstIter j = sources.begin(); j != sources.end(); ++j) {
			if(	(*j).isSet(QueueItem::Source::FLAG_PARTIAL) && (*j).getPartialSource()->getNextQueryTime() <= now &&
				(*j).getPartialSource()->getPendingQueryCount() < 10 && (*j).getPartialSource()->getUdpPort() > 0)
			{
				buffer.insert(make_pair((*j).getPartialSource()->getNextQueryTime(), make_pair(j, q)));
			}
		}

		for(QueueItem::SourceConstIter j = badSources.begin(); j != badSources.end(); ++j) {
			if(	(*j).isSet(QueueItem::Source::FLAG_TTH_INCONSISTENCY) == false && (*j).isSet(QueueItem::Source::FLAG_PARTIAL) &&
				(*j).getPartialSource()->getNextQueryTime() <= now && (*j).getPartialSource()->getPendingQueryCount() < 10 &&
				(*j).getPartialSource()->getUdpPort() > 0)
			{
				buffer.insert(make_pair((*j).getPartialSource()->getNextQueryTime(), make_pair(j, q)));
			}
		}
	}

	// copy to results
	dcassert(sl.empty());
	const int32_t maxElements = 10;
	sl.reserve(maxElements);
	for(Buffer::iterator i = buffer.begin(); i != buffer.end() && sl.size() < maxElements; i++){
		sl.push_back(i->second);
	}
}

TTHValue* QueueManager::FileQueue::findPFSPubTTH()
{
    uint64_t now = GET_TICK();
    QueueItem::Ptr cand = NULL;

    for(QueueItem::StringIter i = queue.begin(); i != queue.end(); i++)
    {
        QueueItem::Ptr qi = i->second;
        if(qi && qi->getSize() >= PARTIAL_SHARE_MIN_SIZE && now >= qi->getNextPublishingTime() && qi->getPriority() > QueueItem::PAUSED)
        {
            if(cand == NULL || cand->getNextPublishingTime() > qi->getNextPublishingTime() || (cand->getNextPublishingTime() == qi->getNextPublishingTime() && cand->getPriority() < qi->getPriority()) )
            {
                if(qi->getDownloadedBytes() > HashManager::getInstance()->getBlockSize(qi->getTTH()))
                    cand = qi;
            }
        }
    }
    if(cand)
    {
        #ifdef DHT
        cand->setNextPublishingTime(now + PFS_REPUBLISH_TIME);          // one hour
        #else
        cand->setNextPublishingTime(now + 1*60*60*1000);          // one hour
        #endif
        return new TTHValue(cand->getTTH());
    }
     return NULL;
}

} // namespace dcpp
