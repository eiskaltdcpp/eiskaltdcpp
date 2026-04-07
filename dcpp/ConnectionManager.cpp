/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

#include "ConnectionManager.h"

#include "Client.h"
#include "ClientManager.h"
#include "ConnectivityManager.h"
#include "CryptoManager.h"
#include "DownloadManager.h"
#include "LogManager.h"
#include "QueueManager.h"
#include "UploadManager.h"
#include "UserConnection.h"
#include "DCPlusPlus.h"

namespace dcpp {

ConnectionManager::ConnectionManager(DCContext& ctx) :
    ContextAware(ctx),
    floodCounter(0),
    server(0),
    secureServer(0),
    shuttingDown(false)
{
    this->ctx().getTimerManager()->addListener(this);

    features = {
        UserConnection::FEATURE_MINISLOTS,
        UserConnection::FEATURE_XML_BZLIST,
        UserConnection::FEATURE_ADCGET,
        UserConnection::FEATURE_TTHL,
        UserConnection::FEATURE_TTHF
    };

    adcFeatures = {
        "AD" + UserConnection::FEATURE_ADC_BAS0,
        "AD" + UserConnection::FEATURE_ADC_BASE,
        "AD" + UserConnection::FEATURE_ADC_TIGR,
        "AD" + UserConnection::FEATURE_ADC_BZIP
    };
}

void ConnectionManager::listen() {
    disconnect();

    server = new Server(ctx(), false, Util::toString(CTX_SETTING(TCP_PORT)), CTX_SETTING(BIND_ADDRESS));

    if(!ctx().getCryptoManager()->TLSOk()) {
        dcdebug("Skipping secure port: %d\n", CTX_SETTING(TLS_PORT));
        return;
    }

    secureServer = new Server(ctx(), true, Util::toString(CTX_SETTING(TLS_PORT)), CTX_SETTING(BIND_ADDRESS));
}

ConnectionQueueItem::ConnectionQueueItem(const HintedUser &aUser, bool aDownload) :
    token(Util::toString(Util::rand())),
    lastAttempt(0),
    errors(0),
    state(WAITING),
    download(aDownload),
    user(aUser)
{
}

/**
 * Request a connection for downloading.
 * DownloadManager::addConnection will be called as soon as the connection is ready
 * for downloading.
 * @param aUser The user to connect to.
 */
void ConnectionManager::getDownloadConnection(const HintedUser& aUser) {
    dcassert((bool)aUser.user);
    {
        Lock l(cs);
        auto i = find(downloads.begin(), downloads.end(), aUser.user);
        if(i == downloads.end()) {
            getCQI(aUser, true);
        } else {
            ctx().getDownloadManager()->checkIdle(aUser.user);
        }
    }
}

ConnectionQueueItem* ConnectionManager::getCQI(const HintedUser& user, bool download) {
    ConnectionQueueItem* cqi = new ConnectionQueueItem(user, download);
    if(download) {
        dcassert(find(downloads.begin(), downloads.end(), user.user) == downloads.end());
        downloads.push_back(cqi);
    } else {
        dcassert(find(uploads.begin(), uploads.end(), user.user) == uploads.end());
        uploads.push_back(cqi);
    }

    fire(ConnectionManagerListener::Added(), cqi);
    return cqi;
}

void ConnectionManager::putCQI(ConnectionQueueItem* cqi) {
    fire(ConnectionManagerListener::Removed(), cqi);
    if(cqi->getDownload()) {
        dcassert(find(downloads.begin(), downloads.end(), cqi) != downloads.end());
        downloads.erase(remove(downloads.begin(), downloads.end(), cqi), downloads.end());
    } else {
        dcassert(find(uploads.begin(), uploads.end(), cqi) != uploads.end());
        uploads.erase(remove(uploads.begin(), uploads.end(), cqi), uploads.end());
    }
    delete cqi;
}

UserConnection* ConnectionManager::getConnection(bool aNmdc, bool secure) {
    fprintf(stderr, "[CM::getConnection] new UserConnection (sizeof=%zu)\n", sizeof(UserConnection));
    UserConnection* uc = nullptr;
    try {
        uc = new UserConnection(secure, ctx());
    } catch (const std::bad_alloc&) {
        fprintf(stderr, "[CM::getConnection] bad_alloc in new UserConnection\n");
        throw;
    }
    uc->addListener(this);
    {
        Lock l(cs);
        fprintf(stderr, "[CM::getConnection] push_back (vec size=%zu, cap=%zu)\n",
                userConnections.size(), userConnections.capacity());
        try {
            userConnections.push_back(uc);
        } catch (const std::bad_alloc&) {
            fprintf(stderr, "[CM::getConnection] bad_alloc in push_back\n");
            delete uc;
            throw;
        }
    }
    if(aNmdc)
        uc->setFlag(UserConnection::FLAG_NMDC);
    return uc;
}

void ConnectionManager::putConnection(UserConnection* aConn) {
    aConn->removeListener(this);
    aConn->disconnect();

    Lock l(cs);
    userConnections.erase(remove(userConnections.begin(), userConnections.end(), aConn), userConnections.end());
}

void ConnectionManager::on(TimerManagerListener::Second, uint64_t aTick) {
    UserList passiveUsers;
    ConnectionQueueItem::List removed;

    {
        Lock l(cs);

        bool attemptDone = false;

        for(auto& cqi: downloads) {

            if(cqi->getState() != ConnectionQueueItem::ACTIVE) {
                if(!cqi->getUser().user->isOnline()) {
                    // Not online anymore...remove it from the pending...
                    removed.push_back(cqi);
                    continue;
                }

                if(cqi->getUser().user->isSet(User::PASSIVE) && !ctx().getClientManager()->isActive()) {
                    passiveUsers.push_back(cqi->getUser());
                    removed.push_back(cqi);
                    continue;
                }

                if(cqi->getErrors() == -1 && cqi->getLastAttempt() != 0) {
                    // protocol error, don't reconnect except after a forced attempt
                    continue;
                }

                if(cqi->getLastAttempt() == 0 || (!attemptDone &&
                                                  cqi->getLastAttempt() + 60 * 1000 * max(1, cqi->getErrors()) < aTick))
                {
                    cqi->setLastAttempt(aTick);

                    QueueItem::Priority prio = ctx().getQueueManager()->hasDownload(cqi->getUser());

                    if(prio == QueueItem::PAUSED) {
                        removed.push_back(cqi);
                        continue;
                    }

                    bool startDown = ctx().getDownloadManager()->startDownload(prio);

                    if(cqi->getState() == ConnectionQueueItem::WAITING) {
                        if(startDown) {
                            cqi->setState(ConnectionQueueItem::CONNECTING);
                            fprintf(stderr, "[CM::onSecond] CONNECTING — calling ClientManager::connect\n");
                            ctx().getClientManager()->connect(cqi->getUser(), cqi->getToken());
                            fire(ConnectionManagerListener::StatusChanged(), cqi);
                            attemptDone = true;
                        } else {
                            cqi->setState(ConnectionQueueItem::NO_DOWNLOAD_SLOTS);
                            fire(ConnectionManagerListener::Failed(), cqi, _("All download slots taken"));
                        }
                    } else if(cqi->getState() == ConnectionQueueItem::NO_DOWNLOAD_SLOTS && startDown) {
                        cqi->setState(ConnectionQueueItem::WAITING);
                    }
                } else if(cqi->getState() == ConnectionQueueItem::CONNECTING && cqi->getLastAttempt() + 50 * 1000 < aTick) {
                    cqi->setErrors(cqi->getErrors() + 1);
                    fprintf(stderr, "[CM::onSecond] connection timeout, errors=%d\n", cqi->getErrors());
                    fire(ConnectionManagerListener::Failed(), cqi, _("Connection timeout"));
                    cqi->setState(ConnectionQueueItem::WAITING);
                }
            }
        }

        for(auto& m : removed) {
            putCQI(m);
        }

    }

    for(auto& ui : passiveUsers) {
        ctx().getQueueManager()->removeSource(ui, QueueItem::Source::FLAG_PASSIVE);
    }
}

void ConnectionManager::on(TimerManagerListener::Minute, uint64_t aTick) {
    Lock l(cs);

    for(auto& j : userConnections) {
        if((j->getLastActivity() + 180*1000) < aTick) {
            j->disconnect(true);
        }
    }
}

const string& ConnectionManager::getPort() const {
    return server ? server->getPort() : Util::emptyString;
}

const string& ConnectionManager::getSecurePort() const {
    return secureServer ? secureServer->getPort() : Util::emptyString;
}

static const uint32_t FLOOD_TRIGGER = 20000;
static const uint32_t FLOOD_ADD = 2000;

ConnectionManager::Server::Server(DCContext& ctx, const bool secure_, const string& aPort, const string& ip_ /* = "0.0.0.0" */) : secure(secure_), die(false), ctx_(ctx) {
    sock.create();
    sock.setSocketOpt(SO_REUSEADDR, 1);
    ip = CTX_SETTING(BIND_IFACE)? sock.getIfaceI4(CTX_SETTING(BIND_IFACE_NAME)).c_str() : ip_;
    port = sock.bind(aPort, ip);
    sock.listen();

    start();
}

static const uint32_t POLL_TIMEOUT = 250;

int ConnectionManager::Server::run() {
    {
        char threadName[17];
        snprintf(threadName, sizeof threadName, "Server_%s", port.c_str());
    }
    while(!die) {
        try {
            while(!die) {
                if(sock.wait(POLL_TIMEOUT, Socket::WAIT_READ) == Socket::WAIT_READ) {
                    ctx().getConnectionManager()->accept(sock, secure);
                }
            }
        } catch(const Exception& e) {
            dcdebug("ConnectionManager::Server::run Error: %s\n", e.getError().c_str());
        }

        bool failed = false;
        while(!die) {
            try {
                sock.disconnect();
                sock.create();
                sock.bind(port, ip);
                sock.listen();
                if(failed) {
                    ctx().getLogManager()->message(_("Connectivity restored"));
                    failed = false;
                }
                break;
            } catch(const SocketException& e) {
                dcdebug("ConnectionManager::Server::run Stopped listening: %s\n", e.getError().c_str());

                if(!failed) {
                    ctx().getLogManager()->message(str(F_("Connectivity error: %1%") % e.getError()));
                    failed = true;
                }

                // Spin for 60 seconds
                for(auto i = 0; i < 60 && !die; ++i) {
                    Thread::sleep(1000);
                }
            }
        }
    }
    return 0;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::accept(const Socket& sock, bool secure) {
    uint64_t now = GET_TICK();

    if(now > floodCounter) {
        floodCounter = now + FLOOD_ADD;
    } else {
        if(false && now + FLOOD_TRIGGER < floodCounter) {
            Socket s;
            try {
                s.accept(sock);
            } catch(const SocketException&) {
                // ...
            }
            dcdebug("Connection flood detected!\n");
            return;
        } else {
            floodCounter += FLOOD_ADD;
        }
    }
    UserConnection* uc = getConnection(false, secure);
    uc->setFlag(UserConnection::FLAG_INCOMING);
    uc->setState(UserConnection::STATE_SUPNICK);
    uc->setLastActivity(GET_TICK());
    try {
        uc->accept(sock);
    } catch(const Exception&) {
        putConnection(uc);
        delete uc;
    }
}

void ConnectionManager::nmdcConnect(const string& aServer, const string& aPort, const string& aNick, const string& hubUrl, const string& encoding, bool secure) {
    nmdcConnect(aServer, aPort, Util::emptyString, BufferedSocket::NAT_NONE, aNick, hubUrl, encoding, secure);
}

void ConnectionManager::nmdcConnect(const string& aServer, const string& aPort, const string& localPort, BufferedSocket::NatRoles natRole, const string& aNick, const string& hubUrl, const string& encoding, bool secure) {
    if(shuttingDown)
        return;

    if (checkHubCCBlock(aServer, aPort, hubUrl))
        return;

    fprintf(stderr, "[CM::nmdcConnect] step 1: getConnection\n");
    UserConnection* uc = getConnection(true, secure);
    fprintf(stderr, "[CM::nmdcConnect] step 2: setToken(%zu bytes)\n", aNick.size());
    try { uc->setToken(aNick); } catch (const std::bad_alloc&) {
        fprintf(stderr, "[CM::nmdcConnect] bad_alloc in setToken\n"); throw;
    }
    fprintf(stderr, "[CM::nmdcConnect] step 3: setHubUrl(%zu bytes)\n", hubUrl.size());
    try { uc->setHubUrl(hubUrl); } catch (const std::bad_alloc&) {
        fprintf(stderr, "[CM::nmdcConnect] bad_alloc in setHubUrl\n"); throw;
    }
    fprintf(stderr, "[CM::nmdcConnect] step 4: setEncoding(%zu bytes)\n", encoding.size());
    try { uc->setEncoding(encoding); } catch (const std::bad_alloc&) {
        fprintf(stderr, "[CM::nmdcConnect] bad_alloc in setEncoding\n"); throw;
    }
    uc->setState(UserConnection::STATE_CONNECT);
    uc->setFlag(UserConnection::FLAG_NMDC);
    fprintf(stderr, "[CM::nmdcConnect] step 5: connect(%s, %s)\n", aServer.c_str(), aPort.c_str());
    try {
        uc->connect(aServer, aPort, localPort, natRole);
    } catch(const Exception& e) {
        fprintf(stderr, "[CM::nmdcConnect] Exception in connect: %s\n", e.what());
        putConnection(uc);
        delete uc;
    } catch(const std::bad_alloc&) {
        fprintf(stderr, "[CM::nmdcConnect] bad_alloc in connect (thread/socket creation)\n");
        putConnection(uc);
        delete uc;
        throw;
    } catch(...) {
        fprintf(stderr, "[CM::nmdcConnect] unknown exception in connect\n");
        putConnection(uc);
        delete uc;
        throw;
    }
    fprintf(stderr, "[CM::nmdcConnect] done\n");
}

void ConnectionManager::adcConnect(const OnlineUser& aUser, const string &aPort, const string& aToken, bool secure) {
    adcConnect(aUser, aPort, Util::emptyString, BufferedSocket::NAT_NONE, aToken, secure);
}

void ConnectionManager::adcConnect(const OnlineUser& aUser, const string &aPort, const string &localPort, BufferedSocket::NatRoles natRole, const string& aToken, bool secure) {
    if(shuttingDown)
        return;

    UserConnection* uc = getConnection(false, secure);
    uc->setToken(aToken);
    uc->setEncoding(Text::utf8);
    uc->setState(UserConnection::STATE_CONNECT);
#ifdef WITH_DHT
    uc->setHubUrl(&aUser.getClient() == NULL ? "DHT" : aUser.getClient().getHubUrl());
#else
    uc->setHubUrl(aUser.getClient().getHubUrl());
#endif
    if(aUser.getIdentity().isOp()) {
        uc->setFlag(UserConnection::FLAG_OP);
    }
    try {
        uc->connect(aUser.getIdentity().getIp(), aPort, localPort, natRole);
    } catch(const Exception&) {
        putConnection(uc);
        delete uc;
    } catch(...) {
        putConnection(uc);
        delete uc;
        throw;
    }
}

void ConnectionManager::disconnect() {
    delete server;
    delete secureServer;
    server = nullptr;
    secureServer = nullptr;
}

void ConnectionManager::on(AdcCommand::SUP, UserConnection* aSource, const AdcCommand& cmd) {
    if(aSource->getState() != UserConnection::STATE_SUPNICK) {
        // Already got this once, ignore...@todo fix support updates
        dcdebug("CM::onSUP %p sent sup twice\n", (void*)aSource);
        return;
    }

    bool baseOk = false;

    for(auto& i: cmd.getParameters()) {
        if(i.compare(0, 2, "AD") == 0) {
            string feat = i.substr(2);
            if(feat == UserConnection::FEATURE_ADC_BASE || feat == UserConnection::FEATURE_ADC_BAS0) {
                baseOk = true;
                // ADC clients must support all these...
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_TTHF);
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
                // For compatibility with older clients...
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
            } else if(feat == UserConnection::FEATURE_ZLIB_GET) {
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
            } else if(feat == UserConnection::FEATURE_ADC_BZIP) {
                aSource->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
            }
        }
    }

    if(!baseOk) {
        aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Invalid SUP"));
        aSource->disconnect();
        return;
    }

    if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
        StringList defFeatures = adcFeatures;
        if(CTX_BOOLSETTING(COMPRESS_TRANSFERS)) {
            defFeatures.push_back("AD" + UserConnection::FEATURE_ZLIB_GET);
        }
        aSource->sup(defFeatures);
        aSource->inf(false);
    } else {
        aSource->inf(true);
    }
    aSource->setState(UserConnection::STATE_INF);
}

