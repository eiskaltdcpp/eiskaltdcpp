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
        TYPE_AUDIO = 1,
        TYPE_COMPRESSED = 2,
        TYPE_DOCUMENT = 3,
        TYPE_EXECUTABLE = 4,
        TYPE_PICTURE = 5,
        TYPE_VIDEO = 6,
        TYPE_DIRECTORY = 7,
        TYPE_TTH = 8,
        TYPE_CD_IMAGE = 9,

        //TYPE_VIDEO_ALL = 101,
        TYPE_VIDEO_FOREIGN_FILMS = 102,
        TYPE_VIDEO_FOREIGN_SERIALS = 103,
        TYPE_VIDEO_FOREIGN_CARTOONS = 104,
        TYPE_VIDEO_OUR_FILMS = 105,
        TYPE_VIDEO_OUR_SERIALS = 106,
        TYPE_VIDEO_OUR_CARTOONS = 107,
        TYPE_VIDEO_TUTORIAL = 108,
        TYPE_VIDEO_CLIPS = 109,
        TYPE_VIDEO_CONCERTS = 110,
        TYPE_VIDEO_KARAOKE = 111,
        TYPE_VIDEO_HUMOR = 112,
        TYPE_VIDEO_ANIME = 113,
        TYPE_VIDEO_18 = 198,
        TYPE_VIDEO_OTHER = 199,

        //TYPE_AUDIO_ALL = 201,
        TYPE_AUDIO_ROCK = 202,
        TYPE_AUDIO_POP = 203,
        TYPE_AUDIO_JAZZ = 204,
        TYPE_AUDIO_BLUES = 205,
        TYPE_AUDIO_ELECTRONIC = 206,
        TYPE_AUDIO_SOUNDTRACKS = 207,
        TYPE_AUDIO_NOTES = 208,
        TYPE_AUDIO_AUDIOBOOKS = 209,
        TYPE_AUDIO_OTHER = 299,

        //TYPE_IMAGE_ALL = 301,
        TYPE_IMAGE_AVATARS = 302,
        TYPE_IMAGE_WALLPAPERS = 303,
        TYPE_IMAGE_ANIME = 304,
        TYPE_IMAGE_NATURE = 305,
        TYPE_IMAGE_ANIMALS = 306,
        TYPE_IMAGE_CARS = 307,
        TYPE_IMAGE_HUMOR = 308,
        TYPE_IMAGE_SPORT = 309,
        TYPE_IMAGE_SCREENSHOTS = 310,
        TYPE_IMAGE_ARTISTIC = 311,
        TYPE_IMAGE_ABSTRACT = 312,
        TYPE_IMAGE_18 = 398,
        TYPE_IMAGE_OTHER = 399,

        //TYPE_GAMES_ALL = 401,
        TYPE_GAMES_ACTION = 402,
        TYPE_GAMES_RPG = 403,
        TYPE_GAMES_STRATEGY = 404,
        TYPE_GAMES_SIM = 405,
        TYPE_GAMES_QUESTS = 406,
        TYPE_GAMES_ARCADE = 407,
        TYPE_GAMES_MINI = 408,
        TYPE_GAMES_CONSOLES = 409,
        TYPE_GAMES_LINUX = 410,
        TYPE_GAMES_MAC = 411,
        TYPE_GAMES_ADD_ON = 412,
        TYPE_GAMES_OTHER = 499,

        //TYPE_SOFT_ALL = 501,
        TYPE_SOFT_SYSTEM = 502,
        TYPE_SOFT_ANTI_VIR = 503,
        TYPE_SOFT_AUDIO = 504,
        TYPE_SOFT_VIDEO = 505,
        TYPE_SOFT_GRAPHIC = 506,
        TYPE_SOFT_OFFICE = 507,
        TYPE_SOFT_DRIVERS = 508,
        TYPE_SOFT_DEVELOP = 509,
        TYPE_SOFT_DIRECTORIES = 510,
        TYPE_SOFT_OTHER = 599,

        //TYPE_BOOK_ALL = 601,
        TYPE_BOOK_HUMOR = 602,
        TYPE_BOOK_MANGA = 603,
        TYPE_BOOK_TUTORIAL = 604,
        TYPE_BOOK_FEATURE = 605,
        TYPE_BOOK_TECHNIKAL = 606,
        TYPE_BOOK_OTHER = 699,

        TYPE_UP_OTHER = 998,
        //TYPE_UP_ALL = 999,
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

    void respond(const AdcCommand& cmd, const CID& cid,  bool isUdpActive, const string& hubIpPort);

    uint16_t getPort() const
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
