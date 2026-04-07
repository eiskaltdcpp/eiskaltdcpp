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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "forward.h"
#include "AdcCommand.h"
#include "Util.h"

namespace dcpp {

class UserConnectionListener {
public:
    virtual ~UserConnectionListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> BytesSent;
    typedef X<1> Connected;
    typedef X<2> Data;
    typedef X<3> Failed;
    typedef X<4> CLock;
    typedef X<5> Key;
    typedef X<6> Direction;
    typedef X<7> Get;
    typedef X<8> Updated;
    typedef X<12> Send;
    typedef X<13> GetListLength;
    typedef X<14> MaxedOut;
    typedef X<15> ModeChange;
    typedef X<16> MyNick;
    typedef X<17> TransmitDone;
    typedef X<18> Supports;
    typedef X<19> ProtocolError;
    typedef X<20> FileNotAvailable;

    virtual void on(BytesSent, UserConnection*, size_t, size_t) { }
    virtual void on(Connected, UserConnection*) { }
    virtual void on(Data, UserConnection*, const uint8_t*, size_t) { }
    virtual void on(Failed, UserConnection*, const string&) { }
    virtual void on(ProtocolError, UserConnection*, const string&) { }
    virtual void on(CLock, UserConnection*, const string&, const string&) { }
    virtual void on(Key, UserConnection*, const string&) { }
    virtual void on(Direction, UserConnection*, const string&, const string&) { }
    virtual void on(Get, UserConnection*, const string&, int64_t) { }
    virtual void on(Send, UserConnection*) { }
    virtual void on(GetListLength, UserConnection*) { }
    virtual void on(MaxedOut, UserConnection*) { }
    virtual void on(ModeChange, UserConnection*) { }
    virtual void on(MyNick, UserConnection*, const string&) { }
    virtual void on(TransmitDone, UserConnection*) { }
    virtual void on(Supports, UserConnection*, const StringList&) { }
    virtual void on(FileNotAvailable, UserConnection*) { }
    virtual void on(Updated, UserConnection*) { }

    virtual void on(AdcCommand::SUP, UserConnection*, const AdcCommand&) { }
    virtual void on(AdcCommand::INF, UserConnection*, const AdcCommand&) { }
    virtual void on(AdcCommand::GET, UserConnection*, const AdcCommand&) { }
    virtual void on(AdcCommand::SND, UserConnection*, const AdcCommand&) { }
    virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) { }
    virtual void on(AdcCommand::RES, UserConnection*, const AdcCommand&) { }
    virtual void on(AdcCommand::GFI, UserConnection*, const AdcCommand&) { }
};

} // namespace dcpp