void ConnectionManager::on(AdcCommand::STA, UserConnection*, const AdcCommand&) {

}

void ConnectionManager::on(UserConnectionListener::Connected, UserConnection* aSource) {
    // incorrect check because aSource->getUser().get() == nullptr
    if(aSource->isSecure() && !aSource->isTrusted() && !CTX_BOOLSETTING(ALLOW_UNTRUSTED_CLIENTS)) {
        putConnection(aSource);
        //        ctx().getQueueManager()->removeSource(aSource->getUser(), QueueItem::Source::FLAG_UNTRUSTED);
        return;
    }

    dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
    if(aSource->isSet(UserConnection::FLAG_NMDC)) {
        aSource->myNick(aSource->getToken());
        aSource->lock(ctx().getCryptoManager()->getLock(), ctx().getCryptoManager()->getPk() + "Ref=" + aSource->getHubUrl());
    } else {
        StringList defFeatures = adcFeatures;
        if(CTX_BOOLSETTING(COMPRESS_TRANSFERS)) {
            defFeatures.push_back("AD" + UserConnection::FEATURE_ZLIB_GET);
        }
        aSource->sup(defFeatures);
        aSource->send(AdcCommand(AdcCommand::SEV_SUCCESS, AdcCommand::SUCCESS, Util::emptyString).addParam("RF", aSource->getHubUrl()));
    }
    aSource->setState(UserConnection::STATE_SUPNICK);
}

