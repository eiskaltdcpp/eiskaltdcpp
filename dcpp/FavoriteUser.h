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

#ifndef DCPLUSPLUS_DCPP_FAVORITE_USER_H
#define DCPLUSPLUS_DCPP_FAVORITE_USER_H

#include "FastAlloc.h"
#include "User.h"
#include "CID.h"

namespace dcpp {

class FavoriteUser : public Flags {
public:
    FavoriteUser(const UserPtr& user_, const string& nick_, const string& hubUrl_) : user(user_), nick(nick_), url(hubUrl_), lastSeen(0) { }

    enum Flags {
        FLAG_GRANTSLOT = 1 << 0
    };

    UserPtr& getUser() { return user; }

    void update(const OnlineUser& info);

    GETSET(UserPtr, user, User);
    GETSET(string, nick, Nick);
    GETSET(string, url, Url);
    GETSET(time_t, lastSeen, LastSeen);
    GETSET(string, description, Description);
};

} // namespace dcpp

#endif // !defined(FAVORITE_USER_H)
