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

#include "forward.h"
#include "noexcept.h"
#include <string>

namespace dcpp {

class HttpConnection;
using std::string;

class HttpConnectionListener {
public:
    virtual ~HttpConnectionListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> Data;
    typedef X<1> Failed;
    typedef X<2> Complete;
    typedef X<3> Redirected;
    typedef X<4> TypeNormal;
    typedef X<5> TypeBZ2;
    typedef X<6> Retried;

    virtual void on(Data, HttpConnection*, const uint8_t*, size_t) noexcept = 0;
    virtual void on(Failed, HttpConnection*, const string&) noexcept { }
    virtual void on(Complete, HttpConnection*, const string&) noexcept { }
    virtual void on(Redirected, HttpConnection*, const string&) noexcept { }
    virtual void on(TypeNormal, HttpConnection*) noexcept { }
    virtual void on(TypeBZ2, HttpConnection*) noexcept { }
    virtual void on(Retried, HttpConnection*, const bool) noexcept { }
};

} // namespace dcpp