void ConnectionManager::on(UserConnectionListener::MyNick, UserConnection* aSource, const string& aNick) {
    if(aSource->getState() != UserConnection::STATE_SUPNICK) {
        // Already got this once, ignore...
        dcdebug("CM::onMyNick %p sent nick twice\n", (void*)aSource);
        return;
    }

    dcassert(!aNick.empty());
    dcdebug("ConnectionManager::onMyNick %p, %s\n", (void*)aSource, aNick.c_str());

    if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
        // Try to guess where this came from...
        pair<string, string> i = expectedConnections.remove(aNick);
        if(i.second.empty()) {
            dcassert(i.first.empty());
            dcdebug("Unknown incoming connection from %s\n", aNick.c_str());
            putConnection(aSource);
            return;
        }
        aSource->setToken(i.first);
        aSource->setHubUrl(i.second);
        aSource->setEncoding(ctx().getClientManager()->findHubEncoding(i.second));
    }

    string nick = Text::toUtf8(aNick, aSource->getEncoding());
    CID cid = ctx().getClientManager()->makeCid(nick, aSource->getHubUrl());

    // First, we try looking in the pending downloads...hopefully it's one of them...
    {
        Lock l(cs);
        for(auto& cqi: downloads) {
            cqi->setErrors(0);
            if((cqi->getState() == ConnectionQueueItem::CONNECTING || cqi->getState() == ConnectionQueueItem::WAITING) &&
                    cqi->getUser().user->getCID() == cid)
            {
                aSource->setUser(cqi->getUser());
                // Indicate that we're interested in this file...
                aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
                break;
            }
        }
    }

    if(!aSource->getUser()) {
        // Make sure we know who it is, i e that he/she is connected...

        aSource->setUser(ctx().getClientManager()->findUser(cid));
        if(!aSource->getUser() || !ctx().getClientManager()->isOnline(aSource->getUser())) {
            dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", nick.c_str());
            putConnection(aSource);
            return;
        }
        // We don't need this connection for downloading...make it an upload connection instead...
        aSource->setFlag(UserConnection::FLAG_UPLOAD);
    }

    if(ctx().getClientManager()->isOp(aSource->getUser(), aSource->getHubUrl()))
        aSource->setFlag(UserConnection::FLAG_OP);

    ctx().getClientManager()->setIPUser(aSource->getUser(), aSource->getRemoteIp());

    if( aSource->isSet(UserConnection::FLAG_INCOMING) ) {
        aSource->myNick(aSource->getToken());
        aSource->lock(ctx().getCryptoManager()->getLock(), ctx().getCryptoManager()->getPk());
    }

    aSource->setState(UserConnection::STATE_LOCK);
}

