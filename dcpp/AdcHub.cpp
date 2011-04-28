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

#include "AdcHub.h"

#include "ChatMessage.h"
#include "ClientManager.h"
#include "ShareManager.h"
#include "StringTokenizer.h"
#include "AdcCommand.h"
#include "ConnectionManager.h"
#include "version.h"
#include "Util.h"
#include "UserCommand.h"
#include "CryptoManager.h"
#include "LogManager.h"
#include "ThrottleManager.h"
#include "UploadManager.h"
#include <cmath>

namespace dcpp {

const string AdcHub::CLIENT_PROTOCOL("ADC/1.0");
const string AdcHub::SECURE_CLIENT_PROTOCOL_TEST("ADCS/0.10");
const string AdcHub::ADCS_FEATURE("ADC0");
const string AdcHub::TCP4_FEATURE("TCP4");
const string AdcHub::UDP4_FEATURE("UDP4");
const string AdcHub::NAT0_FEATURE("NAT0");
const string AdcHub::SEGA_FEATURE("SEGA");
const string AdcHub::BASE_SUPPORT("ADBASE");
const string AdcHub::BAS0_SUPPORT("ADBAS0");
const string AdcHub::TIGR_SUPPORT("ADTIGR");
const string AdcHub::UCM0_SUPPORT("ADUCM0");
const string AdcHub::BLO0_SUPPORT("ADBLO0");

const vector<StringList> AdcHub::searchExts;

AdcHub::AdcHub(const string& aHubURL, bool secure) : Client(aHubURL, '\n', secure), oldPassword(false), sid(0) {
    TimerManager::getInstance()->addListener(this);
}

AdcHub::~AdcHub() throw() {
    TimerManager::getInstance()->removeListener(this);
    clearUsers();
}

OnlineUser& AdcHub::getUser(const uint32_t aSID, const CID& aCID) {
    OnlineUser* ou = findUser(aSID);
    if(ou) {
        return *ou;
    }

    UserPtr p = ClientManager::getInstance()->getUser(aCID);

    {
        Lock l(cs);
        ou = users.insert(make_pair(aSID, new OnlineUser(p, *this, aSID))).first->second;
    }

    if(aSID != AdcCommand::HUB_SID)
        ClientManager::getInstance()->putOnline(ou);
    return *ou;
}

OnlineUser* AdcHub::findUser(const uint32_t aSID) const {
    Lock l(cs);
    SIDMap::const_iterator i = users.find(aSID);
    return i == users.end() ? NULL : i->second;
}

OnlineUser* AdcHub::findUser(const CID& aCID) const {
    Lock l(cs);
    for(SIDMap::const_iterator i = users.begin(); i != users.end(); ++i) {
        if(i->second->getUser()->getCID() == aCID) {
            return i->second;
        }
    }
    return 0;
}

void AdcHub::putUser(const uint32_t aSID, bool disconnect) {
    OnlineUser* ou = 0;
    {
        Lock l(cs);
        SIDIter i = users.find(aSID);
        if(i == users.end())
            return;
        ou = i->second;
        users.erase(i);
    }

    if(aSID != AdcCommand::HUB_SID)
        ClientManager::getInstance()->putOffline(ou, disconnect);

    fire(ClientListener::UserRemoved(), this, *ou);
    delete ou;
}

void AdcHub::clearUsers() {
    SIDMap tmp;
    {
        Lock l(cs);
        users.swap(tmp);
    }

    for(SIDIter i = tmp.begin(); i != tmp.end(); ++i) {
        if(i->first != AdcCommand::HUB_SID)
            ClientManager::getInstance()->putOffline(i->second);
        delete i->second;
    }
}

void AdcHub::handle(AdcCommand::INF, AdcCommand& c) throw() {
    if(c.getParameters().empty())
        return;

    string cid;

    OnlineUser* u = 0;
    if(c.getParam("ID", 0, cid)) {
        u = findUser(CID(cid));
        if(u) {
            if(u->getIdentity().getSID() != c.getFrom()) {
                // Same CID but different SID not allowed - buggy hub?
                string nick;
                if(!c.getParam("NI", 0, nick)) {
                    nick = "[nick unknown]";
                }
                fire(ClientListener::StatusMessage(), this, str(F_("%1% (%2%) has same CID {%3%} as %4% (%5%), ignoring")
                    % u->getIdentity().getNick() % u->getIdentity().getSIDString() % cid % nick % AdcCommand::fromSID(c.getFrom())),
                                        ClientListener::FLAG_IS_SPAM);
                return;
            }
        } else {
            u = &getUser(c.getFrom(), CID(cid));
        }
    } else if(c.getFrom() == AdcCommand::HUB_SID) {
        u = &getUser(c.getFrom(), CID());
    } else {
        u = findUser(c.getFrom());
    }

    if(!u) {
        dcdebug("AdcHub::INF Unknown user / no ID\n");
        return;
    }

    for(StringIterC i = c.getParameters().begin(); i != c.getParameters().end(); ++i) {
        if(i->length() < 2)
            continue;

        u->getIdentity().set(i->c_str(), i->substr(2));
    }

    if(u->getIdentity().isBot()) {
        u->getUser()->setFlag(User::BOT);
    } else {
        u->getUser()->unsetFlag(User::BOT);
    }

    if(u->getIdentity().supports(ADCS_FEATURE)) {
        u->getUser()->setFlag(User::TLS);
    }

    if(!u->getIdentity().get("US").empty()) {
        u->getIdentity().setConnection(str(F_("%1%/s") % Util::formatBytes(u->getIdentity().get("US"))));
    }

    if(u->getUser() == getMyIdentity().getUser()) {
        state = STATE_NORMAL;
        setAutoReconnect(true);
        setMyIdentity(u->getIdentity());
        updateCounts(false);
    }

    if(u->getIdentity().isHub()) {
        setHubIdentity(u->getIdentity());
        fire(ClientListener::HubUpdated(), this);
    } else {
        fire(ClientListener::UserUpdated(), this, *u);
    }
}

void AdcHub::handle(AdcCommand::SUP, AdcCommand& c) throw() {
    if(state != STATE_PROTOCOL) /** @todo SUP changes */
        return;
    bool baseOk = false;
    bool tigrOk = false;
    for(StringIter i = c.getParameters().begin(); i != c.getParameters().end(); ++i) {
        if(*i == BAS0_SUPPORT) {
            baseOk = true;
            tigrOk = true;
        } else if(*i == BASE_SUPPORT) {
            baseOk = true;
        } else if(*i == TIGR_SUPPORT) {
            tigrOk = true;
        }
    }

    if(!baseOk) {
        fire(ClientListener::StatusMessage(), this, _("Failed to negotiate base protocol"));
        disconnect(false);
        return;
    } else if(!tigrOk) {
        oldPassword = true;
        // Some hubs fake BASE support without TIGR support =/
        fire(ClientListener::StatusMessage(), this, _("Hub probably uses an old version of ADC, please encourage the owner to upgrade"));
    }
}

void AdcHub::handle(AdcCommand::SID, AdcCommand& c) throw() {
    if(state != STATE_PROTOCOL) {
        dcdebug("Invalid state for SID\n");
        return;
    }

    if(c.getParameters().empty())
        return;

    sid = AdcCommand::toSID(c.getParam(0));

    state = STATE_IDENTIFY;
    info(true);
}

void AdcHub::handle(AdcCommand::MSG, AdcCommand& c) throw() {
    if(c.getParameters().empty())
        return;

        ChatMessage message = { c.getParam(0), findUser(c.getFrom()) };

        if(!message.from)
            return;

        string temp;
        if(c.getParam("PM", 1, temp)) { // add PM<group-cid> as well
                message.to = findUser(c.getTo());
                if(!message.to)
            return;

                message.replyTo = findUser(AdcCommand::toSID(temp));
                if(!message.replyTo)
                        return;
    }

        message.thirdPerson = c.hasFlag("ME", 1);

        if(c.getParam("TS", 1, temp))
                message.timestamp = Util::toInt64(temp);

        fire(ClientListener::Message(), this, message);
}

void AdcHub::handle(AdcCommand::GPA, AdcCommand& c) throw() {
    if(c.getParameters().empty())
        return;
    salt = c.getParam(0);
    state = STATE_VERIFY;

    fire(ClientListener::GetPassword(), this);
}

void AdcHub::handle(AdcCommand::QUI, AdcCommand& c) throw() {
    uint32_t s = AdcCommand::toSID(c.getParam(0));

    OnlineUser* victim = findUser(s);
        if(victim) {

    string tmp;
    if(c.getParam("MS", 1, tmp)) {
        OnlineUser* source = 0;
        string tmp2;
        if(c.getParam("ID", 1, tmp2)) {
            source = findUser(AdcCommand::toSID(tmp2));
        }

        if(source) {
            tmp = str(F_("%1% was kicked by %2%: %3%") % victim->getIdentity().getNick() %
                source->getIdentity().getNick() % tmp);
        } else {
            tmp = str(F_("%1% was kicked: %2%") % victim->getIdentity().getNick() % tmp);
        }
        fire(ClientListener::StatusMessage(), this, tmp, ClientListener::FLAG_IS_SPAM);
    }

    putUser(s, c.getParam("DI", 1, tmp));
        }

    if(s == sid) {
                // this QUI is directed to us

                string tmp;
        if(c.getParam("TL", 1, tmp)) {
            if(tmp == "-1") {
                setAutoReconnect(false);
            } else {
                setAutoReconnect(true);
                setReconnDelay(Util::toUInt32(tmp));
            }
        }
                if(!victim && c.getParam("MS", 1, tmp)) {
                        fire(ClientListener::StatusMessage(), this, tmp, ClientListener::FLAG_NORMAL);
                }
        if(c.getParam("RD", 1, tmp)) {
            fire(ClientListener::Redirect(), this, tmp);
        }
    }
}

void AdcHub::handle(AdcCommand::CTM, AdcCommand& c) throw() {
    OnlineUser* u = findUser(c.getFrom());
    if(!u || u->getUser() == ClientManager::getInstance()->getMe())
        return;
        if(c.getParameters().size() < 3)
        return;

    const string& protocol = c.getParam(0);
    const string& port = c.getParam(1);
    const string& token = c.getParam(2);

    bool secure = false;
    if(protocol == CLIENT_PROTOCOL) {
        // Nothing special
    } else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
        secure = true;
    } else {
        unknownProtocol(c.getFrom(), protocol, token);
        return;
    }

