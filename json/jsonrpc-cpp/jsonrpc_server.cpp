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
 * \file jsonrpc_server.cpp
 * \brief JSON-RPC server.
 * \author Sebastien Vincent
 */

#include "jsonrpc_server.h"

namespace Json 
{
  namespace Rpc
  {
    Server::Server(const std::string& address, uint16_t port)
    {
      m_sock = -1;
      m_address = address;
      m_port = port;
      SetEncapsulatedFormat(Json::Rpc::RAW);
    }

    Server::~Server()
    {
      if(m_sock != -1)
      {
        Close();
      }
    }

    void Server::SetEncapsulatedFormat(enum EncapsulatedFormat format)
    {
      m_format = format;
    }

    enum EncapsulatedFormat Server::GetEncapsulatedFormat() const
    {
      return m_format;
    }

    int Server::GetSocket() const
    {
      return m_sock;
    }

    std::string Server::GetAddress() const
    {
      return m_address;
    }

    uint16_t Server::GetPort() const
    {
      return m_port;
    }

    bool Server::Bind()
    {
      m_sock = networking::bind(m_protocol, m_address, m_port, NULL, NULL);

      return (m_sock != -1) ? true : false;
    }
    
    void Server::Close()
    {
      ::close(m_sock);
      m_sock = -1;
    }

    void Server::AddMethod(CallbackMethod* method)
    {
      m_jsonHandler.AddMethod(method);
    }

    void Server::DeleteMethod(const std::string& method)
    {
      m_jsonHandler.DeleteMethod(method);
    }
  } /* namespace Rpc */
} /* namespace Json */

