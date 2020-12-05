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

#include "stdinc.h"

#include "User.h"

#include "AdcHub.h"
#include "ClientManager.h"
#include "FavoriteUser.h"
#include "format.h"
#include "StringTokenizer.h"

namespace dcpp {

FastCriticalSection Identity::cs;

OnlineUser::OnlineUser(const UserPtr& ptr, ClientBase& client_, uint32_t sid_) : isInList(false), identity(ptr, sid_), client(client_) {

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

void Identity::getParams(ParamMap& params, const string& prefix, bool compatibility, bool dht) const {
    {
        FastLock l(cs);
        for(auto& i: info) {
            params[prefix + string((char*)(&i.first), 2)] = i.second;
        }
    }
    if(
        #ifdef WITH_DHT
            !dht &&
        #endif
            user) {
        params[prefix + "SID"] = getSIDString();
        params[prefix + "CID"] = user->getCID().toBase32();
        params[prefix + "TAG"] = getTag();
        params[prefix + "SSshort"] = Util::formatBytes(get("SS"));

        if(compatibility) {
            if(prefix == "my") {
                params["mynick"] = getNick();
                params["mycid"] = user->getCID().toBase32();
            } else {
                params["nick"] = getNick();
                params["cid"] = user->getCID().toBase32();
                params["ip"] = get("I4");
                params["tag"] = getTag();
                params["description"] = get("DE");
                params["email"] = get("EM");
                params["share"] = get("SS");
                params["shareshort"] = Util::formatBytes(get("SS"));
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
    return "<" + getApplication() + ",M:" + string(isTcpActive() ? "A" : "P") +
            ",H:" + get("HN") + "/" + get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
}

string Identity::getApplication() const {
    auto application = get("AP");
    auto version = get("VE");

    if(version.empty()) {
        return application;
    }

    if(application.empty()) {
        // AP is an extension, so we can't guarantee that the other party supports it, so default to VE.
        return version;
    }

    return application + ' ' + version;
}

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

bool Identity::isSelf() const {
    FastLock l(cs);
    return Flags::isSet(SELF_ID);
}

void Identity::setSelf() {
    FastLock l(cs);
    if(!Flags::isSet(SELF_ID))
        Flags::setFlag(SELF_ID);
}

bool Identity::noChat() const {
    FastLock l(cs);
    return Flags::isSet(IGNORE_CHAT);
}

void Identity::setNoChat(bool ignoreChat) {
    FastLock l(cs);
    if(ignoreChat) {
        if(!Flags::isSet(IGNORE_CHAT))
            Flags::setFlag(IGNORE_CHAT);
    } else {
        if(Flags::isSet(IGNORE_CHAT))
            Flags::unsetFlag(IGNORE_CHAT);
    }
}

void FavoriteUser::update(const OnlineUser& info) {
    setNick(info.getIdentity().getNick());
    setUrl(info.getClient().getHubUrl());
}

} // namespace dcpp
