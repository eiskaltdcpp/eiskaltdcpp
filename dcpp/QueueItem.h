/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#pragma once

#include "User.h"
#include "FastAlloc.h"
#include "MerkleTree.h"
#include "Flags.h"
#include "forward.h"
#include "Segment.h"

namespace dcpp {

class QueueManager;

class QueueItem : public Flags, public FastAlloc<QueueItem> {
public:
    typedef QueueItem* Ptr;
    typedef deque<Ptr> List;
    typedef List::iterator Iter;
    typedef unordered_map<string*, Ptr, noCaseStringHash, noCaseStringEq> StringMap;
    typedef StringMap::iterator StringIter;
    typedef unordered_map<UserPtr, Ptr, User::Hash> UserMap;
    typedef UserMap::iterator UserIter;
    typedef unordered_map<UserPtr, List, User::Hash> UserListMap;
    typedef UserListMap::iterator UserListIter;

    enum Priority {
        DEFAULT = -1,
        PAUSED = 0,
        LOWEST,
        LOW,
        NORMAL,
        HIGH,
        HIGHEST,
        LAST
    };

    enum FileFlags {
        /** Normal download, no flags set */
        FLAG_NORMAL = 0x00,
        /** This is a user file listing download */
        FLAG_USER_LIST = 0x02,
        /** The file list is downloaded to use for directory download (used with USER_LIST) */
        FLAG_DIRECTORY_DOWNLOAD = 0x04,
        /** The file is downloaded to be viewed in the gui */
        FLAG_CLIENT_VIEW = 0x08,
        /** Flag to indicate that file should be viewed as a text file */
        FLAG_TEXT = 0x20,
        /** Match the queue against this list */
        FLAG_MATCH_QUEUE = 0x80,
        /** The file list downloaded was actually an .xml.bz2 list */
        FLAG_XML_BZLIST = 0x100,
        /** Only download a part of the file list */
        FLAG_PARTIAL_LIST = 0x200
    };

    /**
     * Source parts info
     * Meaningful only when Source::FLAG_PARTIAL is set
     */
    class PartialSource : public FastAlloc<PartialSource>, public intrusive_ptr_base<PartialSource> {
    public:
        PartialSource(const string& aMyNick, const string& aHubIpPort, const string& aIp, uint16_t udp) :
          myNick(aMyNick), hubIpPort(aHubIpPort), ip(aIp), udpPort(udp), nextQueryTime(0), pendingQueryCount(0) { }

        ~PartialSource() { }

        typedef boost::intrusive_ptr<PartialSource> Ptr;

        GETSET(PartsInfo, partialInfo, PartialInfo);
        GETSET(string, myNick, MyNick);                 // for NMDC support only
        GETSET(string, hubIpPort, HubIpPort);
        GETSET(string, ip, Ip);
        GETSET(uint16_t, udpPort, UdpPort);
        GETSET(uint64_t, nextQueryTime, NextQueryTime);
        GETSET(uint8_t, pendingQueryCount, PendingQueryCount);
    };


    class Source : public Flags {
    public:
        enum {
            FLAG_NONE = 0x00,
            FLAG_FILE_NOT_AVAILABLE = 0x01,
            FLAG_PASSIVE = 0x02,
            FLAG_REMOVED = 0x04,
            FLAG_CRC_FAILED = 0x08,
            FLAG_CRC_WARN = 0x10,
            FLAG_NO_TTHF = 0x20,
            FLAG_BAD_TREE = 0x40,
            FLAG_NO_TREE = 0x80,
            FLAG_SLOW_SOURCE = 0x100,
            FLAG_PARTIAL    = 0x200,
            FLAG_NO_NEED_PARTS      = 0x250,
            FLAG_TTH_INCONSISTENCY  = 0x300,
            FLAG_UNTRUSTED = 0x400,
            FLAG_MASK = FLAG_FILE_NOT_AVAILABLE
                | FLAG_PASSIVE | FLAG_REMOVED | FLAG_CRC_FAILED | FLAG_CRC_WARN
                | FLAG_BAD_TREE | FLAG_NO_TREE | FLAG_SLOW_SOURCE | FLAG_TTH_INCONSISTENCY | FLAG_UNTRUSTED
        };

        Source(const HintedUser& aUser) : user(aUser), partialSource(NULL) { }
        Source(const Source& aSource) : Flags(aSource), user(aSource.user), partialSource(aSource.partialSource) { }

        bool operator==(const UserPtr& aUser) const { return user == aUser; }
        PartialSource::Ptr& getPartialSource() { return partialSource; }

        GETSET(HintedUser, user, User);
        GETSET(PartialSource::Ptr, partialSource, PartialSource);
    };

    typedef std::vector<Source> SourceList;
    typedef SourceList::iterator SourceIter;
    typedef SourceList::const_iterator SourceConstIter;

