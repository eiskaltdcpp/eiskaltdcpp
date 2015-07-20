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

#ifndef _BOOTSTRAPMANAGER_H
#define _BOOTSTRAPMANAGER_H

#include "Constants.h"
#include "KBucket.h"

#include "dcpp/CID.h"
#include "dcpp/HttpConnectionListener.h"
#include "dcpp/Singleton.h"

namespace dht
{

class BootstrapManager : public Singleton<BootstrapManager>, private HttpConnectionListener
{
public:
    BootstrapManager(void);
    ~BootstrapManager(void);

    void bootstrap();

    void process();

    void complete(const string& url, const std::string &data);

    void addBootstrapNode(const string& ip, const string& udpPort, const CID& targetCID, const UDPKey& udpKey);

private:

    std::list<std::unique_ptr<HttpConnection>> httpcons;

    CriticalSection cs;

    struct BootstrapNode
    {
        string          ip;
        string          udpPort;
        CID             cid;
        UDPKey          udpKey;
    };

    /** List of bootstrap nodes */
    deque<BootstrapNode> bootstrapNodes;

    // HttpConnectionListener
    void on(HttpConnectionListener::Failed, HttpConnection*, const string&) noexcept;
    void on(HttpConnectionListener::Complete, HttpConnection*, const string &) noexcept;

};

}

#endif  // _BOOTSTRAPMANAGER_H
