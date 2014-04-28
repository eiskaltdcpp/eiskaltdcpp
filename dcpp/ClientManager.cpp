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

#include "AdcHub.h"
#include "ClientManager.h"
#include "ConnectionManager.h"
#include "CryptoManager.h"
#include "FavoriteManager.h"
#include "FinishedManager.h"
#include "NmdcHub.h"
#include "QueueManager.h"
#include "SearchManager.h"
#include "SearchResult.h"
#include "SimpleXML.h"
#include "ShareManager.h"
#include "UserCommand.h"

#ifdef WITH_DHT
#include "dht/DHT.h"
#endif

namespace dcpp {

ClientManager::ClientManager() : udp(Socket::TYPE_UDP) {
    TimerManager::getInstance()->addListener(this);
}

ClientManager::~ClientManager() {
    TimerManager::getInstance()->removeListener(this);
}

Client* ClientManager::getClient(const string& aHubURL) {
    Client* c;
    if(strncmp("adc://", aHubURL.c_str(), 6) == 0) {
        c = new AdcHub(aHubURL, false);
    } else if(strncmp("adcs://", aHubURL.c_str(), 7) == 0) {
        c = new AdcHub(aHubURL, true);
    } else if(strncmp("nmdcs://", aHubURL.c_str(), 8) == 0) {
        c = new NmdcHub(aHubURL, true);
    } else {
        c = new NmdcHub(aHubURL, false);
    }

    {
        Lock l(cs);
       clients.insert(c);
    }

    c->addListener(this);

    return c;
}

void ClientManager::putClient(Client* aClient) {
    fire(ClientManagerListener::ClientDisconnected(), aClient);
    aClient->removeListeners();

    {
        Lock l(cs);
        clients.erase(aClient);
    }
    aClient->shutdown();
    delete aClient;
}

size_t ClientManager::getUserCount() const {
    Lock l(cs);
    return onlineUsers.size();
}

StringList ClientManager::getHubUrls(const CID& cid, const string& hintUrl) {
    Lock l(cs);
    StringList lst;
    OnlinePairC op = onlineUsers.equal_range(cid);
    for(auto i = op.first; i != op.second; ++i) {
        lst.push_back(i->second->getClient().getHubUrl());
    }

    return lst;
}

StringList ClientManager::getHubNames(const CID& cid, const string& hintUrl) {
    Lock l(cs);
    StringList lst;
    OnlinePairC op = onlineUsers.equal_range(cid);
    for(auto i = op.first; i != op.second; ++i) {
        lst.push_back(i->second->getClient().getHubName());
    }
    return lst;
}

StringList ClientManager::getNicks(const CID& cid, const string& hintUrl) {
    Lock l(cs);
    StringSet ret;
    OnlinePairC op = onlineUsers.equal_range(cid);
    for(auto i = op.first; i != op.second; ++i) {
        ret.insert(i->second->getIdentity().getNick());
    }

    if(ret.empty()) {
        auto i = nicks.find(cid);
        if(i != nicks.end()) {
            ret.insert(i->second.first);
        } else {
            // Offline perhaps?
            ret.insert('{' + cid.toBase32() + '}');
        }
    }
    return StringList(ret.begin(), ret.end());
}

StringPairList ClientManager::getHubs(const CID& cid, const string& hintUrl) {
    Lock l(cs);
    StringPairList lst;
    auto op = onlineUsers.equal_range(cid);
    for(auto i = op.first; i != op.second; ++i) {
        lst.push_back(make_pair(i->second->getClient().getHubUrl(), i->second->getClient().getHubName()));
    }
    return lst;
}

vector<Identity> ClientManager::getIdentities(const UserPtr &u) const {
    Lock l(cs);
    auto op = onlineUsers.equal_range(u->getCID());
    auto ret = vector<Identity>();
    for(auto i = op.first; i != op.second; ++i) {
        ret.push_back(i->second->getIdentity());
    }

    return ret;
}

string ClientManager::getField(const CID& cid, const string& hint, const char* field) const {
    Lock l(cs);

    OnlinePairC p;
    auto u = findOnlineUserHint(cid, hint, p);
    if(u) {
        auto value = u->getIdentity().get(field);
        if(!value.empty()) {
            return value;
        }
    }

    for(auto i = p.first; i != p.second; ++i) {
        auto value = i->second->getIdentity().get(field);
        if(!value.empty()) {
            return value;
        }
    }

    return Util::emptyString;
}

string ClientManager::getConnection(const CID& cid) const {
    Lock l(cs);
    auto i = onlineUsers.find(cid);
    if(i != onlineUsers.end()) {
        return i->second->getIdentity().getConnection();
    }
    return _("Offline");
}

int64_t ClientManager::getAvailable() const {
    Lock l(cs);
    int64_t bytes = 0;
    for(auto& i: onlineUsers) {
        bytes += i.second->getIdentity().getBytesShared();
    }

    return bytes;
}

uint8_t ClientManager::getSlots(const CID& cid) const {
    Lock l(cs);
    OnlineIterC i = onlineUsers.find(cid);
    if(i != onlineUsers.end()) {
        return static_cast<uint8_t>(Util::toInt(i->second->getIdentity().get("SL")));
    }
    return 0;
}

bool ClientManager::isConnected(const string& aUrl) const {
    Lock l(cs);

    for(auto i: clients) {
        if(i->getHubUrl() == aUrl) {
            return true;
        }
    }
    return false;
}

string ClientManager::findHub(const string& ipPort) const {
    Lock l(cs);

    string ip;
    string port;
    string::size_type i = ipPort.rfind(':');
    if(i == string::npos) {
        ip = ipPort;
        port = "411";
    } else {
        ip = ipPort.substr(0, i);
        port = ipPort.substr(i+1);
    }

    string url;
    for(auto c: clients) {
        if(c->getIp() == ip) {
            // If exact match is found, return it
            if(c->getPort() == port)
                return c->getHubUrl();

            // Port is not always correct, so use this as a best guess...
            url = c->getHubUrl();
        }
    }

    return url;
}

string ClientManager::findHubEncoding(const string& aUrl) const {
    Lock l(cs);

    for(auto i: clients) {
        if(i->getHubUrl() == aUrl) {
            return i->getEncoding();
        }
    }
    return Text::hubDefaultCharset;
}

HintedUser ClientManager::findLegacyUser(const string& nick) const noexcept {
    if(nick.empty())
        return HintedUser();
    Lock l(cs);

    for(auto i: clients) {
        auto nmdc = dynamic_cast<NmdcHub*>(i);
        if(nmdc) {
            /** @todo run the search directly on non-UTF-8 nicks when we store them. */
            auto ou = nmdc->findUser(nmdc->toUtf8(nick));
            if(ou) {
                return HintedUser(*ou);
            }
        }
    }

    return HintedUser();
}

UserPtr ClientManager::getUser(const string& aNick, const string& aHubUrl) noexcept {
    CID cid = makeCid(aNick, aHubUrl);
    Lock l(cs);

    auto ui = users.find(cid);
    if(ui != users.end()) {
        ui->second->setFlag(User::NMDC);
        return ui->second;
    }

    UserPtr p(new User(cid));
    p->setFlag(User::NMDC);
    users.insert(make_pair(cid, p));

    return p;
}

UserPtr ClientManager::getUser(const CID& cid) noexcept {
    Lock l(cs);
    auto ui = users.find(cid);
    if(ui != users.end()) {
        return ui->second;
    }
    if(cid == getMe()->getCID()) {
        return getMe();
    }
    UserPtr p(new User(cid));
    users.insert(make_pair(cid, p));
    return p;
}

UserPtr ClientManager::findUser(const CID& cid) const noexcept {
    Lock l(cs);
    auto ui = users.find(cid);
    return ui == users.end() ? 0 : ui->second;
}

bool ClientManager::isOp(const UserPtr& user, const string& aHubUrl) const {
    Lock l(cs);
    OnlinePairC p = onlineUsers.equal_range(user->getCID());
    for(auto i = p.first; i != p.second; ++i) {
        if(i->second->getClient().getHubUrl() == aHubUrl) {
            return i->second->getIdentity().isOp();
        }
    }
    return false;
}

CID ClientManager::makeCid(const string& aNick, const string& aHubUrl) const noexcept {
    TigerHash th;
    th.update(aNick.c_str(), aNick.length());
    th.update(aHubUrl.c_str(), aHubUrl.length());
    // Construct hybrid CID from the bits of the tiger hash - should be
    // fairly random, and hopefully low-collision
    return CID(th.finalize());
}

//void ClientManager::updateUsers() {
    //Lock l(cs);
    //for(auto client: clients) {
        //client->updateUsers();
    //}
//}

void ClientManager::putOnline(OnlineUser* ou) noexcept {
    {
        Lock l(cs);
        onlineUsers.insert(make_pair(ou->getUser()->getCID(), ou));
    }

    if(!ou->getUser()->isOnline()) {
        ou->getUser()->setFlag(User::ONLINE);
        fire(ClientManagerListener::UserConnected(), ou->getUser());
    }
}

void ClientManager::putOffline(OnlineUser* ou, bool disconnect) noexcept {
    bool lastUser = false;
    {
        Lock l(cs);
        auto op = onlineUsers.equal_range(ou->getUser()->getCID());
        dcassert(op.first != op.second);
        for(auto i = op.first; i != op.second; ++i) {
            auto ou2 = i->second;
            if(ou == ou2) {
                lastUser = (distance(op.first, op.second) == 1);
                onlineUsers.erase(i);
                break;
            }
        }
    }

    if(lastUser) {
        UserPtr& u = ou->getUser();
        u->unsetFlag(User::ONLINE);
        if(disconnect)
            ConnectionManager::getInstance()->disconnect(u);
        fire(ClientManagerListener::UserDisconnected(), u);
    }
}

OnlineUser* ClientManager::findOnlineUserHint(const CID& cid, const string& hintUrl, OnlinePairC& p) const {
        p = onlineUsers.equal_range(cid);
        if(p.first == p.second) // no user found with the given CID.
        return 0;

    if(!hintUrl.empty()) {
        for(auto i = p.first; i != p.second; ++i) {
            OnlineUser* u = i->second;
            if(u->getClient().getHubUrl() == hintUrl) {
                return u;
            }
        }
    }

        return 0;
}

OnlineUser* ClientManager::findOnlineUser(const HintedUser& user) const {
    return findOnlineUser(user.user->getCID(), user.hint);
}

OnlineUser* ClientManager::findOnlineUserHint(const HintedUser& user) const {
    return findOnlineUserHint(user.user->getCID(), user.hint);
}

OnlineUser* ClientManager::findOnlineUser(const CID& cid, const string& hintUrl) const {
    OnlinePairC p;
    OnlineUser* u = findOnlineUserHint(cid, hintUrl, p);
    if(u) // found an exact match (CID + hint).
        return u;

    if(p.first == p.second) // no user found with the given CID.
        return 0;

    // return a random user that matches the given CID but not the hint.
    return p.first->second;
}

OnlineUser* ClientManager::findOnlineUserHint(const CID& cid, const string& hintUrl) const {
    OnlinePairC p;
    return findOnlineUserHint(cid, hintUrl, p);
}

void ClientManager::connect(const HintedUser& user, const string& token) {

    Lock l(cs);
    OnlineUser* u = findOnlineUser(user);

    if(u) {
        u->getClient().connect(*u, token);
    }
}

void ClientManager::privateMessage(const HintedUser& user, const string& msg, bool thirdPerson) {

    Lock l(cs);
    OnlineUser* u = findOnlineUser(user);

    if(u) {
        u->getClient().privateMessage(*u, msg, thirdPerson);
    }
}

void ClientManager::userCommand(const HintedUser& user, const UserCommand& uc, StringMap& params, bool compatibility) {
    Lock l(cs);
    /** @todo we allow wrong hints for now ("false" param of findOnlineUser) because users
     * extracted from search results don't always have a correct hint; see
     * SearchManager::onRES(const AdcCommand& cmd, ...). when that is done, and SearchResults are
     * switched to storing only reliable HintedUsers (found with the token of the ADC command),
     * change this call to findOnlineUserHint. */
    OnlineUser* ou = findOnlineUserHint(user.user->getCID(), user.hint.empty() ? uc.getHub() : user.hint);
    if(!ou
#ifdef WITH_DHT
       || ou->getClientBase().type == ClientBase::DHT
#endif
                                                             )
        return;

    ou->getIdentity().getParams(params, "user", compatibility);
    ou->getClient().getHubIdentity().getParams(params, "hub", false);
    ou->getClient().getMyIdentity().getParams(params, "my", compatibility);
    ou->getClient().escapeParams(params);
    ou->getClient().sendUserCmd(uc, params);
}

void ClientManager::sendUDP(AdcCommand& cmd, const OnlineUser& user) {
    dcassert(cmd.getType() == AdcCommand::TYPE_UDP);
    if(!user.getIdentity().isUdpActive()) {
#ifdef WITH_DHT
            if (user.getClientBase().getType() == Client::DHT)
                return;
#endif
            cmd.setType(AdcCommand::TYPE_DIRECT);
            cmd.setTo(user.getIdentity().getSID());
            const_cast<Client&>(user.getClient()).send(cmd);
    } else {
        sendUDP(user.getIdentity().getIp(), user.getIdentity().getUdpPort(), cmd.toString(getMe()->getCID()));
    }
}

void ClientManager::send(AdcCommand& cmd, const CID& cid) {
    Lock l(cs);
    auto i = onlineUsers.find(cid);
    if(i != onlineUsers.end()) {
        OnlineUser& u = *i->second;
        if(cmd.getType() == AdcCommand::TYPE_UDP && !u.getIdentity().isUdpActive()) {
            if(u.getUser()->isNMDC()
#ifdef WITH_DHT
                || u.getClientBase().getType() == Client::DHT
#endif
                                                              )
                 return;
            cmd.setType(AdcCommand::TYPE_DIRECT);
            cmd.setTo(u.getIdentity().getSID());
            u.getClient().send(cmd);
        } else {
            try {
                udp.writeTo(u.getIdentity().getIp(), u.getIdentity().getUdpPort(), cmd.toString(getMe()->getCID()));
            } catch(const SocketException&) {
                dcdebug("Socket exception sending ADC UDP command\n");
            }
        }
    }
}

void ClientManager::sendUDP(const string& ip, const string& port, const string& data) {
    try {
        udp.writeTo(ip, port, data);
    } catch(const SocketException&) {
        dcdebug("Socket exception when sending UDP data to %s:%s\n", ip.c_str(), port.c_str());
    }
}

void ClientManager::infoUpdated() {
    Lock l(cs);
    for(auto i: clients) {
        if(i->isConnected()) {
            i->info(false);
        }
    }
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
                                    int aFileType, const string& aString) noexcept
{
    Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);

    bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);
    bool isTTHSearch = ((aFileType == SearchManager::TYPE_TTH) && (aString.compare(0, 4, "TTH:") == 0));

    // We don't wan't to answer passive searches if we're in passive mode...
    if(isPassive && !ClientManager::getInstance()->isActive(aClient->getHubUrl())) {
        return;
    }

    SearchResultList l;
    ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
//      dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
    if(!l.empty()) {
        if(isPassive) {
            string name = aSeeker.substr(4);
            // Good, we have a passive seeker, those are easier...
            string str;
            for(const auto& sr: l) {
                str += sr->toSR(*aClient);
                str[str.length()-1] = 5;
                str += name;
                str += '|';
            }

            if(!str.empty())
                aClient->send(str);

        } else {
                string ip, port, file, proto, query, fragment;
                Util::decodeUrl(aSeeker, proto, ip, port, file, query, fragment);
                ip = Socket::resolve(ip, AF_INET);
                if(static_cast<NmdcHub*>(aClient)->isProtectedIP(ip))
                    return;
                if(port.empty())
                    port = "412";
                for(const auto& sr: l)
                    sendUDP(ip, port, sr->toSR(*aClient));
        }
    } else if(!isPassive && isTTHSearch) {
        PartsInfo partialInfo;
        TTHValue aTTH(aString.substr(4));
        if(!QueueManager::getInstance()->handlePartialSearch(aTTH, partialInfo)) {
            // if not found, try to find in finished list
            if(!FinishedManager::getInstance()->handlePartialRequest(aTTH, partialInfo)) {
                return;
            }
        }

        string ip, file, proto, query, fragment, port;
        Util::decodeUrl(aSeeker, proto, ip, port, file, query, fragment);

        try {
            AdcCommand cmd = SearchManager::getInstance()->toPSR(true, aClient->getMyNick(), aClient->getIpPort(), aTTH.toBase32(), partialInfo);
            ip = Socket::resolve(ip, AF_INET);
            sendUDP(ip, port, cmd.toString(ClientManager::getInstance()->getMe()->getCID()));
        } catch(...) {
            dcdebug("Partial search caught error\n");
        }
    }
}

