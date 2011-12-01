/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file networking.h
 * \brief Networking utils.
 * \author Sebastien Vincent
 */

#ifndef NETWORKING_H
#define NETWORKING_H

#ifdef _WIN32

#ifndef _MSC_VER
#include <stdint.h>
#endif

#include <windows.h>
#include <winsock2.h>

/* to use getaddrinfo, _WIN32_WINNT have to
 * equal at least 0x0501
 */
#define OLD_WIN32_WINNT _WIN32_WINNT

#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#include <ws2tcpip.h>

#if (_WIN32_WINNT != OLD_WIN32_WINNT)
#undef _WIN32_WINNT
#define _WIN32_WINNT OLD_WIN32_WINNT
#endif

typedef int socklen_t;
#define close closesocket

#else

#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <unistd.h>

#include <netinet/in.h>

#include <netdb.h>

#endif

#include <string>

/**
 * \namespace networking
 * \brief Networking related functions.
 */
namespace networking
{
  /**
   * \enum TransportProtocol
   * \brief Transport protocol.
   */
  enum TransportProtocol
  {
    UDP = IPPROTO_UDP, /**< UDP protocol. */
    TCP = IPPROTO_TCP /**< TCP protocol. */
  };

  /**
   * \brief Initialize networking.
   * \return true if network is correctly initialized, false otherwise
   * \note On MS Windows, this step is mandatory to use
   * socket API (socket(), bind(), recvfrom(), ...).
   */
  bool init();

  /**
   * \brief Cleanup networking.
   * \note On MS Windows, after calling this function,
   * it will be impossible to use socket API.
   */
  void cleanup();

  /**
   * \brief Connect to remote machine.
   * \param protocol transport protocol used
   * \param address remote address
   * \param port remote port
   * \param sockaddr if function succeed, sockaddr 
   * representation of address/port
   * \param addrlen if function succeed, length of sockaddr
   * \return socket descriptor if success, -1 otherwise
   */
  int connect(enum TransportProtocol protocol, const std::string& address, uint16_t port, struct sockaddr_storage* sockaddr, socklen_t* addrlen);

  /**
   * \brief Bind on a local address.
   * \param protocol transport protocol used
   * \param address local address
   * \param port local port
   * \param sockaddr if function succeed, sockaddr 
   * representation of address/port
   * \param addrlen if function succeed, length of sockaddr
   * \return socket descriptor if success, -1 otherwise
   */
  int bind(enum TransportProtocol protocol, const std::string& address, uint16_t port, struct sockaddr_storage* sockaddr, socklen_t* addrlen);

} /* namespace networking */

#endif /* NETWORKING_H */

