/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "upnpc.h"
#include <dcpp/Util.h>
#include <dcpp/SettingsManager.h>
#ifndef STATICLIB
#define STATICLIB
#endif
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

static UPNPUrls urls;
static IGDdatas data;
const std::string UPnPc::name = "MiniUPnP";

using namespace std;
using namespace dcpp;

bool UPnPc::init()
{
    string mcastif = SETTING(BIND_IFACE)? Util::getIfaceI4(SETTING(BIND_IFACE_NAME)) : SETTING(BIND_ADDRESS);
    UPNPDev *devices = upnpDiscover(5000, mcastif.c_str(), 0, 0
#if (MINIUPNPC_API_VERSION == 8 || defined(MINIUPNPC16))
                                        , 0, 0);
#else
                                        );
#endif

    if (!devices)
        return false;

    bool ret = UPNP_GetValidIGD(devices, &urls, &data, 0, 0);

    freeUPNPDevlist(devices);

    return ret;
}

bool UPnPc::add(const unsigned short port, const UPnP::Protocol protocol, const string& description)
{
    const string port_ = Util::toString(port);

    return UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port_.c_str(), port_.c_str(),
        (SETTING(BIND_IFACE)? Util::getIfaceI4(SETTING(BIND_IFACE_NAME)).c_str() : SETTING(BIND_ADDRESS).c_str())
                               , description.c_str(), protocols[protocol], NULL
#if (MINIUPNPC_API_VERSION == 8 || defined(MINIUPNPC16))
                                                                                    , 0) == UPNPCOMMAND_SUCCESS;
#else
                                                                                    ) == UPNPCOMMAND_SUCCESS;
#endif
}

bool UPnPc::remove(const unsigned short port, const UPnP::Protocol protocol)
{
    return UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, Util::toString(port).c_str(),
        protocols[protocol], NULL) == UPNPCOMMAND_SUCCESS;
}

string UPnPc::getExternalIP()
{
    char buf[16] = { 0 };
    if (UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, buf) == UPNPCOMMAND_SUCCESS)
        return buf;
    return Util::emptyString;
}