void ClientManager::on(AdcSearch, Client* c, const AdcCommand& adc, const OnlineUser& from) noexcept {
    bool isUdpActive = false;
    isUdpActive = from.getIdentity().isUdpActive();

    SearchManager::getInstance()->respond(adc, from, isUdpActive, c->getIpPort());

    Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), [&adc]() -> string
    {
        auto toCode = [](char a, char b) -> uint16_t {
            return (uint16_t)a | ((uint16_t)b)<<8;
        };

        string result;
        const StringList &params = adc.getParameters();

        for(const string &param: params) {
            if(param.length() <= 2)
                continue;
            uint16_t cmd = toCode(param[0], param[1]);
            if (toCode('T', 'R') == cmd)
                result = "TTH:" + param.substr(2);
            else if (toCode('A', 'N') == cmd)
                result += param.substr(2) + ' ';
        }
        return result;
    }());
}


void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, void* aOwner) {
#ifdef WITH_DHT
    if(BOOLSETTING(USE_DHT) && aFileType == SearchManager::TYPE_TTH)
        dht::DHT::getInstance()->findFile(aString);
#endif
    Lock l(cs);
    for(auto i: clients) {
        if(i->isConnected()) {
            i->search(aSizeMode, aSize, aFileType, aString, aToken, StringList() /*ExtList*/, aOwner);
        }
    }
}