    if(!u->getIdentity().isTcpActive()) {
        send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "IP unknown", AdcCommand::TYPE_DIRECT).setTo(c.getFrom()));
        return;
    }

    ConnectionManager::getInstance()->adcConnect(*u, static_cast<uint16_t>(Util::toInt(port)), token, secure);
}

void AdcHub::handle(AdcCommand::RCM, AdcCommand& c) throw() {
    if(c.getParameters().size() < 2) {
        return;
    }

    OnlineUser* u = findUser(c.getFrom());
    if(!u || u->getUser() == ClientManager::getInstance()->getMe())
        return;

    const string& protocol = c.getParam(0);
    const string& token = c.getParam(1);

    bool secure;
    if(protocol == CLIENT_PROTOCOL) {
        secure = false;
    } else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
        secure = true;
    } else {
        unknownProtocol(c.getFrom(), protocol, token);
        return;
    }

   if(isActive()) {
       connect(*u, token, secure);
       return;
   }

    if (!u->getIdentity().supports(NAT0_FEATURE) && !BOOLSETTING(ALLOW_NATT))
       return;

   // Attempt to traverse NATs and/or firewalls with TCP.
   // If they respond with their own, symmetric, RNT command, both
   // clients call ConnectionManager::adcConnect.
   send(AdcCommand(AdcCommand::CMD_NAT, u->getIdentity().getSID(), AdcCommand::TYPE_DIRECT).
           addParam(protocol).addParam(Util::toString(sock->getLocalPort())).addParam(token));
   return;
}

