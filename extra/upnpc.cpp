/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 *
 * This program uses the MiniUPnP client library by Thomas Bernard
 * http://miniupnp.free.fr http://miniupnp.tuxfamily.org
 */

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
    UPNPDev *devices = upnpDiscover(5000, SettingsManager::getInstance()->isDefault(SettingsManager::BIND_ADDRESS) ? 0 : SETTING(BIND_ADDRESS).c_str(), 0, 0
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

std::pair<bool, bool> UPnPc::add(const unsigned short port, const UPnP::Protocol protocol, const string& description)
{
    const string port_ = Util::toString(port);

    bool ipv4_portmap = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port_.c_str(), port_.c_str(),
        (SETTING(BIND_IFACE)? Util::getIfaceI4(SETTING(BIND_IFACE_NAME)).c_str() : SETTING(BIND_ADDRESS).c_str())
                               , description.c_str(), protocols[protocol], NULL
#if (MINIUPNPC_API_VERSION == 8 || defined(MINIUPNPC16))
                                                                                    , 0) == UPNPCOMMAND_SUCCESS;
#else
                                                                                    ) == UPNPCOMMAND_SUCCESS;
#endif
    bool ipv6_portmap = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port_.c_str(), port_.c_str(),
        (SETTING(BIND_IFACE6)? Util::getIfaceI6(SETTING(BIND_IFACE_NAME6)).c_str() : SETTING(BIND_ADDRESS6).c_str())
                               , description.c_str(), protocols[protocol], NULL
#if (MINIUPNPC_API_VERSION == 8 || defined(MINIUPNPC16))
                                                                                    , 0) == UPNPCOMMAND_SUCCESS;
#else
                                                                                    ) == UPNPCOMMAND_SUCCESS;
#endif
    return std::make_pair(ipv4_portmap,ipv6_portmap);
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
