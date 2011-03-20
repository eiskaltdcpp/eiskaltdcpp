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

#include "Socket.h"
#include "format.h"
#include "SettingsManager.h"
#include "TimerManager.h"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef IP_TOS
#define        IP_TOS          1
#endif
#ifndef IPTOS_TOS
#define IPTOS_TOS(a) ((a) & 0x1E)
#endif

#ifndef _WIN32
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace dcpp {

string Socket::udpServer;
uint16_t Socket::udpPort;

#define checkconnected() if(!isConnected()) throw SocketException(ENOTCONN))

#ifdef _DEBUG

SocketException::SocketException(int aError) throw() {
    error = "SocketException: " + errorToString(aError);
    dcdebug("Thrown: %s\n", error.c_str());
}

#else // _DEBUG

SocketException::SocketException(int aError) throw() : Exception(errorToString(aError)) { }

#endif

Socket::Stats Socket::stats = { 0, 0 };

static const uint32_t SOCKS_TIMEOUT = 30000;

string SocketException::errorToString(int aError) throw() {
    string msg = Util::translateError(aError);
    if(msg.empty()) {
        msg = str(F_("Unknown error: 0x%1$x") % aError);
    }
    return msg;
}

void Socket::create(int aType /* = TYPE_TCP */) throw(SocketException) {
    if(sock != INVALID_SOCKET)
        disconnect();

    switch(aType) {
    case TYPE_TCP:
        sock = checksocket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        break;
    case TYPE_UDP:
        sock = checksocket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
        break;
    default:
        dcassert(0);
    }
    type = aType;

    setBlocking(false);

    if (SETTING(IP_TOS_VALUE) != -1)
        setSocketOpt(IP_TOS, IPTOS_TOS(SETTING(IP_TOS_VALUE)));
}

void Socket::accept(const Socket& listeningSocket) throw(SocketException) {
    if(sock != INVALID_SOCKET) {
        disconnect();
    }
    sockaddr_in sock_addr;
    socklen_t sz = sizeof(sock_addr);

    do {
        sock = ::accept(listeningSocket.sock, (sockaddr*)&sock_addr, &sz);
    } while (sock == SOCKET_ERROR && getLastError() == EINTR);
    check(sock);

#ifdef _WIN32
    // Make sure we disable any inherited windows message things for this socket.
    ::WSAAsyncSelect(sock, NULL, 0, 0);
#endif

    type = TYPE_TCP;

    setIp(inet_ntoa(sock_addr.sin_addr));
    connected = true;
    setBlocking(false);
}


static string getIfaceI4 (const string &iface){
#ifdef _WIN32
    return "0.0.0.0";
#endif
    struct ifreq request;
    string s = "0.0.0.0";

    memset(&request, 0, sizeof(struct ifreq));

    if ((size_t)iface.size() <= sizeof(request.ifr_name)){
        memcpy(request.ifr_name, iface.c_str(), iface.size());

        int sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock != -1 ){
            if (ioctl(sock, SIOCGIFADDR, &request) >= 0){
                struct sockaddr *sa = &request.ifr_addr;

                if ( sa && sa->sa_family == AF_INET )
                    s = inet_ntoa(((struct sockaddr_in*)sa)->sin_addr);
            }

            close(sock);
        }
    }

    return s;
}

uint16_t Socket::bind(uint16_t aPort, const string& aIp /* = 0.0.0.0 */) throw (SocketException){
    sockaddr_in sock_addr;

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(aPort);
    sock_addr.sin_addr.s_addr = inet_addr((SETTING(BIND_IFACE)? getIfaceI4(SETTING(BIND_IFACE_NAME)).c_str() : aIp.c_str()));

    if(::bind(sock, (sockaddr *)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        dcdebug("Bind failed, retrying with INADDR_ANY: %s\n", SocketException(getLastError()).getError().c_str());
        sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        check(::bind(sock, (sockaddr *)&sock_addr, sizeof(sock_addr)));
    }
    socklen_t size = sizeof(sock_addr);
    getsockname(sock, (sockaddr*)&sock_addr, (socklen_t*)&size);
    return ntohs(sock_addr.sin_port);
}

void Socket::listen() throw(SocketException) {
    check(::listen(sock, 20));
    connected = true;
}

void Socket::connect(const string& aAddr, uint16_t aPort) throw(SocketException) {
    sockaddr_in serv_addr;

    if(sock == INVALID_SOCKET) {
        create(TYPE_TCP);
    }

    string addr = resolve(aAddr);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(aPort);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(addr.c_str());

    int result;
    do {
        result = ::connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr));
    } while (result < 0 && getLastError() == EINTR);
    check(result, true);

    connected = true;
    setIp(addr);
}

