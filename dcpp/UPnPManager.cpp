/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "format.h"
#include "UPnPManager.h"

#include "ConnectionManager.h"
#include "SearchManager.h"
#include "LogManager.h"
#include "version.h"
#include "ConnectivityManager.h"
#ifdef USE_MINIUPNP
#include "extra/upnpc.h"
    #ifdef MINIUPNP_WITH_IPV6
    #include "extra/upnpc6.h"
    #endif
#endif
#ifdef WITH_DHT
#include "dht/DHT.h"
#endif

namespace dcpp {
    
UPnPManager::UPnPManager() : opened(false), portMapping(false) {
#ifdef USE_MINIUPNP
    addImplementation(new UPnPc());
    #ifdef MINIUPNP_WITH_IPV6
        addImplementation(new UPnPc6());
    #endif
#endif
}
void UPnPManager::addImplementation(UPnP* impl) {
    impls.push_back(impl);
}

bool UPnPManager::open() {
    if(opened)
        return false;

    if(impls.empty()) {
        log(_("No UPnP implementation available"));
        return false;
    }

    if(portMapping.exchange(true) == true) {
        log(_("Another UPnP port mapping attempt is in progress..."));
        return false;
    }

    start();

    return true;
}

void UPnPManager::close() {
    for(Impls::iterator i = impls.begin(); i != impls.end(); ++i)
        close(*i);
    opened = false;
}

int UPnPManager::run() {
    setThreadName("UPnPManager");
    // cache these
    const unsigned short
        conn_port = Util::toInt(ConnectionManager::getInstance()->getPort()),
        secure_port = Util::toInt(ConnectionManager::getInstance()->getSecurePort()),
        search_port = Util::toInt(SearchManager::getInstance()->getPort());
#ifdef WITH_DHT
        const unsigned short dht_port = Util::toInt(dht::DHT::getInstance()->getPort());
#endif

    for(Impls::iterator i = impls.begin(); i != impls.end(); ++i) {
        UPnP& impl = *i;

        close(impl);

        if(!impl.init()){
            log(str(F_("Failed to initialize the %1% interface") % impl.getName()));
            continue;
        }

        if(conn_port != 0 && !impl.open(conn_port, UPnP::PROTOCOL_TCP, str(F_(APPNAME " Transfer Port (%1% TCP)") % conn_port))){
            log(str(F_("The %1% interface has failed to map the %2% %3% port") % impl.getName() % "TCP" % conn_port));
            continue;
        }

        if(secure_port != 0 && !impl.open(secure_port, UPnP::PROTOCOL_TCP, str(F_(APPNAME " Encrypted Transfer Port (%1% TCP)") % secure_port))){
            log(str(F_("The %1% interface has failed to map the %2% %3% port") % impl.getName() % "TLS" % secure_port));
            continue;
        }

        if(search_port != 0 && !impl.open(search_port, UPnP::PROTOCOL_UDP, str(F_(APPNAME " Search Port (%1% UDP)") % search_port))){
            log(str(F_("The %1% interface has failed to map the %2% %3% port") % impl.getName() % "UDP" % search_port));
            continue;
        }
#ifdef WITH_DHT
        if(dht_port != 0 && !impl.open(dht_port, UPnP::PROTOCOL_UDP, str(F_(APPNAME " DHT Port (%1% UDP)") % dht_port))){
            log(str(F_("The %1% interface has failed to map the %2% %3% port") % impl.getName() % "UDP" % dht_port));
            continue;
        }
#endif

        opened |= true;
        log(str(F_("Successfully created port mappings (TCP: %1%, UDP: %2%, TLS: %3%), mapped using the %4% interface") % conn_port % search_port % secure_port % impl.getName()));
        if (!impl.isIpV6()) {
            if(!BOOLSETTING(NO_IP_OVERRIDE)) {
                // now lets configure the external IP (connect to me) address
                string ExternalIP = impl.getExternalIP();
                if(!ExternalIP.empty()) {
                    // woohoo, we got the external IP from the UPnP framework
                    SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, ExternalIP);
                } else {
                    //:-( Looks like we have to rely on the user setting the external IP manually
                    // no need to do cleanup here because the mappings work
                    log(_("Failed to get external IP"));
                }
            }
        }

        ConnectivityManager::getInstance()->mappingFinished(true);

//        break;
    }

    ConnectivityManager::getInstance()->mappingFinished(opened);
    if(!opened) {
        log(_("Failed to create port mappings"));
        ConnectivityManager::getInstance()->mappingFinished(false);
    }
    portMapping = false;
    return 0;
}

void UPnPManager::close(UPnP& impl) {
        if(impl.hasRules()) {
            log(impl.close() ? str(F_("Successfully removed port mappings with the %1% interface") % impl.getName()) :
                            str(F_("Failed to remove port mappings with the %1% interface") % impl.getName()));
        }
}

void UPnPManager::log(const string& message) {
    ConnectivityManager::getInstance()->log(str(F_("UPnP: %1%") % message));
}

//#ifdef USE_MINIUPNP
//void UPnPManager::runMiniUPnP() {
    //addImplementation(new UPnPc());
//#ifdef MINIUPNP_WITH_IPV6
    //addImplementation(new UPnPc6());
//#endif
//}
//#endif

} // namespace dcpp