    typedef set<Segment> SegmentSet;
    typedef SegmentSet::iterator SegmentIter;
    typedef SegmentSet::const_iterator SegmentConstIter;

    QueueItem(const string& aTarget, int64_t aSize, Priority aPriority, int aFlag,
        time_t aAdded, const TTHValue& tth) :
        Flags(aFlag), target(aTarget), size(aSize),
        priority(aPriority), added(aAdded), tthRoot(tth), nextPublishingTime(0)
    { }

    QueueItem(const QueueItem& rhs) :
        Flags(rhs), done(rhs.done), downloads(rhs.downloads), target(rhs.target),
        size(rhs.size), priority(rhs.priority), added(rhs.added), tthRoot(rhs.tthRoot),
        nextPublishingTime(rhs.nextPublishingTime), sources(rhs.sources), badSources(rhs.badSources),
        tempTarget(rhs.tempTarget)

    { }

    virtual ~QueueItem() { }

    int countOnlineUsers() const;
    bool hasOnlineUsers() const { return countOnlineUsers() > 0; }
    void getOnlineUsers(HintedUserList& l) const;

    SourceList& getSources() { return sources; }
    const SourceList& getSources() const { return sources; }
    SourceList& getBadSources() { return badSources; }
    const SourceList& getBadSources() const { return badSources; }

    string getTargetFileName() const { return Util::getFileName(getTarget()); }

    SourceIter getSource(const UserPtr& aUser) { return find(sources.begin(), sources.end(), aUser); }
    SourceIter getBadSource(const UserPtr& aUser) { return find(badSources.begin(), badSources.end(), aUser); }
    SourceConstIter getSource(const UserPtr& aUser) const { return find(sources.begin(), sources.end(), aUser); }
    SourceConstIter getBadSource(const UserPtr& aUser) const { return find(badSources.begin(), badSources.end(), aUser); }

    bool isSource(const UserPtr& aUser) const { return getSource(aUser) != sources.end(); }
    bool isBadSource(const UserPtr& aUser) const { return getBadSource(aUser) != badSources.end(); }
    bool isBadSourceExcept(const UserPtr& aUser, Flags::MaskType exceptions) const {
        SourceConstIter i = getBadSource(aUser);
        if(i != badSources.end())
            return i->isAnySet(exceptions^Source::FLAG_MASK);
        return false;
    }

    int64_t getDownloadedBytes() const;
    double getDownloadedFraction() const { return static_cast<double>(getDownloadedBytes()) / getSize(); }

    DownloadList& getDownloads() { return downloads; }

    bool isChunkDownloaded(int64_t startPos, int64_t& len) const {
        if(len <= 0) return false;

        for(SegmentSet::const_iterator i = done.begin(); i != done.end(); ++i) {
            int64_t first  = (*i).getStart();
            int64_t second = (*i).getEnd();

            if(first <= startPos && startPos < second){
                len = min(len, second - startPos);
                return true;
            }
        }

        return false;
    }

    /** Next segment that is not done and not being downloaded, zero-sized segment returned if there is none is found */
    Segment getNextSegment(int64_t blockSize, int64_t wantedSize, int64_t lastSpeed, const PartialSource::Ptr partialSource) const;
    /**
     * Is specified parts needed by this download?
     */
    bool isNeededPart(const PartsInfo& partsInfo, int64_t blockSize);
    /**
     * Get shared parts info, max 255 parts range pairs
     */
    void getPartialInfo(PartsInfo& partialInfo, int64_t blockSize) const;


    void addSegment(const Segment& segment);
    void resetDownloaded() { done.clear(); }

    bool isFinished() const {
        return done.size() == 1 && *done.begin() == Segment(0, getSize());
    }

    bool isRunning() const {
        return !isWaiting();
    }
    bool isWaiting() const {
        return downloads.empty();
    }

    string getListName() const {
        dcassert(isSet(QueueItem::FLAG_USER_LIST));
        if(isSet(QueueItem::FLAG_XML_BZLIST)) {
            return getTarget() + ".xml.bz2";
        } else {
            return getTarget() + ".xml";
        }
    }

    const string& getTempTarget();
    void setTempTarget(const string& aTempTarget) { tempTarget = aTempTarget; }

    GETSET(SegmentSet, done, Done);
    GETSET(DownloadList, downloads, Downloads);
    GETSET(string, target, Target);
    GETSET(int64_t, size, Size);
    GETSET(Priority, priority, Priority);
    GETSET(time_t, added, Added);
    GETSET(TTHValue, tthRoot, TTH);
    GETSET(uint64_t, nextPublishingTime, NextPublishingTime);
private:
    QueueItem& operator=(const QueueItem&);

    friend class QueueManager;
    SourceList sources;
    SourceList badSources;
    string tempTarget;

    void addSource(const HintedUser& aUser);
    void removeSource(const UserPtr& aUser, int reason);
};

} // namespace dcpp
