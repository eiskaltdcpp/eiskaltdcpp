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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdafx.h"
#include "Utils.h"

#include "Constants.h"

#include "dcpp/AdcCommand.h"
#include "dcpp/CID.h"
#include "dcpp/MerkleTree.h"
#include "dcpp/TimerManager.h"
#include "dcpp/SettingsManager.h"

namespace dht
{

	CriticalSection Utils::cs;
	std::unordered_map< string, std::unordered_multiset<uint32_t> > Utils::receivedPackets;
	std::list<Utils::OutPacket> Utils::sentPackets;

	CID Utils::getDistance(const CID& cid1, const CID& cid2)
	{
		uint8_t distance[CID::SIZE];

		for(int i = 0; i < CID::SIZE; ++i)
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

	/*
	 * General flooding protection
	 */
	bool Utils::checkFlood(const string& ip, const AdcCommand& cmd)
	{
		// ignore empty commands
		if(cmd.getParameters().empty())
			return false;

		// there maximum allowed request packets from one IP per minute
		// response packets are allowed only if request has been sent to the IP address
		size_t maxAllowedPacketsPerMinute = 0;
		uint32_t requestCmd = AdcCommand::CMD_SCH;
		switch(cmd.getCommand())
		{
			// request packets
			case AdcCommand::CMD_SCH: maxAllowedPacketsPerMinute = 20; break;
			case AdcCommand::CMD_PUB: maxAllowedPacketsPerMinute = 10; break;
			case AdcCommand::CMD_INF: maxAllowedPacketsPerMinute = 3; break;
			case AdcCommand::CMD_CTM: maxAllowedPacketsPerMinute = 2; break;
			case AdcCommand::CMD_RCM: maxAllowedPacketsPerMinute = 2; break;
			case AdcCommand::CMD_GET: maxAllowedPacketsPerMinute = 2; break;
			case AdcCommand::CMD_PSR: maxAllowedPacketsPerMinute = 3; break;

			// response packets
			case AdcCommand::CMD_STA:
				return true; // STA can be response for more commands, but since it is for informative purposes only, there shouldn't be no way to abuse it

			case AdcCommand::CMD_SND: requestCmd = AdcCommand::CMD_GET;
			case AdcCommand::CMD_RES: // default value of requestCmd

				Lock l(cs);
				for(std::list<OutPacket>::iterator i = sentPackets.begin(); i != sentPackets.end(); ++i)
				{
					if(i->cmd == requestCmd && i->ip == ip)
					{
						sentPackets.erase(i);
						return true;
					}
				}

				dcdebug("Received unwanted response from %s. Packet dropped.\n", ip.c_str());
				return false;
		}

		Lock l(cs);
		std::unordered_multiset<uint32_t>& packetsPerIp = receivedPackets[ip];
                packetsPerIp.insert(cmd.getCommand());

		if(packetsPerIp.count(cmd.getCommand()) > maxAllowedPacketsPerMinute)
		{
                        dcdebug("Request flood detected (%li) from %s. Packet dropped.\n", (long int)packetsPerIp.count(cmd.getCommand()), ip.c_str());
			return false;
		}

		return true;
	}

	/*
	 * Removes tracked packets. Called once a minute.
	 */
	void Utils::cleanFlood()
	{
		Lock l(cs);
		receivedPackets.clear();
	}

	/*
	 * Stores outgoing request to avoid receiving invalid responses
	 */
	void Utils::trackOutgoingPacket(const string& ip, const AdcCommand& cmd)
	{
		Lock l(cs);

		uint64_t now = GET_TICK();
		switch(cmd.getCommand())
		{
			// request packets
			case AdcCommand::CMD_SCH:
			case AdcCommand::CMD_PUB:
			case AdcCommand::CMD_INF:
			case AdcCommand::CMD_CTM:
			case AdcCommand::CMD_GET:
			case AdcCommand::CMD_PSR:
				OutPacket p = { ip, now, cmd.getCommand() };
				sentPackets.push_back(p);
				break;
		}

		// clean up old items
		// list is sorted by time, so the first unmatched item can break the loop
		while(!sentPackets.empty())
		{
			uint64_t diff = now - sentPackets.front().time;
			if(diff >= TIME_FOR_RESPONSE)
				sentPackets.pop_front();
			else
				break;
		}
	}

	/*
	 * Generates UDP key for specified IP address
	 */
	CID Utils::getUdpKey(const string& targetIp)
	{
		CID myUdpKey = CID(SETTING(DHT_KEY));

		TigerTree th;
		th.update(myUdpKey.data(), sizeof(CID));
		th.update(targetIp.c_str(), targetIp.size());
		return CID(th.finalize());
	}

	bool IsInvalid(char ch) { return ch == '\r' || ch == '\n' || ch == '\t'; }
	const string& Utils::compressXML(string& xml)
	{
		xml.erase(std::remove_if(xml.begin(), xml.end(), IsInvalid), xml.end());
		return xml;
	}

} // namespace dht
