#include "UPnPMapper.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/SearchManager.h"
#include "dcpp/LogManager.h"

using namespace dcpp;

UPnPMapper::UPnPMapper()
{
}

UPnPMapper::~UPnPMapper(){
    unmap();
}

void UPnPMapper::forward(){
    unmap();

    UPnP *UPNP = UPnP::getInstance();

    if( SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP ) {
        bool ok = true;

        UPnP::Port port = static_cast<UPnP::Port>(ConnectionManager::getInstance()->getPort());

        if(port != 0)
            ok &= UPNP->forward(port, UPnP::TCP);

        mapped.insert(port, UPnP::TCP);

        port = static_cast<UPnP::Port>(ConnectionManager::getInstance()->getSecurePort());

        if(ok && port != 0)
            ok &= UPNP->forward(port, UPnP::TCP);

        mapped.insert(port, UPnP::TCP);

        port = static_cast<UPnP::Port>(SearchManager::getInstance()->getPort());

        if(ok && port != 0)
            UPNP->forward(port, UPnP::UDP);

        mapped.insert(port, UPnP::UDP);

        if(ok) {
            if(!BOOLSETTING(NO_IP_OVERRIDE)) {
                // now lets configure the external IP (connect to me) address
                string ExternalIP = UPNP->getExternalIP().toStdString();

                if ( !ExternalIP.empty() ) {
                    // woohoo, we got the external IP from the UPnP framework
                    SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, ExternalIP );
                }
                else {
                    //:-( Looks like we have to rely on the user setting the external IP manually
                    // no need to do cleanup here because the mappings work
                    LogManager::getInstance()->message(tr("Failed to get external IP via  UPnP. Please set it yourself.").toStdString());
                }
            }

        }
        else {
            LogManager::getInstance()->message(tr("Failed to create port mappings. Please set up your NAT yourself.").toStdString());
            unmap();
        }
    }
}

void UPnPMapper::unmap(){
    UPnP *UPNP = UPnP::getInstance();

    UPnPMap::iterator it = mapped.begin();

    for (; it != mapped.end(); ++it)
        UPNP->unmap(it.key(), it.value());

    mapped.clear();
}
