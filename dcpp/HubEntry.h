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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

#include "GetSet.h"

namespace dcpp {

class HubEntry {
public:
    HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) :
        name(aName), server(aServer), description(aDescription), country(Util::emptyString),
        rating(Util::emptyString), reliability(0.0), shared(0), minShare(0), users(Util::toInt(aUsers)), minSlots(0), maxHubs(0), maxUsers(0) { }

    HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers, const string& aCountry,
             const string& aShared, const string& aMinShare, const string& aMinSlots, const string& aMaxHubs, const string& aMaxUsers,
             const string& aReliability, const string& aRating) : name(aName), server(aServer), description(aDescription), country(aCountry),
        rating(aRating), reliability((float)(Util::toFloat(aReliability) / 100.0)), shared(Util::toInt64(aShared)), minShare(Util::toInt64(aMinShare)),
        users(Util::toInt(aUsers)), minSlots(Util::toInt(aMinSlots)), maxHubs(Util::toInt(aMaxHubs)), maxUsers(Util::toInt(aMaxUsers))
    {

    }

    HubEntry() = default;

    GETSET(string, name, Name);
    GETSET(string, server, Server);
    GETSET(string, description, Description);
    GETSET(string, country, Country);
    GETSET(string, rating, Rating);
    GETSET(float, reliability, Reliability);
    GETSET(int64_t, shared, Shared);
    GETSET(int64_t, minShare, MinShare);
    GETSET(int, users, Users);
    GETSET(int, minSlots, MinSlots);
    GETSET(int, maxHubs, MaxHubs);
    GETSET(int, maxUsers, MaxUsers);
};
const string DEF_FAKE_ID = "";

class FavoriteHubEntry {
public:
    FavoriteHubEntry() : encoding(Text::systemCharset), connect(false),
        mode(0), overrideId(0), clientId(DEF_FAKE_ID),
        useInternetIp(false), disableChat(false),
        searchInterval(SETTING(MINIMUM_SEARCH_INTERVAL))
    { }

    FavoriteHubEntry(const HubEntry& rhs) : name(rhs.getName()),
        server(rhs.getServer()),
        hubDescription(rhs.getDescription()), encoding(Text::systemCharset),
        connect(false), mode(0), overrideId(0),
        clientId(DEF_FAKE_ID), useInternetIp(false), disableChat(false),
        searchInterval(SETTING(MINIMUM_SEARCH_INTERVAL))
    { }

    FavoriteHubEntry(const FavoriteHubEntry& rhs) :
        userDescription(rhs.userDescription), name(rhs.getName()),
        server(rhs.getServer()), hubDescription(rhs.getHubDescription()),
        password(rhs.getPassword()), encoding(rhs.getEncoding()),
        connect(rhs.getConnect()), mode(rhs.mode),
        overrideId(rhs.overrideId), clientId(rhs.clientId),
        externalIP(""), useInternetIp(false), disableChat(false),
        searchInterval(rhs.searchInterval), nick(rhs.nick)
    { }

    const string& getNick(bool useDefault = true) const {
        return (!nick.empty() || !useDefault) ? nick : SETTING(NICK);
    }

    void setNick(const string& aNick) { nick = aNick; }

    GETSET(string, userDescription, UserDescription);
    GETSET(string, name, Name);
    GETSET(string, server, Server);
    GETSET(string, hubDescription, HubDescription);
    GETSET(string, password, Password);
    GETSET(string, encoding, Encoding);
    GETSET(bool, connect, Connect);
    GETSET(int, mode, Mode); // 0 = default, 1 = active, 2 = passive
    GETSET(bool, overrideId, OverrideId);
    GETSET(string, clientId, ClientId);
    GETSET(string, externalIP, ExternalIP);
    GETSET(bool, useInternetIp, UseInternetIP);
    GETSET(bool, disableChat, DisableChat);
    GETSET(string, group, Group);
    GETSET(uint32_t, searchInterval, SearchInterval);

private:
    string nick;
};

}
