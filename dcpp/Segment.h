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

namespace dcpp {

// minimum file size to be PFS : 20M
#define PARTIAL_SHARE_MIN_SIZE 20971520

class Segment {
public:
    Segment() : start(0), size(-1), overlapped(false) { }
    Segment(int64_t start_, int64_t size_, bool overlapped_ = false) : start(start_), size(size_), overlapped(overlapped_) { }

    int64_t getStart() const { return start; }
    int64_t getSize() const { return size; }
    int64_t getEnd() const { return getStart() + getSize(); }

    void setSize(int64_t size_) { size = size_; }

    bool overlaps(const Segment& rhs) const {
        int64_t end = getEnd();
        int64_t rend = rhs.getEnd();
        return getStart() < rend && rhs.getStart() < end;
    }

    void trim(const Segment& rhs) {
        if(!overlaps(rhs)) {
            return;
        }

        if(rhs.getStart() < start) {
            int64_t rend = rhs.getEnd();
            if(rend > getEnd()) {
                start = size = 0;
            } else {
                size -= rend - start;
                start = rend;
            }
            return;
        }
        size = rhs.getStart() - start;
    }

    bool contains(const Segment& rhs) const {
        return getStart() <= rhs.getStart() && getEnd() == rhs.getEnd();
    }

    bool operator==(const Segment& rhs) const {
        return getStart() == rhs.getStart() && getSize() == rhs.getSize();
    }
    bool operator<(const Segment& rhs) const {
        return (getStart() < rhs.getStart()) || (getStart() == rhs.getStart() && getSize() < rhs.getSize());
    }
private:
    int64_t start;
    int64_t size;
    GETSET(bool, overlapped, Overlapped);
};

} //dcpp namespace
