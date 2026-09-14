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
 * http://miniupnp.free.fr https://miniupnp.tuxfamily.org/
 */

#include "upnpc.h"
#include "dcpp/LogManager.h"
#include "dcpp/Util.h"
#include "dcpp/SettingsManager.h"
#ifndef STATICLIB
#define STATICLIB
#endif
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

namespace {

void logMiniUPnPFailure(const std::string& message) {
    dcpp::LogManager::getInstance()->message("UPnP: MiniUPnP: " + message);
}

}

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
    int discoverError = 0;
    UPNPDev *devices = upnpDiscover(5000, multicast_interface, nullptr, 0, 0, 2, &discoverError);
#else
    UPNPDev *devices = upnpDiscover(5000, multicast_interface, nullptr, 0, 0, nullptr);
#endif

    if (!devices) {
#if (MINIUPNPC_API_VERSION >= 14)
        logMiniUPnPFailure("SSDP discovery found no devices (error " + std::to_string(discoverError) + ")");
#else
        logMiniUPnPFailure("SSDP discovery found no devices");
#endif
        return false;
    }

#if (MINIUPNPC_API_VERSION >= 18)
    const int ret = UPNP_GetValidIGD(devices, &urls, &data, nullptr, 0, nullptr, 0);
#else
    const int ret = UPNP_GetValidIGD(devices, &urls, &data, nullptr, 0);
#endif

    freeUPNPDevlist(devices);

    if(ret == 0)
        logMiniUPnPFailure("SSDP discovery found devices, but no valid Internet Gateway Device was available");

    return ret != 0;
}

bool UPnPc::add(const string& port, const UPnP::Protocol protocol, const string& description)
{
    const string localIp = Util::getLocalIp(AF_INET);
    const int addResult = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(),
        localIp.c_str(), description.c_str(), protocols[protocol], nullptr, nullptr);

    if (addResult == UPNPCOMMAND_SUCCESS)
        return true;

    /*
     * Some IGDs reject AddPortMapping when the same mapping already exists.
     * This happens frequently after an unclean app exit or when the router keeps
     * leased mappings around across client restarts. Treat an exact existing
     * mapping to this host and port as success; connectivity is already active,
     * and adding a rule lets normal shutdown clean it up later.
     */
    char intClient[64] = { 0 };
    char intPort[16] = { 0 };
    char desc[128] = { 0 };
    char enabled[8] = { 0 };
    char leaseDuration[16] = { 0 };
    const int existingResult = UPNP_GetSpecificPortMappingEntry(urls.controlURL, data.first.servicetype,
        port.c_str(), protocols[protocol], nullptr, intClient, intPort, desc, enabled, leaseDuration);

    return existingResult == UPNPCOMMAND_SUCCESS && localIp == intClient && port == intPort;
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
