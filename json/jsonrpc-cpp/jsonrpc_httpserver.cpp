/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2011-2014 Eugene Petrov <dhamp@ya.ru>
 *  Copyright (C) 2013 Dorian Scholz <DorianScholz@gmx.de>
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
 * \brief JSON-RPC HTTPServer. Based on JSON-RPC Server.
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
    struct mg_server *server;
    static int exit_flag;

    static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
        int result = MG_FALSE;
        if (ev == MG_AUTH) {
          return MG_TRUE;   // Authorize all requests
        }
        if (ev == MG_REQUEST) {
            if(strcmp(conn->request_method,"OPTIONS") == 0) {
                std::string res = "";
                reinterpret_cast<HTTPServer*>(conn->server_param)->sendResponse(res, conn);
            }
            else if(strcmp(conn->request_method,"POST") == 0) {
                std::string tmp;
                tmp.assign(conn->content, conn->content_len);
                reinterpret_cast<HTTPServer*>(conn->server_param)->onRequest(tmp, conn);
            }
            result = MG_TRUE;
        }
        return result;
    }

    static void *serving_thread_func(void *param) {
      struct mg_server *srv = (struct mg_server *) param;
      while (exit_flag == 0) {
        mg_poll_server(srv, 1000);
      }
      return NULL;
    }

    bool HTTPServer::onRequest(const std::string& str, void* addInfo)
    {
        Json::Value response;
        m_jsonHandler.Process(str, response);
        std::string res = m_jsonHandler.GetString(response);
        sendResponse(res, addInfo);
        return true;
    }

    HTTPServer::HTTPServer(const std::string& address, uint16_t port)
    {
      m_address = address;
      m_port = port;
      exit_flag = 0;
    }

    HTTPServer::~HTTPServer()
    {
    }

    bool HTTPServer::startPolling()
    {
        // Create and configure the server
        if ((server = mg_create_server(this, ev_handler)) == NULL) {
            return false;
        }
        char tmp_port[30];
        sprintf(tmp_port,"%s:%d", GetAddress().c_str(),GetPort());
        mg_set_option(server, "listening_port", tmp_port);
        serving_thread_func(server);
        return true;
    }

    bool HTTPServer::stopPolling()
    {
        exit_flag = 1;
        mg_destroy_server(&server);
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
        snprintf(v, sizeof(v), "%lu", response.size());
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
