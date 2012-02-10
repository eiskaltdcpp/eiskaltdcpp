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
 * \file jsonrpc_httpserver.cpp
 * \brief JSON-RPC HTTPServer.
 * \author Eugene Petrov
 */

#include "jsonrpc_httpserver.h"
#include <string>
#include <cstring>
#include <cstdlib>

namespace Json 
{

  namespace Rpc
  {
    static void *callback(enum mg_event event, struct mg_connection *conn,
            const struct mg_request_info *request_info) {

        char* readBuffer = NULL;
        int postSize = 0;
        HTTPServer* _this = (HTTPServer*) request_info->user_data;

        if(event == MG_NEW_REQUEST) {

            if(strcmp(request_info->request_method,"GET") == 0) {
                //Mark the request as unprocessed.
                return NULL;
            } else if(strcmp(request_info->request_method,"POST") == 0) {
                //get size of postData
                sscanf(mg_get_header(conn,"Content-Length"),"%d",&postSize);
                readBuffer = (char*) malloc(sizeof(char)*(postSize+1));
                mg_read(conn,readBuffer,postSize);
                _this->onRequest(readBuffer,conn);
                //onRequest(readBuffer,conn);
                free(readBuffer);

                //Mark the request as processed by our handler.
                return (void*)"";
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    }
    
    bool HTTPServer::onRequest(const char* request, void* addInfo)
    {
        Json::Value response;
        m_jsonHandler.Process(std::string(request), response);
        std::string res = m_jsonHandler.GetString(response);
        sendResponse(res, addInfo);
        return true;
    }
    
    HTTPServer::HTTPServer(const std::string& address, uint16_t port)
    {
      m_address = address;
      m_port = port;
      ctx = NULL;
    }

    HTTPServer::~HTTPServer()
    {
        mg_stop(ctx);
    }
    
    bool HTTPServer::startPolling()
    {
        char port[6];
        sprintf(port,"%d",m_port);
        const char *options[] = { "document_root", NULL, "listening_ports", port, NULL };
        this->ctx = mg_start(&callback, this, options);
        if(this->ctx != NULL) {
            return true;
        } else {
            return false;
        }
    }
    bool HTTPServer::stopPolling()
    {
        mg_stop(ctx);
        return true;
    }

    uint16_t HTTPServer::GetPort() const
    {
      return m_port;
    }

    void HTTPServer::AddMethod(CallbackMethod* method)
    {
      m_jsonHandler.AddMethod(method);
    }

    void HTTPServer::DeleteMethod(const std::string& method)
    {
      m_jsonHandler.DeleteMethod(method);
    }
    
    bool HTTPServer::sendResponse(std::string & response, void *addInfo)
    {
        struct mg_connection* conn = (struct mg_connection*)addInfo;
        if(mg_printf(conn,"HTTP/1.1 200 OK\r\n\r\n %s",response.c_str()) > 0) {
            return true;
        } else {
            return false;
        }
    }

  } /* namespace Rpc */

} /* namespace Json */