void AdcHub::handle(AdcCommand::CMD, AdcCommand& c) throw() {
    if(c.getParameters().size() < 1)
        return;
    const string& name = c.getParam(0);
    bool rem = c.hasFlag("RM", 1);
    if(rem) {
        fire(ClientListener::HubUserCommand(), this, (int)UserCommand::TYPE_REMOVE, 0, name, Util::emptyString);
        return;
    }
    bool sep = c.hasFlag("SP", 1);
    string sctx;
    if(!c.getParam("CT", 1, sctx))
        return;
    int ctx = Util::toInt(sctx);
    if(ctx <= 0)
        return;
    if(sep) {
        fire(ClientListener::HubUserCommand(), this, (int)UserCommand::TYPE_SEPARATOR, ctx, name, Util::emptyString);
        return;
    }
    bool once = c.hasFlag("CO", 1);
    string txt;
    if(!c.getParam("TT", 1, txt))
        return;
    fire(ClientListener::HubUserCommand(), this, (int)(once ? UserCommand::TYPE_RAW_ONCE : UserCommand::TYPE_RAW), ctx, name, txt);
}

void AdcHub::sendUDP(const AdcCommand& cmd) throw() {
    string command;
    string ip;
    uint16_t port;
    {
        Lock l(cs);
        SIDMap::const_iterator i = users.find(cmd.getTo());
        if(i == users.end()) {
            dcdebug("AdcHub::sendUDP: invalid user\n");
            return;
        }
        OnlineUser& ou = *i->second;
        if(!ou.getIdentity().isUdpActive()) {
            return;
        }
        ip = ou.getIdentity().getIp();
        port = static_cast<uint16_t>(Util::toInt(ou.getIdentity().getUdpPort()));
        command = cmd.toString(ou.getUser()->getCID());
    }
    try {
        udp.writeTo(ip, port, command);
    } catch(const SocketException& e) {
        dcdebug("AdcHub::sendUDP: write failed: %s\n", e.getError().c_str());
        udp.close();
    }
}

void AdcHub::handle(AdcCommand::STA, AdcCommand& c) throw() {
    if(c.getParameters().size() < 2)
        return;

        OnlineUser* u = c.getFrom() == AdcCommand::HUB_SID ? &getUser(c.getFrom(), CID()) : findUser(c.getFrom());
    if(!u)
        return;

    //int severity = Util::toInt(c.getParam(0).substr(0, 1));
    if(c.getParam(0).size() != 3) {
        return;
    }

        switch(Util::toInt(c.getParam(0).substr(1))) {

        case AdcCommand::ERROR_BAD_PASSWORD:
                {
        setPassword(Util::emptyString);
                        break;
                }

        case AdcCommand::ERROR_COMMAND_ACCESS:
                {
                        string tmp;
                        if(c.getParam("FC", 1, tmp) && tmp.size() == 4)
                                forbiddenCommands.insert(AdcCommand::toFourCC(tmp.c_str()));
                        break;
                }

        case AdcCommand::ERROR_PROTOCOL_UNSUPPORTED:
                {
                string tmp;
                if(c.getParam("PR", 1, tmp)) {
                    if(tmp == CLIENT_PROTOCOL) {
                        u->getUser()->setFlag(User::NO_ADC_1_0_PROTOCOL);
                    } else if(tmp == SECURE_CLIENT_PROTOCOL_TEST) {
                        u->getUser()->setFlag(User::NO_ADCS_0_10_PROTOCOL);
                        u->getUser()->unsetFlag(User::TLS);
                    }
                // Try again...
                ConnectionManager::getInstance()->force(u->getUser());
                }
                return;
                }
        }
    ChatMessage message = { c.getParam(1), u };
    fire(ClientListener::Message(), this, message);
}

