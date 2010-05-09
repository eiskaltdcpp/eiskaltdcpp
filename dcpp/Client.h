/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CLIENT_H
#define DCPLUSPLUS_DCPP_CLIENT_H

#include "forward.h"

#include "User.h"
#include "Speaker.h"
#include "BufferedSocketListener.h"
#include "TimerManager.h"
#include "ClientListener.h"

namespace dcpp {

/** Yes, this should probably be called a Hub */
class Client : public Speaker<ClientListener>, public BufferedSocketListener, protected TimerManagerListener {
public:
    typedef Client* Ptr;
    typedef list<Ptr> List;
    typedef List::iterator Iter;

    virtual void connect();
    virtual void disconnect(bool graceless);

    virtual void connect(const OnlineUser& user, const string& token) = 0;
    virtual void hubMessage(const string& aMessage, bool thirdPerson = false) = 0;
    virtual void privateMessage(const OnlineUser& user, const string& aMessage, bool thirdPerson = false) = 0;
    virtual void sendUserCmd(const string& aUserCmd) = 0;
    virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) = 0;
    virtual void password(const string& pwd) = 0;
    virtual void info(bool force) = 0;

    virtual size_t getUserCount() const = 0;
    virtual int64_t getAvailable() const = 0;
    static int getTotalCounts() { return counts.normal + counts.registered + counts.op; }
    virtual void send(const AdcCommand& command) = 0;

    virtual string escape(string const& str) const { return str; }

    bool isConnected() const { return state != STATE_DISCONNECTED; }
    bool isSecure() const;
    bool isTrusted() const;
    std::string getCipherName() const;

    bool isOp() const { return getMyIdentity().isOp(); }

    uint16_t getPort() const { return port; }
    const string& getAddress() const { return address; }

    const string& getIp() const { return ip; }
    string getIpPort() const { return getIp() + ':' + Util::toString(port); }
    string getLocalIp() const;

    void updated(const OnlineUser& aUser) { fire(ClientListener::UserUpdated(), this, aUser); }

    static string getCounts() {
        char buf[128];
        return string(buf, snprintf(buf, sizeof(buf), "%ld/%ld/%ld", counts.normal, counts.registered, counts.op));
    }

    StringMap& escapeParams(StringMap& sm) {
        for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
            i->second = escape(i->second);
        }
        return sm;
    }

    void reconnect();
    void shutdown();

    void send(const string& aMessage) { send(aMessage.c_str(), aMessage.length()); }
    void send(const char* aMessage, size_t aLen);

    string getMyNick() const { return getMyIdentity().getNick(); }
    string getHubName() const { return getHubIdentity().getNick().empty() ? getHubUrl() : getHubIdentity().getNick(); }
    string getHubDescription() const { return getHubIdentity().getDescription(); }

    Identity& getHubIdentity() { return hubIdentity; }

    const string& getHubUrl() const { return hubUrl; }

    GETSET(Identity, myIdentity, MyIdentity);
    GETSET(Identity, hubIdentity, HubIdentity);

    GETSET(string, defpassword, Password);
    GETSET(uint32_t, reconnDelay, ReconnDelay);
    GETSET(uint64_t, lastActivity, LastActivity);
    GETSET(bool, registered, Registered);
    GETSET(bool, autoReconnect, AutoReconnect);
    GETSET(string, encoding, Encoding);
    GETSET(string, clientId, ClientId);
    GETSET(string, currentNick, CurrentNick);
    GETSET(string, currentDescription, CurrentDescription);

    string getFavIp() const { return externalIP; }

    /** Reload details from favmanager or settings */
    void reloadSettings(bool updateNick);
protected:
    friend class ClientManager;
    Client(const string& hubURL, char separator, bool secure_);
    virtual ~Client() throw();
    struct Counts {
        Counts(long n = 0, long r = 0, long o = 0) : normal(n), registered(r), op(o) { }
        volatile long normal;
        volatile long registered;
        volatile long op;
        bool operator !=(const Counts& rhs) { return normal != rhs.normal || registered != rhs.registered || op != rhs.op; }
    };

    enum States {
        STATE_CONNECTING,   ///< Waiting for socket to connect
        STATE_PROTOCOL,     ///< Protocol setup
        STATE_IDENTIFY,     ///< Nick setup
        STATE_VERIFY,       ///< Checking password
        STATE_NORMAL,       ///< Running
        STATE_DISCONNECTED, ///< Nothing in particular
    } state;

    BufferedSocket* sock;

    static Counts counts;
    Counts lastCounts;

    void updateCounts(bool aRemove);
    void updateActivity() { lastActivity = GET_TICK(); }

    virtual string checkNick(const string& nick) = 0;

    // TimerManagerListener
    virtual void on(Second, uint32_t aTick) throw();
    // BufferedSocketListener
    virtual void on(Connecting) throw() { fire(ClientListener::Connecting(), this); }
    virtual void on(Connected) throw();
    virtual void on(Line, const string& aLine) throw();
    virtual void on(Failed, const string&) throw();

private:

    enum CountType {
        COUNT_UNCOUNTED,
        COUNT_NORMAL,
        COUNT_REGISTERED,
        COUNT_OP
    };

    Client(const Client&);
    Client& operator=(const Client&);

    string hubUrl;
    string address;
    string ip;
    string localIp;
    uint16_t port;
    string externalIP;
    char separator;
    bool secure;
    CountType countType;
};

} // namespace dcpp

#endif // !defined(CLIENT_H)
