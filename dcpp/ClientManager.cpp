/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#include "ClientManager.h"

#include "ShareManager.h"
#include "SearchManager.h"
#include "ConnectionManager.h"
#include "CryptoManager.h"
#include "FavoriteManager.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "SearchResult.h"

#include "AdcHub.h"
#include "NmdcHub.h"
#ifdef DHT
#include "../dht/DHT.h"
#endif
#include "FinishedManager.h"//sdc
#include "QueueManager.h"

namespace dcpp {

Client* ClientManager::getClient(const string& aHubURL) {
    Client* c;
    if(Util::strnicmp("adc://", aHubURL.c_str(), 6) == 0) {
        c = new AdcHub(aHubURL, false);
    } else if(Util::strnicmp("adcs://", aHubURL.c_str(), 7) == 0) {
        c = new AdcHub(aHubURL, true);
    } else {
        c = new NmdcHub(aHubURL);
    }

    {
        Lock l(cs);
        clients.push_back(c);
    }

    c->addListener(this);

    return c;
}

void ClientManager::putClient(Client* aClient) {
    fire(ClientManagerListener::ClientDisconnected(), aClient);
    aClient->removeListeners();

    {
        Lock l(cs);
        clients.remove(aClient);
    }
    aClient->shutdown();
    delete aClient;
}

size_t ClientManager::getUserCount() const {
    Lock l(cs);
    return onlineUsers.size();
}

StringList ClientManager::getHubs(const CID& cid, const string& hintUrl) {
        return getHubs(cid, hintUrl, FavoriteManager::getInstance()->isPrivate(hintUrl));
}

StringList ClientManager::getHubNames(const CID& cid, const string& hintUrl) {
        return getHubNames(cid, hintUrl, FavoriteManager::getInstance()->isPrivate(hintUrl));
}

StringList ClientManager::getNicks(const CID& cid, const string& hintUrl) {
        return getNicks(cid, hintUrl, FavoriteManager::getInstance()->isPrivate(hintUrl));
}

StringList ClientManager::getHubs(const CID& cid, const string& hintUrl, bool priv) {
    Lock l(cs);
    StringList lst;
    if(!priv) {
        OnlinePairC op = onlineUsers.equal_range(cid);
        for(OnlineIterC i = op.first; i != op.second; ++i) {
            lst.push_back(i->second->getClient().getHubUrl());
        }
    } else {
        OnlineUser* u = findOnlineUser_hint(cid, hintUrl);
        if(u)
            lst.push_back(u->getClient().getHubUrl());
    }
    return lst;
}

StringList ClientManager::getHubNames(const CID& cid, const string& hintUrl, bool priv) {
    Lock l(cs);
    StringList lst;
    if(!priv) {
        OnlinePairC op = onlineUsers.equal_range(cid);
        for(OnlineIterC i = op.first; i != op.second; ++i) {
            lst.push_back(i->second->getClient().getHubName());
        }
    } else {
        OnlineUser* u = findOnlineUser_hint(cid, hintUrl);
        if(u)
            lst.push_back(u->getClient().getHubName());
    }
    return lst;
}

StringList ClientManager::getNicks(const CID& cid, const string& hintUrl, bool priv) {
    Lock l(cs);
    StringSet ret;
    if(!priv) {
        OnlinePairC op = onlineUsers.equal_range(cid);
        for(OnlineIterC i = op.first; i != op.second; ++i) {
            ret.insert(i->second->getIdentity().getNick());
        }
    } else {
        OnlineUser* u = findOnlineUser_hint(cid, hintUrl);
        if(u)
            ret.insert(u->getIdentity().getNick());
    }
    if(ret.empty()) {
        NickMap::const_iterator i = nicks.find(cid);
        if(i != nicks.end()) {
            ret.insert(i->second.first);
        } else {
            // Offline perhaps?
            ret.insert('{' + cid.toBase32() + '}');
        }
    }
    return StringList(ret.begin(), ret.end());
}

string ClientManager::getConnection(const CID& cid) const {
    Lock l(cs);
    OnlineIterC i = onlineUsers.find(cid);
    if(i != onlineUsers.end()) {
        return i->second->getIdentity().getConnection();
    }
    return _("Offline");
}

int64_t ClientManager::getAvailable() const {
    Lock l(cs);
    int64_t bytes = 0;
    for(OnlineIterC i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
        bytes += i->second->getIdentity().getBytesShared();
    }

    return bytes;
}

bool ClientManager::isConnected(const string& aUrl) const {
    Lock l(cs);

    for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
        if((*i)->getHubUrl() == aUrl) {
            return true;
        }
    }
    return false;
}

