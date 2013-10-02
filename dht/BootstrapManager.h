/*
 * Copyright (C) 2009-2010 Big Muscle, http://strongdc.sf.net
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "Constants.h"
#include "KBucket.h"
#include "dcpp/CID.h"
#include "dcpp/HttpConnection.h"
#include "dcpp/Singleton.h"

namespace dht
{

    class BootstrapManager :
        public Singleton<BootstrapManager>, private HttpConnectionListener
    {
    public:
        BootstrapManager(void);
        ~BootstrapManager(void);

        void bootstrap();

        void process();

        void addBootstrapNode(const string& ip, uint16_t udpPort, const CID& targetCID, const UDPKey& udpKey);

    private:

        CriticalSection cs;

        struct BootstrapNode
        {
            string      ip;
            uint16_t    udpPort;
            CID         cid;
            UDPKey      udpKey;
        };

        /** List of bootstrap nodes */
        deque<BootstrapNode> bootstrapNodes;

        /** HTTP connection for bootstrapping */
        HttpConnection httpConnection;

        /** Downloaded node list */
        string nodesXML;

        // HttpConnectionListener
        void on(HttpConnectionListener::Data, HttpConnection* conn, const uint8_t* buf, size_t len) throw();
        void on(HttpConnectionListener::Complete, HttpConnection* conn, string const& aLine, bool /*fromCoral*/) throw();
        void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw();

    };

}
