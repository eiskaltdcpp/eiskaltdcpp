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

#include <map>
#include <vector>

#include "NonCopyable.h"

#include "Flags.h"
#include "FastAlloc.h"
#include "GetSet.h"
#include "Pointer.h"
#include "Util.h"
#include "User.h"

namespace dcpp {

/** One of possibly many identities of a user, mainly for UI purposes */
class Identity : public Flags {
public:
    enum IdentityFlagBits {
        GOT_INF_BIT,
        NMDC_PASSIVE_BIT
    };
    enum IdentityFlags {
        GOT_INF = 1 << GOT_INF_BIT,
        NMDC_PASSIVE = 1 << NMDC_PASSIVE_BIT
    };
    enum ClientType {
        CT_BOT = 1,
        CT_REGGED = 2,
        CT_OP = 4,
        CT_SU = 8,
        CT_OWNER = 16,
        CT_HUB = 32,
        CT_HIDDEN = 64
    };
    enum StatusFlags {
        NORMAL          = 0x01,
        AWAY            = 0x02,
        TLS             = 0x10,
        NAT             = 0x20
    };

    Identity() : sid(0) { }
    Identity(const UserPtr& ptr, uint32_t aSID) : user(ptr), sid(aSID) { }
    Identity(const Identity& rhs) : Flags(), sid(0) { *this = rhs; } // Use operator= since we have to lock before reading...
    Identity& operator=(const Identity& rhs) {
        FastLock l(cs);
        *static_cast<Flags*>(this) = rhs;
        user = rhs.user;
        sid = rhs.sid;
        info = rhs.info;
        return *this;
    }

#define GETSET_FIELD(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
    GETSET_FIELD(Nick, "NI")
    GETSET_FIELD(Description, "DE")
    GETSET_FIELD(Ip, "I4")
    GETSET_FIELD(UdpPort, "U4")
    GETSET_FIELD(Email, "EM")
    GETSET_FIELD(Connection, "CO")
#undef GETSET_FIELD

    void setBytesShared(const string& bs) { set("SS", bs); }
    int64_t getBytesShared() const { return Util::toInt64(get("SS")); }

    void setStatus(const string& st) { set("ST", st); }
    StatusFlags getStatus() const { return static_cast<StatusFlags>(Util::toInt(get("ST"))); }

    void setOp(bool op) { set("OP", op ? "1" : Util::emptyString); }
    void setHub(bool hub) { set("HU", hub ? "1" : Util::emptyString); }
    void setBot(bool bot) { set("BO", bot ? "1" : Util::emptyString); }
    void setHidden(bool hidden) { set("HI", hidden ? "1" : Util::emptyString); }
    string getTag() const;
    string getApplication() const;
    bool supports(const string& name) const;
    bool isHub() const { return isClientType(CT_HUB) || isSet("HU"); }
    bool isOp() const { return isClientType(CT_OP) || isClientType(CT_SU) || isClientType(CT_OWNER) || isSet("OP"); }
    bool isRegistered() const { return isClientType(CT_REGGED) || isSet("RG"); }
    bool isHidden() const { return isClientType(CT_HIDDEN) || isSet("HI"); }
    bool isBot() const { return isClientType(CT_BOT) || isSet("BO"); }
    bool isAway() const { return isSet("AW"); }
    bool isTcpActive(const Client* = NULL) const;
    bool isUdpActive() const;
    std::map<string, string> getInfo() const;
    string get(const char* name) const;
    void set(const char* name, const string& val);
    bool isSet(const char* name) const;
    string getSIDString() const { return string((const char*)&sid, 4); }

    bool isClientType(ClientType ct) const;

    void getParams(StringMap& map, const string& prefix, bool compatibility, bool dht = false) const;

    const UserPtr& getUser() const { return user; }
    UserPtr& getUser() { return user; }
    uint32_t getSID() const { return sid; }

private:
    UserPtr user;
    uint32_t sid;

    typedef std::unordered_map<short, string> InfMap;
    typedef InfMap::iterator InfIter;
    typedef InfMap::const_iterator InfIterC;
    InfMap info;

    static FastCriticalSection cs;
};

class NmdcHub;

class OnlineUser : public FastAlloc<OnlineUser>, private NonCopyable, public intrusive_ptr_base<OnlineUser> {
public:
    typedef vector<OnlineUser*> List;
    typedef List::iterator Iter;

    OnlineUser(const UserPtr& ptr, ClientBase& client_, uint32_t sid_);
    virtual ~OnlineUser() noexcept { }
    operator UserPtr&() { return getUser(); }
    operator const UserPtr&() const { return getUser(); }

    UserPtr& getUser() { return getIdentity().getUser(); }
    const UserPtr& getUser() const { return getIdentity().getUser(); }
    Identity& getIdentity() { return identity; }
    Client& getClient() { return (Client&)client; }
    const Client& getClient() const { return (const Client&)client; }
    ClientBase& getClientBase() { return client; }
    const ClientBase& getClientBase() const { return client; }
    bool isInList;
    GETSET(Identity, identity, Identity);

private:
    friend class NmdcHub;
    ClientBase& client;
};

} // namespace dcpp
