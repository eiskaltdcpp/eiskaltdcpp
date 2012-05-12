/*
 * Copyright (C) 2008 cologic, cologic@parsoma.net
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

#include "Singleton.h"
#include "User.h"
#include "Socket.h"
#include "TimerManager.h"
#include "ClientManagerListener.h"
#include "CriticalSection.h"

#include "extra/lunar.h"

namespace dcpp {

class ScriptManagerListener {
    public:
        typedef ScriptManagerListener* Ptr;
        typedef vector<Ptr> List;
        typedef List::iterator Iter;

        enum Types {
        DEBUG_MESSAGE
        };
        virtual void onAction(Types, const string&) noexcept = 0;
    };

struct LuaManager  {
    static const char className[];
    static Lunar<LuaManager>::RegType methods[];

    LuaManager(lua_State* /* L */) { }
        int SendClientMessage(lua_State* L);
        int SendHubMessage(lua_State* L);
        int GenerateDebugMessage(lua_State* L);
        int GetHubIpPort(lua_State* L);
        int GetHubUrl(lua_State* L);
        int GetClientIp(lua_State* L);
        int SendUDPPacket(lua_State* L);
        int InjectHubMessageNMDC(lua_State* L);
        int InjectHubMessageADC(lua_State* L);
        //int FindWindow(lua_State* L);
        //int PostMessage(lua_State* L);
        int DropUserConnection(lua_State* L);

        int CreateClient(lua_State* L);
        int DeleteClient(lua_State* L);

        int RunTimer(lua_State* L);

        int GetSetting(lua_State* L);
        int GetAppPath(lua_State* L);
        int GetConfigPath(lua_State* L);
        int GetScriptsPath(lua_State* L);
        int GetConfigScriptsPath(lua_State* L);

        int ToUtf8(lua_State* L);
        int FromUtf8(lua_State* L);
};

class ScriptInstance {
        bool MakeCallRaw(const string& table, const string& method , int args, int ret) noexcept;
    protected:
        virtual ~ScriptInstance() { }
        static lua_State* L;
        static CriticalSection cs;

        template <typename T> bool MakeCall(const string& table, const string& method,
                int ret, const T& t) noexcept {
        Lock l(cs);
        dcassert(lua_gettop(L) == 0);
        LuaPush(t);
        return MakeCallRaw(table, method, 1 , ret);
        }
        template <typename T, typename T2> bool MakeCall(const string& table, const string& method,
                int ret, const T& t, const T2& t2) noexcept {
        Lock l(cs);
        dcassert(lua_gettop(L) == 0);
        LuaPush(t);
        LuaPush(t2);
        return MakeCallRaw(table, method, 2, ret);
        }
        template <typename T> void LuaPush(T* p) { lua_pushlightuserdata(L, p); }

        void LuaPush(int i);
        void LuaPush(const string& s);
        bool GetLuaBool();
        string GetClientType(Client* aClient);
    public:
        void EvaluateFile(const string& fn);
        void EvaluateChunk(const string& chunk);
};

class ScriptManager: public ScriptInstance, public Singleton<ScriptManager>, public Speaker<ScriptManagerListener>, private ClientManagerListener, private TimerManagerListener
{
    Socket s;

    friend class Singleton<ScriptManager>;
    ScriptManager();
    virtual ~ScriptManager() throw () { if (L) lua_close(L); if(timerEnabled) TimerManager::getInstance()->removeListener(this); }
public:
    void load();
    void  SendDebugMessage(const string& s);
    GETSET(bool , timerEnabled, TimerEnabled);
private:
    friend struct LuaManager;
    friend class ScriptInstance;

    virtual void on(ClientConnected, Client* aClient) noexcept;
    virtual void on(ClientDisconnected, Client* aClient) noexcept;
    virtual void on(Second, uint64_t /* ticks */) noexcept;


};

}//namespace dcpp


/**
 * @file ScriptManager.h
 * $Id: ScriptManager.h,v 1.2 2008/01/18 16:08:14 cologic Exp $
 */