uint64_t ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList, void* aOwner) {
#ifdef WITH_DHT
    if(BOOLSETTING(USE_DHT) && aFileType == SearchManager::TYPE_TTH)
        dht::DHT::getInstance()->findFile(aString, aToken);
#endif
    Lock l(cs);
    uint64_t estimateSearchSpan = 0;
    for(auto& client: who) {
        for(auto c: clients) {
            if(c->isConnected() && c->getHubUrl() == client) {
                uint64_t ret = c->search(aSizeMode, aSize, aFileType, aString, aToken, aExtList, aOwner);
                estimateSearchSpan = max(estimateSearchSpan, ret);
            }
        }
    }
    return estimateSearchSpan;
}

void ClientManager::on(TimerManagerListener::Minute, uint64_t /* aTick */) noexcept {
    Lock l(cs);

    // Collect some garbage...
    auto i = users.begin();
    while(i != users.end()) {
        if(i->second->unique()) {
            users.erase(i++);
        } else {
            ++i;
        }
    }

    for(auto j: clients) {
        j->info(false);
    }
}

UserPtr& ClientManager::getMe() {
    if(!me) {
        Lock l(cs);
        if(!me) {
            me = new User(getMyCID());
            users.insert(make_pair(me->getCID(), me));
        }
    }
    return me;
}

