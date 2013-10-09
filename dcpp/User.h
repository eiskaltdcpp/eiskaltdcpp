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
#include "Pointer.h"
#include "CID.h"
#include "FastAlloc.h"
#include "CriticalSection.h"
#include "Flags.h"

namespace dcpp {

class ClientBase;

/** A user connected to one or more hubs. */
class User : public FastAlloc<User>, public intrusive_ptr_base<User>, public Flags, private boost::noncopyable
{
public:
    enum Bits {
        ONLINE_BIT,
        PASSIVE_BIT,
        NMDC_BIT,
        BOT_BIT,
        TLS_BIT,
        OLD_CLIENT_BIT,
        NO_ADC_1_0_PROTOCOL_BIT,
        NO_ADCS_0_10_PROTOCOL_BIT,
        DHT_BIT,
        NAT_TRAVERSAL_BIT
    };

    /** Each flag is set if it's true in at least one hub */
    enum UserFlags {
        ONLINE = 1<<ONLINE_BIT,
        PASSIVE = 1<<PASSIVE_BIT,
        NMDC = 1<<NMDC_BIT,
        BOT = 1<<BOT_BIT,
        TLS = 1<<TLS_BIT,               //< Client supports TLS
        OLD_CLIENT = 1<<OLD_CLIENT_BIT,  //< Can't download - old client
        NO_ADC_1_0_PROTOCOL = 1<<NO_ADC_1_0_PROTOCOL_BIT,   //< Doesn't support "ADC/1.0" (dc++ <=0.703)
        NO_ADCS_0_10_PROTOCOL = 1<< NO_ADCS_0_10_PROTOCOL_BIT,   //< Doesn't support "ADCS/0.10"
        DHT = 1<<DHT_BIT,
        NAT_TRAVERSAL = 1<<NAT_TRAVERSAL_BIT
    };

    struct Hash {
        size_t operator()(const UserPtr& x) const { return ((size_t)(&(*x)))/sizeof(User); }
    };

    User(const CID& aCID) : cid(aCID) { }

    ~User() { }

    const CID& getCID() const { return cid; }
    operator const CID&() const { return cid; }

    bool isOnline() const { return isSet(ONLINE); }
    bool isNMDC() const { return isSet(NMDC); }

private:
    CID cid;
};

} // namespace dcpp
