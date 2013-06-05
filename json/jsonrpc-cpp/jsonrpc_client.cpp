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
 * \file jsonrpc_client.cpp
 * \brief JSON-RPC client.
 * \author Sebastien Vincent
 */

#include <cstring>

#include "jsonrpc_client.h"

namespace Json
{
  namespace Rpc
  {
    Client::Client()
    {
    }

    Client::Client(const std::string& address, uint16_t port)
    {
      m_sock = -1;
      m_address = address;
      m_port = port;
      SetEncapsulatedFormat(Json::Rpc::RAW);
      memset(&m_sockaddr, 0x00, sizeof(struct sockaddr_storage));
      m_sockaddrlen = 0;
    }

    Client::~Client()
    {
      if(m_sock != -1)
      {
        Close();
      }
    }

    void Client::SetEncapsulatedFormat(enum EncapsulatedFormat format)
    {
      m_format = format;
    }

    enum EncapsulatedFormat Client::GetEncapsulatedFormat() const
    {
      return m_format;
    }

    int Client::GetSocket() const
    {
      return m_sock;
    }

    std::string Client::GetAddress() const
    {
      return m_address;
    }

    void Client::SetAddress(const std::string& address)
    {
        m_address = address;
    }

    void Client::SetPort(uint16_t port)
    {
        m_port = port;
    }

    uint16_t Client::GetPort() const
    {
      return m_port;
    }

    bool Client::Connect()
    {
      m_sock = networking::connect(m_protocol, GetAddress(), GetPort(), 
          &m_sockaddr, &m_sockaddrlen);

      return (m_sock != -1) ? true : false;
    }

    void Client::Close()
    {
      ::close(m_sock);
      m_sock = -1;
    }
  } /* namespace Rpc */
} /* namespace Json */

