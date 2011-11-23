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
 * \file jsonrpc_common.h
 * \brief Common functions and enumerations for JsonRpc-Cpp.
 * \author Sebastien Vincent
 */

#ifndef JSONRPC_COMMON_H
#define JSONRPC_COMMON_H

#ifndef _WIN32

#include <stdint.h>

#else

#ifndef _MSC_VER
#include <stdint.h>
#endif

#endif

namespace Json
{

  namespace Rpc
  {
    /**
     * \enum EncapsulatedFormat
     * \brief JSON-RPC message encoding format (nothing to do with UTF-8 or languages).
     */
    enum EncapsulatedFormat
    {
      RAW, /**< Raw format. */
      NETSTRING /**< Encapsulate the message with NetString (see http://cr.yp.to/proto/netstrings.txt). */
#if 0
      HTTP_POST, /**< Encapsulate the message in HTTP POST. */
      HTTP_GET, /**< Encapsulate the message in HTTP POST. */
#endif
    };

    /**
     * \enum ErrorCode
     * \brief JSON-RPC error codes.
     * \note Value from -32099 to -32000 are reserved for implementation-defined server-errors.
     */
    enum ErrorCode
    {
      PARSING_ERROR = -32700, /**< Invalid JSON. An error occurred on the server while parsing the JSON text. */
      INVALID_REQUEST = -32600, /**< The received JSON not a valid JSON-RPC Request. */
      METHOD_NOT_FOUND = -32601, /**< The requested remote-procedure does not exist / is not available. */
      INVALID_PARAMS = -32602, /**< Invalid method parameters. */
      INTERNAL_ERROR = -32603 /**< Internal JSON-RPC error. */
    };

  } /* namespace Rpc */

} /* namespace Json */

#endif /* JSONRPC_COMMON_H */

