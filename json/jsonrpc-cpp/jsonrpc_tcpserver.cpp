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
 * \file jsonrpc_tcpserver.cpp
 * \brief JSON-RPC TCP server.
 * \author Sebastien Vincent
 */

#include <stdexcept>

#include "jsonrpc_tcpserver.h"

#include "netstring.h"

#include <cstring>
#include <cerrno>
#include <cstdlib>

namespace Json
{

  namespace Rpc
  {

    TcpServer::TcpServer(const std::string& address, uint16_t port) : Server(address, port)
    {
      m_protocol = networking::TCP;
    }

    TcpServer::~TcpServer()
    {
      if(m_sock != -1)
      {
        Close();
      }
    }

    ssize_t TcpServer::Send(int fd, const std::string& data)
    {
      std::string rep = data;

      /* encoding if any */
      if(GetEncapsulatedFormat() == Json::Rpc::NETSTRING)
      {
        rep = netstring::encode(rep);
      }
      if (GetEncapsulatedFormat() == Json::Rpc::HTTP_POST)
      {
        std::string tmp = "HTTP/1.1 200 OK\r\nServer: eidcppd server\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: ";
        char v[16];
        snprintf(v, sizeof(v), "%u", rep.size());
        tmp += v;
        tmp += "\r\n\r\n";
        rep = tmp + rep;
      }

      return ::send(fd, rep.c_str(), rep.length(), 0);
    }

    bool TcpServer::Recv(int fd)
    {
      Json::Value response;
      ssize_t nb = -1;
      char buf[1500];

      nb = recv(fd, buf, sizeof(buf), 0);

      /* give the message to JsonHandler */
      if(nb > 0)
      {
        std::string msg = std::string(buf, nb);

        if(GetEncapsulatedFormat() == Json::Rpc::NETSTRING)
        {
          try
          {
            msg = netstring::decode(msg);
          }
          catch(const netstring::NetstringException& e)
          {
            /* error parsing Netstring */
            std::cerr << e.what() << std::endl;
            return false;
          }
        }
        if (GetEncapsulatedFormat() == Json::Rpc::HTTP_POST)
        {
            if (0 == msg.compare(0,15,"POST / HTTP/1.1"))
            {
                //printf("orig_msg: %s\n\n", msg.c_str()); fflush(stdout);
                size_t istart = msg.find("Content-Length: ");
                size_t iend = msg.find("\r\n\r\n{");
                size_t content_msg_size = ::atoi(msg.substr(istart+16,iend-istart-16).c_str());
                //printf("content_msg_size: %d\n", content_msg_size); fflush(stdout);
                if (iend != std::string::npos)
                {
                    msg = msg.substr(iend+4);
                    //printf("msg: %s msg_size: %d\n", msg.c_str(), msg.size()); fflush(stdout);
                    if (content_msg_size != msg.size())
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }

        m_jsonHandler.Process(msg, response);

        /* in case of notification message received, the response could be Json::Value::null */
        if(response != Json::Value::null)
        {
          Send(fd, m_jsonHandler.GetString(response));
        }

        return true;
      }
      else
      {
        m_purge.push_back(fd);
        return false;
      }
    }

    void TcpServer::WaitMessage(uint32_t ms)
    {
      fd_set fdsr;
      struct timeval tv;
      int max_sock = m_sock;

      tv.tv_sec = ms / 1000;
      tv.tv_usec = (ms % 1000 ) / 1000;

      FD_ZERO(&fdsr);

#ifdef _WIN32
      /* on Windows, a socket is not an int but a SOCKET (unsigned int) */
      FD_SET((SOCKET)m_sock, &fdsr);
#else
      FD_SET(m_sock, &fdsr);
#endif

      for(std::list<int>::iterator it = m_clients.begin() ; it != m_clients.end() ; ++it)
      {
#ifdef _WIN32
        FD_SET((SOCKET)(*it), &fdsr);
#else
        FD_SET((*it), &fdsr);
#endif

        if((*it) > max_sock)
        {
          max_sock = (*it);
        }
      }

      max_sock++;

      if(select(max_sock, &fdsr, NULL, NULL, ms ? &tv : NULL) > 0)
      {
        if(FD_ISSET(m_sock, &fdsr))
        {
          Accept();
        }

        for(std::list<int>::iterator it = m_clients.begin() ; it != m_clients.end() ; ++it)
        {
          if(FD_ISSET((*it), &fdsr))
          {
            Recv((*it));
          }
        }

        /* remove disconnect socket descriptor */
        for(std::list<int>::iterator it = m_purge.begin() ; it != m_purge.end() ; ++it)
        {
          m_clients.remove((*it));
        }

        /* purge disconnected list */
        m_purge.erase(m_purge.begin(), m_purge.end());
      }
      else
      {
        /* error */
      }
    }

    bool TcpServer::Listen() const
    {
      if(m_sock == -1)
      {
        return false;
      }

      if(listen(m_sock, 5) == -1)
      {
        return false;
      }

      return true;
    }

    bool TcpServer::Accept()
    {
      int client = -1;
      socklen_t addrlen = sizeof(struct sockaddr_storage);

      if(m_sock == -1)
      {
        return false;
      }

      client = accept(m_sock, 0, &addrlen);

      if(client == -1)
      {
        return false;
      }

      m_clients.push_back(client);
      return true;
    }

    void TcpServer::Close()
    {
      /* close all client sockets */
      for(std::list<int>::iterator it = m_clients.begin() ; it != m_clients.end() ; ++it)
      {
        ::close((*it));
      }
      m_clients.erase(m_clients.begin(), m_clients.end());

      /* listen socket should be closed in Server destructor */
    }

    const std::list<int> TcpServer::GetClients() const
    {
      return m_clients;
    }

  } /* namespace Rpc */

} /* namespace Json */

