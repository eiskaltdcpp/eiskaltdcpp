/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef USERCONNECTIONLISTENER_H_
#define USERCONNECTIONLISTENER_H_

#include "forward.h"

#include "AdcCommand.h"

namespace dcpp {

class UserConnectionListener {
public:
	virtual ~UserConnectionListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

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
	typedef X<19> FileNotAvailable;

	virtual void on(BytesSent, UserConnection*, size_t, size_t) throw() { }
	virtual void on(Connected, UserConnection*) throw() { }
	virtual void on(Data, UserConnection*, const uint8_t*, size_t) throw() { }
	virtual void on(Failed, UserConnection*, const string&) throw() { }
	virtual void on(CLock, UserConnection*, const string&, const string&) throw() { }
	virtual void on(Key, UserConnection*, const string&) throw() { }
	virtual void on(Direction, UserConnection*, const string&, const string&) throw() { }
	virtual void on(Get, UserConnection*, const string&, int64_t) throw() { }
	virtual void on(Send, UserConnection*) throw() { }
	virtual void on(GetListLength, UserConnection*) throw() { }
	virtual void on(MaxedOut, UserConnection*) throw() { }
	virtual void on(ModeChange, UserConnection*) throw() { }
	virtual void on(MyNick, UserConnection*, const string&) throw() { }
	virtual void on(TransmitDone, UserConnection*) throw() { }
	virtual void on(Supports, UserConnection*, const StringList&) throw() { }
	virtual void on(FileNotAvailable, UserConnection*) throw() { }
	virtual void on(Updated, UserConnection*) throw() { }

	virtual void on(AdcCommand::SUP, UserConnection*, const AdcCommand&) throw() { }
	virtual void on(AdcCommand::INF, UserConnection*, const AdcCommand&) throw() { }
	virtual void on(AdcCommand::GET, UserConnection*, const AdcCommand&) throw() { }
	virtual void on(AdcCommand::SND, UserConnection*, const AdcCommand&) throw() { }
	virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) throw() { }
	virtual void on(AdcCommand::RES, UserConnection*, const AdcCommand&) throw() { }
	virtual void on(AdcCommand::GFI, UserConnection*, const AdcCommand&) throw() { }
};

} // namespace dcpp

#endif /*USERCONNECTIONLISTENER_H_*/
