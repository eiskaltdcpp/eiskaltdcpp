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

#if !defined(SERVER_SOCKET_H)
#define SERVER_SOCKET_H

#include "Socket.h"
#include "Speaker.h"

namespace dcpp {

class ServerSocketListener {
public:
	virtual ~ServerSocketListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> IncomingConnection;
	virtual void on(IncomingConnection) throw() = 0;
};

class ServerSocket : public Speaker<ServerSocketListener> {
public:
	ServerSocket() throw() { }

	void listen(uint16_t port) throw(SocketException);
	void disconnect() throw() { socket.disconnect(); }

	/** This is called by windows whenever an "FD_ACCEPT" is sent...doesn't work with unix... */
	void incoming() {
		fire(ServerSocketListener::IncomingConnection());
	}

	operator const Socket&() const { return socket; }
private:
	ServerSocket(const ServerSocket&);
	ServerSocket& operator=(const ServerSocket&);

	friend class Socket;

	Socket socket;
};

} // namespace dcpp

#endif // !defined(SERVER_SOCKET_H)
