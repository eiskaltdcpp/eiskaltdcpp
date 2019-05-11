/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2019 Boris Pek <tehnick-8@yandex.ru>
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
    const string bind_address = SETTING(BIND_ADDRESS);
    const char *multicast_interface = SettingsManager::getInstance()->isDefault(SettingsManager::BIND_ADDRESS) ? nullptr : bind_address.c_str();

#if (MINIUPNPC_API_VERSION >= 14)
    UPNPDev *devices = upnpDiscover(5000, multicast_interface, nullptr, 0, 0, 2, nullptr);
#else
    UPNPDev *devices = upnpDiscover(5000, multicast_interface, nullptr, 0, 0, nullptr);
#endif

    if (!devices)
        return false;

    bool ret = UPNP_GetValidIGD(devices, &urls, &data, nullptr, 0);

    freeUPNPDevlist(devices);

    return ret;
}

bool UPnPc::add(const string& port, const UPnP::Protocol protocol, const string& description)
{
    return UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(),
        Util::getLocalIp(AF_INET).c_str(), description.c_str(), protocols[protocol], nullptr, nullptr) == UPNPCOMMAND_SUCCESS;
}

bool UPnPc::remove(const string& port, const UPnP::Protocol protocol)
{
    return UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, port.c_str(),
        protocols[protocol], nullptr) == UPNPCOMMAND_SUCCESS;
}

string UPnPc::getExternalIP()
{
    char buf[16] = { 0 };
    if (UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, buf) == UPNPCOMMAND_SUCCESS)
        return string(buf);
    return Util::emptyString;
}
