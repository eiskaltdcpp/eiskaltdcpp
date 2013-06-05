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
 * \file jsonrpc_udpclient.cpp
 * \brief JSON-RPC UDP client.
 * \author Sebastien Vincent
 */

#include "jsonrpc_udpclient.h"

#include "netstring.h"

namespace Json
{
  namespace Rpc
  {
    UdpClient::UdpClient(const std::string& address, uint16_t port) : Client(address, port)
    {
      m_protocol = networking::UDP;
    }

    UdpClient::~UdpClient()
    {
    }

    ssize_t UdpClient::Send(const std::string& data)
    {
      std::string rep = data;

      /* encoding if any */
      if(GetEncapsulatedFormat() == Json::Rpc::NETSTRING)
      {
        rep = netstring::encode(rep);
      }

      return ::sendto(m_sock, rep.c_str(), rep.length(), 0,
          (struct sockaddr*)&m_sockaddr, m_sockaddrlen);
    }

    ssize_t UdpClient::Recv(std::string& data)
    {
      char buf[1500];
      ssize_t nb = -1;

      if((nb = ::recvfrom(m_sock, buf, sizeof(buf), 0, NULL, NULL)) == -1)
      {
        std::cerr << "Error while receiving" << std::endl;
        return -1;
      }

      data = std::string(buf, nb);

      /* decoding if any */
      if(GetEncapsulatedFormat() == Json::Rpc::NETSTRING)
      {
        try
        {
          data = netstring::decode(data);
        }
        catch(const netstring::NetstringException& e)
        {
          std::cerr << e.what() << std::endl;
        }
      }

      return nb;
    }
  } /* namespace Rpc */
} /* namespace Json */

