/*
 * Copyright (C) 2009-2010 Big Muscle, http://strongdc.sourceforge.net/
 * Copyright (C) 2019 Boris Pek <tehnick-8@yandex.ru>
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

#include "stdafx.h"

#include "DHT.h"
#include "BootstrapManager.h"
#include "ConnectionManager.h"
#include "IndexManager.h"
#include "SearchManager.h"
#include "TaskManager.h"
#include "Utils.h"
#include "dcpp/AdcCommand.h"
#include "dcpp/ChatMessage.h"
#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/CryptoManager.h"
#include "dcpp/File.h"
#include "dcpp/LogManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/ThrottleManager.h"
#include "dcpp/UploadManager.h"
#include "dcpp/User.h"
#include "dcpp/version.h"
#include "dcpp/DCPlusPlus.h"

namespace dht
{

    DHT::DHT(dcpp::DCContext& ctx)
        : bucket(nullptr)
        , lastExternalIP(Util::getLocalIp()) // hack
        , lastPacket(0)
        , firewalled(true)
        , requestFWCheck(true)
        , dirty(false)
        , ctx_(ctx)
    {
        type = ClientBase::DHT;

        indexMgr_ = std::make_unique<IndexManager>(*this);
    }

    DHT::~DHT(void) throw()
    {
        // when DHT is disabled, we shouldn't try to perform exit cleanup
        if(bucket == nullptr) {
            indexMgr_.reset();
            return;
        }

        stop(true);

        indexMgr_.reset();
    }

    /*
     * Starts DHT.
     */
    void DHT::start()
    {
        if(!CTX_BOOLSETTING(USE_DHT))
            return;

        // start with global firewalled status
        firewalled = !ctx_.getClientManager()->isActive(Util::emptyString);
        requestFWCheck = true;

        if(!bucket)
        {
            if(!CTX_BOOLSETTING(NO_IP_OVERRIDE))
                ctx_.getSettingsManager()->set(SettingsManager::EXTERNAL_IP, Util::emptyString);

            bucket = new KBucket(*this);

            bootstrapMgr_ = std::make_unique<BootstrapManager>(*this);
            searchMgr_ = std::make_unique<SearchManager>(*this);
            taskMgr_ = std::make_unique<TaskManager>(*this);
            connectionMgr_ = std::make_unique<ConnectionManager>(*this);

            loadData();
        }

        socket.setDHT(*this);
        socket.listen();
        getBootstrapManager().bootstrap();
    }

    void DHT::stop(bool exiting)
    {
        if(!bucket)
            return;

        socket.disconnect();

        if(!CTX_BOOLSETTING(USE_DHT) || exiting)
        {
            saveData();

            lastPacket = 0;

            connectionMgr_.reset();
            taskMgr_.reset();
            searchMgr_.reset();
            bootstrapMgr_.reset();

            delete bucket;
            bucket = nullptr;
        }
    }

    /*
     * Process incoming command
     */
    void DHT::dispatch(const string& aLine, const string& ip, const string& port, bool isUdpKeyValid)
    {
        // check node's IP address
        if(!Utils::isGoodIPPort(ip, port))
        {
            //socket.send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_BAD_IP, "Your client supplied invalid IP: " + ip, AdcCommand::TYPE_UDP), ip, port);
            return; // invalid ip/port supplied
        }

        try
        {
            AdcCommand cmd(aLine);

            // flood protection
            if(!Utils::checkFlood(ip, cmd))
                return;

            string cid = cmd.getParam(0);
            if(cid.size() != 39)
                return;

            // ignore message from myself
            if(CID(cid) == ctx_.getClientManager()->getMe()->getCID() || ip == lastExternalIP)
                return;

            lastPacket = GET_TICK();

            // don't add node here, because it may block verified nodes when table becomes full of unverified nodes
            Node::Ptr node = createNode(CID(cid), ip, port, true, isUdpKeyValid);

            // all communication to this node will be encrypted with this key
            string udpKey;
            if(cmd.getParam("UK", 1, udpKey))
            {
                node->setUdpKey(CID(udpKey));
            }

            // node is requiring FW check
            string internalUdpPort;
            if(cmd.getParam("FW", 1, internalUdpPort))
            {
                bool firewalled = (internalUdpPort != port);
                if(firewalled)
                    node->getUser()->setFlag(User::PASSIVE);

                // send him his external ip and port
                AdcCommand cmd(AdcCommand::SEV_SUCCESS, AdcCommand::SUCCESS, !firewalled ? "UDP port opened" : "UDP port closed", AdcCommand::TYPE_UDP);
                cmd.addParam("FC", "FWCHECK");
                cmd.addParam("I4", ip);
                cmd.addParam("U4", port);
                send(cmd, ip, port, node->getUser()->getCID(), node->getUdpKey());
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
    void DHT::send(AdcCommand& cmd, const string& ip, const string& port, const CID& targetCID, const CID& udpKey)
    {
        {
            // FW check
            Lock l(fwCheckCs);
            if(requestFWCheck && (firewalledWanted.size() + firewalledChecks.size() < FW_RESPONSES))
            {
                if(firewalledWanted.count(ip) == 0) // only when not requested from this node yet
                {
                    cmd.addParam("FW", getPort());
                    firewalledWanted.insert(ip);
                }
            }
        }
        socket.send(cmd, ip, port, targetCID, udpKey);
    }

    /*
     * Creates new (or update existing) node which is NOT added to our routing table
     */
    Node::Ptr DHT::createNode(const CID& cid, const string& ip, const string& port, bool update, bool isUdpKeyValid)
    {
        // create user as offline (only TCP connected users will be online)
        UserPtr u = ctx_.getClientManager()->getUser(cid);

        Lock l(cs);
        return bucket->createNode(u, ip, port, update, isUdpKeyValid);
    }

    /*
     * Adds node to routing table
     */
    bool DHT::addNode(const Node::Ptr& node, bool makeOnline)
    {
        bool isAcceptable = true;
        if(!node->isOnline())
        {
            {
                Lock l(cs);
                isAcceptable = bucket->insert(node); // insert node to our routing table
            }

            if(makeOnline)
            {
                // put him online so we can make a connection with him
                node->inc();
                node->setOnline(true);
                ctx_.getClientManager()->putOnline(node.get());
            }
        }

        return isAcceptable;
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
        {
            Lock l(cs);
            if(bucket->checkExpiration(aTick))
                setDirty();
        }

        {
            Lock l(fwCheckCs);
            firewalledWanted.clear();
        }
    }

    /*
     * Finds the file in the network
     */
    void DHT::findFile(const string& tth, const string& token)
    {
        if(isConnected())
            getSearchManager().findFile(tth, token);
    }

    /*
     * Sends our info to specified ip:port
     */
    void DHT::info(const string& ip, const string& port, uint32_t type, const CID& targetCID, const CID& udpKey)
    {
        // TODO: what info is needed?
        AdcCommand cmd(AdcCommand::CMD_INF, AdcCommand::TYPE_UDP);

        cmd.addParam("TY", Util::toString(type));
        cmd.addParam("VE", fullADCVersionString);
        cmd.addParam("NI", CTX_SETTING(NICK));
        cmd.addParam("SL", Util::toString(ctx_.getUploadManager()->getSlots()));

        int limit = ctx_.getThrottleManager()->getUpLimit();
        if (CTX_SETTING(THROTTLE_ENABLE) && limit > 0) {
            cmd.addParam("US", Util::toString(limit*1024));
        } else {
            cmd.addParam("US", Util::toString((long)(Util::toDouble(CTX_SETTING(UPLOAD_SPEED))*1024*1024/8)));
        }

        string su;
        if(ctx_.getCryptoManager()->TLSOk())
            su += ADCS_FEATURE ",";

        // TCP status according to global status
        if(ctx_.getClientManager()->isActive(Util::emptyString))
            su += TCP4_FEATURE ",";

        // UDP status according to UDP status check
        if(!isFirewalled())
            su += UDP4_FEATURE ",";

        if(!su.empty()) {
            su.erase(su.size() - 1);
        }
        cmd.addParam("SU", su);

        send(cmd, ip, port, targetCID, udpKey);
    }

    /*
     * Sends Connect To Me request to online node
     */
    void DHT::connect(const OnlineUser& ou, const string& token)
    {
        // this is DHT's node, so we can cast ou to Node
        getConnectionManager().connect((Node*)&ou, token);
    }

    /*
     * Sends private message to online node
     */
    void DHT::privateMessage(const OnlineUser& /*ou*/, const string& /*aMessage*/, bool /*thirdPerson*/)
    {
        //AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_UDP);
        //cmd.addParam(aMessage);
        //if(thirdPerson)
        //  cmd.addParam("ME", "1");
        //
        //send(cmd, ou.getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(ou.getIdentity().getUdpPort())));
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
            if(f.getLastModified() > time(nullptr) - 7 * 24 * 60 * 60)
                bucket->loadNodes(xml);

            // load indexes
            getIndexManager().loadIndexes(xml);
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
        getIndexManager().saveIndexes(xml);

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
        //string ip = node->getIdentity().getIp();
        string udpPort = node->getIdentity().getUdpPort();

        InfType it = NONE;
        for(StringIterC i = c.getParameters().begin() + 1; i != c.getParameters().end(); ++i)
        {
            if(i->length() < 2)
                continue;

            string parameter_name = i->substr(0, 2);
            if(parameter_name == "TY")
                it = (InfType)Util::toInt(i->substr(2));
            else if((parameter_name != "I4") && (parameter_name != "U4") && (parameter_name != "UK")) // avoid IP+port spoofing + don't store key into map
                node->getIdentity().set(i->c_str(), i->substr(2));
        }

        if(node->getIdentity().supports(ADCS_FEATURE))
        {
            node->getUser()->setFlag(User::TLS);
        }

        if(!node->getIdentity().get("US").empty()) {
            node->getIdentity().setConnection(Util::formatBytes(node->getIdentity().get("US")) + "/s");
        }

        // add node to our routing table and put him online if it is required
        addNode(node, true);

        // do we wait for any search results from this user?
        getSearchManager().processSearchResults(node->getUser(), Util::toInt(node->getIdentity().get("SL")));

        if(it & PING)
        {
            // remove ping flag to avoid ping-pong-ping-pong-ping...
            info(node->getIdentity().getIp(), udpPort, it & ~PING, node->getUser()->getCID(), node->getUdpKey());
        }
    }

    // incoming search request
    void DHT::handle(AdcCommand::SCH, const Node::Ptr& node, AdcCommand& c) throw()
    {
        getSearchManager().processSearchRequest(node, c);
    }

    // incoming search result
    void DHT::handle(AdcCommand::RES, const Node::Ptr& node, AdcCommand& c) throw()
    {
        getSearchManager().processSearchResult(node, c);
    }

    // incoming publish request
    void DHT::handle(AdcCommand::PUB, const Node::Ptr& node, AdcCommand& c) throw()
    {
        if(!isFirewalled()) // we should index this entry only if our UDP port is opened
            getIndexManager().processPublishSourceRequest(node, c);
    }

    // connection request
    void DHT::handle(AdcCommand::CTM, const Node::Ptr& node, AdcCommand& c) throw()
    {
        getConnectionManager().connectToMe(node, c);
    }

    // reverse connection request
    void DHT::handle(AdcCommand::RCM, const Node::Ptr& node, AdcCommand& c) throw()
    {
        getConnectionManager().revConnectToMe(node, c);
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
#ifdef _DEBUG
                // don't do anything
                string tth;
                if(!c.getParam("TR", 1, tth))
                    return;

                try
                {
                    string fileName = Util::getFileName(ctx_.getShareManager()->toVirtual(TTHValue(tth)));
                    ctx_.getLogManager()->message(str(F_("DHT (%1%): File published: %2%") % fromIP % fileName));
                }
                catch(ShareException&)
                {
                    // published non-shared file??? Maybe partial file
                    ctx_.getLogManager()->message(str(F_("DHT (%1%): Partial file published: %2%") % fromIP % tth));

                }
#endif
            }
            else if(resTo == "FWCHECK")
            {
                Lock l(fwCheckCs);
                if(!firewalledWanted.count(fromIP))
                    return; // we didn't requested firewall check from this node

                firewalledWanted.erase(fromIP);
                if(firewalledChecks.count(fromIP))
                    return; // already received firewall check from this node

                string externalIP;
                string externalUdpPort;
                if(!c.getParam("I4", 1, externalIP) || !c.getParam("U4", 1, externalUdpPort))
                    return; // no IP and port in response

                firewalledChecks.insert(std::make_pair(fromIP, std::make_pair(externalIP, externalUdpPort)));

                if(firewalledChecks.size() == FW_RESPONSES)
                {
                    // when we received more firewalled statuses, we will be firewalled
                    int fw = 0; string lastIP;
                    for(const auto &i : firewalledChecks)
                    {
                        const string ip = i.second.first;
                        const string udpPort = i.second.second;

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
                        //This happens for one reason.. a client responded with a bad IP.
                        if(ip == lastIP)
                            externalIP = ip;
                        else
                            lastIP = ip;
                    }

                    if(fw >= 0)
                    {
                        // we are probably firewalled, so our internal UDP port is unaccessible
                        if(externalIP != lastExternalIP || !firewalled)
                            ctx_.getLogManager()->message(str(F_("DHT: Firewalled UDP status set (IP: %1%)") % externalIP));
                        firewalled = true;
                    }
                    else
                    {
                        if(externalIP != lastExternalIP || firewalled)
                            ctx_.getLogManager()->message(str(F_("DHT: Our UDP port seems to be opened (IP: %1%)") % externalIP));

                        firewalled = false;
                    }

                    if(!CTX_BOOLSETTING(NO_IP_OVERRIDE))
                        ctx_.getSettingsManager()->set(SettingsManager::EXTERNAL_IP, externalIP);

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
        //  ctx_.getLogManager()->message("DHT (" + fromIP + "): " + msg);
    }

    // partial file request
    void DHT::handle(AdcCommand::PSR, const Node::Ptr& node, AdcCommand& c) throw()
    {
        c.getParameters().erase(c.getParameters().begin());  // remove CID from UDP command
        ctx_.getSearchManager()->onPSR(c, node->getUser(), node->getIdentity().getIp());
    }

    // private message
    void DHT::handle(AdcCommand::MSG, const Node::Ptr& /*node*/, AdcCommand& /*c*/) throw()
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
            getClosestNodes(CID::generate(), nodes, 20, 2);

            // add nodelist in XML format
            for(Node::Map::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
                xml.addTag("Node");
                xml.addChildAttrib("CID", i->second->getUser()->getCID().toBase32());
                xml.addChildAttrib("I4", i->second->getIdentity().getIp());
                xml.addChildAttrib("U4", i->second->getIdentity().getUdpPort());
            }

            xml.stepOut();

            string nodesXML;
            StringRefOutputStream sos(nodesXML);
            //sos.write(SimpleXML::utf8Header);
            xml.toXML(&sos);

            cmd.addParam(Utils::compressXML(nodesXML));

            send(cmd, node->getIdentity().getIp(), node->getIdentity().getUdpPort(), node->getUser()->getCID(), node->getUdpKey());
        }
    }

    void DHT::handle(AdcCommand::SND, const Node::Ptr& node, AdcCommand& c) throw()
    {
        if(c.getParam(1) == "nodes" && c.getParam(2) == "dht.xml")
        {
            // add node to our routing table
            if(node->isIpVerified())
                addNode(node, false);

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

                    if(!cid)
                        continue;

                    // don't bother with myself
                    if(ctx_.getClientManager()->getMe()->getCID() == cid)
                        continue;

                    const string i4    = xml.getChildAttrib("I4");
                    const string u4    = Util::toString(xml.getIntChildAttrib("U4"));

                    // don't bother with private IPs
                    if(!Utils::isGoodIPPort(i4, u4))
                        continue;

                    // create verified node, it's not big risk here and allows faster bootstrapping
                    // if this node already exists in our routing table, don't update it's ip/port for security reasons
                    Node::Ptr node = createNode(cid, i4, u4, false, true);
                    addNode(node, false);
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
