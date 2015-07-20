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

#include "stdafx.h"
#include "BootstrapManager.h"

#include "Constants.h"
#include "DHT.h"
#include "SearchManager.h"

#include "dcpp/AdcCommand.h"
#include "dcpp/ClientManager.h"
#include "dcpp/HttpConnection.h"
#include "dcpp/LogManager.h"
#include "dcpp/Streams.h"
#include <zlib.h>
#include <atomic>

namespace dht
{
    vector<string> dhtservers;
    std::atomic<uint8_t> dhtbootstrapcomplete;

    BootstrapManager::BootstrapManager(void)
    {
        // TODO: move this in config
        dhtservers.push_back("http://strongdc.sourceforge.net/bootstrap/");
        dhtservers.push_back("http://dht.fly-server.ru/dcDHT.php");
        dhtbootstrapcomplete = 0;
    }

    BootstrapManager::~BootstrapManager(void)
    {
        for (auto& i : httpcons) {
            i.release();
        }
    }

    void BootstrapManager::bootstrap()
    {
//        if(bootstrapNodes.empty())
        if (dhtbootstrapcomplete < dhtservers.size())
        {
            for(auto i : dhtservers) {
                // TODO: make URL settable
                string url = i  + "?cid=" + ClientManager::getInstance()->getMe()->getCID().toBase32() + "&encryption=1";

                // store only active nodes to database
                if(ClientManager::getInstance()->isActive(Util::emptyString))
                {
                        url += "&u4=" + DHT::getInstance()->getPort();
                }
                LogManager::getInstance()->message("DHT bootstrapping started from " + i);
                std::unique_ptr<HttpConnection> c;
                c.reset(new HttpConnection(url.find("strongdc") != string::npos ? "User-Agent: StrongDC++ v2.42" : ""));
                c->addListener(this);
                c->setShortUrl(i);
                c->setUrl(url);
                c->download();
                httpcons.push_back(std::move(c));
            }
        }
    }

    void BootstrapManager::on(HttpConnectionListener::Failed, HttpConnection* c, const string& str) noexcept {
        auto it = httpcons.begin();
        while (it != httpcons.end()) {
            if ((*it).get() == c) { break; }
            it++;
        }
        if (it == httpcons.end()) { return; }
        (*it)->removeListener(this);
        LogManager::getInstance()->message("DHT bootstrap error (" + (*it)->getShortUrl() +"): " + str);
        (*it).release();
//        bootstrap(); // may be no more need
    }

    void BootstrapManager::on(HttpConnectionListener::Complete, HttpConnection* c, const string&  data) noexcept {
        auto it = httpcons.begin();
        while (it != httpcons.end()) {
            if ((*it).get() == c) { break; }
            it++;
        }
        if (it == httpcons.end()) { return; }
        (*it)->removeListener(this);
        string url = (*it)->getShortUrl();
        (*it).release();

        complete(url, data);
    }

    #define BUFSIZE 16384
    void BootstrapManager::complete(const string& url, const string& data)
    {
//        printf("url->%s \ndata->%s \n", url.c_str(), data.c_str());fflush(stdout);
        if(!data.empty())
        {
            try
            {
                uLongf destLen = BUFSIZE;
                std::unique_ptr<uint8_t[]> destBuf;
                // decompress incoming packet
                int result;

                do
                {
                    destLen *= 2;
                    destBuf.reset(new uint8_t[destLen]);

                    result = uncompress(&destBuf[0], &destLen, (Bytef*)data.data(), data.length());
                }
                while (result == Z_BUF_ERROR);

                if(result != Z_OK)
                {
                    // decompression error!!!
                    throw Exception("Decompress error.");
                }

                SimpleXML remoteXml;
                remoteXml.fromXML(string((char*)&destBuf[0], destLen));
                remoteXml.stepIn();

                while(remoteXml.findChild("Node"))
                {
                    CID cid         = CID(remoteXml.getChildAttrib("CID"));
                    string i4       = remoteXml.getChildAttrib("I4");
                    string u4       = remoteXml.getChildAttrib("U4");

                    addBootstrapNode(i4, u4, cid, UDPKey());
                }

                remoteXml.stepOut();

                ++dhtbootstrapcomplete;
            }
            catch(Exception& e)
            {
                LogManager::getInstance()->message("DHT bootstrap error (" + url +"): " + e.getError());
            }
        }

    }

    void BootstrapManager::addBootstrapNode(const string& ip, const string& udpPort, const CID& targetCID, const UDPKey& udpKey)
    {
            BootstrapNode node = { ip, udpPort, targetCID, udpKey };
            bootstrapNodes.push_back(node);
    }

    void BootstrapManager::process()
    {
        Lock l(cs);
        if(!bootstrapNodes.empty())
        {
            // send bootstrap request
            AdcCommand cmd(AdcCommand::CMD_GET, AdcCommand::TYPE_UDP);
            cmd.addParam("nodes");
            cmd.addParam("dht.xml");

            const BootstrapNode& node = bootstrapNodes.front();

            CID key;
            // if our external IP changed from the last time, we can't encrypt packet with this key
            // this won't probably work now
            if(DHT::getInstance()->getLastExternalIP() == node.udpKey.ip)
                    key = node.udpKey.key;

            DHT::getInstance()->send(cmd, node.ip, node.udpPort, node.cid, key);

            bootstrapNodes.pop_front();
        }
    }

}
