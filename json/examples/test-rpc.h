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
 * \file test-rpc.h
 * \brief RPC example.
 * \author Sebastien Vincent
 */

#ifndef TEST_RPC_H
#define TEST_RPC_H 

#include <json/json.h>

/**
 * \class TestRpc
 * \brief RPC example.
 */
class TestRpc
{
  public:
    /**
     * \brief Reply with success.
     * \param root JSON-RPC request
     * \param response JSON-RPC response
     * \return true if correctly processed, false otherwise
     */
    bool Print(const Json::Value& root, Json::Value& response);
    
    /**
     * \brief Notification.
     * \param root JSON-RPC request
     * \param response JSON-RPC response
     * \return true if correctly processed, false otherwise
     */
    bool Notify(const Json::Value& root, Json::Value& response);
    
    /**
     * \brief Get the description in JSON format.
     * \return JSON description
     */
    Json::Value GetDescription();
};

#endif /* TEST_RPC_H */

