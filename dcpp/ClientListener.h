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

#ifndef CLIENTLISTENER_H_
#define CLIENTLISTENER_H_

#include "typedefs.h"

namespace dcpp {

class ClientListener
{
public:
    virtual ~ClientListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> Connecting;
    typedef X<1> Connected;
    typedef X<3> UserUpdated;
    typedef X<4> UsersUpdated;
    typedef X<5> UserRemoved;
    typedef X<6> Redirect;
    typedef X<7> Failed;
    typedef X<8> GetPassword;
    typedef X<9> HubUpdated;
    typedef X<11> Message;
    typedef X<12> StatusMessage;
    typedef X<13> HubUserCommand;
    typedef X<14> HubFull;
    typedef X<15> NickTaken;
    typedef X<16> SearchFlood;
    typedef X<17> NmdcSearch;
    typedef X<18> AdcSearch;

    enum StatusFlags {
        FLAG_NORMAL = 0x00,
        FLAG_IS_SPAM = 0x01
    };

    virtual void on(Connecting, Client*) noexcept { }
    virtual void on(Connected, Client*) noexcept { }
    virtual void on(UserUpdated, Client*, const OnlineUser&) noexcept { }
    virtual void on(UsersUpdated, Client*, const OnlineUserList&) noexcept { }
    virtual void on(UserRemoved, Client*, const OnlineUser&) noexcept { }
    virtual void on(Redirect, Client*, const string&) noexcept { }
    virtual void on(Failed, Client*, const string&) noexcept { }
    virtual void on(GetPassword, Client*) noexcept { }
    virtual void on(HubUpdated, Client*) noexcept { }
    virtual void on(Message, Client*, const ChatMessage&) noexcept { }
    virtual void on(StatusMessage, Client*, const string&, int = FLAG_NORMAL) noexcept { }
    virtual void on(HubUserCommand, Client*, int, int, const string&, const string&) noexcept { }
    virtual void on(HubFull, Client*) noexcept { }
    virtual void on(NickTaken, Client*) noexcept { }
    virtual void on(SearchFlood, Client*, const string&) noexcept { }
    virtual void on(NmdcSearch, Client*, const string&, int, int64_t, int, const string&) noexcept { }
    virtual void on(AdcSearch, Client*, const AdcCommand&, const CID&) noexcept { }
};

} // namespace dcpp

#endif /*CLIENTLISTENER_H_*/
