/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ConnectivityManager.h"
#include "SettingsManager.h"
#include "ClientManager.h"
#include "ConnectionManager.h"
#include "SearchManager.h"
#include "LogManager.h"
#include "UPnPManager.h"

namespace dcpp {

ConnectivityManager::ConnectivityManager() :
autoDetected(false),
running(false)
{
}

void ConnectivityManager::startSocket() {
   autoDetected = false;

   disconnect();

   if(ClientManager::getInstance()->isActive()) {
       listen();

       // must be done after listen calls; otherwise ports won't be set
       if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP)
           UPnPManager::getInstance()->open();
   }
}

void ConnectivityManager::detectConnection() {
        if (running)
                return;

        running = true;

    SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, Util::emptyString);
    SettingsManager::getInstance()->set(SettingsManager::NO_IP_OVERRIDE, false);

   if (UPnPManager::getInstance()->getOpened()) {
       UPnPManager::getInstance()->close();
   }

   disconnect();

   log(_("Determining connection type..."));
   try {
       listen();
   } catch(const Exception& e) {
       SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
       log(str(F_("Unable to open %1% port(s). You must set up your connection manually") % e.getError()));
       fire(ConnectivityManagerListener::Finished());
                running = false;
       return;
   }

   autoDetected = true;

   if (!Util::isPrivateIp(Util::getLocalIp())) {
       SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
       log(_("Public IP address detected, selecting active mode with direct connection"));
       fire(ConnectivityManagerListener::Finished());
                running = false;
       return;
   }

   SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_UPNP);
   log(_("Local network with possible NAT detected, trying to map the ports using UPnP..."));

        if (!UPnPManager::getInstance()->open()) {
                running = false;
        }
}

void ConnectivityManager::setup(bool settingsChanged, int lastConnectionMode) {
   if(BOOLSETTING(AUTO_DETECT_CONNECTION)) {
       if (!autoDetected) detectConnection();
   } else {
        if(autoDetected || settingsChanged || SearchManager::getInstance()->getPort() != SETTING(UDP_PORT) || ConnectionManager::getInstance()->getPort() != SETTING(TCP_PORT) || ConnectionManager::getInstance()->getSecurePort() != SETTING(TLS_PORT)) {
           if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP || lastConnectionMode == SettingsManager::INCOMING_FIREWALL_UPNP) {
               UPnPManager::getInstance()->close();
           }
           startSocket();
       } else if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP && !UPnPManager::getInstance()->getOpened()) {
           // previous UPnP mappings had failed; try again
           UPnPManager::getInstance()->open();
       }
   }
}

void ConnectivityManager::mappingFinished(bool success) {
   if(BOOLSETTING(AUTO_DETECT_CONNECTION)) {
       if (!success) {
           disconnect();
           SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
           log(_("Automatic setup of active mode has failed. You may want to set up your connection manually for better connectivity"));
       }
       fire(ConnectivityManagerListener::Finished());
   }

        running = false;
}

void ConnectivityManager::listen() {
   try {
       ConnectionManager::getInstance()->listen();
   } catch(const Exception&) {
       throw Exception(_("TCP/TLS"));
   }

   try {
       SearchManager::getInstance()->listen();
   } catch(const Exception&) {
       throw Exception(_("UDP"));
   }
}

void ConnectivityManager::disconnect() {
        SearchManager::getInstance()->disconnect();
        ConnectionManager::getInstance()->disconnect();
}

void ConnectivityManager::log(const string& message) {
   if(BOOLSETTING(AUTO_DETECT_CONNECTION)) {
       LogManager::getInstance()->message(_("Connectivity: ") + message);
       fire(ConnectivityManagerListener::Message(), message);
   } else {
       LogManager::getInstance()->message(message);
   }
}

} // namespace dcpp
