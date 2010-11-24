/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_SEARCH_MANAGER_H
#define DCPLUSPLUS_DCPP_SEARCH_MANAGER_H

#include "SettingsManager.h"

#include "Socket.h"
#include "User.h"
#include "Thread.h"
#include "Client.h"
#include "Singleton.h"

#include "SearchManagerListener.h"
#include "TimerManager.h"
#include "AdcCommand.h"

namespace dcpp {

class SearchManager;
class SocketException;

class SearchManager : public Speaker<SearchManagerListener>, public Singleton<SearchManager>, public Thread
{
public:
    enum SizeModes {
        SIZE_DONTCARE = 0x00,
        SIZE_ATLEAST = 0x01,
        SIZE_ATMOST = 0x02
    };

    enum TypeModes {
        TYPE_ANY = 0,
        TYPE_AUDIO,
        TYPE_COMPRESSED,
        TYPE_DOCUMENT,
        TYPE_EXECUTABLE,
        TYPE_PICTURE,
        TYPE_VIDEO,
        TYPE_DIRECTORY,
                TYPE_TTH,
                TYPE_LAST
    };
private:
        static const char* types[TYPE_LAST];
public:
        static const char* getTypeStr(int type);

    void search(const string& aName, int64_t aSize, TypeModes aTypeMode, SizeModes aSizeMode, const string& aToken);
    void search(const string& aName, const string& aSize, TypeModes aTypeMode, SizeModes aSizeMode, const string& aToken) {
        search(aName, Util::toInt64(aSize), aTypeMode, aSizeMode, aToken);
    }

        void search(StringList& who, const string& aName, int64_t aSize, TypeModes aTypeMode, SizeModes aSizeMode, const string& aToken, const StringList& aExtList);
        void search(StringList& who, const string& aName, const string& aSize, TypeModes aTypeMode, SizeModes aSizeMode, const string& aToken, const StringList& aExtList) {
                search(who, aName, Util::toInt64(aSize), aTypeMode, aSizeMode, aToken, aExtList);
    }

    void respond(const AdcCommand& cmd, const CID& cid,  bool isUdpActive);

    uint16_t getPort()
    {
        return port;
    }

    void listen() throw(SocketException);
    void disconnect() throw();
    void onSearchResult(const string& aLine) {
        onData((const uint8_t*)aLine.data(), aLine.length(), Util::emptyString);
    }

    void onRES(const AdcCommand& cmd, const UserPtr& from, const string& removeIp = Util::emptyString);

    int32_t timeToSearch() {
        return 5 - (static_cast<int32_t>(GET_TICK() - lastSearch) / 1000);
    }

    bool okToSearch() {
        return timeToSearch() <= 0;
    }
        void onPSR(const AdcCommand& cmd, UserPtr from, const string& remoteIp = Util::emptyString);
        AdcCommand toPSR(bool wantResponse, const string& myNick, const string& hubIpPort, const string& tth, const vector<uint16_t>& partialInfo) const;
private:

    std::auto_ptr<Socket> socket;
    uint16_t port;
    bool stop;
    uint64_t lastSearch;
    friend class Singleton<SearchManager>;

    SearchManager();

    static std::string normalizeWhitespace(const std::string& aString);
    virtual int run();
        string getPartsString(const PartsInfo& partsInfo) const;

    virtual ~SearchManager() throw();
    void onData(const uint8_t* buf, size_t aLen, const string& address);
};

} // namespace dcpp

#endif // !defined(SEARCH_MANAGER_H)
