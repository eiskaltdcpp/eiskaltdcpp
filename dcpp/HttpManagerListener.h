/*
 * Copyright (C) 2001-2013 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_HTTP_MANAGER_LISTENER_H
#define DCPLUSPLUS_DCPP_HTTP_MANAGER_LISTENER_H

#include <string>

#include "forward.h"
#include "noexcept.h"

namespace dcpp {

using std::string;

class HttpManagerListener {
public:
	virtual ~HttpManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Added;
	typedef X<1> Updated;
	typedef X<2> Failed;
	typedef X<3> Complete;
	typedef X<4> ResetStream;
	typedef X<5> Removed;

	virtual void on(Added, HttpConnection*) noexcept { }
	virtual void on(Updated, HttpConnection*) noexcept { }
	virtual void on(Failed, HttpConnection*, const string&) noexcept { }
	virtual void on(Complete, HttpConnection*, OutputStream*) noexcept { }
	virtual void on(ResetStream, HttpConnection*) noexcept { }
	virtual void on(Removed, HttpConnection*) noexcept { }
};

} // namespace dcpp

#endif
