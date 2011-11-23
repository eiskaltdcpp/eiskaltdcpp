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
 * \file jsonrpc.h
 * \brief Version information and all 
 * related include files in order to JsonRpc-Cpp.
 * \author Sebastien Vincent
 */

#ifndef JSONRPC_H
#define JSONRPC_H

/**
 * \def JSONRPC_CPP_VERSION
 * \brief JsonRpc-Cpp version in integer format.
 * 
 * The number is the result of the following computation:\n
 * major * 10000 + minor * 100 + micro\n
 * So version 1.2.3 produces 10203.
 */
#define JSONRPC_CPP_VERSION 300

/**
 * \def JSONRPC_CPP_VERSION_STRING
 * \brief JsonRpc-Cpp version in string format.
 */
#define JSONRPC_CPP_VERSION_STRING "0.3.0"

/* include from external jsoncpp lib */
#include <json/json.h>

/* include all headers from JsonRpc-Cpp lib */
#include "jsonrpc_common.h"
#include "jsonrpc_handler.h"
#include "jsonrpc_server.h"
#include "jsonrpc_udpserver.h"
#include "jsonrpc_tcpserver.h"
#include "jsonrpc_client.h"
#include "jsonrpc_udpclient.h"
#include "jsonrpc_tcpclient.h"

#include "netstring.h"
#include "networking.h"

/**
 * \namespace Json
 * \brief JSON (JavaScript Object Notation).
 */
namespace Json 
{
  /**
   * \namespace Json::Rpc
   * \brief JSON-RPC (remote procedure call using JSON as encoding format).
   */
  namespace Rpc
  {
  } /* namespace Rpc */

} /* namespace Json */

#endif /* JSONRPC_H */