void AdcHub::handle(AdcCommand::SCH, AdcCommand& c) throw() {
    OnlineUser* ou = findUser(c.getFrom());
    if(!ou) {
        dcdebug("Invalid user in AdcHub::onSCH\n");
        return;
    }

    fire(ClientListener::AdcSearch(), this, c, ou->getUser()->getCID());
}

void AdcHub::handle(AdcCommand::RES, AdcCommand& c) throw() {
    OnlineUser* ou = findUser(c.getFrom());
    if(!ou) {
        dcdebug("Invalid user in AdcHub::onRES\n");
        return;
    }
    SearchManager::getInstance()->onRES(c, ou->getUser());
}

void AdcHub::handle(AdcCommand::PSR, AdcCommand& c) throw() {
    OnlineUser* ou = findUser(c.getFrom());
    if(!ou) {
        dcdebug("Invalid user in AdcHub::onPSR\n");
        return;
    }
    SearchManager::getInstance()->onPSR(c, ou->getUser());
}

void AdcHub::handle(AdcCommand::GET, AdcCommand& c) throw() {
    if(c.getParameters().size() < 5) {
        if(c.getParameters().size() > 0) {
            if(c.getParam(0) == "blom") {
                send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC,
                        "Too few parameters for blom", AdcCommand::TYPE_HUB));
            } else {
                send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
                        "Unknown transfer type", AdcCommand::TYPE_HUB));
            }
        } else {
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC,
                    "Too few parameters for GET", AdcCommand::TYPE_HUB));
        }
        return;
    }
    const string& type = c.getParam(0);
    string sk, sh;
    if(type == "blom" && c.getParam("BK", 4, sk) && c.getParam("BH", 4, sh))  {
        ByteVector v;
        size_t m = Util::toUInt32(c.getParam(3)) * 8;
        size_t k = Util::toUInt32(sk);
        size_t h = Util::toUInt32(sh);

        if(k > 8 || k < 1) {
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
                            "Unsupported k", AdcCommand::TYPE_HUB));
            return;
        }
        if(h > 64 || h < 1) {
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
                            "Unsupported h", AdcCommand::TYPE_HUB));
            return;
        }
        size_t n = ShareManager::getInstance()->getSharedFiles();

        // Ideal size for m is n * k / ln(2), but we allow some slack
        if(m > (5 * Util::roundUp((int64_t)(n * k / log(2.)), (int64_t)64)) || m > static_cast<size_t>(1 << h)) {
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC,
                            "Unsupported m", AdcCommand::TYPE_HUB));
            return;
        }
        if (m > 0) {
            ShareManager::getInstance()->getBloom(v, k, m, h);
        }
        AdcCommand cmd(AdcCommand::CMD_SND, AdcCommand::TYPE_HUB);
        cmd.addParam(c.getParam(0));
        cmd.addParam(c.getParam(1));
        cmd.addParam(c.getParam(2));
        cmd.addParam(c.getParam(3));
        cmd.addParam(c.getParam(4));
        send(cmd);
        if (m > 0) {
            send((char*)&v[0], v.size());
        }
    }
}
void AdcHub::handle(AdcCommand::NAT, AdcCommand& c) throw() {
    if (!BOOLSETTING(ALLOW_NATT))
        return;

       OnlineUser* u = findUser(c.getFrom());
       if(!u || u->getUser() == ClientManager::getInstance()->getMe() || c.getParameters().size() < 3)
               return;

       const string& protocol = c.getParam(0);
       const string& port = c.getParam(1);
       const string& token = c.getParam(2);

       // bool secure = secureAvail(c.getFrom(), protocol, token);
       bool secure = false;
       if(protocol == CLIENT_PROTOCOL) {
               // Nothing special
       } else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
               secure = true;
       } else {
               unknownProtocol(c.getFrom(), protocol, token);
               return;
       }

       // Trigger connection attempt sequence locally ...
       dcdebug("triggering connecting attempt in NAT: remote port = %s, local IP = %s, local port = %d\n", port.c_str(), sock->getLocalIp().c_str(), sock->getLocalPort());
       ConnectionManager::getInstance()->adcConnect(*u, static_cast<uint16_t>(Util::toInt(port)), sock->getLocalPort(), BufferedSocket::NAT_CLIENT, token, secure);

       // ... and signal other client to do likewise.
       send(AdcCommand(AdcCommand::CMD_RNT, u->getIdentity().getSID(), AdcCommand::TYPE_DIRECT).addParam(protocol).
               addParam(Util::toString(sock->getLocalPort())).addParam(token));
}