string ClientManager::findHub(const string& ipPort) const {
    Lock l(cs);

    string ip;
    uint16_t port = 411;
    string::size_type i = ipPort.find(':');
    if(i == string::npos) {
        ip = ipPort;
    } else {
        ip = ipPort.substr(0, i);
        port = static_cast<uint16_t>(Util::toInt(ipPort.substr(i+1)));
    }

    string url;
    for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
        const Client* c = *i;
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

    for(Client::List::const_iterator i = clients.begin(); i != clients.end(); ++i) {
        if((*i)->getHubUrl() == aUrl) {
            return (*i)->getEncoding();
        }
    }
    return Text::hubDefaultCharset;
}

UserPtr ClientManager::findLegacyUser(const string& aNick) const throw() {
    if (aNick.empty())
        return UserPtr();
    Lock l(cs);

    for(OnlineMap::const_iterator i = onlineUsers.begin(); i != onlineUsers.end(); ++i) {
        const OnlineUser* ou = i->second;
        if(ou->getUser()->isSet(User::NMDC) && Util::stricmp(ou->getIdentity().getNick(), aNick) == 0)
            return ou->getUser();
    }
    return UserPtr();
}

UserPtr ClientManager::getUser(const string& aNick, const string& aHubUrl) throw() {
    CID cid = makeCid(aNick, aHubUrl);
    Lock l(cs);

    UserIter ui = users.find(cid);
    if(ui != users.end()) {
        ui->second->setFlag(User::NMDC);
        return ui->second;
    }

    UserPtr p(new User(cid));
    p->setFlag(User::NMDC);
    users.insert(make_pair(cid, p));

    return p;
}

UserPtr ClientManager::getUser(const CID& cid) throw() {
    Lock l(cs);
    UserIter ui = users.find(cid);
    if(ui != users.end()) {
        return ui->second;
    }

    UserPtr p(new User(cid));
    users.insert(make_pair(cid, p));
    return p;
}

UserPtr ClientManager::findUser(const CID& cid) const throw() {
    Lock l(cs);
    UserMap::const_iterator ui = users.find(cid);
    if(ui != users.end()) {
        return ui->second;
    }
    return 0;
}

bool ClientManager::isOp(const UserPtr& user, const string& aHubUrl) const {
    Lock l(cs);
    OnlinePairC p = onlineUsers.equal_range(user->getCID());
    for(OnlineIterC i = p.first; i != p.second; ++i) {
        if(i->second->getClient().getHubUrl() == aHubUrl) {
            return i->second->getIdentity().isOp();
        }
    }
    return false;
}

CID ClientManager::makeCid(const string& aNick, const string& aHubUrl) const throw() {
    string n = Text::toLower(aNick);
    TigerHash th;
    th.update(n.c_str(), n.length());
    th.update(Text::toLower(aHubUrl).c_str(), aHubUrl.length());
    // Construct hybrid CID from the bits of the tiger hash - should be
    // fairly random, and hopefully low-collision
    return CID(th.finalize());
}

void ClientManager::putOnline(OnlineUser* ou) throw() {
    {
        Lock l(cs);
        onlineUsers.insert(make_pair(ou->getUser()->getCID(), ou));
    }

    if(!ou->getUser()->isOnline()) {
        ou->getUser()->setFlag(User::ONLINE);
        fire(ClientManagerListener::UserConnected(), ou->getUser());
    }
}

