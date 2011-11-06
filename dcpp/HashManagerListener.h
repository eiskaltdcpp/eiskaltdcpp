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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef HASHMANAGERLISTENER_H_
#define HASHMANAGERLISTENER_H_

#include "MerkleTree.h"
#include "noexcept.h"

namespace dcpp {

class HashManagerListener {
public:
    virtual ~HashManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> TTHDone;

    virtual void on(TTHDone, const string& /* fileName */, const TTHValue& /* root */) noexcept = 0;
};

}
#endif /*HASHMANAGERLISTENER_H_*/