void AdcHub::handle(AdcCommand::RNT, AdcCommand& c) throw() {
       // Sent request for NAT traversal cooperation, which
       // was acknowledged (with requisite local port information).
       	if(!BOOLSETTING(ALLOW_NATT))
            return;

       OnlineUser* u = findUser(c.getFrom());
       if(!u || u->getUser() == ClientManager::getInstance()->getMe() || c.getParameters().size() < 3)
               return;

       const string& protocol = c.getParam(0);
       const string& port = c.getParam(1);
       const string& token = c.getParam(2);

       bool secure = false;
       if(protocol == CLIENT_PROTOCOL) {
               // Nothing special
       } else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
               secure = true;
       } else {
               unknownProtocol(c.getFrom(), protocol, token);
               return;
       }

       // Trigger connection attempt sequence locally
       dcdebug("triggering connecting attempt in RNT: remote port = %s, local IP = %s, local port = %d\n", port.c_str(), sock->getLocalIp().c_str(), sock->getLocalPort());
       ConnectionManager::getInstance()->adcConnect(*u, static_cast<uint16_t>(Util::toInt(port)), sock->getLocalPort(), BufferedSocket::NAT_SERVER, token, secure);
}

void AdcHub::connect(const OnlineUser& user, const string& token) {
    connect(user, token, CryptoManager::getInstance()->TLSOk() && user.getUser()->isSet(User::TLS));
}

void AdcHub::connect(const OnlineUser& user, string const& token, bool secure) {
    if(state != STATE_NORMAL)
        return;

    const string* proto;
    if(secure) {
        if(user.getUser()->isSet(User::NO_ADCS_0_10_PROTOCOL)) {
            /// @todo log
            return;
        }
        proto = &SECURE_CLIENT_PROTOCOL_TEST;
    } else {
        if(user.getUser()->isSet(User::NO_ADC_1_0_PROTOCOL)) {
            /// @todo log
            return;
        }
        proto = &CLIENT_PROTOCOL;
    }

    if(isActive()) {
        uint16_t port = secure ? ConnectionManager::getInstance()->getSecurePort() : ConnectionManager::getInstance()->getPort();
        if(port == 0) {
            // Oops?
            LogManager::getInstance()->message(str(F_("Not listening for connections - please restart %1%") % APPNAME));
            return;
        }
        send(AdcCommand(AdcCommand::CMD_CTM, user.getIdentity().getSID(), AdcCommand::TYPE_DIRECT).addParam(*proto).addParam(Util::toString(port)).addParam(token));
    } else {
        send(AdcCommand(AdcCommand::CMD_RCM, user.getIdentity().getSID(), AdcCommand::TYPE_DIRECT).addParam(*proto).addParam(token));
    }
}

void AdcHub::hubMessage(const string& aMessage, bool thirdPerson) {
    if(state != STATE_NORMAL)
        return;
    AdcCommand c(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST);
    c.addParam(aMessage);
    if(thirdPerson)
        c.addParam("ME", "1");
    send(c);
}

void AdcHub::privateMessage(const OnlineUser& user, const string& aMessage, bool thirdPerson) {
    if(state != STATE_NORMAL)
        return;
    AdcCommand c(AdcCommand::CMD_MSG, user.getIdentity().getSID(), AdcCommand::TYPE_ECHO);
    c.addParam(aMessage);
    if(thirdPerson)
        c.addParam("ME", "1");
    c.addParam("PM", getMySID());
    send(c);
}

void AdcHub::sendUserCmd(const UserCommand& command, const StringMap& params) {
        if(state != STATE_NORMAL)
                return;
        string cmd = Util::formatParams(command.getCommand(), params, false);
        if(command.isChat()) {
                if(command.getTo().empty()) {
                        hubMessage(cmd);
                } else {
                        const string& to = command.getTo();
                        Lock l(cs);
                        for(SIDMap::const_iterator i = users.begin(); i != users.end(); ++i) {
                                if(i->second->getIdentity().getNick() == to) {
                                        privateMessage(*i->second, cmd);
                                        return;
                                }
                        }
                }
        } else {
                send(cmd);
        }
}

