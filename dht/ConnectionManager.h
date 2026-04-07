/*
 * Copyright (C) 2009-2010 Big Muscle, http://strongdc.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "KBucket.h"
#include "dcpp/User.h"

namespace dht
{
    class DHT;

    class ConnectionManager
    {
    public:
        explicit ConnectionManager(DHT& dht);
        ~ConnectionManager(void);

        ConnectionManager(const ConnectionManager&) = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;

        /** Sends Connect To Me request to online node */
        void connect(const Node::Ptr& node, const string& token);
        void connect(const Node::Ptr& node, const string& token, bool secure);

        /** Creates connection to specified node */
        void connectToMe(const Node::Ptr& node, const AdcCommand& cmd);

        /** Sends request to create connection with me */
        void revConnectToMe(const Node::Ptr& node, const AdcCommand& cmd);

    private:
        DHT& dht_;
    };

}
