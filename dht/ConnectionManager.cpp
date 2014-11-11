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

#include "Constants.h"
#include "ConnectionManager.h"
#include "DHT.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/CryptoManager.h"
#include "dcpp/ClientManager.h"

namespace dht
{

    ConnectionManager::ConnectionManager(void)
    {
    }

    ConnectionManager::~ConnectionManager(void)
    {
    }

    /*
     * Sends Connect To Me request to online node
     */
    void ConnectionManager::connect(const Node::Ptr& node, const string& token)
    {
        connect(node, token, CryptoManager::getInstance()->TLSOk() && node->getUser()->isSet(User::TLS));
    }

    void ConnectionManager::connect(const Node::Ptr& node, const string& token, bool secure)
    {
        // don't allow connection if we didn't proceed a handshake
        if(!node->isOnline())
        {
            // do handshake at first
            DHT::getInstance()->info(node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())),
                DHT::PING | DHT::MAKE_ONLINE, node->getUser()->getCID(), node->getUdpKey());
            return;
        }

        bool active = ClientManager::getInstance()->isActive();

        // if I am not active, send reverse connect to me request
        AdcCommand cmd(active ? AdcCommand::CMD_CTM : AdcCommand::CMD_RCM, AdcCommand::TYPE_UDP);
        cmd.addParam(secure ? SECURE_CLIENT_PROTOCOL_TEST : CLIENT_PROTOCOL);

        if(active)
        {
            uint16_t port = secure ? dcpp::ConnectionManager::getInstance()->getSecurePort() : dcpp::ConnectionManager::getInstance()->getPort();
            cmd.addParam(Util::toString(port));
        }

        cmd.addParam(token);

        DHT::getInstance()->send(cmd, node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())),
            node->getUser()->getCID(), node->getUdpKey());
    }

    /*
     * Creates connection to specified node
     */
    void ConnectionManager::connectToMe(const Node::Ptr& node, const AdcCommand& cmd)
    {
        // don't allow connection if we didn't proceed a handshake
        if(!node->isOnline())
        {
            // do handshake at first
            DHT::getInstance()->info(node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())),
                DHT::PING | DHT::MAKE_ONLINE, node->getUser()->getCID(), node->getUdpKey());
            return;
        }

        const string& protocol = cmd.getParam(1);
        const string& port = cmd.getParam(2);
        const string& token = cmd.getParam(3);

        bool secure = false;
        if(protocol == CLIENT_PROTOCOL)
        {
            // Nothing special
        }
        else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk())
        {
            secure = true;
        }
        else
        {
            AdcCommand cmd(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_UDP);
            cmd.addParam("PR", protocol);
            cmd.addParam("TO", token);

            DHT::getInstance()->send(cmd, node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())),
                node->getUser()->getCID(), node->getUdpKey());
            return;
        }

        if(!node->getIdentity().isTcpActive())
        {
            AdcCommand err(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "IP unknown", AdcCommand::TYPE_UDP);
            DHT::getInstance()->send(err, node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())),
                node->getUser()->getCID(), node->getUdpKey());
            return;
        }

        dcpp::ConnectionManager::getInstance()->adcConnect(*node, static_cast<uint16_t>(Util::toInt(port)), token, secure);
    }

    /*
     * Sends request to create connection with me
     */
    void ConnectionManager::revConnectToMe(const Node::Ptr& node, const AdcCommand& cmd)
    {
        // don't allow connection if we didn't proceed a handshake
        //if(!node->isOnline())
        //  return;

        // this is valid for active-passive connections only
        if(!ClientManager::getInstance()->isActive())
            return;

        const string& protocol = cmd.getParam(1);
        const string& token = cmd.getParam(2);

        bool secure;
        if(protocol == CLIENT_PROTOCOL)
        {
            secure = false;
        }
        else if(protocol == SECURE_CLIENT_PROTOCOL_TEST && CryptoManager::getInstance()->TLSOk())
        {
            secure = true;
        }
        else
        {
            AdcCommand sta(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_UNSUPPORTED, "Protocol unknown", AdcCommand::TYPE_UDP);
            sta.addParam("PR", protocol);
            sta.addParam("TO", token);

            DHT::getInstance()->send(sta, node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())),
                node->getUser()->getCID(), node->getUdpKey());
            return;
        }

        connect(node, token, secure);
    }

}