const vector<StringList>& AdcHub::getSearchExts() {
    if(!searchExts.empty())
        return searchExts;

    // the list is always immutable except for this function where it is initially being filled.
    vector<StringList>& xSearchExts = const_cast<vector<StringList>&>(searchExts);

    xSearchExts.resize(7);

    /// @todo simplify this as searchExts[0] = { "mp3", "etc" } when VC++ supports initializer lists
    // these extensions *must* be sorted alphabetically!

    {
        StringList& l = xSearchExts[0];
        l.push_back("ape"); l.push_back("flac"); l.push_back("m4a"); l.push_back("mid");
        l.push_back("mp3"); l.push_back("mpc"); l.push_back("ogg"); l.push_back("ra");
        l.push_back("wav"); l.push_back("wma");
    }

    {
        StringList& l = xSearchExts[1];
        l.push_back("7z"); l.push_back("ace"); l.push_back("arj"); l.push_back("bz2");
        l.push_back("gz"); l.push_back("lha"); l.push_back("lzh"); l.push_back("rar");
        l.push_back("tar"); l.push_back("z"); l.push_back("zip");
    }

    {
        StringList& l = xSearchExts[2];
        l.push_back("doc"); l.push_back("docx"); l.push_back("htm"); l.push_back("html");
        l.push_back("nfo"); l.push_back("odf"); l.push_back("odp"); l.push_back("ods");
        l.push_back("odt"); l.push_back("pdf"); l.push_back("ppt"); l.push_back("pptx");
        l.push_back("rtf"); l.push_back("txt"); l.push_back("xls"); l.push_back("xlsx");
        l.push_back("xml"); l.push_back("xps");
    }

    {
        StringList& l = xSearchExts[3];
        l.push_back("app"); l.push_back("bat"); l.push_back("cmd"); l.push_back("com");
        l.push_back("dll"); l.push_back("exe"); l.push_back("jar"); l.push_back("msi");
        l.push_back("ps1"); l.push_back("vbs"); l.push_back("wsf");
    }

    {
        StringList& l = xSearchExts[4];
        l.push_back("bmp"); l.push_back("cdr"); l.push_back("eps"); l.push_back("gif");
        l.push_back("ico"); l.push_back("img"); l.push_back("jpeg"); l.push_back("jpg");
        l.push_back("png"); l.push_back("ps"); l.push_back("psd"); l.push_back("sfw");
        l.push_back("tga"); l.push_back("tif"); l.push_back("webp");
    }

    {
        StringList& l = xSearchExts[5];
        l.push_back("3gp"); l.push_back("asf"); l.push_back("asx"); l.push_back("avi");
        l.push_back("divx"); l.push_back("flv"); l.push_back("mkv"); l.push_back("mov");
        l.push_back("mp4"); l.push_back("mpeg"); l.push_back("mpg"); l.push_back("ogm");
        l.push_back("pxp"); l.push_back("qt"); l.push_back("rm"); l.push_back("rmvb");
        l.push_back("swf"); l.push_back("vob"); l.push_back("webm"); l.push_back("wmv");
    }

    {
        StringList& l = xSearchExts[6];
        l.push_back("iso"); l.push_back("mdf"); l.push_back("mds"); l.push_back("nrg");
        l.push_back("vcd"); l.push_back("bwt"); l.push_back("ccd"); l.push_back("cdi");
        l.push_back("pdi"); l.push_back("cue"); l.push_back("isz"); l.push_back("img");
        l.push_back("vc4");
    }
    return searchExts;
}

StringList AdcHub::parseSearchExts(int flag) {
    StringList ret;
    const vector<StringList>& searchExts = getSearchExts();
    for(std::vector<StringList>::const_iterator i = searchExts.begin(), iend = searchExts.end(); i != iend; ++i) {
        if(flag & (1 << (i - searchExts.begin()))) {
            ret.insert(ret.begin(), i->begin(), i->end());
        }
    }
    return ret;
}

void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList) {
    if(state != STATE_NORMAL)
        return;

    AdcCommand c(AdcCommand::CMD_SCH, AdcCommand::TYPE_BROADCAST);

    if(!aToken.empty())
        c.addParam("TO", aToken);

    if(aFileType == SearchManager::TYPE_TTH) {
        c.addParam("TR", aString);
    } else {
        if(aSizeMode == SearchManager::SIZE_ATLEAST) {
            c.addParam("GE", Util::toString(aSize));
        } else if(aSizeMode == SearchManager::SIZE_ATMOST) {
            c.addParam("LE", Util::toString(aSize));
        }
        StringTokenizer<string> st(aString, ' ');
        for(StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
            c.addParam("AN", *i);
        }
        if(aFileType == SearchManager::TYPE_DIRECTORY) {
            c.addParam("TY", "2");
        }

        if(!aExtList.empty()) {
            StringList exts = aExtList;

            if(exts.size() > 2) {
                sort(exts.begin(), exts.end());

                uint8_t gr = 0;
                StringList rx;

                const vector<StringList>& searchExts = getSearchExts();
                for(std::vector<StringList>::const_iterator i = searchExts.begin(), iend = searchExts.end(); i != iend; ++i) {
                    const StringList& def = *i;

                    // gather the exts not present in any of the lists
                    StringList temp(def.size() + exts.size());
                    temp = StringList(temp.begin(), set_symmetric_difference(def.begin(), def.end(),
                        exts.begin(), exts.end(), temp.begin()));

                    // figure out whether the remaining exts have to be added or removed from the set
                    StringList rm;
                    bool ok = true;
                    for(StringIter diff = temp.begin(); diff != temp.end();) {
                        if(find(def.begin(), def.end(), *diff) == def.end()) {
                            ++diff; // will be added further below as an "EX"
                        } else {
                            if(rm.size() == 2) {
                                ok = false;
                                break;
                            }
                            rm.push_back(*diff);
                            diff = temp.erase(diff);
                        }
                    }
                    if(!ok) // too many "RX"s necessary - disregard this group
                        continue;

                    // let's include this group!
                    gr += 1 << (i - searchExts.begin());

                    exts = temp; // the exts to still add (that were not defined in the group)

                    rx.insert(rx.begin(), rm.begin(), rm.end());

                    if(exts.size() <= 2)
                        break;
                    // keep looping to see if there are more exts that can be grouped
                }

                if(gr) {
                    // some extensions can be grouped; let's send a command with grouped exts.
                    AdcCommand c_gr(AdcCommand::CMD_SCH, AdcCommand::TYPE_FEATURE);
                    c_gr.setFeatures('+' + SEGA_FEATURE);

                    const StringList& params = c.getParameters();
                    for(StringIterC i = params.begin(), iend = params.end(); i != iend; ++i)
                        c_gr.addParam(*i);

                    for(StringIterC i = exts.begin(), iend = exts.end(); i != iend; ++i)
                        c_gr.addParam("EX", *i);
                    c_gr.addParam("GR", Util::toString(gr));
                    for(StringIterC i = rx.begin(), iend = rx.end(); i != iend; ++i)
                        c_gr.addParam("RX", *i);

                    sendSearch(c_gr);

                    // make sure users with the feature don't receive the search twice.
                    c.setType(AdcCommand::TYPE_FEATURE);
                    c.setFeatures('-' + SEGA_FEATURE);
                }
            }

            for(StringIterC i = aExtList.begin(); i != aExtList.end(); ++i)
                c.addParam("EX", *i);
        }
    }

    sendSearch(c);
}
void AdcHub::sendSearch(AdcCommand& c) {
    if(isActive()) {
        send(c);
    } else {
        string features = c.getFeatures();
        c.setType(AdcCommand::TYPE_FEATURE);
        if (BOOLSETTING(ALLOW_NATT)) {
            c.setFeatures(features + '+' + TCP4_FEATURE + '-' + NAT0_FEATURE);
            send(c);
            c.setFeatures(features + '+' + NAT0_FEATURE);
        } else {
            c.setFeatures(features + '+' + TCP4_FEATURE);
        }
        send(c);
    }
}