void ConnectionManager::on(UserConnectionListener::CLock, UserConnection* aSource, const string& aLock, const string& aPk) {
    (void)aPk;

    if(aSource->getState() != UserConnection::STATE_LOCK) {
        dcdebug("CM::onLock %p received lock twice, ignoring\n", (void*)aSource);
        return;
    }

    if( ctx().getCryptoManager()->isExtended(aLock) ) {
        StringList defFeatures = features;
        if(CTX_BOOLSETTING(COMPRESS_TRANSFERS)) {
            defFeatures.push_back(UserConnection::FEATURE_ZLIB_GET);
        }

        aSource->supports(defFeatures);
    }

    aSource->setState(UserConnection::STATE_DIRECTION);
    aSource->direction(aSource->getDirectionString(), aSource->getNumber());
    aSource->key(ctx().getCryptoManager()->makeKey(aLock));
}

void ConnectionManager::on(UserConnectionListener::Direction, UserConnection* aSource, const string& dir, const string& num) {
    if(aSource->getState() != UserConnection::STATE_DIRECTION) {
        dcdebug("CM::onDirection %p received direction twice, ignoring\n", (void*)aSource);
        return;
    }

    dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));
    if(dir == "Upload") {
        // Fine, the other fellow want's to send us data...make sure we really want that...
        if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
            // Huh? Strange...disconnect...
            putConnection(aSource);
            return;
        }
    } else {
        if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
            int number = Util::toInt(num);
            // Damn, both want to download...the one with the highest number wins...
            if(aSource->getNumber() < number) {
                // Damn! We lost!
                aSource->unsetFlag(UserConnection::FLAG_DOWNLOAD);
                aSource->setFlag(UserConnection::FLAG_UPLOAD);
            } else if(aSource->getNumber() == number) {
                putConnection(aSource);
                return;
            }
        }
    }

    dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));

    aSource->setState(UserConnection::STATE_KEY);
}

