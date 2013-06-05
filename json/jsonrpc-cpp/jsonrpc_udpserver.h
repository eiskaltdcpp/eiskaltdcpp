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
 * \file jsonrpc_udpserver.h
 * \brief JSON-RPC UDP server.
 * \author Sebastien Vincent
 */

#ifndef JSONRPC_UDPSERVER_H
#define JSONRPC_UDPSERVER_H

#include "jsonrpc_common.h"
#include "jsonrpc_server.h"

namespace Json
{
  namespace Rpc
  {
    /**
     * \class UdpServer
     * \brief JSON-RPC UDP server implementation.
     */
    class UdpServer : public Server
    {
      public:
        /**
         * \brief Constructor.
         * \param address network address or FQDN to bind
         * \param port local port to bind
         */
        UdpServer(const std::string& address, uint16_t port);

        /**
         * \brief Destructor.
         */
        virtual ~UdpServer();

        /**
         * \brief Receive data from the network and process it.
         * \param fd file descriptor on which receive
         * \return true if message has been correctly received, processed and
         * response sent, false otherwise (mainly sendto/recvfrom error)
         * \note This method will blocked until data comes.
         */
        virtual bool Recv(int fd);

        /**
         * \brief Send data.
         * \param data data to send
         * \param addr sockaddr address
         * \param addrlen sizeof addr
         * \return number of bytes sent or -1 if error
         */
        ssize_t Send(const std::string& data, const struct sockaddr* addr, 
            socklen_t addrlen);

        /**
         * \brief Wait message.
         *
         * This function do a select() on the socket and Process() immediately 
         * the JSON-RPC message.
         * \param ms millisecond to wait (0 means infinite)
         */
        virtual void WaitMessage(uint32_t ms);

      private:
        /**
         * \brief Copy constructor (private because of "resource" class).
         * \param obj object to copy
         */
        UdpServer(const UdpServer& obj);

        /**
         * \brief Operator copy assignment (private because of "resource"
         * class).
         * \param obj object to copy
         * \return copied object reference
         */
        UdpServer& operator=(const UdpServer& obj);

    };
  } /* namespace Rpc */
} /* namespace Json */

#endif /* JSONRPC_UDPSERVER_H */

