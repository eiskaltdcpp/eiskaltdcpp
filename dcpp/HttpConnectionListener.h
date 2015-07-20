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

#pragma once

#include <string>
#include "forward.h"
#include "noexcept.h"

namespace dcpp {

using std::string;

class HttpConnectionListener {
public:
    virtual ~HttpConnectionListener() { }
    template<int I> struct X { enum { TYPE = I }; };
    typedef X<0> Complete;
    typedef X<1> Failed;

    virtual void on(Failed, HttpConnection*, const string&) noexcept = 0;
    virtual void on(Complete, HttpConnection*, const string&) noexcept = 0;
};

} // namespace dcpp
