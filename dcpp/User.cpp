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

#include "stdinc.h"

#include "User.h"

#include "AdcHub.h"
#include "FavoriteUser.h"
#include "StringTokenizer.h"
#include "ClientManager.h"

namespace dcpp {

FastCriticalSection Identity::cs;

OnlineUser::OnlineUser(const UserPtr& ptr, ClientBase& client_, uint32_t sid_) : identity(ptr, sid_), client(client_) {

}

bool Identity::isTcpActive(const Client* c) const {
    if(c != NULL && user == ClientManager::getInstance()->getMe()) {
        return c->isActive(); // userlist should display our real mode
    } else {
        return (!user->isSet(User::NMDC)) ?
                !getIp().empty() && supports(AdcHub::TCP4_FEATURE) :
                !user->isSet(User::PASSIVE);
    }
}

bool Identity::isUdpActive() const {
        if(getIp().empty() || getUdpPort().empty())
                return false;
        return (!user->isSet(User::NMDC)) ? supports(AdcHub::UDP4_FEATURE) : !user->isSet(User::PASSIVE);
}

void Identity::getParams(StringMap& sm, const string& prefix, bool compatibility
#ifdef WITH_DHT
                                                                                , bool dht
#endif
                                                                                          ) const {
    {
        FastLock l(cs);
        for(InfMap::const_iterator i = info.begin(); i != info.end(); ++i) {
            sm[prefix + string((char*)(&i->first), 2)] = i->second;
        }
    }
    if(
#ifdef WITH_DHT
       !dht &&
#endif
               user) {
        sm[prefix + "SID"] = getSIDString();
        sm[prefix + "CID"] = user->getCID().toBase32();
        sm[prefix + "TAG"] = getTag();
        sm[prefix + "SSshort"] = Util::formatBytes(get("SS"));

        if(compatibility) {
            if(prefix == "my") {
                sm["mynick"] = getNick();
                sm["mycid"] = user->getCID().toBase32();
            } else {
                sm["nick"] = getNick();
                sm["cid"] = user->getCID().toBase32();
                sm["ip"] = get("I4");
                sm["tag"] = getTag();
                sm["description"] = get("DE");
                sm["email"] = get("EM");
                sm["share"] = get("SS");
                sm["shareshort"] = Util::formatBytes(get("SS"));
            }
        }
    }
}

bool Identity::isClientType(ClientType ct) const {
    int type = Util::toInt(get("CT"));
    return (type & ct) == ct;
}

string Identity::getTag() const {
    if(!get("TA").empty())
        return get("TA");
    if(get("VE").empty() || get("HN").empty() || get("HR").empty() ||get("HO").empty() || get("SL").empty())
        return Util::emptyString;
    return "<" + get("VE") + ",M:" + string(isTcpActive() ? "A" : "P") + ",H:" + get("HN") + "/" +
        get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
}

string Identity::get(const char* name) const {
    FastLock l(cs);
    InfMap::const_iterator i = info.find(*(short*)name);
    return i == info.end() ? Util::emptyString : i->second;
}

bool Identity::isSet(const char* name) const {
    FastLock l(cs);
    InfMap::const_iterator i = info.find(*(short*)name);
    return i != info.end();
}


void Identity::set(const char* name, const string& val) {
    FastLock l(cs);
    if(val.empty())
        info.erase(*(short*)name);
    else
        info[*(short*)name] = val;
}

bool Identity::supports(const string& name) const {
    string su = get("SU");
    StringTokenizer<string> st(su, ',');
    for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
        if(*i == name)
            return true;
    }
    return false;
}

std::map<string, string> Identity::getInfo() const {
    std::map<string, string> ret;

    FastLock l(cs);
    for(InfIterC i = info.begin(); i != info.end(); ++i) {
        ret[string((char*)(&i->first), 2)] = i->second;
    }

    return ret;
}

void FavoriteUser::update(const OnlineUser& info) {
    setNick(info.getIdentity().getNick());
    setUrl(info.getClient().getHubUrl());
}

} // namespace dcpp