void ConnectionManager::addDownloadConnection(UserConnection* uc) {
    bool addConn = false;
    {
        Lock l(cs);

        auto i = find(downloads.begin(), downloads.end(), uc->getUser());
        if(i != downloads.end()) {
            auto& cqi = *i;
            if(cqi->getState() == ConnectionQueueItem::WAITING || cqi->getState() == ConnectionQueueItem::CONNECTING) {
                cqi->setState(ConnectionQueueItem::ACTIVE);
                uc->setFlag(UserConnection::FLAG_ASSOCIATED);

                fire(ConnectionManagerListener::Connected(), cqi);

                dcdebug("ConnectionManager::addDownloadConnection, leaving to downloadmanager\n");
                addConn = true;
            }
        }
    }

    if(addConn) {
        ctx().getDownloadManager()->addConnection(uc);
    } else {
        putConnection(uc);
    }
}

void ConnectionManager::addUploadConnection(UserConnection* uc) {
    dcassert(uc->isSet(UserConnection::FLAG_UPLOAD));

    bool addConn = false;
    {
        Lock l(cs);

        auto i = find(uploads.begin(), uploads.end(), uc->getUser());
        if(i == uploads.end()) {
            ConnectionQueueItem* cqi = getCQI(uc->getHintedUser(), false);

            cqi->setState(ConnectionQueueItem::ACTIVE);
            uc->setFlag(UserConnection::FLAG_ASSOCIATED);

            fire(ConnectionManagerListener::Connected(), cqi);

            dcdebug("ConnectionManager::addUploadConnection, leaving to uploadmanager\n");
            addConn = true;
        }
    }

    if(addConn) {
        ctx().getUploadManager()->addConnection(uc);
    } else {
        putConnection(uc);
    }
}

