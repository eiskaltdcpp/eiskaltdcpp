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

#ifndef DCPLUSPLUS_DCPP_FINISHED_ITEM_H
#define DCPLUSPLUS_DCPP_FINISHED_ITEM_H

#include "forward.h"

#include "Pointer.h"
#include "Util.h"

namespace dcpp {

class FinishedItemBase : boost::noncopyable {
public:
    explicit FinishedItemBase(
        int64_t transferred_,
        int64_t milliSeconds_,
        time_t time_
        );

    void update(
        int64_t transferred_,
        int64_t milliSeconds_,
        time_t time_
        );

    int64_t getAverageSpeed() const;

    GETSET(int64_t, transferred, Transferred);
    GETSET(int64_t, milliSeconds, MilliSeconds);
    GETSET(time_t, time, Time);
};

class FinishedFileItem : public FinishedItemBase, public intrusive_ptr_base<FinishedFileItem> {
public:
    explicit FinishedFileItem(
        int64_t transferred_,
        int64_t milliSeconds_,
        time_t time_,
        int64_t fileSize_,
        bool crc32Checked_,
                const HintedUser& user
        );

    void update(
        int64_t transferred_,
        int64_t milliSeconds_,
        time_t time_,
        bool crc32Checked_,
                const HintedUser& user
        );

    double getTransferredPercentage() const;
    bool isFull() const;

        GETSET(HintedUserList, users, Users);
    GETSET(int64_t, fileSize, FileSize);
    GETSET(bool, crc32Checked, Crc32Checked);
};

class FinishedUserItem : public FinishedItemBase, public intrusive_ptr_base<FinishedUserItem> {
public:
    explicit FinishedUserItem(
        int64_t transferred_,
        int64_t milliSeconds_,
        time_t time_,
        const string& file
        );

    void update(
        int64_t transferred_,
        int64_t milliSeconds_,
        time_t time_,
        const string& file
        );

    GETSET(StringList, files, Files);
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_FINISHED_ITEM_H)
