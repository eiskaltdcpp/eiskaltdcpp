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
 * \file jsonrpc_httpserver.h
 * \brief JSON-RPC HTTPServer.
 * \author Eugene Petrov
 */

#ifndef JSONRPC_HTTPSERVER_H
#define JSONRPC_HTTPSERVER_H

#include "jsonrpc_common.h"
#include "jsonrpc_handler.h"
#include "mongoose.h"

namespace Json
{

  namespace Rpc
  {
    /**
     * \class HTTPServer
     * \brief Abstract JSON-RPC HTTPServer.
     */
    class HTTPServer
    {
      public:
        /**
         * \brief Constructor.
         * \param address network address or FQDN to bind
         * \param port local port to bind
         */
        HTTPServer(const std::string& address, uint16_t port);

        /**
         * \brief Destructor.
         */
        virtual ~HTTPServer();

        bool startPolling();
        bool stopPolling();
        bool onRequest(const char* request, void* addInfo);
        bool sendResponse(std::string& response, void* addInfo = NULL);
        
        /**
         * \brief Get the port.
         * \return local port
         */
        uint16_t GetPort() const;

        /**
         * \brief Add a RPC method.
         * \param method RPC method
         */
        void AddMethod(CallbackMethod* method);

        /**
         * \brief Delete a RPC method.
         * \param method RPC method name
         */
        void DeleteMethod(const std::string& method);

      protected:

        /**
         * \brief JSON-RPC handler.
         */
        Handler m_jsonHandler;

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
         * \brief mongoose context.
         */
        struct mg_context *ctx;
    };

  } /* namespace Rpc */

} /* namespace Json */

#endif /* JSONRPC_HTTPServer_H */