void ConnectionManager::on(UserConnectionListener::Key, UserConnection* aSource, const string&/* aKey*/) {
    if(aSource->getState() != UserConnection::STATE_KEY) {
        dcdebug("CM::onKey Bad state, ignoring");
        return;
    }

    if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
        addDownloadConnection(aSource);
    } else {
        addUploadConnection(aSource);
    }
}

void ConnectionManager::on(AdcCommand::INF, UserConnection* aSource, const AdcCommand& cmd) {
    if(aSource->getState() != UserConnection::STATE_INF) {
        aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Expecting INF"));
        aSource->disconnect();
        return;
    }

    // Leaks CSUPs, other client's CINF, and ADC connection's presence. Allows removing
    // user from queue by waiting long enough for aSource->getUser() to function.
    if(CTX_SETTING(REQUIRE_TLS) && !aSource->isSet(UserConnection::FLAG_NMDC) && !aSource->isSecure()) {
        putConnection(aSource);
        ctx().getQueueManager()->removeSource(aSource->getUser(), QueueItem::Source::FLAG_UNENCRYPTED);
        return;
    }

    string cid;
    if(!cmd.getParam("ID", 0, cid)) {
        aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_INF_FIELD, "INF ID missing").addParam("FM", "ID"));
        dcdebug("CM::onINF missing ID\n");
        aSource->disconnect();
        return;
    }

    if(!aSource->getUser()) {
        aSource->setUser(ctx().getClientManager()->findUser(CID(cid)));

        if(!aSource->getUser()) {
            aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_INF_FIELD, "INF ID: user not found").addParam("FB", "ID"));
            putConnection(aSource);
            return;
        }
    }

    // without a valid KeyPrint this degrades into normal turst check
    if(!checkKeyprint(aSource)) {
        ctx().getQueueManager()->removeSource(aSource->getUser(), QueueItem::Source::FLAG_UNTRUSTED);
        putConnection(aSource);
        return;
    }

    string token;
    if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
        if(!cmd.getParam("TO", 0, token)) {
            aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_INF_FIELD, "INF TO missing").addParam("FM", "TO"));
            putConnection(aSource);
            return;
        }
    } else {
        token = aSource->getToken();
    }

    bool down = false;
    {
        Lock l(cs);
        auto i = find(downloads.begin(), downloads.end(), aSource->getUser());

        if(i != downloads.end()) {
            (*i)->setErrors(0);

            const string& to = (*i)->getToken();

            if(to == token) {
                down = true;
            }
        }
        /** @todo check tokens for upload connections */
    }

    if(down) {
        aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
        addDownloadConnection(aSource);
    } else {
        aSource->setFlag(UserConnection::FLAG_UPLOAD);
        addUploadConnection(aSource);
    }
}