void ClientManager::putOffline(OnlineUser* ou, bool disconnect) throw() {
    bool lastUser = false;
    {
        Lock l(cs);
        OnlinePair op = onlineUsers.equal_range(ou->getUser()->getCID());
        dcassert(op.first != op.second);
        for(OnlineIter i = op.first; i != op.second; ++i) {
            OnlineUser* ou2 = i->second;
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

OnlineUser* ClientManager::findOnlineUser_hint(const CID& cid, const string& hintUrl, OnlinePair& p) throw() {
        p = onlineUsers.equal_range(cid);
        if(p.first == p.second) // no user found with the given CID.
        return 0;

    if(!hintUrl.empty()) {
        for(OnlineIter i = p.first; i != p.second; ++i) {
            OnlineUser* u = i->second;
            if(u->getClient().getHubUrl() == hintUrl) {
                return u;
            }
        }
    }

        return 0;
}

OnlineUser* ClientManager::findOnlineUser(const CID& cid, const string& hintUrl, bool priv) throw() {
        OnlinePair p;
        OnlineUser* u = findOnlineUser_hint(cid, hintUrl, p);
        if(u) // found an exact match (CID + hint).
                return u;

        if(p.first == p.second) // no user found with the given CID.
                return 0;

        // if the hint hub is private, don't allow connecting to the same user from another hub.
        if(priv)
                return 0;

        // ok, hub not private, return a random user that matches the given CID but not the hint.
    return p.first->second;
}

void ClientManager::connect(const HintedUser& user, const string& token) {
        bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);

    Lock l(cs);
        OnlineUser* u = findOnlineUser(user.user->getCID(), user.hint, priv);

        if(u) {
                u->getClient().connect(*u, token);
        }
}

void ClientManager::privateMessage(const HintedUser& user, const string& msg, bool thirdPerson) {
        bool priv = FavoriteManager::getInstance()->isPrivate(user.hint);

        Lock l(cs);
        OnlineUser* u = findOnlineUser(user.user->getCID(), user.hint, priv);

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
         * change this call to findOnlineUser_hint. */
        OnlineUser* ou = findOnlineUser(user.user->getCID(), user.hint.empty() ? uc.getHub() : user.hint, false);
        if(!ou)
                return;

        ou->getIdentity().getParams(params, "user", compatibility);
        ou->getClient().getHubIdentity().getParams(params, "hub", false);
        ou->getClient().getMyIdentity().getParams(params, "my", compatibility);
        ou->getClient().escapeParams(params);
        ou->getClient().sendUserCmd(uc, params);
}

void ClientManager::send(AdcCommand& cmd, const CID& cid) {
    Lock l(cs);
    OnlineIter i = onlineUsers.find(cid);
    if(i != onlineUsers.end()) {
        OnlineUser& u = *i->second;
        if(cmd.getType() == AdcCommand::TYPE_UDP && !u.getIdentity().isUdpActive()) {
            cmd.setType(AdcCommand::TYPE_DIRECT);
            cmd.setTo(u.getIdentity().getSID());
            u.getClient().send(cmd);
        } else {
            try {
                udp.writeTo(u.getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(u.getIdentity().getUdpPort())), cmd.toString(getMe()->getCID()));
            } catch(const SocketException&) {
                dcdebug("Socket exception sending ADC UDP command\n");
            }
        }
    }
}

void ClientManager::infoUpdated() {
    Lock l(cs);
    for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
        if((*i)->isConnected()) {
            (*i)->info(false);
        }
    }
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
                                    int aFileType, const string& aString) throw()
{
    Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);

    bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);

    // We don't wan't to answer passive searches if we're in passive mode...
    if(isPassive && !ClientManager::getInstance()->isActive(aClient->getHubUrl())) {
        return;
    }

    SearchResultList l;
    ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
//      dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
    if(l.size() > 0) {
        if(isPassive) {
            string name = aSeeker.substr(4);
            // Good, we have a passive seeker, those are easier...
            string str;
            for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
                const SearchResultPtr& sr = *i;
                str += sr->toSR(*aClient);
                str[str.length()-1] = 5;
                str += name;
                str += '|';
            }

            if(str.size() > 0)
                aClient->send(str);

        } else {
            try {
                string ip, file;
                uint16_t port = 0;
                Util::decodeUrl(aSeeker, ip, port, file);
                ip = Socket::resolve(ip);
                if(static_cast<NmdcHub*>(aClient)->isProtectedIP(ip))
                    return;
                if(port == 0)
                    port = 412;
                for(SearchResultList::const_iterator i = l.begin(); i != l.end(); ++i) {
                    const SearchResultPtr& sr = *i;
                    udp.writeTo(ip, port, sr->toSR(*aClient));
                }
            } catch(const SocketException& /* e */) {
                dcdebug("Search caught error\n");
            }
        }
    } else if(!isPassive && (aFileType == SearchManager::TYPE_TTH) && (aString.compare(0, 4, "TTH:") == 0)) {
        PartsInfo partialInfo;
        TTHValue aTTH(aString.substr(4));
        if(!QueueManager::getInstance()->handlePartialSearch(aTTH, partialInfo)) {
            // if not found, try to find in finished list
            if(!FinishedManager::getInstance()->handlePartialRequest(aTTH, partialInfo)) {
                return;
            }
        }

        string ip, file;
        uint16_t port = 0;
        Util::decodeUrl(aSeeker, ip, port, file);

        try {
            AdcCommand cmd = SearchManager::getInstance()->toPSR(true, aClient->getMyNick(), aClient->getIpPort(), aTTH.toBase32(), partialInfo);
            Socket s;
            s.writeTo(Socket::resolve(ip), port, cmd.toString(ClientManager::getInstance()->getMe()->getCID()));
        } catch(...) {
            dcdebug("Partial search caught error\n");
        }
    }
}

