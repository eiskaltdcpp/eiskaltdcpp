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
#include "Socket.h"

#include "ConnectivityManager.h"
#include "format.h"
#include "SettingsManager.h"
#include "TimerManager.h"

#ifndef _WIN32
#ifndef __HAIKU__
  #include <ifaddrs.h>
#endif
#include <net/if.h>
#endif

#ifdef __HAIKU__
#include <sys/sockio.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/// @todo remove when MinGW has this
#ifdef __MINGW32__
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#endif
#endif

#ifndef IP_TOS
#define        IP_TOS          1
#endif
#ifndef IPTOS_TOS
#define IPTOS_TOS(a) ((a) & 0x1E)
#endif

#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

#ifndef IPV6_V6ONLY
#ifdef _WIN32 // Mingw seems to lack this...
#define IPV6_V6ONLY 27
#endif
#endif

namespace dcpp {

namespace {

#ifdef _WIN32

template<typename F>
inline auto check(F f, bool blockOk = false) -> decltype(f()) {
    for(;;) {
        auto ret = f();
        if(ret != static_cast<decltype(ret)>(SOCKET_ERROR)) {
            return ret;
        }

        auto error = Socket::getLastError();
        if(blockOk && error == WSAEWOULDBLOCK) {
            return static_cast<decltype(ret)>(-1);
        }

        if(error != EINTR) {
            //printf("Socket::check %d\n", error); fflush(stdout);
            throw SocketException(error);
        }
    }
}

inline void setBlocking2(socket_t sock, bool block) noexcept {
    u_long b = block ? 0 : 1;
    ioctlsocket(sock, FIONBIO, &b);
}

#else

template<typename F>
inline auto check(F f, bool blockOk = false) -> decltype(f()) {
    for(;;) {
        auto ret = f();
        if(ret != static_cast<decltype(ret)>(-1)) {
            return ret;
        }

        auto error = Socket::getLastError();
        if(blockOk && (error == EWOULDBLOCK || error == ENOBUFS || error == EINPROGRESS || error == EAGAIN)) {
            return -1;
        }

        if(error != EINTR) {
            //printf("Socket::check %d\n", error); fflush(stdout);
            throw SocketException(error);
        }
    }
}

inline void setBlocking2(socket_t sock, bool block) noexcept {
    int flags = fcntl(sock, F_GETFL, 0);
    if(block) {
        fcntl(sock, F_SETFL, flags & (~O_NONBLOCK));
    } else {
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }
}

#endif

inline int getSocketOptInt2(socket_t sock, int option) {
    int val;
    socklen_t len = sizeof(val);
    check([&] { return ::getsockopt(sock, SOL_SOCKET, option, (char*)&val, &len); });
    return val;
}

inline int setSocketOpt2(socket_t sock, int level, int option, int val) {
    int len = sizeof(val);
    return ::setsockopt(sock, level, option, (char*)&val, len);
}

inline bool isConnected(socket_t sock) {
    fd_set wfd;
    struct timeval tv = { 0 };

    FD_ZERO(&wfd);
    FD_SET(sock, &wfd);

    if(::select(sock + 1, NULL, &wfd, NULL, &tv) == 1) {
        if (getSocketOptInt2(sock, SO_ERROR) == 0) {
            return true;
        }
    }

        return false;
}

inline int readable(socket_t sock0, socket_t sock1) {
    fd_set rfd;
    struct timeval tv = { 0 };

    FD_ZERO(&rfd);
    FD_SET(sock0, &rfd);
    FD_SET(sock1, &rfd);

    if(::select(std::max(sock0, sock1) + 1, &rfd, NULL, NULL, &tv) > 0) {
        return FD_ISSET(sock0, &rfd) ? sock0 : sock1;
    }

    return sock0;
}

}

Socket::addr Socket::udpAddr;
socklen_t Socket::udpAddrLen;

#ifdef _DEBUG

SocketException::SocketException(int aError) noexcept {
    error = "SocketException: " + errorToString(aError);
    dcdebug("Thrown: %s\n", error.c_str());
}

#else // _DEBUG

SocketException::SocketException(int aError) noexcept : Exception(errorToString(aError)) { }

#endif

#ifdef _WIN32

void SocketHandle::reset(socket_t s) {
    if(valid()) {
        ::closesocket(sock);
    }

    sock = s;
}

int Socket::getLastError() { return ::WSAGetLastError(); }

#else

void SocketHandle::reset(socket_t s) {
    if(valid()) {
        ::close(sock);
    }

    sock = s;
}

int Socket::getLastError() { return errno; }

#endif

Socket::Stats Socket::stats = { 0, 0 };

static const uint32_t SOCKS_TIMEOUT = 30000;

string SocketException::errorToString(int aError) noexcept {
    string msg = Util::translateError(aError);
    if(msg.empty()) {
        msg = str(F_("Unknown error: 0x%1$x") % aError);
    }
    //printf("SocketException::errorToString %s\n", msg.c_str()); fflush(stdout);

    return msg;
}

socket_t Socket::setSock(socket_t s, int af) {
    setBlocking2(s, false);
    setSocketOpt2(s, SOL_SOCKET, SO_REUSEADDR, 1);

    if(af == AF_INET) {
        dcassert(sock4 == INVALID_SOCKET);
        sock4 = s;
    } else if(af == AF_INET6) {
        dcassert(sock6 == INVALID_SOCKET);
        setSocketOpt2(s, IPPROTO_IPV6, IPV6_V6ONLY, 1);
        sock6 = s;
    } else {
        throw SocketException(str(F_("Unknown protocol %d") % af));
    }

    return s;
}

socket_t Socket::getSock() const {
    if(sock6.valid()) {
        if(sock4.valid()) {
            if(isConnected(sock6)) {
                dcdebug("Closing IPv4, IPv6 connected");
                sock4.reset();
            } else if(isConnected(sock4)) {
                dcdebug("Closing IPv6, IPv4 connected");
                sock6.reset();
                return sock4;
            }

            dcdebug("Both v4 & v6 sockets valid and unconnected, returning v6...\n");
            // TODO Neither connected - but this will become a race if the IPv4 socket connects while
            // we're still using the IPv6 one...
        }

        return sock6;
    }

    return sock4;
}

void Socket::setBlocking(bool block) noexcept {
    if(sock4.valid()) setBlocking2(sock4, block);
    if(sock6.valid()) setBlocking2(sock6, block);
}

socket_t Socket::create(const addrinfo& ai) {
    return setSock(check([&] {
        if (SETTING(IP_TOS_VALUE) != -1) setSocketOpt(IP_TOS, IPTOS_TOS(SETTING(IP_TOS_VALUE)));
        return ::socket(ai.ai_family, ai.ai_socktype, ai.ai_protocol);
        }), ai.ai_family);
}

uint16_t Socket::accept(const Socket& listeningSocket) {
    disconnect();

    addr sock_addr = { { 0 } };
    socklen_t sz = sizeof(sock_addr);

    auto sock = check([&] { return ::accept(readable(listeningSocket.sock4, listeningSocket.sock6), &sock_addr.sa, &sz); });
    setSock(sock, sock_addr.sa.sa_family);

#ifdef _WIN32
    // Make sure we disable any inherited windows message things for this socket.
    ::WSAAsyncSelect(sock, NULL, 0, 0);
#endif

    // remote IP
    setIp(resolveName(&sock_addr.sa, sz));

    // return the remote port
    if(sock_addr.sa.sa_family == AF_INET) {
            return ntohs(sock_addr.sai.sin_port);
    }
    if(sock_addr.sa.sa_family == AF_INET6) {
            return ntohs(sock_addr.sai6.sin6_port);
    }
    return 0;
}

string Socket::listen(const string& port) {
    disconnect();

    // For server sockets we create both ipv4 and ipv6 if possible
    // We use the same port for both sockets to deal with the fact that
    // there's no way in ADC to have different ports for v4 and v6 TCP sockets

    uint16_t ret = 0;

    addrinfo_p ai(nullptr, nullptr);

    if(!v4only) {
        try { ai = resolveAddr(localIp6, port, AF_INET6, AI_PASSIVE | AI_ADDRCONFIG); }
        catch(const SocketException&) { ai.reset(); }
        for(auto a = ai.get(); a && !sock6.valid(); a = a->ai_next) {
            try {
                create(*a);
                if(ret != 0) {
                    ((sockaddr_in6*)a->ai_addr)->sin6_port = ret;
                }

                check([&] { return ::bind(sock6, a->ai_addr, a->ai_addrlen); });
                check([&] { return ::getsockname(sock6, a->ai_addr, (socklen_t*)&a->ai_addrlen); });
                ret = ((sockaddr_in6*)a->ai_addr)->sin6_port;

                if(type == TYPE_TCP) {
                    check([&] { return ::listen(sock6, 20); });
                }
            } catch(const SocketException&) { }
        }
    }

    try { ai = resolveAddr(localIp4, port, AF_INET, AI_PASSIVE | AI_ADDRCONFIG); }
    catch(const SocketException&) { ai.reset(); }
    for(auto a = ai.get(); a && !sock4.valid(); a = a->ai_next) {
        try {
            create(*a);
            if(ret != 0) {
                ((sockaddr_in*)a->ai_addr)->sin_port = ret;
            }

            check([&] { return ::bind(sock4, a->ai_addr, a->ai_addrlen); });
            check([&] { return ::getsockname(sock4, a->ai_addr, (socklen_t*)&a->ai_addrlen); });
            ret = ((sockaddr_in*)a->ai_addr)->sin_port;

            if(type == TYPE_TCP) {
                check([&] { return ::listen(sock4, 20); });
            }
        } catch(const SocketException&) { }
    }

    if(ret == 0) {
        throw SocketException(_("Could not open port for listening"));
    }
    return Util::toString(ntohs(ret));
}

void Socket::connect(const string& aAddr, const string& aPort, const string& localPort) {
    disconnect();

    // We try to connect to both IPv4 and IPv6 if available
    auto addr = resolveAddr(aAddr, aPort);

    for(auto ai = addr.get(); ai; ai = ai->ai_next) {
        if((ai->ai_family == AF_INET && !sock4.valid()) ||
        (ai->ai_family == AF_INET6 && !sock6.valid() && !v4only))
        {
            auto sock = create(*ai);
            auto &localIp = ai->ai_family == AF_INET ? getLocalIp4() : getLocalIp6();

            if(!localPort.empty() || !localIp.empty()) {
                auto local = resolveAddr(localIp, localPort, ai->ai_family);
                check([&] { return ::bind(sock, local->ai_addr, local->ai_addrlen); });
            }

            check([&] { return ::connect(sock, ai->ai_addr, ai->ai_addrlen); }, true);
            setIp(resolveName(ai->ai_addr, ai->ai_addrlen));
        }
    }
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

void Socket::socksConnect(const string& aAddr, const string& aPort, uint32_t timeout) {
    if(SETTING(SOCKS_SERVER).empty() || SETTING(SOCKS_PORT) == 0) {
        throw SocketException(_("The socks server failed establish a connection"));
    }

    uint64_t start = GET_TICK();

    connect(SETTING(SOCKS_SERVER), Util::toString(SETTING(SOCKS_PORT)));

    if(!waitConnected(timeLeft(start, timeout))) {
        throw SocketException(_("The socks server failed establish a connection"));
    }

    socksAuth(timeLeft(start, timeout));

    ByteVector connStr;

    // Authenticated, let's get on with it...
    connStr.push_back(5);                   // SOCKSv5
    connStr.push_back(1);                   // Connect
    connStr.push_back(0);                   // Reserved

    if(SETTING(SOCKS_RESOLVE)) {
        connStr.push_back(3);           // Address type: domain name
        connStr.push_back((uint8_t)aAddr.size());
        connStr.insert(connStr.end(), aAddr.begin(), aAddr.end());
    } else {
        connStr.push_back(1);           // Address type: IPv4;
        unsigned long addr = inet_addr(resolve(aAddr, AF_INET).c_str());
        uint8_t* paddr = (uint8_t*)&addr;
        connStr.insert(connStr.end(), paddr, paddr+4);
    }

    uint16_t port = htons(static_cast<uint16_t>(Util::toInt(aPort)));
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

void Socket::socksAuth(uint32_t timeout) {
    vector<uint8_t> connStr;

    uint64_t start = GET_TICK();

    if(SETTING(SOCKS_USER).empty() && SETTING(SOCKS_PASSWORD).empty()) {
        // No username and pw, easier...=)
        connStr.push_back(5);                   // SOCKSv5
        connStr.push_back(1);                   // 1 method
        connStr.push_back(0);                   // Method 0: No auth...

        writeAll(&connStr[0], 3, timeLeft(start, timeout));

        if(readAll(&connStr[0], 2, timeLeft(start, timeout)) != 2) {
            throw SocketException(_("The socks server failed establish a connection"));
        }

        if(connStr[1] != 0) {
            throw SocketException(_("The socks server requires authentication"));
        }
    } else {
        // We try the username and password auth type (no, we don't support gssapi)

        connStr.push_back(5);                   // SOCKSv5
        connStr.push_back(1);                   // 1 method
        connStr.push_back(2);                   // Method 2: Name/Password...
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

int Socket::getSocketOptInt(int option) {
    int val;
    socklen_t len = sizeof(val);
    check([&] { return ::getsockopt(getSock(), SOL_SOCKET, option, (char*)&val, &len); });
    return val;
}

void Socket::setSocketOpt(int option, int val) {
    int len = sizeof(val);
    if(sock4.valid()) {
        check([&] { return ::setsockopt(sock4, SOL_SOCKET, option, (char*)&val, len); });
    }

    if(sock6.valid()) {
        check([&] { return ::setsockopt(sock6, SOL_SOCKET, option, (char*)&val, len); });
    }
}

int Socket::read(void* aBuffer, int aBufLen) {
    auto len = check([&] {
        return type == TYPE_TCP
            ? ::recv(getSock(), (char*)aBuffer, aBufLen, 0)
            : ::recvfrom(readable(sock4, sock6), (char*)aBuffer, aBufLen, 0, NULL, NULL);
    }, true);

    if(len > 0) {
        stats.totalDown += len;
    }

    return len;
}

int Socket::read(void* aBuffer, int aBufLen, string &aIP, string &aPort) {
    dcassert(type == TYPE_UDP);

    addr remote_addr = { { 0 } };
    socklen_t addr_length = sizeof(remote_addr);

    auto len = check([&] {
        return ::recvfrom(readable(sock4, sock6), (char*)aBuffer, aBufLen, 0, &remote_addr.sa, &addr_length);
    }, true);

    if (remote_addr.sa.sa_family == AF_INET)
        aPort = Util::toString(remote_addr.sai.sin_port);
    else if (remote_addr.sa.sa_family == AF_INET6)
        aPort = Util::toString(remote_addr.sai6.sin6_port);

    if(len > 0) {
        aIP = resolveName(&remote_addr.sa, addr_length);
        stats.totalDown += len;
    } else {
        aIP.clear();
    }

    return len;
}

int Socket::readAll(void* aBuffer, int aBufLen, uint32_t timeout) {
    uint8_t* buf = (uint8_t*)aBuffer;
    int i = 0;
    while(i < aBufLen) {
        int j = read(buf + i, aBufLen - i);
        if(j == 0) {
            return i;
        } else if(j == -1) {
            if(!wait(timeout, true, false).first) {
                return i;
            }
            continue;
        }

        i += j;
    }
    return i;
}

void Socket::writeAll(const void* aBuffer, int aLen, uint32_t timeout) {
    const uint8_t* buf = (const uint8_t*)aBuffer;
    int pos = 0;
    // No use sending more than this at a time...
    int sendSize = getSocketOptInt(SO_SNDBUF);

    while(pos < aLen) {
        int i = write(buf+pos, (int)std::min(aLen-pos, sendSize));
        if(i == -1) {
            wait(timeout, false, true);
        } else {
            pos+=i;
            stats.totalUp += i;
        }
    }
}

int Socket::write(const void* aBuffer, int aLen) {
    auto sent = check([&] { return ::send(getSock(), (const char*)aBuffer, aLen, 0); }, true);
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
void Socket::writeTo(const string& aAddr, const string& aPort, const void* aBuffer, int aLen, bool proxy) {
    if(aLen <= 0)
        return;

    if(aAddr.empty() || aPort.empty()) {
        throw SocketException(EADDRNOTAVAIL);
    }

    auto buf = (const uint8_t*)aBuffer;

    int sent;
    if(proxy && SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5) {
        if(udpAddr.sa.sa_family == 0) {
            throw SocketException(_("Failed to set up the socks server for UDP relay (check socks address and port)"));
        }

        vector<uint8_t> connStr;

        connStr.reserve(aLen + 24);

        connStr.push_back(0);           // Reserved
        connStr.push_back(0);           // Reserved
        connStr.push_back(0);           // Fragment number, always 0 in our case...

        if(SETTING(SOCKS_RESOLVE)) {
            connStr.push_back(3);
            connStr.push_back((uint8_t)aAddr.size());
            connStr.insert(connStr.end(), aAddr.begin(), aAddr.end());
        } else {
            auto ai = resolveAddr(aAddr, aPort);

            if(ai->ai_family == AF_INET) {
                connStr.push_back(1);           // Address type: IPv4
                uint8_t* paddr = (uint8_t*)&((sockaddr_in*)ai->ai_addr)->sin_addr;
                connStr.insert(connStr.end(), paddr, paddr+4);
            } else if(ai->ai_family == AF_INET6) {
                connStr.push_back(4);           // Address type: IPv6
                uint8_t* paddr = (uint8_t*)&((sockaddr_in6*)ai->ai_addr)->sin6_addr;
                connStr.insert(connStr.end(), paddr, paddr+16);
            }
        }

        connStr.insert(connStr.end(), buf, buf + aLen);

        sent = check([&] { return ::sendto(udpAddr.sa.sa_family == AF_INET ? sock4 : sock6,
                (const char*)&connStr[0], (int)connStr.size(), 0, &udpAddr.sa, udpAddrLen); });
    } else {
        auto ai = resolveAddr(aAddr, aPort);
        if((ai->ai_family == AF_INET && !sock4.valid()) || (ai->ai_family == AF_INET6 && !sock6.valid())) {
            create(*ai);
        }
        sent = check([&] { return ::sendto(ai->ai_family == AF_INET ? sock4 : sock6,
            (const char*)aBuffer, (int)aLen, 0, ai->ai_addr, ai->ai_addrlen); });
    }

    stats.totalUp += sent;
}

/**
 * Blocks until timeout is reached one of the specified conditions have been fulfilled
 * @param millis Max milliseconds to block.
 * @param checkRead Check for reading
 * @param checkWrite Check for writing
 * @return pair with read/write state respectively
 * @throw SocketException Select or the connection attempt failed.
 */
std::pair<bool, bool> Socket::wait(uint32_t millis, bool checkRead, bool checkWrite) {
    timeval tv = { millis/1000, (millis%1000)*1000 };
    fd_set rfd, wfd;
    fd_set *rfdp = NULL, *wfdp = NULL;

    int nfds = -1;

    if(checkRead) {
        rfdp = &rfd;
        FD_ZERO(rfdp);
        if(sock4.valid()) {
            FD_SET(sock4, &rfd);
            nfds = std::max((int)sock4, nfds);
        }

        if(sock6.valid()) {
            FD_SET(sock6, &rfd);
            nfds = std::max((int)sock6, nfds);
        }
    }

    if(checkWrite) {
        wfdp = &wfd;
        FD_ZERO(wfdp);
        if(sock4.valid()) {
            FD_SET(sock4, &wfd);
            nfds = std::max((int)sock4, nfds);
        }

        if(sock6.valid()) {
            FD_SET(sock6, &wfd);
            nfds = std::max((int)sock6, nfds);
        }
    }

    check([&] { return ::select(nfds + 1, rfdp, wfdp, NULL, &tv); });

    return std::make_pair(
        rfdp && ((sock4.valid() && FD_ISSET(sock4, rfdp)) || (sock6.valid() && FD_ISSET(sock6, rfdp))),
        wfdp && ((sock4.valid() && FD_ISSET(sock4, wfdp)) || (sock6.valid() && FD_ISSET(sock6, wfdp))));
}

bool Socket::waitConnected(uint32_t millis) {
    timeval tv = { millis/1000, (millis%1000)*1000 };
    fd_set fd;
    FD_ZERO(&fd);

    int nfds = -1;
    if(sock4.valid()) {
        FD_SET(sock4, &fd);
        nfds = sock4;
    }

    if(sock6.valid()) {
        FD_SET(sock6, &fd);
        nfds = std::max((int)sock6, nfds);
    }

    check([&] { return ::select(nfds + 1, NULL, &fd, NULL, &tv); });

    if(sock6.valid() && FD_ISSET(sock6, &fd)) {
        int err6 = getSocketOptInt2(sock6, SO_ERROR);
        if(err6 == 0) {
            sock4.reset(); // We won't be needing this any more...
            return true;
        }

        if(!sock4.valid()) {
            //printf("!sock4.valid() %d\n", err6); fflush(stdout);
            throw SocketException(err6);
        }

        sock6.reset();
    }

    if(sock4.valid() && FD_ISSET(sock4, &fd)) {
        int err4 = getSocketOptInt2(sock4, SO_ERROR);
        if(err4 == 0) {
            sock6.reset(); // We won't be needing this any more...
            return true;
        }

        if(!sock6.valid()) {
            //printf("!sock6.valid() %d\n", err4); fflush(stdout);
            throw SocketException(err4);
        }

        sock4.reset();
    }

    return false;
}

bool Socket::waitAccepted(uint32_t millis) {
    // Normal sockets are always connected after a call to accept
    return true;
}

string Socket::resolve(const string& aDns, int af) noexcept {
    addrinfo hints = { 0 };
    hints.ai_family = af;

    addrinfo *result = 0;

    string ret;

    if(!::getaddrinfo(aDns.c_str(), NULL, &hints, &result)) {
        try { ret = resolveName(result->ai_addr, result->ai_addrlen); }
        catch(const SocketException&) { }

        ::freeaddrinfo(result);
    }

    return ret;
}

Socket::addrinfo_p Socket::resolveAddr(const string& name, const string& port, int family, int flags) {
    addrinfo hints = { 0 };
    hints.ai_family = family;
    hints.ai_flags = flags;
    //hints.ai_socktype = type == TYPE_TCP ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = type;

    addrinfo *result = 0;

    auto err = ::getaddrinfo(name.c_str(), port.empty() ? NULL : port.c_str(), &hints, &result);
    if(err) {
        string err_str = gai_strerror(err);
        //printf("Socket::resolveAddr name->%s port->%s %s %d\n",name.c_str(), port.empty()? "0": port.c_str() , err_str.c_str(), err); fflush(stdout);
        throw SocketException(err);
    }


    dcdebug("Resolved %s:%s to %s, next is %p\n", name.c_str(), port.c_str(),
        resolveName(result->ai_addr, result->ai_addrlen).c_str(), result->ai_next);

    return addrinfo_p(result, &freeaddrinfo);
}

string Socket::resolveName(const sockaddr* sa, socklen_t sa_len, int flags) {
    char buf[1024];

    auto err = ::getnameinfo(sa, sa_len, buf, sizeof(buf), NULL, 0, flags);
    if(err) {
        //printf("Socket::resolveName %d\n", err); fflush(stdout);
        throw SocketException(err);
    }

    return string(buf);
}

string Socket::getLocalIp() noexcept {
    if(getSock() == INVALID_SOCKET)
        return Util::emptyString;

    addr sock_addr;
    socklen_t len = sizeof(sock_addr);
    if(::getsockname(getSock(), &sock_addr.sa, &len) == 0) {
        try { return resolveName(&sock_addr.sa, len); }
        catch(const SocketException&) { }
    }

    return Util::emptyString;
}

uint16_t Socket::getLocalPort() noexcept {
    if(getSock() == INVALID_SOCKET)
        return 0;

    addr sock_addr;
    socklen_t len = sizeof(sock_addr);
    if(::getsockname(getSock(), &sock_addr.sa, &len) == 0) {
        if(sock_addr.sa.sa_family == AF_INET) {
            return ntohs(sock_addr.sai.sin_port);
        } else if(sock_addr.sa.sa_family == AF_INET6) {
            return ntohs(sock_addr.sai6.sin6_port);
        }
    }
    return 0;
}

void Socket::socksUpdated() {
    memset(&udpAddr, 0, sizeof(udpAddr));
    udpAddrLen = sizeof(udpAddr);

    if(SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5) {
        try {
            Socket s(TYPE_TCP);
            s.setBlocking(false);
            s.connect(SETTING(SOCKS_SERVER), static_cast<uint16_t>(SETTING(SOCKS_PORT)));
            s.socksAuth(SOCKS_TIMEOUT);

            char connStr[10];
            connStr[0] = 5;                 // SOCKSv5
            connStr[1] = 3;                 // UDP Associate
            connStr[2] = 0;                 // Reserved
            connStr[3] = 1;                 // Address type: IPv4;
            *((long*)(&connStr[4])) = 0;            // No specific outgoing UDP address
            *((uint16_t*)(&connStr[8])) = 0;        // No specific port...

            s.writeAll(connStr, 10, SOCKS_TIMEOUT);

            // We assume we'll get a ipv4 address back...therefore, 10 bytes...if not, things
            // will break, but hey...noone's perfect (and I'm tired...)...
            if(s.readAll(connStr, 10, SOCKS_TIMEOUT) != 10) {
                return;
            }

            if(connStr[0] != 5 || connStr[1] != 0) {
                return;
            }

            udpAddr.sa.sa_family = AF_INET;
            udpAddr.sai.sin_port = *((uint16_t*)(&connStr[8]));
            udpAddr.sai.sin_addr.s_addr = htonl(*((long*)(&connStr[4])));//*((long*)(&connStr[4]));
            udpAddrLen = sizeof(udpAddr.sai);
        } catch(const SocketException&) {
            dcdebug("Socket: Failed to register with socks server\n");
        }
    }
}

void Socket::shutdown() noexcept {
    if(sock4.valid()) ::shutdown(sock4, 2);
    if(sock6.valid()) ::shutdown(sock6, 2);
}

void Socket::close() noexcept {
    sock4.reset();
    sock6.reset();
}

void Socket::disconnect() noexcept {
    shutdown();
    close();
}

} // namespace dcpp