const CID& ClientManager::getMyPID() {
    if(pid.isZero())
        pid = CID(SETTING(PRIVATE_ID));
    return pid;
}

CID ClientManager::getMyCID() {
    TigerHash tiger;
    tiger.update(getMyPID().data(), CID::SIZE);
    return CID(tiger.finalize());
}

void ClientManager::updateUser(const OnlineUser& user) noexcept {
    if(!user.getIdentity().getNick().empty()) {
        Lock l(cs);
        auto i = nicks.find(user.getUser()->getCID());
        if(i == nicks.end()) {
                nicks[user.getUser()->getCID()] = std::make_pair(user.getIdentity().getNick(), false);
        } else {
                i->second.first = user.getIdentity().getNick();
        }
    }
    fire(ClientManagerListener::UserUpdated(), user);
}

void ClientManager::loadUsers() {
    try {
        SimpleXML xml;
        xml.fromXML(File(getUsersFile(), File::READ, File::OPEN).read());

        if(xml.findChild("Users")) {
            xml.stepIn();

            {
                Lock l(cs);
                while(xml.findChild("User")) {
                    nicks[CID(xml.getChildAttrib("CID"))] = std::make_pair(xml.getChildAttrib("Nick"), false);
                }
            }

            xml.stepOut();
        }
    } catch(const Exception&) { }
}

