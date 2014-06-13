/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATICLIB
#define STATICLIB
#endif
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

#if (MINIUPNPC_API_VERSION >= 8)

#include "upnpc6.h"
#include <dcpp/Util.h>
#include <dcpp/SettingsManager.h>

static UPNPUrls urls6;
static IGDdatas data6;
const std::string UPnPc6::name = "MiniUPnP6";
static std::map<uint64_t,uint16_t> uniqIDMap;

using namespace std;
using namespace dcpp;

bool UPnPc6::init()
{
    string mcastif = SETTING(BIND_IFACE6)? Util::getIfaceI6(SETTING(BIND_IFACE_NAME6)) : SETTING(BIND_ADDRESS6);
    UPNPDev *devices = upnpDiscover(5000, mcastif.c_str(), 0, 0, 1, 0);

    if (!devices)
        return false;

    bool ret = UPNP_GetValidIGD(devices, &urls6, &data6, 0, 0);

    freeUPNPDevlist(devices);

    return ret;
}

bool UPnPc6::add(const unsigned short port, const UPnP::Protocol protocol, const string& description)
{
    const string port_ = Util::toString(port);
    char uniqID[8];
    string proto;
    if (protocols[protocol] == "TCP") 
        proto.append(Util::toString(IPPROTO_TCP));
    if (protocols[protocol] == "UDP") 
        proto.append(Util::toString(IPPROTO_UDP));
    int ret = UPNP_AddPinhole(urls6.controlURL, data6.first.servicetype, "::",  "0",
            (SETTING(BIND_IFACE6)? Util::getIfaceI6(SETTING(BIND_IFACE_NAME6)).c_str() : SETTING(BIND_ADDRESS6).c_str()),
                              port_.c_str(),
                              proto.c_str(), "86400", (char*)&uniqID);
    if (ret == UPNPCOMMAND_SUCCESS)
        uniqIDMap.insert(std::map<uint64_t,uint16_t>::value_type((port*(protocol+1)), atoi(uniqID)));
    return ret == UPNPCOMMAND_SUCCESS;
}

bool UPnPc6::remove(const unsigned short port, const UPnP::Protocol protocol)
{
    auto it = uniqIDMap.find((port*(protocol+1)));
    if (it == uniqIDMap.end())
            return false;
    int ret = UPNP_DeletePinhole(urls6.controlURL,data6.first.servicetype, Util::toString(it->second).c_str());
    return ret == UPNPCOMMAND_SUCCESS;
}

string UPnPc6::getExternalIP()
{
    return Util::emptyString;
}

#endif