void AdcHub::password(const string& pwd) {
    if(state != STATE_VERIFY)
        return;
    if(!salt.empty()) {
        size_t saltBytes = salt.size() * 5 / 8;
        boost::scoped_array<uint8_t> buf(new uint8_t[saltBytes]);
        Encoder::fromBase32(salt.c_str(), &buf[0], saltBytes);
        TigerHash th;
        if(oldPassword) {
            CID cid = getMyIdentity().getUser()->getCID();
            th.update(cid.data(), CID::SIZE);
        }
        th.update(pwd.data(), pwd.length());
        th.update(&buf[0], saltBytes);
        send(AdcCommand(AdcCommand::CMD_PAS, AdcCommand::TYPE_HUB).addParam(Encoder::toBase32(th.finalize(), TigerHash::BYTES)));
        salt.clear();
    }
}

static void addParam(StringMap& lastInfoMap, AdcCommand& c, const string& var, const string& value) {
    StringMapIter i = lastInfoMap.find(var);

    if(i != lastInfoMap.end()) {
        if(i->second != value) {
            if(value.empty()) {
                lastInfoMap.erase(i);
            } else {
                i->second = value;
            }
            c.addParam(var, value);
        }
    } else if(!value.empty()) {
        lastInfoMap.insert(make_pair(var, value));
        c.addParam(var, value);
    }
}