void ConnectionManager::force(const UserPtr& aUser) {
    Lock l(cs);

    auto i = find(downloads.begin(), downloads.end(), aUser);
    if(i != downloads.end()) {
        (*i)->setLastAttempt(0);
    }
}

bool ConnectionManager::checkKeyprint(UserConnection *aSource) {
    auto kp = aSource->getKeyprint();
    if(kp.empty()) {
        return true;
    }

    auto kp2 = ctx().getClientManager()->getField(aSource->getUser()->getCID(), aSource->getHubUrl(), "KP");
    if(kp2.empty()) {
        // TODO false probably
        return true;
    }

    if(kp2.compare(0, 7, "SHA256/") != 0) {
        // Unsupported hash
        return true;
    }

    dcdebug("Keyprint: %s vs %s\n", Encoder::toBase32(&kp[0], kp.size()).c_str(), kp2.c_str() + 7);

    vector<uint8_t> kp2v(kp.size());
    Encoder::fromBase32(&kp2[7], &kp2v[0], kp2v.size());
    if(!std::equal(kp.begin(), kp.end(), kp2v.begin())) {
        dcdebug("Not equal...\n");
        return false;
    }

    return true;
}

void ConnectionManager::failed(UserConnection* aSource, const string& aError, bool protocolError) {
    Lock l(cs);

    if(aSource->isSet(UserConnection::FLAG_ASSOCIATED)) {
        if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
            auto i = find(downloads.begin(), downloads.end(), aSource->getUser());
            dcassert(i != downloads.end());

            auto& cqi = *i;
            cqi->setState(ConnectionQueueItem::WAITING);
            cqi->setLastAttempt(GET_TICK());
            cqi->setErrors(protocolError ? -1 : (cqi->getErrors() + 1));
            fire(ConnectionManagerListener::Failed(), cqi, aError);
        } else {
            if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
                auto i = find(uploads.begin(), uploads.end(), aSource->getUser());
                dcassert(i != uploads.end());
                ConnectionQueueItem* cqi = *i;
                putCQI(cqi);
            }
        }
    }
    putConnection(aSource);
}

