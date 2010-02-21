/*
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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

#include <math.h>

namespace dcpp {

const string AdcHub::CLIENT_PROTOCOL("ADC/1.0");
const string AdcHub::CLIENT_PROTOCOL_TEST("ADC/0.10");
const string AdcHub::SECURE_CLIENT_PROTOCOL_TEST("ADCS/0.10");
const string AdcHub::ADCS_FEATURE("ADC0");
const string AdcHub::TCP4_FEATURE("TCP4");
const string AdcHub::UDP4_FEATURE("UDP4");
const string AdcHub::BASE_SUPPORT("ADBASE");
const string AdcHub::BAS0_SUPPORT("ADBAS0");
const string AdcHub::TIGR_SUPPORT("ADTIGR");
const string AdcHub::UCM0_SUPPORT("ADUCM0");
const string AdcHub::BLO0_SUPPORT("ADBLO0");

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
                    % u->getIdentity().getNick() % u->getIdentity().getSIDString() % cid % nick % AdcCommand::fromSID(c.getFrom())));
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

    OnlineUser* from = findUser(c.getFrom());
    if(!from)
        return;

    string pmFrom;
    if(c.getParam("PM", 1, pmFrom)) { // add PM<group-cid> as well
        OnlineUser* to = findUser(c.getTo());
        if(!to)
            return;

        OnlineUser* replyTo = findUser(AdcCommand::toSID(pmFrom));
        if(!replyTo)
            return;

        fire(ClientListener::PrivateMessage(), this, *from, *to, *replyTo, c.getParam(0), c.hasFlag("ME", 1));
    } else {
        fire(ClientListener::Message(), this, *from, c.getParam(0), c.hasFlag("ME", 1));
    }
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
    if(!victim) {
        return;
    }

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

    if(s == sid) {
        if(c.getParam("TL", 1, tmp)) {
            if(tmp == "-1") {
                setAutoReconnect(false);
            } else {
                setAutoReconnect(true);
                setReconnDelay(Util::toUInt32(tmp));
            }
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
    if(c.getParameters().size() < 2)
        return;

    const string& protocol = c.getParam(0);
    const string& port = c.getParam(1);

    string token;
    if(c.getParameters().size() == 3) {
        const string& tok = c.getParam(2);

        // 0.699 put TO before the token, keep this bug fix for a while
        if(tok.compare(0, 2, "TO") == 0) {
            token = tok.substr(2);
        } else {
            token = tok;
        }
    } else {
        // <= 0.703 would send an empty token for passive connections when replying to RCM
    }

    bool secure = false;
    if(protocol == CLIENT_PROTOCOL || protocol == CLIENT_PROTOCOL_TEST) {
        // Nothing special
    } else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
        secure = true;
    } else {
        AdcCommand cmd(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_DIRECT);
        cmd.setTo(c.getFrom());
        cmd.addParam("PR", protocol);
        cmd.addParam("TO", token);

        send(cmd);
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
    if(!ClientManager::getInstance()->isActive())
        return;
    OnlineUser* u = findUser(c.getFrom());
    if(!u || u->getUser() == ClientManager::getInstance()->getMe())
        return;

    const string& protocol = c.getParam(0);
    const string& tok = c.getParam(1);
    string token;
    // 0.699 sent a token with "TO" prefix
    if(tok.compare(0, 2, "TO") == 0) {
        token = tok.substr(2);
    } else {
        token = tok;
    }

    bool secure;
    if(protocol == CLIENT_PROTOCOL || protocol == CLIENT_PROTOCOL_TEST) {
        secure = false;
    } else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk()) {
        secure = true;
    } else {
        AdcCommand cmd(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_DIRECT);
        cmd.setTo(c.getFrom());
        cmd.addParam("PR", protocol);
        cmd.addParam("TO", token);

        send(cmd);
        return;
    }
    connect(*u, token, secure);
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

    OnlineUser* u = findUser(c.getFrom());
    if(!u)
        return;

    //int severity = Util::toInt(c.getParam(0).substr(0, 1));
    if(c.getParam(0).size() != 3) {
        return;
    }

    int code = Util::toInt(c.getParam(0).substr(1));

    if(code == AdcCommand::ERROR_BAD_PASSWORD) {
        setPassword(Util::emptyString);
    } else if(code == AdcCommand::ERROR_PROTOCOL_UNSUPPORTED) {
        string tmp;
        if(c.getParam("PR", 1, tmp)) {
            if(tmp == CLIENT_PROTOCOL) {
                u->getUser()->setFlag(User::NO_ADC_1_0_PROTOCOL);
            } else if(tmp == CLIENT_PROTOCOL_TEST) {
                u->getUser()->setFlag(User::NO_ADC_0_10_PROTOCOL);
            } else if(tmp == SECURE_CLIENT_PROTOCOL_TEST) {
                u->getUser()->setFlag(User::NO_ADCS_0_10_PROTOCOL);
                u->getUser()->unsetFlag(User::TLS);
            }
            // Try again...
            ConnectionManager::getInstance()->force(u->getUser());
        }
    }
    fire(ClientListener::Message(), this, *u, c.getParam(1));
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

void AdcHub::handle(AdcCommand::GET, AdcCommand& c) throw() {
    if(c.getParameters().size() < 5) {
        dcdebug("Get with few parameters");
        // TODO return STA?
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
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC, "Unsupported k"));
            return;
        }
        if(h > 64 || h < 1) {
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC, "Unsupported h"));
            return;
        }
        size_t n = ShareManager::getInstance()->getSharedFiles();

        // Ideal size for m is n * k / ln(2), but we allow some slack
        if(m > (5 * n * k / log(2.)) || m > static_cast<size_t>(1 << h)) {
            send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_TRANSFER_GENERIC, "Unsupported m"));
            return;
        }

        ShareManager::getInstance()->getBloom(v, k, m, h);
        AdcCommand cmd(AdcCommand::CMD_SND, AdcCommand::TYPE_HUB);
        cmd.addParam(c.getParam(0));
        cmd.addParam(c.getParam(1));
        cmd.addParam(c.getParam(2));
        cmd.addParam(c.getParam(3));
        cmd.addParam(c.getParam(4));
        send(cmd);
        send((char*)&v[0], v.size());
    }
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
        // dc++ <= 0.703 has a bug which makes it respond with CSTA to the hub if an unrecognised protocol is used *sigh*
        // so we try 0.10 first...
        if(user.getUser()->isSet(User::NO_ADC_0_10_PROTOCOL)) {
            if(user.getUser()->isSet(User::NO_ADC_1_0_PROTOCOL)) {
                /// @todo log
                return;
            }
            proto = &CLIENT_PROTOCOL;
        } else {
            proto = &CLIENT_PROTOCOL_TEST;
        }
    }

    if(ClientManager::getInstance()->isActive()) {
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

void AdcHub::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
    if(state != STATE_NORMAL)
        return;

    AdcCommand c(AdcCommand::CMD_SCH, AdcCommand::TYPE_BROADCAST);

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
    }

    if(!aToken.empty())
        c.addParam("TO", aToken);

    if(ClientManager::getInstance()->isActive()) {
        send(c);
    } else {
        c.setType(AdcCommand::TYPE_FEATURE);
        c.setFeatures("+TCP4");
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

    updateCounts(false);

    addParam(lastInfoMap, c, "ID", ClientManager::getInstance()->getMyCID().toBase32());
    addParam(lastInfoMap, c, "PD", ClientManager::getInstance()->getMyPID().toBase32());
    addParam(lastInfoMap, c, "NI", getCurrentNick());
    addParam(lastInfoMap, c, "DE", getCurrentDescription());
    addParam(lastInfoMap, c, "SL", Util::toString(SETTING(SLOTS)));
    addParam(lastInfoMap, c, "SS", ShareManager::getInstance()->getShareSizeString());
    addParam(lastInfoMap, c, "SF", Util::toString(ShareManager::getInstance()->getSharedFiles()));
    addParam(lastInfoMap, c, "EM", SETTING(EMAIL));
    addParam(lastInfoMap, c, "HN", Util::toString(counts.normal));
    addParam(lastInfoMap, c, "HR", Util::toString(counts.registered));
    addParam(lastInfoMap, c, "HO", Util::toString(counts.op));
    addParam(lastInfoMap, c, "VE", getClientId().c_str());

    addParam(lastInfoMap, c, "US", Util::toString((long)(Util::toDouble(SETTING(UPLOAD_SPEED))*1024*1024/8)));
    addParam(lastInfoMap, c, "AW", Util::getAway() ? "1" : Util::emptyString);

    if(SETTING(MAX_DOWNLOAD_SPEED) > 0) {
        addParam(lastInfoMap, c, "DS", Util::toString((SETTING(MAX_DOWNLOAD_SPEED)*1024*8)));
    } else {
        addParam(lastInfoMap, c, "DS", Util::emptyString);
    }

    string su;
    if(CryptoManager::getInstance()->TLSOk()) {
        su += ADCS_FEATURE + ",";
    }

    if(ClientManager::getInstance()->isActive()) {
        if(BOOLSETTING(NO_IP_OVERRIDE) && !SETTING(EXTERNAL_IP).empty()) {
            addParam(lastInfoMap, c, "I4", Socket::resolve(SETTING(EXTERNAL_IP)));
        } else {
            addParam(lastInfoMap, c, "I4", "0.0.0.0");
        }
        addParam(lastInfoMap, c, "U4", Util::toString(SearchManager::getInstance()->getPort()));
        su += TCP4_FEATURE + ",";
        su += UDP4_FEATURE + ",";
    } else {
        addParam(lastInfoMap, c, "I4", "");
        addParam(lastInfoMap, c, "U4", "");
    }

    if(!su.empty()) {
        su.erase(su.size() - 1);
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
    if(cmd.getType() == AdcCommand::TYPE_UDP)
        sendUDP(cmd);
    send(cmd.toString(sid));
}

void AdcHub::on(Connected c) throw() {
    Client::on(c);

    lastInfoMap.clear();
    sid = 0;

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
    dispatch(aLine);
}

void AdcHub::on(Failed f, const string& aLine) throw() {
    clearUsers();
    Client::on(f, aLine);
}

void AdcHub::on(Second s, uint32_t aTick) throw() {
    Client::on(s, aTick);
    if(state == STATE_NORMAL && (aTick > (getLastActivity() + 120*1000)) ) {
        send("\n", 1);
    }
}

} // namespace dcpp
