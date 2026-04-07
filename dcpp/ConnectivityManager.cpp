/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "ConnectivityManager.h"

#include "ClientManager.h"
#include "ConnectionManager.h"
#include "debug.h"
#include "format.h"
#include "LogManager.h"
#include "MappingManager.h"
#include "SearchManager.h"
#include "SettingsManager.h"
#include "version.h"
#ifdef WITH_DHT
#include "dht/DHT.h"
#endif

namespace dcpp {

ConnectivityManager::ConnectivityManager(DCContext& ctx) :
    ContextAware(ctx),
    autoDetected(false),
    running(false)
{
    updateLast();
}

void ConnectivityManager::startSocket() {
    autoDetected = false;

    disconnect();

    if(ctx().getClientManager()->isActive()) {
        listen();

        // must be done after listen calls; otherwise ports won't be set
        if(CTX_SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP)
            ctx().getMappingManager()->open();
    }

    updateLast();
}

void ConnectivityManager::detectConnection() {
    if (running)
        return;

    running = true;

    //fire(ConnectivityManagerListener::Started());

    // restore connectivity settings to their default value.
    ctx().getSettingsManager()->unset(SettingsManager::TCP_PORT);
    ctx().getSettingsManager()->unset(SettingsManager::UDP_PORT);
    ctx().getSettingsManager()->unset(SettingsManager::TLS_PORT);
    ctx().getSettingsManager()->unset(SettingsManager::EXTERNAL_IP);
    ctx().getSettingsManager()->unset(SettingsManager::NO_IP_OVERRIDE);
    //ctx().getSettingsManager()->unset(SettingsManager::MAPPER);
    ctx().getSettingsManager()->unset(SettingsManager::BIND_ADDRESS);

    if (ctx().getMappingManager()->getOpened()) {
        ctx().getMappingManager()->close();
    }

    disconnect();

    log(_("Determining the best connectivity settings..."));
    try {
        listen();
    } catch(const Exception& e) {
        ctx().getSettingsManager()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
        log(str(F_("Unable to open %1% port(s); connectivity settings must be configured manually") % e.getError()));
        fire(ConnectivityManagerListener::Finished());
        running = false;
        return;
    }

    autoDetected = true;

    if (!Util::isPrivateIp(Util::getLocalIp(AF_INET))) {
        ctx().getSettingsManager()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
        log(_("Public IP address detected, selecting active mode with direct connection"));
        fire(ConnectivityManagerListener::Finished());
        running = false;
        return;
    }

    ctx().getSettingsManager()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_UPNP);
    log(_("Local network with possible NAT detected, trying to map the ports using UPnP..."));

    if (!ctx().getMappingManager()->open()) {
        running = false;
    }
}

void ConnectivityManager::setup(bool settingsChanged) {
    if(CTX_BOOLSETTING(AUTO_DETECT_CONNECTION)) {
        if (!autoDetected) detectConnection();
    } else {
        if(autoDetected || (settingsChanged && (ctx().getSearchManager()->getPort() != Util::toString(CTX_SETTING(UDP_PORT)) || ctx().getConnectionManager()->getPort() != Util::toString(CTX_SETTING(TCP_PORT)) || ctx().getConnectionManager()->getSecurePort() != Util::toString(CTX_SETTING(TLS_PORT)) || CTX_SETTING(BIND_ADDRESS) != lastBind))) {
            if(settingsChanged || CTX_SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_UPNP) {
                ctx().getMappingManager()->close();
            }
            startSocket();
        } else if(CTX_SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP && !ctx().getMappingManager()->getOpened()) {
            // previous UPnP mappings had failed; try again
            ctx().getMappingManager()->open();
        }
    }
}

void ConnectivityManager::mappingFinished(bool success) {
    if(CTX_BOOLSETTING(AUTO_DETECT_CONNECTION)) {
        if (!success) {
            disconnect();
            ctx().getSettingsManager()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
            log(_("Automatic setup of active mode has failed. You may want to set up your connection manually for better connectivity"));
        }
        fire(ConnectivityManagerListener::Finished());
    }

    running = false;
}

void ConnectivityManager::listen() {
    try {
        ctx().getConnectionManager()->listen();
    } catch(const Exception&) {
        throw Exception(_("Transfer (TCP)"));
    }

    try {
        ctx().getSearchManager()->listen();
    } catch(const Exception&) {
        throw Exception(_("Search (UDP)"));
    }
#ifdef WITH_DHT
    try {
        ctx().getDHT()->start();
    } catch (const Exception&) {
        throw Exception(_("DHT (UDP)"));
    }
#endif
}

void ConnectivityManager::disconnect() {
    ctx().getSearchManager()->disconnect();
    ctx().getConnectionManager()->disconnect();
#ifdef WITH_DHT
    ctx().getDHT()->stop();
#endif
}

void ConnectivityManager::log(const string& message) {
    if(CTX_BOOLSETTING(AUTO_DETECT_CONNECTION)) {
        ctx().getLogManager()->message(_("Connectivity: ") + message);
        fire(ConnectivityManagerListener::Message(), message);
    } else {
        ctx().getLogManager()->message(message);
    }
}

void ConnectivityManager::updateLast() {
    lastBind = CTX_SETTING(BIND_ADDRESS);
}

} // namespace dcpp