namespace {
    inline uint64_t timeLeft(uint64_t start, uint64_t timeout) {
        if(timeout == 0) {
            return 0;
        }
        uint64_t now = GET_TICK();
        if(start + timeout < now)
            throw SocketException(_("Connection timeout"));
        return start + timeout - now;
    }
}

void Socket::socksConnect(const string& aAddr, uint16_t aPort, uint32_t timeout) throw(SocketException) {

    if(SETTING(SOCKS_SERVER).empty() || SETTING(SOCKS_PORT) == 0) {
        throw SocketException(_("The socks server failed establish a connection"));
    }

    uint64_t start = GET_TICK();

    connect(SETTING(SOCKS_SERVER), static_cast<uint16_t>(SETTING(SOCKS_PORT)));

    if(wait(timeLeft(start, timeout), WAIT_CONNECT) != WAIT_CONNECT) {
        throw SocketException(_("The socks server failed establish a connection"));
    }

    socksAuth(timeLeft(start, timeout));

    ByteVector connStr;

    // Authenticated, let's get on with it...
    connStr.push_back(5);           // SOCKSv5
    connStr.push_back(1);           // Connect
    connStr.push_back(0);           // Reserved

    if(BOOLSETTING(SOCKS_RESOLVE)) {
        connStr.push_back(3);       // Address type: domain name
        connStr.push_back((uint8_t)aAddr.size());
        connStr.insert(connStr.end(), aAddr.begin(), aAddr.end());
    } else {
        connStr.push_back(1);       // Address type: IPv4;
        unsigned long addr = inet_addr(resolve(aAddr).c_str());
        uint8_t* paddr = (uint8_t*)&addr;
        connStr.insert(connStr.end(), paddr, paddr+4);
    }

    uint16_t port = htons(aPort);
    uint8_t* pport = (uint8_t*)&port;
    connStr.push_back(pport[0]);
    connStr.push_back(pport[1]);

    writeAll(&connStr[0], connStr.size(), timeLeft(start, timeout));

    // We assume we'll get a ipv4 address back...therefore, 10 bytes...
    /// @todo add support for ipv6
    if(readAll(&connStr[0], 10, timeLeft(start, timeout)) != 10) {
        throw SocketException(_("The socks server failed establish a connection"));
    }

    if(connStr[0] != 5 || connStr[1] != 0) {
        throw SocketException(_("The socks server failed establish a connection"));
    }

    in_addr sock_addr;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.s_addr = *((unsigned long*)&connStr[4]);
    setIp(inet_ntoa(sock_addr));
}

void Socket::socksAuth(uint32_t timeout) throw(SocketException) {
    vector<uint8_t> connStr;

    uint64_t start = GET_TICK();

    if(SETTING(SOCKS_USER).empty() && SETTING(SOCKS_PASSWORD).empty()) {
        // No username and pw, easier...=)
        connStr.push_back(5);           // SOCKSv5
        connStr.push_back(1);           // 1 method
        connStr.push_back(0);           // Method 0: No auth...

        writeAll(&connStr[0], 3, timeLeft(start, timeout));

        if(readAll(&connStr[0], 2, timeLeft(start, timeout)) != 2) {
            throw SocketException(_("The socks server failed establish a connection"));
        }

        if(connStr[1] != 0) {
            throw SocketException(_("The socks server requires authentication"));
        }
    } else {
        // We try the username and password auth type (no, we don't support gssapi)

        connStr.push_back(5);           // SOCKSv5
        connStr.push_back(1);           // 1 method
        connStr.push_back(2);           // Method 2: Name/Password...
        writeAll(&connStr[0], 3, timeLeft(start, timeout));

        if(readAll(&connStr[0], 2, timeLeft(start, timeout)) != 2) {
            throw SocketException(_("The socks server failed establish a connection"));
        }
        if(connStr[1] != 2) {
            throw SocketException(_("The socks server doesn't support login / password authentication"));
        }

        connStr.clear();
        // Now we send the username / pw...
        connStr.push_back(1);
        connStr.push_back((uint8_t)SETTING(SOCKS_USER).length());
        connStr.insert(connStr.end(), SETTING(SOCKS_USER).begin(), SETTING(SOCKS_USER).end());
        connStr.push_back((uint8_t)SETTING(SOCKS_PASSWORD).length());
        connStr.insert(connStr.end(), SETTING(SOCKS_PASSWORD).begin(), SETTING(SOCKS_PASSWORD).end());

        writeAll(&connStr[0], connStr.size(), timeLeft(start, timeout));

        if(readAll(&connStr[0], 2, timeLeft(start, timeout)) != 2) {
            throw SocketException(_("Socks server authentication failed (bad login / password?)"));
        }

        if(connStr[1] != 0) {
            throw SocketException(_("Socks server authentication failed (bad login / password?)"));
        }
    }
}