void AdcHub::info(bool /*alwaysSend*/) {
    if(state != STATE_IDENTIFY && state != STATE_NORMAL)
        return;

    reloadSettings(false);

    AdcCommand c(AdcCommand::CMD_INF, AdcCommand::TYPE_BROADCAST);
    if (state == STATE_NORMAL) {
    updateCounts(false);
    }
    addParam(lastInfoMap, c, "ID", ClientManager::getInstance()->getMyCID().toBase32());
    addParam(lastInfoMap, c, "PD", ClientManager::getInstance()->getMyPID().toBase32());
    addParam(lastInfoMap, c, "NI", getCurrentNick());
    addParam(lastInfoMap, c, "DE", getCurrentDescription());
    addParam(lastInfoMap, c, "SL", Util::toString(SETTING(SLOTS)));
    addParam(lastInfoMap, c, "FS", Util::toString(UploadManager::getInstance()->getFreeSlots()));
    addParam(lastInfoMap, c, "SS", ShareManager::getInstance()->getShareSizeString());
    addParam(lastInfoMap, c, "SF", Util::toString(ShareManager::getInstance()->getSharedFiles()));
    addParam(lastInfoMap, c, "EM", SETTING(EMAIL));
    addParam(lastInfoMap, c, "HN", Util::toString(counts.normal));
    addParam(lastInfoMap, c, "HR", Util::toString(counts.registered));
    addParam(lastInfoMap, c, "HO", Util::toString(counts.op));
    addParam(lastInfoMap, c, "VE", getClientId().c_str());
    addParam(lastInfoMap, c, "AW", Util::getAway() ? "1" : Util::emptyString);
    int limit = ThrottleManager::getInstance()->getDownLimit();
    if (limit > 0 && BOOLSETTING(THROTTLE_ENABLE)) {
        addParam(lastInfoMap, c, "DS", Util::toString(limit * 1024));
    } else {
         addParam(lastInfoMap, c, "DS", Util::emptyString);
    }
    limit = ThrottleManager::getInstance()->getUpLimit();
    if (limit > 0 && BOOLSETTING(THROTTLE_ENABLE)) {
        addParam(lastInfoMap, c, "US", Util::toString(limit * 1024));
    } else {
        addParam(lastInfoMap, c, "US", Util::toString((long)(Util::toDouble(SETTING(UPLOAD_SPEED))*1024*1024/8)));
    }

    string su(SEGA_FEATURE);
    if(CryptoManager::getInstance()->TLSOk()) {
        su += "," + ADCS_FEATURE;
        const vector<uint8_t> &kp = CryptoManager::getInstance()->getKeyprint();
        addParam(lastInfoMap, c, "KP", "SHA256/" + Encoder::toBase32(&kp[0], kp.size()));
    }

    if (!getFavIp().empty()) {
        addParam(lastInfoMap, c, "I4", getFavIp());
   } else if(BOOLSETTING(NO_IP_OVERRIDE) && !SETTING(EXTERNAL_IP).empty()) {
           addParam(lastInfoMap, c, "I4", Socket::resolve(SETTING(EXTERNAL_IP)));
   } else {
           addParam(lastInfoMap, c, "I4", "0.0.0.0");
   }

   if(isActive()) {
           addParam(lastInfoMap, c, "U4", Util::toString(SearchManager::getInstance()->getPort()));
           su += "," + TCP4_FEATURE;
            su += "," + UDP4_FEATURE;
   } else {
        if (BOOLSETTING(ALLOW_NATT))
            su += "," + NAT0_FEATURE;
        else
            addParam(lastInfoMap, c, "I4", "");
        addParam(lastInfoMap, c, "U4", "");
   }

    addParam(lastInfoMap, c, "SU", su);

    if(c.getParameters().size() > 0) {
        send(c);
    }
}

int64_t AdcHub::getAvailable() const {
    Lock l(cs);
    int64_t x = 0;
    for(SIDMap::const_iterator i = users.begin(); i != users.end(); ++i) {
        x+=i->second->getIdentity().getBytesShared();
    }
    return x;
}

string AdcHub::checkNick(const string& aNick) {
    string tmp = aNick;
    for(size_t i = 0; i < aNick.size(); ++i) {
        if(static_cast<uint8_t>(tmp[i]) <= 32) {
            tmp[i] = '_';
        }
    }
    return tmp;
}

void AdcHub::send(const AdcCommand& cmd) {
    if(forbiddenCommands.find(AdcCommand::toFourCC(cmd.getFourCC().c_str())) == forbiddenCommands.end()) {
        if(cmd.getType() == AdcCommand::TYPE_UDP)
            sendUDP(cmd);
        send(cmd.toString(sid));
    }
}

void AdcHub::unknownProtocol(uint32_t target, const string& protocol, const string& token) {
       AdcCommand cmd(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_DIRECT);
       cmd.setTo(target);
       cmd.addParam("PR", protocol);
       cmd.addParam("TO", token);

       send(cmd);
}

void AdcHub::on(Connected c) throw() {
    Client::on(c);

    lastInfoMap.clear();
    sid = 0;
    forbiddenCommands.clear();

    AdcCommand cmd(AdcCommand::CMD_SUP, AdcCommand::TYPE_HUB);
    cmd.addParam(BAS0_SUPPORT).addParam(BASE_SUPPORT).addParam(TIGR_SUPPORT);

    if(BOOLSETTING(HUB_USER_COMMANDS)) {
        cmd.addParam(UCM0_SUPPORT);
    }
    if(BOOLSETTING(SEND_BLOOM)) {
        cmd.addParam(BLO0_SUPPORT);
    }
    send(cmd);
}

void AdcHub::on(Line l, const string& aLine) throw() {
    Client::on(l, aLine);

    if(!Text::validateUtf8(aLine)) {
        // @todo report to user?
        return;
    }

    if(BOOLSETTING(ADC_DEBUG)) {
        fire(ClientListener::StatusMessage(), this, "<ADC>" + aLine + "</ADC>");
    }
#ifdef LUA_SCRIPT
    if (onClientMessage(this, aLine))
        return;
#endif
    dispatch(aLine);
}

void AdcHub::on(Failed f, const string& aLine) throw() {
    clearUsers();
    Client::on(f, aLine);
}

void AdcHub::on(Second s, uint64_t aTick) throw() {
    Client::on(s, aTick);
    if(state == STATE_NORMAL && (aTick > (getLastActivity() + 120*1000)) ) {
        send("\n", 1);
    }
}
#ifdef LUA_SCRIPT
//aded
bool AdcScriptInstance::onClientMessage(AdcHub* aClient, const string& aLine) {
        Lock l(cs);
        MakeCall("adch", "DataArrival", 1, aClient, aLine);
        return GetLuaBool();

}
//end
#endif
} // namespace dcpp
