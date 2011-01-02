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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "QueueItem.h"
#include "HashManager.h"
#include "Download.h"
#include "File.h"
#include "Util.h"

namespace dcpp {

namespace {
    const string TEMP_EXTENSION = ".dctmp";

    string getTempName(const string& aFileName, const TTHValue& aRoot) {
        string tmp(aFileName);
        tmp += "." + aRoot.toBase32();
        tmp += TEMP_EXTENSION;
        return tmp;
    }
}

int QueueItem::countOnlineUsers() const {
    int n = 0;
        for(SourceConstIter i = sources.begin(), iend = sources.end(); i != iend; ++i) {
                if(i->getUser().user->isOnline())
            n++;
    }
    return n;
}

void QueueItem::getOnlineUsers(HintedUserList& l) const {
        for(SourceConstIter i = sources.begin(), iend = sources.end(); i != iend; ++i)
                if(i->getUser().user->isOnline())
                        l.push_back(i->getUser());
}

void QueueItem::addSource(const HintedUser& aUser) {
        dcassert(!isSource(aUser.user));
    SourceIter i = getBadSource(aUser);
    if(i != badSources.end()) {
        sources.push_back(*i);
        badSources.erase(i);
    } else {
        sources.push_back(Source(aUser));
    }
}

void QueueItem::removeSource(const UserPtr& aUser, int reason) {
    SourceIter i = getSource(aUser);
    dcassert(i != sources.end());
    i->setFlag(reason);
    badSources.push_back(*i);
    sources.erase(i);
}

const string& QueueItem::getTempTarget() {
    if(!isSet(QueueItem::FLAG_USER_LIST) && tempTarget.empty()) {
        if(!SETTING(TEMP_DOWNLOAD_DIRECTORY).empty() && (File::getSize(getTarget()) == -1)) {
            string tmp;
#ifdef _WIN32
            dcpp::StringMap sm;
            if(target.length() >= 3 && target[1] == ':' && target[2] == '\\')
                sm["targetdrive"] = target.substr(0, 3);
            else
                sm["targetdrive"] = Util::getPath(Util::PATH_USER_LOCAL).substr(0, 3);
            if (SETTING(NO_USE_TEMP_DIR))
                tmp = Util::formatParams(target, sm, false) + getTempName("", getTTH());
            else
                tmp = Util::formatParams(SETTING(TEMP_DOWNLOAD_DIRECTORY), sm, false) + getTempName(getTargetFileName(), getTTH());
#else //_WIN32
            if (SETTING(NO_USE_TEMP_DIR))
                tmp = target + getTempName("", getTTH());
            else
                tmp = SETTING(TEMP_DOWNLOAD_DIRECTORY) + getTempName(getTargetFileName(), getTTH());
#endif //_WIN32
            int len = Util::getFileName(tmp).size()+1;
            if (len >= 255){
                string tmp1 = tmp.erase(tmp.find(".dctmp")-33,tmp.find(".dctmp")+5) + ".dctmp";
                tmp = tmp1;
            }
            len = Util::getFileName(tmp).size()+1;
            if (len >= 255) {
                char tmp3[206]; int i=0;
                string tmp2 = tmp.erase(tmp.find(".dctmp")-7,tmp.find(".dctmp")+5);
                memcpy (tmp3, Util::getFileName(tmp2).c_str(), 206);
                tmp = Util::getFilePath(tmp) + string(tmp3) + "~." + getTTH().toBase32() + ".dctmp";
                delete tmp3;
            }
            setTempTarget(tmp);
        }
    }
    return tempTarget;
}

Segment QueueItem::getNextSegment(int64_t blockSize, int64_t wantedSize, int64_t lastSpeed, const PartialSource::Ptr partialSource) const {
    if(getSize() == -1 || blockSize == 0) {
        return Segment(0, -1);
    }

    if(!BOOLSETTING(SEGMENTED_DL)) {
        if(!downloads.empty()) {
            return Segment(0, 0);
        }

        int64_t start = 0;
        int64_t end = getSize();

        if(!done.empty()) {
            const Segment& first = *done.begin();

            if(first.getStart() > 0) {
                                end = Util::roundUp(first.getStart(), blockSize);
            } else {
                                start = Util::roundDown(first.getEnd(), blockSize);

                if(done.size() > 1) {
                    const Segment& second = *(++done.begin());
                                        end = Util::roundUp(second.getStart(), blockSize);
                }
            }
        }

        return Segment(start, std::min(getSize(), end) - start);
    }


        /* added for PFS */
        vector<int64_t> posArray;
        vector<Segment> neededParts;

        if(partialSource) {
                posArray.reserve(partialSource->getPartialInfo().size());

                // Convert block index to file position
                for(PartsInfo::const_iterator i = partialSource->getPartialInfo().begin(); i != partialSource->getPartialInfo().end(); i++)
                        posArray.push_back(min(getSize(), (int64_t)(*i) * blockSize));
        }

        /***************************/


    double donePart = static_cast<double>(getDownloadedBytes()) / getSize();

    // We want smaller blocks at the end of the transfer, squaring gives a nice curve...
    int64_t targetSize = wantedSize * std::max(0.25, (1. - (donePart * donePart)));

    if(targetSize > blockSize) {
        // Round off to nearest block size
                targetSize = Util::roundDown(targetSize, blockSize);
    } else {
        targetSize = blockSize;
    }

    int64_t start = 0;
    int64_t curSize = targetSize;

    while(start < getSize()) {
        int64_t end = std::min(getSize(), start + curSize);
        Segment block(start, end - start);
        bool overlaps = false;
        for(SegmentConstIter i = done.begin(); !overlaps && i != done.end(); ++i) {
            if(curSize <= blockSize) {
                int64_t dstart = i->getStart();
                int64_t dend = i->getEnd();
                // We accept partial overlaps, only consider the block done if it is fully consumed by the done block
                if(dstart <= start && dend >= end) {
                    overlaps = true;
                }
            } else {
                overlaps = block.overlaps(*i);
            }
        }

        for(DownloadList::const_iterator i = downloads.begin(); !overlaps && i !=downloads.end(); ++i) {
            overlaps = block.overlaps((*i)->getSegment());
        }

        if(!overlaps) {
            if(partialSource) {
                // store all chunks we could need
                for(vector<int64_t>::const_iterator j = posArray.begin(); j < posArray.end(); j += 2){
                        if( (*j <= start && start < *(j+1)) || (start <= *j && *j < end) ) {
                            int64_t b = max(start, *j);
                            int64_t e = min(end, *(j+1));

                        // segment must be blockSize aligned
                        dcassert(b % blockSize == 0);
                        dcassert(e % blockSize == 0 || e == getSize());

                        bool merged = false;
                        if(!neededParts.empty())
                        {
                            Segment& prev = neededParts.back();
                            if(b == prev.getEnd() && e > prev.getEnd())
                            {
                                 prev.setSize(prev.getSize() + (e - b));
                                 merged = true;
                            }
                        }

                            if(!merged)
                                neededParts.push_back(Segment(b, e - b));
                        }
                }
            } else {
                return block;
            }
        }

        if(!partialSource && curSize > blockSize) {
            curSize -= blockSize;
        } else {
            start = end;
            curSize = targetSize;
        }
    }

        if(!neededParts.empty()) {
                // select random chunk for PFS
                dcdebug("Found partial chunks: %d\n", neededParts.size());

                Segment& selected = neededParts[Util::rand(0, neededParts.size())];
                selected.setSize(std::min(selected.getSize(), targetSize));     // request only wanted size

                return selected;
        }

        if(partialSource == NULL && BOOLSETTING(OVERLAP_CHUNKS) && lastSpeed > 0) {
                // overlap slow running chunk

                for(DownloadList::const_iterator i = downloads.begin(); i != downloads.end(); ++i) {
                        Download* d = *i;

                        // current chunk mustn't be already overlapped
                        if(d->getOverlapped())
                                continue;

                        // current chunk must be running at least for 2 seconds
                        if(d->getStart() == 0 || GET_TIME() - d->getStart() < 2000)
                                continue;

                        // current chunk mustn't be finished in next 10 seconds
                        if(d->getSecondsLeft() < 10)
                                continue;

                        // overlap current chunk at last block boundary
                        int64_t pos = d->getPos() - (d->getPos() % blockSize);
                        int64_t size = d->getSize() - pos;

                        // new user should finish this chunk more than 2x faster
                        int64_t newChunkLeft = size / lastSpeed;
                        if(2 * newChunkLeft < d->getSecondsLeft()) {
                                dcdebug("Overlapping... old user: %I64d s, new user: %I64d s\n", d->getSecondsLeft(), newChunkLeft);
                                return Segment(d->getStartPos() + pos, size/*, true*/);//TODO bool
                        }
                }
        }


    return Segment(0, 0);
}

int64_t QueueItem::getDownloadedBytes() const {
    int64_t total = 0;
    for(SegmentSet::const_iterator i = done.begin(); i != done.end(); ++i) {
        total += i->getSize();
    }
    return total;
}

void QueueItem::addSegment(const Segment& segment) {
    done.insert(segment);

    // Consolidate segments
    if(done.size() == 1)
        return;

    for(SegmentSet::iterator i = ++done.begin() ; i != done.end(); ) {
        SegmentSet::iterator prev = i;
        prev--;
        if(prev->getEnd() >= i->getStart()) {
            Segment big(prev->getStart(), i->getEnd() - prev->getStart());
            done.erase(prev);
            done.erase(i++);
            done.insert(big);
        } else {
            ++i;
        }
    }
}
//Partial
bool QueueItem::isNeededPart(const PartsInfo& partsInfo, int64_t blockSize)
{
        dcassert(partsInfo.size() % 2 == 0);

        SegmentConstIter i  = done.begin();
        for(PartsInfo::const_iterator j = partsInfo.begin(); j != partsInfo.end(); j+=2){
                while(i != done.end() && (*i).getEnd() <= (*j) * blockSize)
                        i++;

                if(i == done.end() || !((*i).getStart() <= (*j) * blockSize && (*i).getEnd() >= (*(j+1)) * blockSize))
                        return true;
        }

        return false;

}

void QueueItem::getPartialInfo(PartsInfo& partialInfo, int64_t blockSize) const {
        size_t maxSize = min(done.size() * 2, (size_t)510);
        partialInfo.reserve(maxSize);

        SegmentConstIter i = done.begin();
        for(; i != done.end() && partialInfo.size() < maxSize; i++) {

                uint16_t s = (uint16_t)((*i).getStart() / blockSize);
                uint16_t e = (uint16_t)(((*i).getEnd() - 1) / blockSize + 1);

                partialInfo.push_back(s);
                partialInfo.push_back(e);
        }
}

}
