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

#if !defined(SOCKET_H)
#define SOCKET_H

#include "Util.h"
#include "Exception.h"

#include <errno.h>

#ifdef _WIN32
// Berkely constants converted to the windows equivs...
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL

typedef int socklen_t;
typedef SOCKET socket_t;

#else

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

typedef int socket_t;
const int INVALID_SOCKET = -1;
#define SOCKET_ERROR -1
#endif

namespace dcpp {

class SocketException : public Exception {
public:
#ifdef _DEBUG
	SocketException(const string& aError) throw() : Exception("SocketException: " + aError) { }
#else //_DEBUG
	SocketException(const string& aError) throw() : Exception(aError) { }
#endif // _DEBUG

	SocketException(int aError) throw();
	virtual ~SocketException() throw() { }
private:
	static string errorToString(int aError) throw();
};

class Socket
{
public:
	enum {
		WAIT_NONE = 0x00,
		WAIT_CONNECT = 0x01,
		WAIT_READ = 0x02,
		WAIT_WRITE = 0x04
	};

	enum {
		TYPE_TCP,
		TYPE_UDP
	};

	Socket() throw(SocketException) : sock(INVALID_SOCKET), connected(false) { }
	Socket(const string& aIp, uint16_t aPort) throw(SocketException) : sock(INVALID_SOCKET), connected(false) { connect(aIp, aPort); }
	virtual ~Socket() throw() { Socket::disconnect(); }

	/**
	 * Connects a socket to an address/ip, closing any other connections made with
	 * this instance.
	 * @param aAddr Server address, in dns or xxx.xxx.xxx.xxx format.
	 * @param aPort Server port.
	 * @throw SocketException If any connection error occurs.
	 */
	virtual void connect(const string& aIp, uint16_t aPort) throw(SocketException);
	void connect(const string& aIp, const string& aPort) throw(SocketException) { connect(aIp, static_cast<uint16_t>(Util::toInt(aPort))); }
	/**
	 * Same as connect(), but through the SOCKS5 server
	 */
	void socksConnect(const string& aIp, uint16_t aPort, uint32_t timeout = 0) throw(SocketException);

	/**
	 * Sends data, will block until all data has been sent or an exception occurs
	 * @param aBuffer Buffer with data
	 * @param aLen Data length
	 * @throw SocketExcpetion Send failed.
	 */
	void writeAll(const void* aBuffer, int aLen, uint32_t timeout = 0) throw(SocketException);
	virtual int write(const void* aBuffer, int aLen) throw(SocketException);
	int write(const string& aData) throw(SocketException) { return write(aData.data(), (int)aData.length()); }
	virtual void writeTo(const string& aIp, uint16_t aPort, const void* aBuffer, int aLen, bool proxy = true) throw(SocketException);
	void writeTo(const string& aIp, uint16_t aPort, const string& aData) throw(SocketException) { writeTo(aIp, aPort, aData.data(), (int)aData.length()); }
	virtual void shutdown() throw();
	virtual void close() throw();
	void disconnect() throw();

	virtual bool waitConnected(uint32_t millis);
	virtual bool waitAccepted(uint32_t millis);

	/**
	 * Reads zero to aBufLen characters from this socket,
	 * @param aBuffer A buffer to store the data in.
	 * @param aBufLen Size of the buffer.
	 * @return Number of bytes read, 0 if disconnected and -1 if the call would block.
	 * @throw SocketException On any failure.
	 */
	virtual int read(void* aBuffer, int aBufLen) throw(SocketException);
	/**
	 * Reads zero to aBufLen characters from this socket,
	 * @param aBuffer A buffer to store the data in.
	 * @param aBufLen Size of the buffer.
	 * @param aIP Remote IP address
	 * @return Number of bytes read, 0 if disconnected and -1 if the call would block.
	 * @throw SocketException On any failure.
	 */
	virtual int read(void* aBuffer, int aBufLen, string &aIP) throw(SocketException);
	/**
	 * Reads data until aBufLen bytes have been read or an error occurs.
	 * If the socket is closed, or the timeout is reached, the number of bytes read
	 * actually read is returned.
	 * On exception, an unspecified amount of bytes might have already been read.
	 */
	int readAll(void* aBuffer, int aBufLen, uint32_t timeout = 0) throw(SocketException);

	virtual int wait(uint32_t millis, int waitFor) throw(SocketException);
	bool isConnected() { return connected; }

	static string resolve(const string& aDns);
	static uint64_t getTotalDown() { return stats.totalDown; }
	static uint64_t getTotalUp() { return stats.totalUp; }

#ifdef _WIN32
	void setBlocking(bool block) throw() {
		u_long b = block ? 0 : 1;
		ioctlsocket(sock, FIONBIO, &b);
	}
#else
	void setBlocking(bool block) throw() {
		int flags = fcntl(sock, F_GETFL, 0);
		if(block) {
			fcntl(sock, F_SETFL, flags & (~O_NONBLOCK));
		} else {
			fcntl(sock, F_SETFL, flags | O_NONBLOCK);
		}
	}
#endif

	string getLocalIp() throw();
	uint16_t getLocalPort() throw();

	// Low level interface
	virtual void create(int aType = TYPE_TCP) throw(SocketException);

	/** Binds a socket to a certain local port and possibly IP. */
	virtual uint16_t bind(uint16_t aPort = 0, const string& aIp = "0.0.0.0") throw(SocketException);
	virtual void listen() throw(SocketException);
	virtual void accept(const Socket& listeningSocket) throw(SocketException);

	int getSocketOptInt(int option) throw(SocketException);
	void setSocketOpt(int option, int value) throw(SocketException);

	virtual bool isSecure() const throw() { return false; }
	virtual bool isTrusted() const throw() { return false; }
	virtual std::string getCipherName() const throw() { return Util::emptyString; }
	virtual std::string getDigest() const throw() { return Util::emptyString; }

	/** When socks settings are updated, this has to be called... */
	static void socksUpdated();

	GETSET(string, ip, Ip);
protected:
	socket_t sock;
	int type;
	bool connected;

	class Stats {
	public:
		uint64_t totalDown;
		uint64_t totalUp;
	};
	static Stats stats;

	static string udpServer;
	static uint16_t udpPort;

private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);


	void socksAuth(uint32_t timeout) throw(SocketException);

#ifdef _WIN32
	static int getLastError() { return ::WSAGetLastError(); }
	static int checksocket(int ret) {
		if(ret == SOCKET_ERROR) {
			throw SocketException(getLastError());
		}
		return ret;
	}
	static int check(int ret, bool blockOk = false) {
		if(ret == SOCKET_ERROR) {
			int error = getLastError();
			if(blockOk && error == WSAEWOULDBLOCK) {
				return -1;
			} else {
				throw SocketException(error);
			}
		}
		return ret;
	}
#else
	static int getLastError() { return errno; }
	static int checksocket(int ret) {
		if(ret < 0) {
			throw SocketException(getLastError());
		}
		return ret;
	}
	static int check(int ret, bool blockOk = false) {
		if(ret == -1) {
			int error = getLastError();
			if(blockOk && (error == EWOULDBLOCK || error == ENOBUFS || error == EINPROGRESS || error == EAGAIN) ) {
				return -1;
			} else {
				throw SocketException(error);
			}
		}
		return ret;
	}
#endif

};

} // namespace dcpp

#endif // !defined(SOCKET_H)