void ClientManager::saveUsers() const {
    try {
        SimpleXML xml;
        xml.addTag("Users");
        xml.stepIn();

        {
            Lock l(cs);
            for(auto& i: nicks) {
                if(i.second.second) {
                    xml.addTag("User");
                    xml.addChildAttrib("CID", i.first.toBase32());
                    xml.addChildAttrib("Nick", i.second.first);
                }
            }
        }

        xml.stepOut();

        const string fName = getUsersFile();
        File out(fName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
        BufferedOutputStream<false> f(&out);
        f.write(SimpleXML::utf8Header);
        xml.toXML(&f);
        f.flush();
        out.close();
        File::deleteFile(fName);
        File::renameFile(fName + ".tmp", fName);
    } catch(const Exception&) { }
}

void ClientManager::saveUser(const CID& cid) {
    Lock l(cs);
    auto i = nicks.find(cid);
    if(i != nicks.end())
        i->second.second = true;
}

void ClientManager::on(Connected, Client* c) noexcept {
    fire(ClientManagerListener::ClientConnected(), c);
}

void ClientManager::on(UserUpdated, Client*, const OnlineUser& user) noexcept {
    updateUser(user);
}

void ClientManager::on(UsersUpdated, Client*, const OnlineUserList& l) noexcept {
    for(auto& i: l) {
        updateUser(*i);
    }
}

void ClientManager::on(HubUpdated, Client* c) noexcept {
    fire(ClientManagerListener::ClientUpdated(), c);
}

void ClientManager::on(Failed, Client* client, const string&) noexcept {
    fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(HubUserCommand, Client* client, int aType, int ctx, const string& name, const string& command) noexcept {
    if(BOOLSETTING(HUB_USER_COMMANDS)) {
        if(aType == UserCommand::TYPE_REMOVE) {
            int cmd = FavoriteManager::getInstance()->findUserCommand(name, client->getHubUrl());
            if(cmd != -1)
                FavoriteManager::getInstance()->removeUserCommand(cmd);
        } else if(aType == UserCommand::TYPE_CLEAR) {
            FavoriteManager::getInstance()->removeHubUserCommands(ctx, client->getHubUrl());
        } else {
            FavoriteManager::getInstance()->addUserCommand(aType, ctx, UserCommand::FLAG_NOSAVE, name, command, "", client->getHubUrl());
        }
    }
}
int ClientManager::getMode(const string& aHubUrl) const {

    if(aHubUrl.empty())
        return SETTING(INCOMING_CONNECTIONS);

    int mode = 0;
    const FavoriteHubEntry* hub = FavoriteManager::getInstance()->getFavoriteHubEntry(aHubUrl);
    if(hub) {
        switch(hub->getMode()) {
            case 1 :
                mode = SettingsManager::INCOMING_DIRECT;
                break;
            case 2 :
                mode = SettingsManager::INCOMING_FIREWALL_PASSIVE;
                break;
            default:
                mode = SETTING(INCOMING_CONNECTIONS);
        }
    } else {
        mode = SETTING(INCOMING_CONNECTIONS);
    }
    return mode;
}

void ClientManager::cancelSearch(void* aOwner) {
    Lock l(cs);

    for(auto i = clients.begin(); i != clients.end(); ++i) {
        (*i)->cancelSearch(aOwner);
    }
}

#ifdef WITH_DHT
OnlineUserPtr ClientManager::findDHTNode(const CID& cid) const {
    Lock l(cs);

    OnlinePairC op = onlineUsers.equal_range(cid);
    for(auto i = op.first; i != op.second; ++i) {
        OnlineUser* ou = i->second;

        // user not in DHT, so don't bother with other hubs
        if(!ou->getUser()->isSet(User::DHT))
            break;

        if(ou->getClientBase().getType() == Client::DHT)
            return ou;
    }

    return NULL;
}
#endif

#ifdef LUA_SCRIPT
bool ClientManager::ucExecuteLua(const string& ucCommand, StringMap& params) noexcept {
    bool executedlua = false;
    string::size_type i, j, k;
    i = j = k = 0;
    string tmp = ucCommand;
    while( (i = tmp.find("%[lua:", i)) != string::npos) {
        i += 6;
        j = tmp.find(']', i);
        if(j == string::npos)
            break;
        string chunk = tmp.substr(i, j-i);
        // for making possible using %[nick] and similar parameters too
        // !%!{nick!}
        k = 0;
        while( (k = chunk.find("!%")) != string::npos) {
            chunk.erase(k, 2);
            chunk.insert(k, "%");
        }
        k = 0;
        while( (k = chunk.find("!{")) != string::npos) {
            chunk.erase(k, 2);
            chunk.insert(k, "[");
        }
        k = 0;
        while( (k = chunk.find("!}")) != string::npos) {
            chunk.erase(k, 2);
            chunk.insert(k, "]");
        }
        //@todo: use filter? I opted for no here, but this means Lua has to be careful about
        //filtering if it cares.
        ScriptManager::getInstance()->EvaluateChunk(Util::formatParams(chunk, params, false));
        executedlua = true;
        i = j + 1;
    }
    return executedlua;
}
#endif // LUA_SCRIPT
} // namespace dcpp
