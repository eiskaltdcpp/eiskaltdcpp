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

#include "FinishedItem.h"

#include "User.h"

namespace dcpp {

FinishedItemBase::FinishedItemBase(
    int64_t transferred_,
    int64_t milliSeconds_,
    time_t time_
    ) :
transferred(transferred_),
milliSeconds(milliSeconds_),
time(time_)
{
}

void FinishedItemBase::update(
    int64_t transferred_,
    int64_t milliSeconds_,
    time_t time_
    )
{
    transferred += transferred_;
    milliSeconds += milliSeconds_;
    time = time_;
}

int64_t FinishedItemBase::getAverageSpeed() const {
    return milliSeconds > 0 ? (transferred * ((int64_t)1000) / milliSeconds) : 0;
}

FinishedFileItem::FinishedFileItem(
    int64_t transferred_,
    int64_t milliSeconds_,
    time_t time_,
    int64_t fileSize_,
    bool crc32Checked_,
        const HintedUser& user
    ) :
FinishedItemBase(transferred_, milliSeconds_, time_),
fileSize(fileSize_),
crc32Checked(crc32Checked_)
{
    users.push_back(user);
}

void FinishedFileItem::update(
    int64_t transferred_,
    int64_t milliSeconds_,
    time_t time_,
    bool crc32Checked_,
        const HintedUser& user
    )
{
    FinishedItemBase::update(transferred_, milliSeconds_, time_);

    if(crc32Checked_)
        crc32Checked = true;

        HintedUserList::iterator i = find(users.begin(), users.end(), user);
        if(i == users.end())
        users.push_back(user);
        else
                *i = user; // update, the hint might have changed
}

double FinishedFileItem::getTransferredPercentage() const {
    return fileSize > 0 ? (getTransferred() * 100. / fileSize) : 0;
}

bool FinishedFileItem::isFull() const {
    return getTransferred() >= fileSize;
}

FinishedUserItem::FinishedUserItem(
    int64_t transferred_,
    int64_t milliSeconds_,
    time_t time_,
    const string& file
    ) :
FinishedItemBase(transferred_, milliSeconds_, time_)
{
    files.push_back(file);
}

void FinishedUserItem::update(
    int64_t transferred_,
    int64_t milliSeconds_,
    time_t time_,
    const string& file
    )
{
    FinishedItemBase::update(transferred_, milliSeconds_, time_);
    if(find(files.begin(), files.end(), file) == files.end())
        files.push_back(file);
}

} // namespace dcpp
