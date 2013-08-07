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
#include "mongoose.h"

namespace Json
{

  namespace Rpc
  {
    struct mg_context *ctx;

    static int begin_request_handler(struct mg_connection *conn) {
        const struct mg_request_info *ri = mg_get_request_info(conn);
        int post_data_len = 0;
        char* post_data = NULL;

        HTTPServer* serv = (HTTPServer*) ri->user_data;
        if(strcmp(ri->request_method,"OPTIONS") == 0) {
            std::string res = "";
            serv->sendResponse(res, conn);
        }
        else if(strcmp(ri->request_method,"POST") == 0) {
            sscanf(mg_get_header(conn,"Content-Length"),"%d",&post_data_len);
            post_data = (char*)malloc(post_data_len+1);
            mg_read(conn, post_data, post_data_len);
            post_data[post_data_len] = 0; // make sure this is null terminated
            serv->onRequest(post_data,conn);
            free(post_data);
        }
        return 1; // Mark request as processed
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
    }

    bool HTTPServer::startPolling()
    {
        struct mg_callbacks callbacks;
        char tmp_port[30];
        sprintf(tmp_port,"%s:%d", GetAddress().c_str(),GetPort());
        const char *options[] = {"listening_ports", tmp_port,"num_threads", "1", NULL };
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.begin_request = begin_request_handler;
        ctx = mg_start(&callbacks, this, options);
        if(!ctx)
            return false;
        return true;
    }

    bool HTTPServer::stopPolling()
    {
        if(ctx)
            mg_stop(ctx);
        return true;
    }

    std::string HTTPServer::GetAddress() const
    {
      return m_address;
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
        std::string tmp = "HTTP/1.1 200 OK\r\nServer: eidcppd server\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Content-Type\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: ";
        char v[16];
        snprintf(v, sizeof(v), "%u", response.size());
        tmp += v;
        tmp += "\r\n\r\n";
        tmp += response;
        if(mg_write(conn, tmp.c_str(), tmp.size()) > 0) {
            return true;
        } else {
            return false;
        }
    }

  } /* namespace Rpc */

} /* namespace Json */