int Socket::getSocketOptInt(int option) throw(SocketException) {
    int val;
    socklen_t len = sizeof(val);
    check(::getsockopt(sock, SOL_SOCKET, option, (char*)&val, &len));
    return val;
}

void Socket::setSocketOpt(int option, int val) throw(SocketException) {
    int len = sizeof(val);
    check(::setsockopt(sock, SOL_SOCKET, option, (char*)&val, len));
}

int Socket::read(void* aBuffer, int aBufLen) throw(SocketException) {
    int len = 0;

    dcassert(type == TYPE_TCP || type == TYPE_UDP);
    do {
        if(type == TYPE_TCP) {
            len = ::recv(sock, (char*)aBuffer, aBufLen, 0);
        } else {
            len = ::recvfrom(sock, (char*)aBuffer, aBufLen, 0, NULL, NULL);
        }
    } while (len < 0 && getLastError() == EINTR);
    check(len, true);

    if(len > 0) {
        stats.totalDown += len;
    }

    return len;
}

int Socket::read(void* aBuffer, int aBufLen, string &aIP) throw(SocketException) {
    dcassert(type == TYPE_UDP);

    sockaddr_in remote_addr = { 0 };
    socklen_t addr_length = sizeof(remote_addr);

    int len;
    do {
        len = ::recvfrom(sock, (char*)aBuffer, aBufLen, 0, (sockaddr*)&remote_addr, &addr_length);
    } while (len < 0 && getLastError() == EINTR);

    check(len, true);
    if(len > 0) {
        aIP = inet_ntoa(remote_addr.sin_addr);
        stats.totalDown += len;
    } else {
        aIP.clear();
    }
    return len;
}

int Socket::readAll(void* aBuffer, int aBufLen, uint32_t timeout) throw(SocketException) {
    uint8_t* buf = (uint8_t*)aBuffer;
    int i = 0;
    while(i < aBufLen) {
        int j = read(buf + i, aBufLen - i);
        if(j == 0) {
            return i;
        } else if(j == -1) {
            if(wait(timeout, WAIT_READ) != WAIT_READ) {
                return i;
            }
            continue;
        }

        i += j;
    }
    return i;
}

void Socket::writeAll(const void* aBuffer, int aLen, uint32_t timeout) throw(SocketException) {
    const uint8_t* buf = (const uint8_t*)aBuffer;
    int pos = 0;
    // No use sending more than this at a time...
    int sendSize = getSocketOptInt(SO_SNDBUF);

    while(pos < aLen) {
        int i = write(buf+pos, (int)min(aLen-pos, sendSize));
        if(i == -1) {
            wait(timeout, WAIT_WRITE);
        } else {
            pos+=i;
            stats.totalUp += i;
        }
    }
}

int Socket::write(const void* aBuffer, int aLen) throw(SocketException) {
    int sent;
    do {
        sent = ::send(sock, (const char*)aBuffer, aLen, MSG_NOSIGNAL);
    } while (sent < 0 && getLastError() == EINTR);

    check(sent, true);
    if(sent > 0) {
        stats.totalUp += sent;
    }
    return sent;
}

