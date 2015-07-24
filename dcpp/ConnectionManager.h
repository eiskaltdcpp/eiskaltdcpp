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

#include "TimerManager.h"
#include "UserConnection.h"
#include "User.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "Util.h"
#include "ConnectionManagerListener.h"

namespace dcpp {

class SocketException;

class ConnectionQueueItem : boost::noncopyable {
public:
    typedef ConnectionQueueItem* Ptr;
    typedef vector<Ptr> List;
    typedef List::iterator Iter;

    enum State {
        CONNECTING,                 // Recently sent request to connect
        WAITING,                    // Waiting to send request to connect
        NO_DOWNLOAD_SLOTS,          // Not needed right now
        ACTIVE                      // In one up/downmanager
    };

    ConnectionQueueItem(const HintedUser& aUser, bool aDownload) : token(Util::toString(Util::rand())),
                lastAttempt(0), errors(0), state(WAITING), download(aDownload), user(aUser) { }

    GETSET(string, token, Token);
    GETSET(uint64_t, lastAttempt, LastAttempt);
    GETSET(int, errors, Errors); // Number of connection errors, or -1 after a protocol error
    GETSET(State, state, State);
    GETSET(bool, download, Download);

    const HintedUser& getUser() const { return user; }

private:
    HintedUser user;
};

class ExpectedMap {
public:
    void add(const string& aNick, const string& aMyNick, const string& aHubUrl) {
        Lock l(cs);
        expectedConnections.insert(make_pair(aNick, make_pair(aMyNick, aHubUrl)));
    }

    StringPair remove(const string& aNick) {
        Lock l(cs);
        auto i = expectedConnections.find(aNick);

        if(i == expectedConnections.end())
            return make_pair(Util::emptyString, Util::emptyString);

        StringPair tmp = i->second;
        expectedConnections.erase(i);

        return tmp;
    }

private:
    /** Nick -> myNick, hubUrl for expected NMDC incoming connections */
    typedef map<string, StringPair> ExpectMap;
    ExpectMap expectedConnections;

    CriticalSection cs;
};

// Comparing with a user...
inline bool operator==(ConnectionQueueItem::Ptr ptr, const UserPtr& aUser) { return ptr->getUser() == aUser; }

class ConnectionManager : public Speaker<ConnectionManagerListener>,
    public UserConnectionListener, TimerManagerListener,
    public Singleton<ConnectionManager>
{
public:
    void nmdcExpect(const string& aNick, const string& aMyNick, const string& aHubUrl) {
        expectedConnections.add(aNick, aMyNick, aHubUrl);
    }

    void nmdcConnect(const string& aServer, uint16_t aPort, const string& aMyNick, const string& hubUrl, const string& encoding, bool secure);
    void nmdcConnect(const string& aServer, uint16_t aPort, uint16_t localPort, BufferedSocket::NatRoles natRole, const string& aNick, const string& hubUrl, const string& encoding, bool secure);
    void adcConnect(const OnlineUser& aUser, uint16_t aPort, const string& aToken, bool secure);
    void adcConnect(const OnlineUser& aUser, uint16_t aPort, uint16_t localPort, BufferedSocket::NatRoles natRole, const string& aToken, bool secure);

    void getDownloadConnection(const HintedUser& aUser);
    void force(const UserPtr& aUser);

    void disconnect(const UserPtr& aUser); // disconnect downloads and uploads
    void disconnect(const UserPtr& aUser, int isDownload);

    void shutdown();

    /** Find a suitable port to listen on, and start doing it */
    void listen();
    void disconnect() noexcept;

    uint16_t getPort() { return server ? static_cast<uint16_t>(server->getPort()) : 0; }
    uint16_t getSecurePort() { return secureServer ? static_cast<uint16_t>(secureServer->getPort()) : 0; }

    void addCTM2HUB(const string &server, const string &port);

private:

    unordered_set<string> ddosctm2hub;

    class Server : public Thread {
    public:
        Server(bool secure_, uint16_t port, const string& ip = "0.0.0.0");
        uint16_t getPort() { return port; }
        virtual ~Server() { die = true; join(); }
    private:
        virtual int run() noexcept;

        Socket sock;
        uint16_t port;
        string ip;
        bool secure;
        bool die;
    };

    friend class Server;

    CriticalSection cs;

    /** All ConnectionQueueItems */
    ConnectionQueueItem::List downloads;
    ConnectionQueueItem::List uploads;

    /** All active connections */
    UserConnectionList userConnections;

    StringList features;
    StringList adcFeatures;

    ExpectedMap expectedConnections;

    uint32_t floodCounter;

    Server* server;
    Server* secureServer;

    bool shuttingDown;

    friend class Singleton<ConnectionManager>;
    ConnectionManager();

    virtual ~ConnectionManager() { shutdown(); }

    UserConnection* getConnection(bool aNmdc, bool secure) noexcept;
    void putConnection(UserConnection* aConn);

    void addUploadConnection(UserConnection* uc);
    void addDownloadConnection(UserConnection* uc);

    ConnectionQueueItem* getCQI(const HintedUser& aUser, bool download);
    void putCQI(ConnectionQueueItem* cqi);

    bool checkKeyprint(UserConnection *aSource);

    void accept(const Socket& sock, bool secure) noexcept;

    void failed(UserConnection* aSource, const string& aError, bool protocolError);

    // UserConnectionListener
    virtual void on(Connected, UserConnection*) noexcept;
    virtual void on(Failed, UserConnection*, const string&) noexcept;
    virtual void on(ProtocolError, UserConnection*, const string&) noexcept;
    virtual void on(CLock, UserConnection*, const string&, const string&) noexcept;
    virtual void on(Key, UserConnection*, const string&) noexcept;
    virtual void on(Direction, UserConnection*, const string&, const string&) noexcept;
    virtual void on(MyNick, UserConnection*, const string&) noexcept;
    virtual void on(Supports, UserConnection*, const StringList&) noexcept;

    virtual void on(AdcCommand::SUP, UserConnection*, const AdcCommand&) noexcept;
    virtual void on(AdcCommand::INF, UserConnection*, const AdcCommand&) noexcept;
    virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) noexcept;

    // TimerManagerListener
    virtual void on(TimerManagerListener::Second, uint64_t aTick) noexcept;
    virtual void on(TimerManagerListener::Minute, uint64_t aTick) noexcept;

};

} // namespace dcpp
