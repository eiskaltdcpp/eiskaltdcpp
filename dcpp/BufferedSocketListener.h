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

#ifndef DCPLUSPLUS_DCPP_BUFFEREDSOCKETLISTENER_H_
#define DCPLUSPLUS_DCPP_BUFFEREDSOCKETLISTENER_H_

#include "noexcept.h"
#include <string>

namespace dcpp {

using std::string;

class BufferedSocketListener {
public:
	virtual ~BufferedSocketListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Connecting;
	typedef X<1> Connected;
	typedef X<2> Line;
	typedef X<3> Data;
	typedef X<4> BytesSent;
	typedef X<5> ModeChange;
	typedef X<6> TransmitDone;
	typedef X<7> Failed;
	typedef X<8> Updated;

	virtual void on(Connecting) noexcept { }
	virtual void on(Connected) noexcept { }
	virtual void on(Line, const string&) noexcept { }
	virtual void on(Data, uint8_t*, size_t) noexcept { }
	virtual void on(BytesSent, size_t, size_t) noexcept { }
	virtual void on(ModeChange) noexcept { }
	virtual void on(TransmitDone) noexcept { }
	virtual void on(Failed, const string&) noexcept { }
	virtual void on(Updated) noexcept { }
};

} // namespace dcpp

#endif /*BUFFEREDSOCKETLISTENER_H_*/
