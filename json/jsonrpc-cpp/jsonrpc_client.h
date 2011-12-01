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
 * \file jsonrpc_client.h
 * \brief JSON-RPC client.
 * \author Sebastien Vincent
 */

#ifndef JSONRPC_CLIENT_H
#define JSONRPC_CLIENT_H

#include "jsonrpc_common.h"
#include "jsonrpc_handler.h"

#include "networking.h"

namespace Json
{

  namespace Rpc
  {
    /**
     * \class Client
     * \brief Abstract JSON-RPC client.
     */
    class Client
    {
      public:
        /**
         * \brief Constructor.
         * \param address remote network address or FQDN to contact
         * \param port remote local port to contact
         */
        Client(const std::string& address, uint16_t port);

        /**
         * \brief Destructor.
         */
        virtual ~Client();

        /**
         * \brief Set the encapsulated format (default is RAW).
         * \param format encapsulated format
         */
        void SetEncapsulatedFormat(enum EncapsulatedFormat format);

        /**
         * \brief Get the encapsulated format.
         * \return encapsulated format
         */
        enum EncapsulatedFormat GetEncapsulatedFormat() const;

        /**
         * \brief Get socket descriptor.
         * \return socket descriptor.
         */
        int GetSocket() const;

        /**
         * \brief Get the address.
         * \return address or FQDN
         */
        std::string GetAddress() const;

        /**
         * \brief Get the port.
         * \return local port
         */
        uint16_t GetPort() const;

        /**
         * \brief Connect to the remote machine
         * \return true if success, false otherwise
         * \note on connectionless protocol like UDP, this function
         * always returns true even if remote peer is not reachable.
         */
        virtual bool Connect();

        /**
         * \brief Receive data from the network.
         * \param data if data is received it will put in this reference
         * \return number of bytes received or -1 if error
         * \note This method will blocked until data comes.
         */
        virtual ssize_t Recv(std::string& data) = 0;

        /**
         * \brief Close socket.
         */
        virtual void Close();

      protected:
        /**
         * \brief Socket descriptor.
         */
        int m_sock;

        /**
         * \brief Transport protocol of the socket.
         */
        enum networking::TransportProtocol m_protocol;

        /**
         * \brief Remote socket address.
         */
        struct sockaddr_storage m_sockaddr;

        /**
         * \brief Remote socket address length.
         */
        socklen_t m_sockaddrlen;

      private:
        /**
         * \brief Network address or FQDN.
         */
        std::string m_address;

        /**
         * \brief Local port.
         */
        uint16_t m_port;

        /**
         * \brief Encapsulated format.
         */
        enum EncapsulatedFormat m_format;
    };

  } /* namespace Rpc */

} /* namespace Json */

#endif /* JSONRPC_CLIENT_H */

