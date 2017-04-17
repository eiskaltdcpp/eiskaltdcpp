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
    UPNPDev *devices = upnpDiscover(5000, SettingsManager::getInstance()->isDefault(SettingsManager::BIND_ADDRESS) ? 0 : SETTING(BIND_ADDRESS).c_str(), NULL, 0
#if (MINIUPNPC_API_VERSION >= 8 || defined(MINIUPNPC16))
                                        , 0
#if (MINIUPNPC_API_VERSION >= 14)
                                        , 2
#endif
                                        , NULL);
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
        Util::getLocalIp(AF_INET).c_str(), description.c_str(), protocols[protocol], NULL
#if (MINIUPNPC_API_VERSION >= 8 || defined(MINIUPNPC16))
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