void ClientManager::on(AdcSearch, Client*, const AdcCommand& adc, const CID& from) throw() {
    bool isUdpActive = false;
    {
        Lock l(cs);

        OnlineIter i = onlineUsers.find(from);
        if(i != onlineUsers.end()) {
            OnlineUser& u = *i->second;
            isUdpActive = u.getIdentity().isUdpActive();
        }

    }
    SearchManager::getInstance()->respond(adc, from, isUdpActive);
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
#ifdef DHT
    if(BOOLSETTING(USE_DHT) && aFileType == SearchManager::TYPE_TTH)
        dht::DHT::getInstance()->findFile(aString);
#endif
    Lock l(cs);

    for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
        if((*i)->isConnected()) {
            (*i)->search(aSizeMode, aSize, aFileType, aString, aToken, StringList() /*ExtList*/);
        }
    }
}

void ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList) {
#ifdef DHT
    if(BOOLSETTING(USE_DHT) && aFileType == SearchManager::TYPE_TTH)
        dht::DHT::getInstance()->findFile(aString, aToken);
#endif
    Lock l(cs);

    for(StringIter it = who.begin(); it != who.end(); ++it) {
        string& client = *it;
        for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
            Client* c = *j;
            if(c->isConnected() && c->getHubUrl() == client) {
                c->search(aSizeMode, aSize, aFileType, aString, aToken, aExtList);
            }
        }
    }
}

void ClientManager::on(TimerManagerListener::Minute, uint32_t /* aTick */) throw() {
    Lock l(cs);

    // Collect some garbage...
    UserIter i = users.begin();
    while(i != users.end()) {
        if(i->second->unique()) {
            users.erase(i++);
        } else {
            ++i;
        }
    }

    for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
        (*j)->info(false);
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

void ClientManager::updateNick(const OnlineUser& user) throw() {
        if(!user.getIdentity().getNick().empty()) {
    Lock l(cs);
                NickMap::iterator i = nicks.find(user.getUser()->getCID());
                if(i == nicks.end()) {
                        nicks[user.getUser()->getCID()] = std::make_pair(user.getIdentity().getNick(), false);
                } else {
                        i->second.first = user.getIdentity().getNick();
                }
        }
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
                        for(NickMap::const_iterator i = nicks.begin(), iend = nicks.end(); i != iend; ++i) {
                                if(i->second.second) {
                                        xml.addTag("User");
                                        xml.addChildAttrib("CID", i->first.toBase32());
                                        xml.addChildAttrib("Nick", i->second.first);
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
        NickMap::iterator i = nicks.find(cid);
        if(i != nicks.end())
                i->second.second = true;
}

void ClientManager::on(Connected, Client* c) throw() {
    fire(ClientManagerListener::ClientConnected(), c);
}

void ClientManager::on(UserUpdated, Client*, const OnlineUser& user) throw() {
    updateNick(user);
    fire(ClientManagerListener::UserUpdated(), user);
}

void ClientManager::on(UsersUpdated, Client* c, const OnlineUserList& l) throw() {
    for(OnlineUserList::const_iterator i = l.begin(), iend = l.end(); i != iend; ++i) {
        updateNick(*(*i));
        fire(ClientManagerListener::UserUpdated(), *(*i));
    }
}

void ClientManager::on(HubUpdated, Client* c) throw() {
    fire(ClientManagerListener::ClientUpdated(), c);
}

void ClientManager::on(Failed, Client* client, const string&) throw() {
    fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(HubUserCommand, Client* client, int aType, int ctx, const string& name, const string& command) throw() {
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
} // namespace dcpp
