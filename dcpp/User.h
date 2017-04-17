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

#include "Util.h"
#include "Pointer.h"
#include "CID.h"
#include "FastAlloc.h"
#include "CriticalSection.h"
#include "Flags.h"
#include "forward.h"
#include <boost/utility.hpp>
#include <map>
#include <vector>

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
#ifdef WITH_DHT
        DHT_BIT,
#endif
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
#ifdef WITH_DHT
        DHT = 1<<DHT_BIT,
#endif
        NAT_TRAVERSAL = 1<<NAT_TRAVERSAL_BIT
    };

    struct Hash {
        size_t operator()(const UserPtr& x) const {
            if (x.get() == nullptr) {
//                printf("User failed ptr == nullptr\n");fflush(stdout);
                return 0;
            }
            return ((size_t)(&(*x)))/sizeof(User); }
    };

    User(const CID& aCID) : cid(aCID) { }

    ~User() noexcept { }

    const CID& getCID() const { return cid; }
    operator const CID&() const { return cid; }

    bool isOnline() const { return isSet(ONLINE); }
    bool isNMDC() const { return isSet(NMDC); }

private:
    CID cid;
};

/** User pointer associated to a hub url */
struct HintedUser {
    UserPtr user;
    string hint;

    explicit HintedUser(const UserPtr& user_, const string& hint_) : user(user_), hint(hint_) { }

    bool operator==(const UserPtr& rhs) const {
        return user == rhs;
    }
    bool operator==(const HintedUser& rhs) const {
        return user == rhs.user;
        // ignore the hint, we don't want lists with multiple instances of the same user...
    }

    operator UserPtr() const { return user; }
};

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

    Identity() { }
    Identity(const UserPtr& ptr, uint32_t aSID) : user(ptr) { setSID(aSID); }
    Identity(const Identity& rhs) { *this = rhs; } // Use operator= since we have to lock before reading...
    Identity& operator=(const Identity& rhs) { FastLock l(cs); user = rhs.user; info = rhs.info; return *this; }
    ~Identity() { }
// GS is already defined on some systems (e.g. OpenSolaris)
#ifdef GS
#undef GS
#endif

#define GS(n, x) string get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
    GS(Nick, "NI")
    GS(Description, "DE")
    GS(Ip, "I4")
    GS(UdpPort, "U4")
    GS(Email, "EM")
    GS(Connection, "CO")

    void setBytesShared(const string& bs) { set("SS", bs); }
    int64_t getBytesShared() const { return Util::toInt64(get("SS")); }

    void setStatus(const string& st) { set("ST", st); }
    StatusFlags getStatus() const { return static_cast<StatusFlags>(Util::toInt(get("ST"))); }

    void setOp(bool op) { set("OP", op ? "1" : Util::emptyString); }
    void setHub(bool hub) { set("HU", hub ? "1" : Util::emptyString); }
    void setBot(bool bot) { set("BO", bot ? "1" : Util::emptyString); }
    void setHidden(bool hidden) { set("HI", hidden ? "1" : Util::emptyString); }
    string getTag() const;
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
    UserPtr& getUser() { return user; }
    GETSET(UserPtr, user, User);
    GETSET(uint32_t, sid, SID);
private:
    typedef std::unordered_map<short, string> InfMap;
    typedef InfMap::iterator InfIter;
    typedef InfMap::const_iterator InfIterC;
    InfMap info;

    static FastCriticalSection cs;
};

class Client;
class NmdcHub;

class OnlineUser : public FastAlloc<OnlineUser>, public intrusive_ptr_base<OnlineUser> {
public:
    typedef std::vector<OnlineUser*> List;
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
    OnlineUser(const OnlineUser&);
    OnlineUser& operator=(const OnlineUser&);
    ClientBase& client;
};

} // namespace dcpp
