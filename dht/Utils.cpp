/*
 * Copyright (C) 2008 Big Muscle, http://strongdc.sf.net
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

#include "stdafx.h"
#include "Utils.h"

#include "../dcpp/CID.h"

namespace dht
{

    CID Utils::getDistance(const CID& cid1, const CID& cid2)
    {
        uint8_t distance[CID::SIZE];

        for(int i = 0; i < CID::SIZE; i++)
        {
            distance[i] = cid1.data()[i] ^ cid2.data()[i];
        }

        return CID(distance);
    }

    /*
     * Detect whether it is correct to use IP:port in DHT network
     */
    bool Utils::isGoodIPPort(const string& ip, uint16_t port)
    {
        // don't allow empty IP and port lower than 1024
        // ports below 1024 are known service ports, so they shouldn't be used for DHT else it could be used for attacks
        if(ip.empty() || port < 1024)
            return false;

        // don't allow private IPs
        if(Util::isPrivateIp(ip))
            return false;

        return true;
    }

} // namespace dht
