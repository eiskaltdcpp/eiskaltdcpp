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
 * \file jsonrpc_udpclient.h
 * \brief JSON-RPC UDP client.
 * \author Sebastien Vincent
 */

#ifndef JSONRPC_UDPCLIENT_H 
#define JSONRPC_UDPCLIENT_H 

#include <iostream>

#include "jsonrpc_client.h"

namespace Json
{

  namespace Rpc
  {
    /**
     * \class UdpClient
     * \brief JSON-RPC UDP client.
     */
    class UdpClient : public Client
    {
      public:
        /**
         * \brief Constructor.
         * \param address network address or FQDN to bind
         * \param port local port to bind
         */
        UdpClient(const std::string& address, uint16_t port);

        /**
         * \brief Destructor.
         */
        virtual ~UdpClient();

        /**
         * \brief Receive data from the network.
         * \param data if data is received it will put in this reference
         * \return number of bytes received or -1 if error
         * \note This method will blocked until data comes.
         */
        virtual ssize_t Recv(std::string& data);
        
        /**
         * \brief Send data.
         * \param data data to send
         * \return number of bytes sent or -1 if error
         */
        ssize_t Send(const std::string& data);
    };

  } /* namespace Rpc */

} /* namespace Json */

#endif /* JSONRPC_UDPCLIENT_H */

