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

namespace Json
{

  namespace Rpc
  {
    static int exit_flag;

    static void Json_Rpc_HTTPServer_ev_handler(struct ns_connection *nc, int ev, void *ev_data) {
        HTTPServer* c = ((HTTPServer*)nc->mgr->user_data);

        switch (ev) {
            case NS_HTTP_REQUEST: {
                struct http_message *hm = (struct http_message *) ev_data;
                if (ns_vcmp(&hm->method, "OPTIONS") == 0) {
                    std::string tmp = "";
                    c->sendResponse(tmp, nc);
                } else if (ns_vcmp(&hm->method, "POST") == 0) {
                    std::string tmp(hm->body.p, hm->body.len);
                    c->onRequest(tmp, nc);
                }
                break;
            }
            default: break;
        }
    }

    static void *serving_thread_func(void *param) {
      struct ns_mgr *srv = (struct ns_mgr *) param;
      while (exit_flag == 0) {
        ns_mgr_poll(srv, 1000);
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
        ns_mgr_init(&server, this);
        char tmp_port[30];
        sprintf(tmp_port,"%s:%d", GetAddress().c_str(),GetPort());
        ns_connection* nc = ns_bind(&server, tmp_port, Json_Rpc_HTTPServer_ev_handler);
        ns_set_protocol_http_websocket(nc);
        serving_thread_func(&server);
        return true;
    }

    bool HTTPServer::stopPolling()
    {
        exit_flag = 1;
        ns_mgr_free(&server);
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

    bool HTTPServer::sendResponse(std::string& response, void *addInfo)
    {
        struct ns_connection* conn = (struct ns_connection*)addInfo;
        std::string tmp = "HTTP/1.1 200 OK\r\nServer: eidcppd server\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Content-Type\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: ";
        char v[16];
        snprintf(v, sizeof(v), "%lu", response.size());
        tmp += v;
        tmp += "\r\n\r\n";
        tmp += response;
        if(ns_send(conn, tmp.c_str(), tmp.size()) > 0) {
            return true;
        } else {
            return false;
        }
    }

  } /* namespace Rpc */

} /* namespace Json */
