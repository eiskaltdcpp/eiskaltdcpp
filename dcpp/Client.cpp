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
#include "DCPlusPlus.h"

#include "Client.h"

#include "BufferedSocket.h"
#include "DebugManager.h"
#include "FavoriteManager.h"
#include "TimerManager.h"
#include "ClientManager.h"
#include "version.h"

namespace dcpp {

Client::Counts Client::counts;

Client::Client(const string& hubURL, char separator_, bool secure_) :
    myIdentity(ClientManager::getInstance()->getMe(), 0),
    reconnDelay(120), lastActivity(GET_TICK()), registered(false), autoReconnect(false),
    encoding(Text::hubDefaultCharset), state(STATE_DISCONNECTED), sock(0),
    hubUrl(hubURL), port(0), separator(separator_),
    secure(secure_), countType(COUNT_UNCOUNTED)
{
    string file, proto, query, fragment;
    Util::decodeUrl(hubURL, proto, address, port, file, query, fragment);

    if(!query.empty()) {
            map<string, string> q = Util::decodeQuery(query);
            map<string, string>::iterator kp = q.find("kp");
            if(kp != q.end()) {
                    keyprint = kp->second;
            }
    }

    TimerManager::getInstance()->addListener(this);
}

Client::~Client() throw() {
    dcassert(!sock);

    // In case we were deleted before we Failed
    FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
    TimerManager::getInstance()->removeListener(this);
    updateCounts(true);
}

void Client::reconnect() {
    disconnect(true);
    setAutoReconnect(true);
    setReconnDelay(0);
}

void Client::shutdown() {
    if(sock) {
        BufferedSocket::putSocket(sock);
        sock = 0;
    }
}

void Client::reloadSettings(bool updateNick) {
    const FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(getHubUrl());

    string ClientId;
    if (::strncmp(getHubUrl().c_str(),"adc://", 6) == 0 ||
        ::strncmp(getHubUrl().c_str(),"adcs://", 7) == 0)
        ClientId = fullADCVersionString;
    else
        ClientId = fullVersionString;

    if(hub) {
        if(updateNick) {
            setCurrentNick(checkNick(hub->getNick(true)));
        }

        if(!hub->getUserDescription().empty()) {
            setCurrentDescription(hub->getUserDescription());
        } else {
            setCurrentDescription(SETTING(DESCRIPTION));
        }

        if(!hub->getPassword().empty())
            setPassword(hub->getPassword());
        if (hub->getOverrideId() && strlen(hub->getClientId().c_str()) > 1)
            ClientId = hub->getClientId();
        if (!hub->getExternalIP().empty())
            externalIP = hub->getExternalIP();
        if (!hub->getEncoding().empty()){
            setEncoding(hub->getEncoding());
        }
        if (hub->getUseInternetIP() && !SETTING(INTERNETIP).empty()){
            externalIP = SETTING(INTERNETIP);
        }
        if(hub->getSearchInterval() < 10)
            setSearchInterval(SETTING(MINIMUM_SEARCH_INTERVAL) * 1000);
        else
            setSearchInterval(hub->getSearchInterval() * 1000);
    } else {
        if(updateNick) {
            setCurrentNick(checkNick(SETTING(NICK)));
        }
        setCurrentDescription(SETTING(DESCRIPTION));
        setSearchInterval(SETTING(MINIMUM_SEARCH_INTERVAL) * 1000);
    }
    setClientId(ClientId);
}

bool Client::isActive() const {
        return ClientManager::getInstance()->isActive(hubUrl);
}

void Client::connect() {
    if(sock)
        BufferedSocket::putSocket(sock);

    setAutoReconnect(true);
    setReconnDelay(SETTING(RECONNECT_DELAY));
    reloadSettings(true);
    setRegistered(false);
    setMyIdentity(Identity(ClientManager::getInstance()->getMe(), 0));
    setHubIdentity(Identity());

    state = STATE_CONNECTING;

    try {
        sock = BufferedSocket::getSocket(separator);
        sock->addListener(this);
        sock->connect(address, port, secure, BOOLSETTING(ALLOW_UNTRUSTED_HUBS), true);
    } catch(const Exception& e) {
        shutdown();
        /// @todo at this point, this hub instance is completely useless
        fire(ClientListener::Failed(), this, e.getError());
    }
    updateActivity();
}

void Client::send(const char* aMessage, size_t aLen) {
    if(!isReady()) {
        dcassert(0);
        return;
    }
    updateActivity();
    sock->write(aMessage, aLen);
    COMMAND_DEBUG(aMessage, DebugManager::HUB_OUT, getIpPort());
}

void Client::on(Connected) throw() {
    updateActivity();
    ip = sock->getIp();
    localIp = sock->getLocalIp();
    if(sock->isSecure() && keyprint.compare(0, 7, "SHA256/") == 0) {
        vector<uint8_t> kp = sock->getKeyprint();
        if(!kp.empty()) {
            vector<uint8_t> kp2v(kp.size());
            Encoder::fromBase32(keyprint.c_str() + 7, &kp2v[0], kp2v.size());
            if(!std::equal(kp.begin(), kp.end(), kp2v.begin())) {
                state = STATE_DISCONNECTED;
                sock->removeListener(this);
                fire(ClientListener::Failed(), this, "Keyprint mismatch");
                return;
            }
        }
    }
    fire(ClientListener::Connected(), this);
    state = STATE_PROTOCOL;
}

void Client::on(Failed, const string& aLine) throw() {
    state = STATE_DISCONNECTED;
    FavoriteManager::getInstance()->removeUserCommand(getHubUrl());
    sock->removeListener(this);
    fire(ClientListener::Failed(), this, aLine);
}

void Client::disconnect(bool graceLess) {
    if(sock)
        sock->disconnect(graceLess);
}

bool Client::isSecure() const {
    return isReady() && sock->isSecure();
}

bool Client::isTrusted() const {
    return isReady() && sock->isTrusted();
}

std::string Client::getCipherName() const {
    return isReady() ? sock->getCipherName() : Util::emptyString;
}

vector<uint8_t> Client::getKeyprint() const {
    return isReady() ? sock->getKeyprint() : vector<uint8_t>();
}
void Client::updateCounts(bool aRemove) {
    // We always remove the count and then add the correct one if requested...
    if(countType == COUNT_NORMAL) {
        counts.normal.dec();
    } else if(countType == COUNT_REGISTERED) {
        counts.registered.dec();
    } else if(countType == COUNT_OP) {
        counts.op.dec();
    }

    countType = COUNT_UNCOUNTED;

    if(!aRemove) {
        if(getMyIdentity().isOp()) {
            counts.op.inc();
            countType = COUNT_OP;
        } else if(getMyIdentity().isRegistered()) {
            counts.registered.inc();
            countType = COUNT_REGISTERED;
        } else {
            counts.normal.inc();
            countType = COUNT_NORMAL;
        }
    }
}

string Client::getLocalIp() const {
    if (!externalIP.empty())
        return Socket::resolve(externalIP);

    // Best case - the server detected it
    if((!BOOLSETTING(NO_IP_OVERRIDE) || SETTING(EXTERNAL_IP).empty()) && !getMyIdentity().getIp().empty()) {
        return getMyIdentity().getIp();
    }

    if(!SETTING(EXTERNAL_IP).empty()) {
        return Socket::resolve(SETTING(EXTERNAL_IP));
    }

    if(localIp.empty()) {
        return Util::getLocalIp();
    }

    return localIp;
}

uint64_t Client::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList, void* owner){
    dcdebug("Queue search %s\n", aString.c_str());

    if(searchQueue.interval) {
        SearchCore s;
        s.fileType = aFileType;
        s.size     = aSize;
        s.query    = aString;
        s.sizeType = aSizeMode;
        s.token    = aToken;
        s.exts	   = aExtList;
        s.owners.insert(owner);

        searchQueue.add(s);
    }
    search(aSizeMode, aSize, aFileType , aString, aToken, aExtList);

    return searchQueue.interval ? searchQueue.getSearchTime(owner) - GET_TICK() : 0;

}

