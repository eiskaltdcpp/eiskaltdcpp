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

#include "stdinc.h"
#include "User.h"

#include "AdcHub.h"
#include "FavoriteUser.h"
#include "format.h"
#include "StringTokenizer.h"
#include "ClientManager.h"

namespace dcpp {

FastCriticalSection Identity::cs;

OnlineUser::OnlineUser(const UserPtr& ptr, ClientBase& client_, uint32_t sid_) : identity(ptr, sid_), client(client_) {

}
bool Identity::isTcpActive(const Client* c) const {
    return isTcp4Active(c) || isTcp6Active(c);
}

bool Identity::isTcp4Active(const Client* c) const {
    if(c && user == ClientManager::getInstance()->getMe()) {
        return c->isActive(); // userlist should display our real mode
    } else {
        return (!user->isSet(User::NMDC)) ?
                !getIp4().empty() && supports(AdcHub::TCP4_FEATURE) :
                !user->isSet(User::PASSIVE);
    }
}

bool Identity::isTcp6Active(const Client* c) const {
    if(c && user == ClientManager::getInstance()->getMe()) {
        return c->isActive(); // userlist should display our real mode
    } else {
        return (!user->isSet(User::NMDC)) ?
                !getIp6().empty() && supports(AdcHub::TCP6_FEATURE) :
                !user->isSet(User::PASSIVE);
    }
}

bool Identity::isUdpActive() const {
    return isUdp4Active() || isUdp6Active();
}

bool Identity::isUdp4Active() const {
    if(getIp4().empty() || getUdp4Port().empty())
        return false;
    return user->isSet(User::NMDC) ? !user->isSet(User::PASSIVE) : supports(AdcHub::UDP4_FEATURE);
}

bool Identity::isUdp6Active() const {
    if(getIp6().empty() || getUdp6Port().empty())
        return user->isSet(User::NMDC) ? !user->isSet(User::PASSIVE) : supports(AdcHub::UDP6_FEATURE);
    return false;
}

string Identity::getUdpPort() const {
    if(getIp6().empty() || getUdp6Port().empty()) {
        return getUdp4Port();
    }
    return getUdp6Port();
}

string Identity::getIp() const {
    return getIp6().empty() ? getIp4() : getIp6();
}

void Identity::getParams(StringMap& sm, const string& prefix, bool compatibility, bool dht) const {
    {
        FastLock l(cs);
        for(auto& i: info) {
            sm[prefix + string((char*)(&i.first), 2)] = i.second;
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
    return "<" + getApplication() + ",M:" + string(isTcpActive() ? "A" : "P") + ",H:" + get("HN") + "/" + get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
}

string Identity::getApplication() const {
    auto application = get("AP");
    auto version = get("VE");

    if(version.empty())
        return application;

    if(application.empty())
        return version;

    return application + ' ' + version;
}

//string Identity::getConnection() const {
    //if(!get("US").empty())
        //return str(F_("%1%/s") % Util::formatBytes(get("US")));
    //return get("CO");
//}

string Identity::get(const char* name) const {
    FastLock l(cs);
    auto i = info.find(*(short*)name);
    return i == info.end() ? Util::emptyString : i->second;
}

bool Identity::isSet(const char* name) const {
    FastLock l(cs);
    auto i = info.find(*(short*)name);
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
    for(auto& i: st.getTokens()) {
        if(i == name)
            return true;
    }
    return false;
}

std::map<string, string> Identity::getInfo() const {
    std::map<string, string> ret;

    FastLock l(cs);
    for(auto& i: info) {
        ret[string((char*)(&i.first), 2)] = i.second;
    }

    return ret;
}

void FavoriteUser::update(const OnlineUser& info) {
    setNick(info.getIdentity().getNick());
    setUrl(info.getClient().getHubUrl());
}

} // namespace dcpp