/**
* Sends data, will block until all data has been sent or an exception occurs
* @param aBuffer Buffer with data
* @param aLen Data length
* @throw SocketExcpetion Send failed.
*/
void Socket::writeTo(const string& aAddr, uint16_t aPort, const void* aBuffer, int aLen, bool proxy) throw(SocketException) {
    if(aLen <= 0)
        return;

    uint8_t* buf = (uint8_t*)aBuffer;
    if(sock == INVALID_SOCKET) {
        create(TYPE_UDP);
    }

    dcassert(type == TYPE_UDP);

    sockaddr_in serv_addr;

    if(aAddr.empty() || aPort == 0) {
        throw SocketException(EADDRNOTAVAIL);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    int sent;
    if(SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5 && proxy) {
        if(udpServer.empty() || udpPort == 0) {
            throw SocketException(_("Failed to set up the socks server for UDP relay (check socks address and port)"));
        }

        serv_addr.sin_port = htons(udpPort);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(udpServer.c_str());

        string s = BOOLSETTING(SOCKS_RESOLVE) ? resolve(ip) : ip;

        vector<uint8_t> connStr;

        connStr.push_back(0);       // Reserved
        connStr.push_back(0);       // Reserved
        connStr.push_back(0);       // Fragment number, always 0 in our case...

        if(BOOLSETTING(SOCKS_RESOLVE)) {
            connStr.push_back(3);
            connStr.push_back((uint8_t)s.size());
            connStr.insert(connStr.end(), aAddr.begin(), aAddr.end());
        } else {
            connStr.push_back(1);       // Address type: IPv4;
            unsigned long addr = inet_addr(resolve(aAddr).c_str());
            uint8_t* paddr = (uint8_t*)&addr;
            connStr.insert(connStr.end(), paddr, paddr+4);
        }

        connStr.insert(connStr.end(), buf, buf + aLen);

        do {
            sent = ::sendto(sock, (const char*)&connStr[0], connStr.size(), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        } while (sent < 0 && getLastError() == EINTR);
    } else {
        serv_addr.sin_port = htons(aPort);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(resolve(aAddr).c_str());
        do {
            sent = ::sendto(sock, (const char*)aBuffer, (int)aLen, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        } while (sent < 0 && getLastError() == EINTR);
    }

    check(sent);
    stats.totalUp += sent;
}

/**
 * Blocks until timeout is reached one of the specified conditions have been fulfilled
 * @param millis Max milliseconds to block.
 * @param waitFor WAIT_*** flags that set what we're waiting for, set to the combination of flags that
 *                triggered the wait stop on return (==WAIT_NONE on timeout)
 * @return WAIT_*** ored together of the current state.
 * @throw SocketException Select or the connection attempt failed.
 */
int Socket::wait(uint32_t millis, int waitFor) throw(SocketException) {
    timeval tv;
    fd_set rfd, wfd, efd;
    fd_set *rfdp = NULL, *wfdp = NULL;
    tv.tv_sec = millis/1000;
    tv.tv_usec = (millis%1000)*1000;

    if(waitFor & WAIT_CONNECT) {
        dcassert(!(waitFor & WAIT_READ) && !(waitFor & WAIT_WRITE));

        int result;
        do {
            FD_ZERO(&wfd);
            FD_ZERO(&efd);

            FD_SET(sock, &wfd);
            FD_SET(sock, &efd);
            result = select((int)(sock+1), 0, &wfd, &efd, &tv);
        } while (result < 0 && getLastError() == EINTR);
        check(result);

        if(FD_ISSET(sock, &wfd)) {
            return WAIT_CONNECT;
        }

        if(FD_ISSET(sock, &efd)) {
            int y = 0;
            socklen_t z = sizeof(y);
            check(getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&y, &z));

            if(y != 0)
                throw SocketException(y);
            // No errors! We're connected (?)...
            return WAIT_CONNECT;
        }
        return 0;
    }

    int result;
    do {
        if(waitFor & WAIT_READ) {
            dcassert(!(waitFor & WAIT_CONNECT));
            rfdp = &rfd;
            FD_ZERO(rfdp);
            FD_SET(sock, rfdp);
        }
        if(waitFor & WAIT_WRITE) {
            dcassert(!(waitFor & WAIT_CONNECT));
            wfdp = &wfd;
            FD_ZERO(wfdp);
            FD_SET(sock, wfdp);
        }

        result = select((int)(sock+1), rfdp, wfdp, NULL, &tv);
    } while (result < 0 && getLastError() == EINTR);
    check(result);

    waitFor = WAIT_NONE;

    if(rfdp && FD_ISSET(sock, rfdp)) {
        waitFor |= WAIT_READ;
    }
    if(wfdp && FD_ISSET(sock, wfdp)) {
        waitFor |= WAIT_WRITE;
    }

    return waitFor;
}

bool Socket::waitConnected(uint32_t millis) {
    return wait(millis, Socket::WAIT_CONNECT) == WAIT_CONNECT;
}

bool Socket::waitAccepted(uint32_t millis) {
    // Normal sockets are always connected after a call to accept
    return true;
}

string Socket::resolve(const string& aDns) {
#ifdef _WIN32
    sockaddr_in sock_addr;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_port = 0;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(aDns.c_str());

    if (sock_addr.sin_addr.s_addr == INADDR_NONE) {   /* server address is a name or invalid */
        hostent* host;
        host = gethostbyname(aDns.c_str());
        if (host == NULL) {
            return Util::emptyString;
        }
        sock_addr.sin_addr.s_addr = *((uint32_t*)host->h_addr);
        return inet_ntoa(sock_addr.sin_addr);
    } else {
        return aDns;
    }
#else
        // POSIX doesn't guarantee the gethostbyname to be thread safe. And it may (will) return a pointer to static data.
   string address = Util::emptyString;
   addrinfo hints = { 0 };
   addrinfo *result;
   hints.ai_family = AF_INET;

   if (getaddrinfo(aDns.c_str(), NULL, &hints, &result) == 0) {
       if (result->ai_addr != NULL)
           address = inet_ntoa(((sockaddr_in*)(result->ai_addr))->sin_addr);

       freeaddrinfo(result);
   }

   return address;
#endif
}

string Socket::getLocalIp() throw() {
    if(sock == INVALID_SOCKET)
        return Util::emptyString;

    sockaddr_in sock_addr;
    socklen_t len = sizeof(sock_addr);
    if(getsockname(sock, (sockaddr*)&sock_addr, &len) == 0) {
        return inet_ntoa(sock_addr.sin_addr);
    }
    return Util::emptyString;
}

uint16_t Socket::getLocalPort() throw() {
    if(sock == INVALID_SOCKET)
        return 0;

    sockaddr_in sock_addr;
    socklen_t len = sizeof(sock_addr);
    if(getsockname(sock, (sockaddr*)&sock_addr, &len) == 0) {
        return ntohs(sock_addr.sin_port);
    }
    return 0;
}

void Socket::socksUpdated() {
    udpServer.clear();
    udpPort = 0;

    if(SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5) {
        try {
            Socket s;
            s.setBlocking(false);
            s.connect(SETTING(SOCKS_SERVER), static_cast<uint16_t>(SETTING(SOCKS_PORT)));
            s.socksAuth(SOCKS_TIMEOUT);

            char connStr[10];
            connStr[0] = 5;         // SOCKSv5
            connStr[1] = 3;         // UDP Associate
            connStr[2] = 0;         // Reserved
            connStr[3] = 1;         // Address type: IPv4;
            *((long*)(&connStr[4])) = 0;        // No specific outgoing UDP address
            *((uint16_t*)(&connStr[8])) = 0;    // No specific port...

            s.writeAll(connStr, 10, SOCKS_TIMEOUT);

            // We assume we'll get a ipv4 address back...therefore, 10 bytes...if not, things
            // will break, but hey...noone's perfect (and I'm tired...)...
            if(s.readAll(connStr, 10, SOCKS_TIMEOUT) != 10) {
                return;
            }

            if(connStr[0] != 5 || connStr[1] != 0) {
                return;
            }

            udpPort = static_cast<uint16_t>(ntohs(*((uint16_t*)(&connStr[8]))));

            in_addr serv_addr;

            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.s_addr = *((long*)(&connStr[4]));
            udpServer = inet_ntoa(serv_addr);
        } catch(const SocketException&) {
            dcdebug("Socket: Failed to register with socks server\n");
        }
    }
}

void Socket::shutdown() throw() {
    if(sock != INVALID_SOCKET)
        ::shutdown(sock, 2);
}

void Socket::close() throw() {
    if(sock != INVALID_SOCKET) {
#ifdef _WIN32
        ::closesocket(sock);
#else
        ::close(sock);
#endif
        connected = false;
        sock = INVALID_SOCKET;
    }
}

void Socket::disconnect() throw() {
    shutdown();
    close();
}

} // namespace dcpp