bool ConnectionManager::checkHubCCBlock(const string& aServer, const string& aPort, const string& aHubUrl)
{
    const auto server_lower = Text::toLower(aServer);
    dcassert(server_lower == aServer);

    bool cc_blocked = false;

    {
        Lock l(cs);
        cc_blocked = !hubsBlockingCC.empty() && hubsBlockingCC.find(server_lower) != hubsBlockingCC.end();
    }

    if(cc_blocked)
    {
        ctx().getLogManager()->message(str(F_("Blocked a C-C connection to a hub ('%1%:%2%'; request from '%3%')") % aServer % aPort % aHubUrl));
        return true;
    }
    return false;
}

void ConnectionManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) {
    failed(aSource, aError, false);
}

void ConnectionManager::on(UserConnectionListener::ProtocolError, UserConnection* aSource, const string& aError) {
    if(aError.compare(0, 7, "CTM2HUB", 7) == 0) {
        {
            Lock l(cs);
            hubsBlockingCC.insert(Text::toLower(aSource->getRemoteIp()));
        }

        string aServerPort = aSource->getRemoteIp() + ":" + aSource->getPort();
        ctx().getLogManager()->message(str(F_("Blocking '%1%', potential DDoS detected (originating hub '%2%')") % aServerPort % aSource->getHubUrl() ));
    }

    failed(aSource, aError, true);
}

void ConnectionManager::disconnect(const UserPtr& user) {
    Lock l(cs);
    for(auto uc: userConnections) {
        if(uc->getUser() == user)
            uc->disconnect(true);
    }
}

void ConnectionManager::disconnect(const UserPtr& user, int isDownload) {
    Lock l(cs);
    for(auto uc: userConnections) {
        if(uc->getUser() == user && uc->isSet(isDownload ? UserConnection::FLAG_DOWNLOAD : UserConnection::FLAG_UPLOAD)) {
            uc->disconnect(true);
            break;
        }
    }
}

void ConnectionManager::shutdown() {
    ctx().getTimerManager()->removeListener(this);
    shuttingDown = true;
    disconnect();
    {
        Lock l(cs);
        for(auto j: userConnections) {
            j->disconnect(true);
        }
    }
    // Wait until all connections have died out...
    while(true) {
        {
            Lock l(cs);
            if(userConnections.empty()) {
                break;
            }
        }
        Thread::sleep(50);
    }
}

// UserConnectionListener
void ConnectionManager::on(UserConnectionListener::Supports, UserConnection* conn, const StringList& feat) {
    for(auto& i: feat) {
        if(i == UserConnection::FEATURE_MINISLOTS) {
            conn->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
        } else if(i == UserConnection::FEATURE_XML_BZLIST) {
            conn->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
        } else if(i == UserConnection::FEATURE_ADCGET) {
            conn->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
        } else if(i == UserConnection::FEATURE_ZLIB_GET) {
            conn->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
        } else if(i == UserConnection::FEATURE_TTHL) {
            conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
        } else if(i == UserConnection::FEATURE_TTHF) {
            conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHF);
        }
    }
}

} // namespace dcpp
