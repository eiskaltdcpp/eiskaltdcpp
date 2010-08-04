/*
 * Copyright (C) 2009 Big Muscle, http://strongdc.sf.net
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdafx.h"

#include "DHT.h"
#include "BootstrapManager.h"
#include "ConnectionManager.h"
#include "IndexManager.h"
#include "SearchManager.h"
#include "TaskManager.h"
#include "Utils.h"

#include "../dcpp/AdcCommand.h"
#include "../dcpp/CID.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/CryptoManager.h"
#include "../dcpp/LogManager.h"
#include "../dcpp/SettingsManager.h"
#include "../dcpp/ShareManager.h"
#include "../dcpp/UploadManager.h"
#include "../dcpp/User.h"
#include "../dcpp/version.h"

namespace dht
{

    DHT::DHT(void) : bucket(NULL), lastPacket(0), dirty(false), requestFWCheck(true)
    {
        // start with global firewalled status
        firewalled = !ClientManager::getInstance()->isActive(/*Util::emptyString*/);

        if(!BOOLSETTING(USE_DHT))
            return;

        BootstrapManager::newInstance();
        SearchManager::newInstance();
        IndexManager::newInstance();
        TaskManager::newInstance();
        ConnectionManager::newInstance();

        bucket = new KBucket();

        loadData();

        if(BOOLSETTING(NO_IP_OVERRIDE))//if(BOOLSETTING(UPDATE_IP))
            SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, Util::emptyString);
    }

    DHT::~DHT(void)
    {
        disconnect();

        dirty = true;
        saveData();

        delete bucket;

        ConnectionManager::deleteInstance();
        TaskManager::deleteInstance();
        IndexManager::deleteInstance();
        SearchManager::deleteInstance();
        BootstrapManager::deleteInstance();
    }

    void DHT::listen()
    {
        if(!BOOLSETTING(USE_DHT))
            return;

        if(!bucket)
        {
            DHT();
        }

        socket.listen();
        BootstrapManager::getInstance()->bootstrap();
    }

    /*
     * Process incoming command
     */
    void DHT::dispatch(const string& aLine, const string& ip, uint16_t port)
    {
        // check node's IP address
        if(!Utils::isGoodIPPort(ip, port))
        {
            //socket.send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_BAD_IP, "Your client supplied invalid IP: " + ip, AdcCommand::TYPE_UDP), ip, port);
            return; // invalid ip/port supplied
        }

        lastPacket = GET_TICK();

        try
        {
            AdcCommand cmd(aLine);

            // ignore empty commands
            if(cmd.getParameters().empty())
                return;

            string cid = cmd.getParam(0);
            if(cid.size() != 39)
                return;

            // ignore message from myself
            if(CID(cid) == ClientManager::getInstance()->getMe()->getCID())
                return;

            // add user to routing table
            Node::Ptr node = addUser(CID(cid), ip, port);

            // node is requiring FW check
            string internalUdpPort;
            if(cmd.getParam("FW", 1, internalUdpPort))
            {
                bool firewalled = (Util::toInt(internalUdpPort) != port);
                if(firewalled)
                    node->getUser()->setFlag(User::PASSIVE);

                // send him his external ip and port
                AdcCommand cmd(AdcCommand::SEV_SUCCESS, AdcCommand::SUCCESS, !firewalled ? "UDP port opened" : "UDP port closed", AdcCommand::TYPE_UDP);
                cmd.addParam("FC", "FWCHECK");
                cmd.addParam("I4", ip);
                cmd.addParam("U4", Util::toString(port));
                socket.send(cmd, ip, port);
            }

#define C(n) case AdcCommand::CMD_##n: handle(AdcCommand::n(), node, cmd); break;
            switch(cmd.getCommand())
            {
                C(INF); // user's info
                C(SCH); // search request
                C(RES); // response to SCH
                C(PUB); // request to publish file
                C(CTM); // connection request
                C(RCM); // reverse connection request
                C(STA); // status message
                C(PSR); // partial file request
                C(MSG); // private message
                C(GET); // get some data
                C(SND); // response to GET

            default:
                dcdebug("Unknown ADC command: %.50s\n", aLine.c_str());
                break;
#undef C

            }
        }
        catch(const ParseException&)
        {
            dcdebug("Invalid ADC command: %.50s\n", aLine.c_str());
        }
    }

    /*
     * Sends command to ip and port
     */
    void DHT::send(AdcCommand& cmd, const string& ip, uint16_t port)
    {
        {
            // FW check
            Lock l(cs);
            if(requestFWCheck && (firewalledWanted.size() + firewalledChecks.size() < FW_RESPONSES))
            {
                if(firewalledWanted.count(ip) == 0) // only when not requested from this node yet
                {
                    firewalledWanted.insert(ip);
                    cmd.addParam("FW", Util::toString(getPort()));
                }
            }
        }
        socket.send(cmd, ip, port);
    }

    /*
     * Insert (or update) user into routing table
     */
    Node::Ptr DHT::addUser(const CID& cid, const string& ip, uint16_t port)
    {
        // create user as offline (only TCP connected users will be online)
        UserPtr u = ClientManager::getInstance()->getUser(cid);
        u->setFlag(User::DHT);

        Lock l(cs);
        Node::Ptr node = bucket->insert(u);
        node->getIdentity().setIp(ip);
        node->getIdentity().setUdpPort(Util::toString(port));

        return node;
    }

    /*
     * Finds "max" closest nodes and stores them to the list
     */
    void DHT::getClosestNodes(const CID& cid, std::map<CID, Node::Ptr>& closest, unsigned int max, uint8_t maxType)
    {
        Lock l(cs);
        bucket->getClosestNodes(cid, closest, max, maxType);
    }

    /*
     * Removes dead nodes
     */
    void DHT::checkExpiration(uint64_t aTick)
    {
        Lock l(cs);
        if(bucket->checkExpiration(aTick))
            setDirty();

        firewalledWanted.clear();
    }

    /*
     * Finds the file in the network
     */
    void DHT::findFile(const string& tth, const string& token)
    {
        if(isConnected())
            SearchManager::getInstance()->findFile(tth, token);
    }

    /*
     * Sends our info to specified ip:port
     */
    void DHT::info(const string& ip, uint16_t port, uint32_t type)
    {
        // TODO: what info is needed?
        AdcCommand cmd(AdcCommand::CMD_INF, AdcCommand::TYPE_UDP);

#ifdef SVNVERSION
#define VER VERSIONSTRING SVNVERSION
#else
#define VER VERSIONSTRING
#endif

        cmd.addParam("TY", Util::toString(type));
        cmd.addParam("VE", ("StrgDC++ " VER));
        cmd.addParam("NI", SETTING(NICK));
        cmd.addParam("SL", Util::toString(UploadManager::getInstance()->getSlots()));

        if (SETTING(THROTTLE_ENABLE) && SETTING(MAX_UPLOAD_SPEED_LIMIT) != 0) {
            cmd.addParam("US", Util::toString(SETTING(MAX_UPLOAD_SPEED_LIMIT)*1024));
        } else {
            cmd.addParam("US", Util::toString((long)(Util::toDouble(SETTING(UPLOAD_SPEED))*1024*1024/8)));
        }

        string su;
        if(CryptoManager::getInstance()->TLSOk())
            su += ADCS_FEATURE ",";

        // TCP status according to global status
        if(ClientManager::getInstance()->isActive(Util::emptyString))
            su += TCP4_FEATURE ",";

        // UDP status according to UDP status check
        if(!isFirewalled())
            su += UDP4_FEATURE ",";


        if(!su.empty()) {
            su.erase(su.size() - 1);
        }
        cmd.addParam("SU", su);

        send(cmd, ip, port);
    }

    /*
     * Sends Connect To Me request to online node
     */
    void DHT::connect(const OnlineUser& ou, const string& token)
    {
        ConnectionManager::getInstance()->connect(ou, token);
    }

    /*
     * Sends private message to online node
     */
    void DHT::privateMessage(const OnlineUser& ou, const string& aMessage, bool thirdPerson)
    {
        AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_UDP);
        cmd.addParam(aMessage);
        if(thirdPerson)
            cmd.addParam("ME", "1");

        send(cmd, ou.getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(ou.getIdentity().getUdpPort())));
    }

    /*
     * Loads network information from XML file
     */
    void DHT::loadData()
    {
        try
        {
            dcpp::File f(Util::getPath(Util::PATH_USER_CONFIG) + DHT_FILE, dcpp::File::READ, dcpp::File::OPEN);
            SimpleXML xml;
            xml.fromXML(f.read());

            xml.stepIn();

            // load nodes; when file is older than 7 days, bootstrap from database later
            if(f.getLastModified() > time(NULL) - 7 * 24 * 60 * 60)
                bucket->loadNodes(xml);

            // load indexes
            IndexManager::getInstance()->loadIndexes(xml);
            xml.stepOut();
        }
        catch(Exception& e)
        {
            dcdebug("%s\n", e.getError().c_str());
        }
    }

    /*
     * Finds "max" closest nodes and stores them to the list
     */
    void DHT::saveData()
    {
        if(!dirty)
            return;

        Lock l(cs);

        SimpleXML xml;
        xml.addTag("DHT");
        xml.stepIn();

        // save nodes
        bucket->saveNodes(xml);

        // save foreign published files
        IndexManager::getInstance()->saveIndexes(xml);

        xml.stepOut();

        try
        {
            dcpp::File file(Util::getPath(Util::PATH_USER_CONFIG) + DHT_FILE + ".tmp", dcpp::File::WRITE, dcpp::File::CREATE | dcpp::File::TRUNCATE);
            BufferedOutputStream<false> bos(&file);
            bos.write(SimpleXML::utf8Header);
            xml.toXML(&bos);
            bos.flush();
            file.close();
            dcpp::File::deleteFile(Util::getPath(Util::PATH_USER_CONFIG) + DHT_FILE);
            dcpp::File::renameFile(Util::getPath(Util::PATH_USER_CONFIG) + DHT_FILE + ".tmp", Util::getPath(Util::PATH_USER_CONFIG) + DHT_FILE);
        }
        catch(const FileException&)
        {
        }
    }

    /*
     * Message processing
     */

    // user's info
    void DHT::handle(AdcCommand::INF, const Node::Ptr& node, AdcCommand& c) throw()
    {
        string ip = node->getIdentity().getIp();
        string udpPort = node->getIdentity().getUdpPort();

        InfType it = NONE;
        for(StringIterC i = c.getParameters().begin() + 1; i != c.getParameters().end(); ++i)
        {
            if(i->length() < 2)
                continue;

            string parameter_name = i->substr(0, 2);
            if(parameter_name == "TY")
                it = (InfType)Util::toInt(i->substr(2));
            else if((parameter_name != "I4") && (parameter_name != "U4")) // avoid IP+port spoofing
                node->getIdentity().set(i->c_str(), i->substr(2));
        }

        if(node->getIdentity().supports(ADCS_FEATURE))
        {
            node->getUser()->setFlag(User::TLS);
        }

        if(!node->getIdentity().get("US").empty()) {
            node->getIdentity().setConnection(Util::formatBytes(node->getIdentity().get("US")) + "/s");
        }

        if(!node->isInList)
        {
            // put him online so we can make a connection with him
            ClientManager::getInstance()->putOnline(node.get());
            node->isInList = true;
        }

        // do we wait for any search results from this user?
        SearchManager::getInstance()->processSearchResults(node->getUser());

        if(it & PING)
            info(node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(udpPort)), it & ~PING); // remove ping flag to avoid ping-pong-ping-pong-ping...
    }

    // incoming search request
    void DHT::handle(AdcCommand::SCH, const Node::Ptr& node, AdcCommand& c) throw()
    {
        SearchManager::getInstance()->processSearchRequest(node, c);
    }

    // incoming search result
    void DHT::handle(AdcCommand::RES, const Node::Ptr& node, AdcCommand& c) throw()
    {
        SearchManager::getInstance()->processSearchResult(node, c);
    }

    // incoming publish request
    void DHT::handle(AdcCommand::PUB, const Node::Ptr& node, AdcCommand& c) throw()
    {
        IndexManager::getInstance()->processPublishSourceRequest(node, c);
    }

    // connection request
    void DHT::handle(AdcCommand::CTM, const Node::Ptr& node, AdcCommand& c) throw()
    {
        ConnectionManager::getInstance()->connectToMe(node, c);
    }

    // reverse connection request
    void DHT::handle(AdcCommand::RCM, const Node::Ptr& node, AdcCommand& c) throw()
    {
        ConnectionManager::getInstance()->revConnectToMe(node, c);
    }

    // status message
    void DHT::handle(AdcCommand::STA, const Node::Ptr& node, AdcCommand& c) throw()
    {
        if(c.getParameters().size() < 3)
            return;

        string fromIP = node->getIdentity().getIp();
        int code = Util::toInt(c.getParam(1).substr(1));

        if(code == 0)
        {
            string resTo;
            if(!c.getParam("FC", 2, resTo))
                return;

            if(resTo == "PUB")
            {
                // don't do anything
                //string tth;
                //if(!c.getParam("TR", 1, tth))
                //  return;
                //
                //try
                //{
                //  string fileName = Util::getFileName(ShareManager::getInstance()->toVirtual(TTHValue(tth)));
                //  LogManager::getInstance()->message("DHT (" + fromIP + "): File published: " + fileName);
                //}
                //catch(ShareException&)
                //{
                //  // published non-shared file??? Maybe partial file
                //  LogManager::getInstance()->message("DHT (" + fromIP + "): Partial file published: " + tth);
                //
                //}
            }
            else if(resTo == "FWCHECK")
            {
                Lock l(cs);
                if(!firewalledWanted.count(fromIP))
                    return; // we didn't requested firewall check from this node

                firewalledWanted.erase(fromIP);
                if(firewalledChecks.count(fromIP))
                    return; // already received firewall check from this node

                string externalIP;
                string externalUdpPort;
                if(!c.getParam("I4", 1, externalIP) || !c.getParam("U4", 1, externalUdpPort))
                    return; // no IP and port in response

                firewalledChecks.insert(std::make_pair(fromIP, std::make_pair(externalIP, static_cast<uint16_t>(Util::toInt(externalUdpPort)))));

                if(firewalledChecks.size() == FW_RESPONSES)
                {
                    // when we received more firewalled statuses, we will be firewalled
                    int fw = 0; string lastIP;
                    for(std::tr1::unordered_map<string, std::pair<string, uint16_t>>::const_iterator i = firewalledChecks.begin(); i != firewalledChecks.end(); i++)
                    {
                        string ip = i->second.first;
                        uint16_t udpPort = i->second.second;

                        if(udpPort != getPort())
                            fw++;
                        else
                            fw--;

                        if(lastIP.empty())
                        {
                            externalIP = ip;
                            lastIP = ip;
                        }

                        //If the last check matches this one, reset our current IP.
                        //If the last check does not match, wait for our next incoming IP.
                        //This happens for one reason.. a client responsed with a bad IP.
                        if(ip == lastIP)
                            externalIP = ip;
                        else
                            lastIP = ip;
                    }

                    if(fw >= 0)
                    {
                        // we are probably firewalled, so our internal UDP port is unaccessible
                        if(externalIP != lastExternalIP || !firewalled)
                            LogManager::getInstance()->message("DHT: Firewalled UDP status set (IP: " + externalIP + ")");
                        firewalled = true;
                    }
                    else
                    {
                        if(externalIP != lastExternalIP || firewalled)
                            LogManager::getInstance()->message("DHT: Our UDP port seems to be opened (IP: " + externalIP + ")");

                        firewalled = false;
                    }

                    if(BOOLSETTING(UPDATE_IP))
                        SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, externalIP);

                    firewalledChecks.clear();
                    firewalledWanted.clear();

                    lastExternalIP = externalIP;
                    requestFWCheck = false;
                }
            }
            return;
        }

        // display message in all other cases
        //string msg = c.getParam(2);
        //if(!msg.empty())
        //  LogManager::getInstance()->message("DHT (" + fromIP + "): " + msg);
    }

    // partial file request
    void DHT::handle(AdcCommand::PSR, const Node::Ptr& node, AdcCommand& c) throw()
    {
        dcpp::SearchManager::getInstance()->onPSR(c, node->getUser(), node->getIdentity().getIp());
    }

    // private message
    void DHT::handle(AdcCommand::MSG, const Node::Ptr& node, AdcCommand& c) throw()
    {
        // not implemented yet
        //fire(ClientListener::PrivateMessage(), this, *node, to, node, c.getParam(0), c.hasFlag("ME", 1));

        //privateMessage(*node, "Sorry, private messages aren't supported yet!", false);
    }

    void DHT::handle(AdcCommand::GET, const Node::Ptr& node, AdcCommand& c) throw()
    {
        if(c.getParam(1) == "nodes" && c.getParam(2) == "dht.xml")
        {
            AdcCommand cmd(AdcCommand::CMD_SND, AdcCommand::TYPE_UDP);
            cmd.addParam(c.getParam(1));
            cmd.addParam(c.getParam(2));

            SimpleXML xml;
            xml.addTag("Nodes");
            xml.stepIn();

            // get 20 random contacts
            Node::Map nodes;
            DHT::getInstance()->getClosestNodes(CID::generate(), nodes, 20, 2);

            // add nodelist in XML format
            for(Node::Map::const_iterator i = nodes.begin(); i != nodes.end(); i++)
            {
                xml.addTag("Node");
                xml.addChildAttrib("CID", i->second->getUser()->getCID().toBase32());
                xml.addChildAttrib("I4", i->second->getIdentity().getIp());
                xml.addChildAttrib("U4", i->second->getIdentity().getUdpPort());
            }

            xml.stepOut();

            string nodesXML;
            StringOutputStream sos(nodesXML);
            sos.write(SimpleXML::utf8Header);
            xml.toXML(&sos);

            cmd.addParam(nodesXML);

            send(cmd, node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())));
        }
    }

    void DHT::handle(AdcCommand::SND, const Node::Ptr& node, AdcCommand& c) throw()
    {
        if(c.getParam(1) == "nodes" && c.getParam(2) == "dht.xml")
        {
            try
            {
                SimpleXML xml;
                xml.fromXML(c.getParam(3));
                xml.stepIn();

                // extract bootstrap nodes
                unsigned int n = 20;
                while(xml.findChild("Node") && n-- > 0)
                {
                    CID cid = CID(xml.getChildAttrib("CID"));

                    // don't bother with myself
                    if(ClientManager::getInstance()->getMe()->getCID() == cid)
                        continue;

                    const string& i4    = xml.getChildAttrib("I4");
                    uint16_t u4         = static_cast<uint16_t>(Util::toInt(xml.getChildAttrib("U4")));

                    // don't bother with private IPs
                    if(!Utils::isGoodIPPort(i4, u4))
                        continue;

                    // add user to our routing table
                    addUser(CID(cid), i4, u4);
                }

                xml.stepOut();
            }
            catch(const SimpleXMLException&)
            {
                // malformed node list
            }
        }
    }

}
