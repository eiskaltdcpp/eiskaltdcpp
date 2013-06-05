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
 * \file networking.cpp
 * \brief Networking utils.
 * \author Sebastien Vincent
 */

#include <cstdio>
#include <cstring>

#include "networking.h"

namespace networking
{
#ifdef _WIN32
  /**
   * \var wsaData
   * \brief MS Windows object to start
   * networking stuff.
   */
  static WSAData wsaData;
#endif

  bool init()
  {
    bool ret = false;

#ifdef _WIN32
    ret = (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0);
#else
    /* unix-like */
    ret = true;
#endif

    return ret;
  }

  void cleanup()
  {
#ifdef _WIN32
    WSACleanup();
#endif
  }

  int connect(enum TransportProtocol protocol, const std::string& address,
      uint16_t port, struct sockaddr_storage* sockaddr, socklen_t* addrlen)
  {
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    struct addrinfo* p = NULL;
    char service[8];
    int sock = -1;

    if(!port || address == "")
    {
      return -1;
    }

    snprintf(service, sizeof(service), "%u", port);
    service[sizeof(service)-1] = 0x00;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = protocol == UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = protocol;
    hints.ai_flags = 0;

    if(getaddrinfo(address.c_str(), service, &hints, &res) != 0)
    {
      return -1;
    }

    for(p = res ; p ; p = p->ai_next)
    {
      sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

      if(sock == -1)
      {
        continue;
      }

      if(protocol == TCP && ::connect(sock, (struct sockaddr*)p->ai_addr,
            p->ai_addrlen) == -1)
      {
        ::close(sock);
        sock = -1;
        continue;
      }

      if(sockaddr)
      {
        memcpy(sockaddr, p->ai_addr, p->ai_addrlen);
      }

      if(addrlen)
      {
        *addrlen = p->ai_addrlen;
      }

      /* ok so now we have a socket bound, break the loop */
      break;
    }

    freeaddrinfo(res);
    p = NULL;

    return sock;
  }

  int bind(enum TransportProtocol protocol, const std::string& address,
      uint16_t port, struct sockaddr_storage* sockaddr, socklen_t* addrlen)
  {
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    struct addrinfo* p = NULL;
    char service[8];
    int sock = -1;

    if(!port || address == "")
    {
      return -1;
    }

    snprintf(service, sizeof(service), "%u", port);
    service[sizeof(service)-1] = 0x00;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = protocol == UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = protocol;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(address.c_str(), service, &hints, &res) != 0)
    {
      return -1;
    }

    for(p = res ; p ; p = p->ai_next)
    {
      int on = 1;

      sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

      if(sock == -1)
      {
        continue;
      }

#ifndef _WIN32
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

      /* accept IPv6 OR IPv4 on the same socket */
      on = 1;
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
#else
      (void)on;
#endif

      if(::bind(sock, p->ai_addr, p->ai_addrlen) == -1)
      {
        ::close(sock);
        sock = -1;
        continue;
      }

      if(sockaddr)
      {
        memcpy(sockaddr, p->ai_addr, p->ai_addrlen);
      }
        
      if(addrlen)
      {
        *addrlen = p->ai_addrlen;
      }

      /* ok so now we have a socket bound, break the loop */
      break;
    }

    freeaddrinfo(res);
    p = NULL;

    return sock;
  }
} /* namespace networking */

