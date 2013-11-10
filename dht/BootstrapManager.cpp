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
#include <zlib.h>

namespace dht
{
    vector<string> dhtservers;
 
    BootstrapManager::BootstrapManager(void)
    {
        dhtservers.push_back("http://strongdc.sourceforge.net/bootstrap/");
        dhtservers.push_back("http://ssa.in.ua/dcDHT.php");
        httpConnection.addListener(this);
    }

    BootstrapManager::~BootstrapManager(void)
    {
        httpConnection.removeListener(this);
    }

    void BootstrapManager::bootstrap()
    {
        if(bootstrapNodes.empty())
        {
            LogManager::getInstance()->message("DHT bootstrapping started");
            string dhturl = dhtservers[Util::rand(dhtservers.size())];
            // TODO: make URL settable
            string url = dhturl  + "?cid=" + ClientManager::getInstance()->getMe()->getCID().toBase32() + "&encryption=1";

            // store only active nodes to database
            if(ClientManager::getInstance()->isActive(Util::emptyString))
            {
                url += "&u4=" + Util::toString(DHT::getInstance()->getPort());
            }

            httpConnection.setCoralizeState(HttpConnection::CST_NOCORALIZE);
            httpConnection.downloadFile(url);
        }
    }

    void BootstrapManager::on(HttpConnectionListener::Data, HttpConnection*, const uint8_t* buf, size_t len) throw()
    {
        nodesXML += string((const char*)buf, len);
    }

    #define BUFSIZE 16384
    void BootstrapManager::on(HttpConnectionListener::Complete, HttpConnection*, string const&, bool /*fromCoral*/) throw()
    {
        if(!nodesXML.empty())
        {
            try
            {
                uLongf destLen = BUFSIZE;
                boost::scoped_array<uint8_t> destBuf;

                // decompress incoming packet
                int result;

                do
                {
                    destLen *= 2;
                    destBuf.reset(new uint8_t[destLen]);

                    result = uncompress(&destBuf[0], &destLen, (Bytef*)nodesXML.data(), nodesXML.length());
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
                    CID cid     = CID(remoteXml.getChildAttrib("CID"));
                    string i4   = remoteXml.getChildAttrib("I4");
                    string u4   = remoteXml.getChildAttrib("U4");

                    addBootstrapNode(i4, static_cast<uint16_t>(Util::toInt(u4)), cid, UDPKey());
                }

                remoteXml.stepOut();
            }
            catch(Exception& e)
            {
                LogManager::getInstance()->message("DHT bootstrap error: " + e.getError());
            }
        }
    }

    void BootstrapManager::on(HttpConnectionListener::Failed, HttpConnection*, const string& aLine) throw()
    {
        LogManager::getInstance()->message("DHT bootstrap error: " + aLine);
    }

    void BootstrapManager::addBootstrapNode(const string& ip, uint16_t udpPort, const CID& targetCID, const UDPKey& udpKey)
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
