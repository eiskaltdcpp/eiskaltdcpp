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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "TimerManager.h"
#include "SettingsManager.h"
#include "forward.h"
#include "CriticalSection.h"
#include "Text.h"
#include "Client.h"
#include "BufferedSocket.h"
#include "ClientManager.h"

#ifdef LUA_SCRIPT
#include "ScriptManager.h"
#endif
namespace dcpp {

#ifdef LUA_SCRIPT
struct NmdcHubScriptInstance : public ScriptInstance {
    friend class ClientManager;
    bool onClientMessage(NmdcHub* aClient, const string& aLine);
};
#endif

class ClientManager;

class NmdcHub : public Client, private Flags
#ifdef LUA_SCRIPT
,public NmdcHubScriptInstance
#endif
{
public:
    using Client::send;
    using Client::connect;

    void onLine(const string& aLine) noexcept;
    virtual void connect(const OnlineUser& aUser, const string&);

    virtual void hubMessage(const string& aMessage, bool /*thirdPerson*/ = false);
    virtual void privateMessage(const OnlineUser& aUser, const string& aMessage, bool /*thirdPerson*/ = false);
    virtual void sendUserCmd(const UserCommand& command, const StringMap& params);
    virtual void search(int aSizeType, int64_t aSize, int aFileType, const string& aString, const string& aToken, const StringList& aExtList);
    virtual void password(const string& aPass) { send("$MyPass " + fromUtf8(aPass) + "|"); }
    virtual void info(bool force) { myInfo(force); }

    virtual size_t getUserCount() const { Lock l(cs); return users.size(); }
    virtual int64_t getAvailable() const;

    virtual string escape(string const& str) const { return validateMessage(str, false); }
    static string unescape(const string& str) { return validateMessage(str, true); }

    virtual void send(const AdcCommand&) { dcassert(0); }

    static string validateMessage(string tmp, bool reverse);

private:
friend class ClientManager;
    enum SupportFlags {
        SUPPORTS_USERCOMMAND = 0x01,
        SUPPORTS_NOGETINFO = 0x02,
        SUPPORTS_USERIP2 = 0x04
    };

    mutable CriticalSection cs;

    typedef unordered_map<string, OnlineUser*, CaseStringHash, CaseStringEq> NickMap;
    typedef NickMap::iterator NickIter;

    NickMap users;

    int supportFlags;

    uint64_t lastUpdate;
    string lastMyInfoA;
    string lastMyInfoB;
    string lastMyInfoC;
    string lastMyInfoD;

    typedef list<pair<string, uint32_t> > FloodMap;
    typedef FloodMap::iterator FloodIter;
    FloodMap seekers;
    FloodMap flooders;

    NmdcHub(const string& aHubURL, bool secure);
    virtual ~NmdcHub();

    // Dummy
    NmdcHub(const NmdcHub&);
    NmdcHub& operator=(const NmdcHub&);

    void clearUsers();

    OnlineUser& getUser(const string& aNick);
    OnlineUser* findUser(const string& aNick);
    void putUser(const string& aNick);

    string toUtf8(const string& str) const { return Text::validateUtf8(str) ? str : Text::toUtf8(str, getEncoding()); }
    string fromUtf8(const string& str) const { return Text::fromUtf8(str, getEncoding()); }
    void privateMessage(const string& nick, const string& aMessage);
    void validateNick(const string& aNick) { send("$ValidateNick " + fromUtf8(aNick) + "|"); }
    void key(const string& aKey) { send("$Key " + aKey + "|"); }
    void version() { send("$Version 1,0091|"); }
    void getNickList() { send("$GetNickList|"); }
    void connectToMe(const OnlineUser& aUser);
    void revConnectToMe(const OnlineUser& aUser);
    void myInfo(bool alwaysSend);
    void supports(const StringList& feat);
    void clearFlooders(uint64_t tick);

    void updateFromTag(Identity& id, const string& tag);

    virtual string checkNick(const string& aNick);

    // TimerManagerListener
    virtual void on(Second, uint64_t aTick) noexcept;

    virtual void on(Connected) noexcept;
    virtual void on(Line, const string& l) noexcept;
    virtual void on(Failed, const string&) noexcept;

};

} // namespace dcpp