void Client::on(Line, const string& aLine) throw() {
    updateActivity();
    COMMAND_DEBUG(aLine, DebugManager::HUB_IN, getIpPort())
}

void Client::on(Second, uint64_t aTick) throw() {
    if(state == STATE_DISCONNECTED && getAutoReconnect() && (aTick > (getLastActivity() + getReconnDelay() * 1000)) ) {
        // Try to reconnect...
        connect();
    }
    if(!searchQueue.interval) return;

    if(isConnected()){
        SearchCore s;

        if(searchQueue.pop(s)){
            search(s.sizeType, s.size, s.fileType , s.query, s.token, s.exts);
        }
    }
}
#ifdef LUA_SCRIPT
string ClientScriptInstance::formatChatMessage(const tstring& aLine) {
        Lock l(cs);
        // this string is probably in UTF-8.  Does lua want/need strings in the active code page?
        string processed = Text::fromT(aLine);
        MakeCall("dcpp", "FormatChatText", 1, (Client*)this, processed);

        if (lua_isstring(L, -1)) processed = lua_tostring(L, -1);

        lua_settop(L, 0);
        return Text::toT(processed);
}

bool ClientScriptInstance::onHubFrameEnter(Client* aClient, const string& aLine) {
        Lock l(cs);
        // ditto the comment above
        MakeCall("dcpp", "OnCommandEnter", 1, aClient, aLine);
        return GetLuaBool();
}
#endif
} // namespace dcpp
