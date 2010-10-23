/*
* Copyright (C) 2010 cologic, ne5@parsoma.net
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

#if !defined(SCRIPT_MANAGER_H)
#define SCRIPT_MANAGER_H

#include <iostream>



#include "DCPlusPlus.h"
#include "Singleton.h"
#include "Socket.h"
#include "TimerManager.h"
#include "User.h"
#include "ClientManagerListener.h"
#include "CriticalSection.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace dcpp {

class ScriptManagerListener {
    public:
        typedef ScriptManagerListener* Ptr;
        typedef vector<Ptr> List;
        typedef List::iterator Iter;

        enum Types {
        DEBUG_MESSAGE,
        };
        virtual void onAction(Types, const string&) throw() = 0;
    };

struct LuaManager  {
    LuaManager(lua_State* /* L */) { }
    static int SendClientMessage(lua_State* L);
    static int SendHubMessage(lua_State* L);
    static int GenerateDebugMessage(lua_State* L);
    static int GetHubIpPort(lua_State* L);
    static int GetHubUrl(lua_State* L);
    static int GetClientIp(lua_State* L);
    static int SendUDPPacket(lua_State* L);

    static int InjectHubMessageADC(lua_State* L);
    static int InjectHubMessageNMDC(lua_State* L);

//  int FindWindow(lua_State *L);
//  int PostMessage(lua_State *L);
    static int DropUserConnection(lua_State* L);
//*/*/*/*/
    static int CreateClient(lua_State* L);
    static int DeleteClient(lua_State* L);
//**/*/*/*/*/**/
    static int RunTimer(lua_State* L);
/**/
    static int GetSetting(lua_State* L);
    static int GetAppPath(lua_State* L);
    static int GetConfigPath(lua_State* L);
/*Misc*/
    static int ToUtf8(lua_State* L);
    static int FromUtf8(lua_State* L);

};

class ScriptInstance {
        bool MakeCallRaw(const string& table, const string& method , int args, int ret) throw();
    protected:
        virtual ~ScriptInstance() { }
        static lua_State* L;
        static CriticalSection cs;
        template <typename T> bool MakeCall(const string& table, const string& method, int ret , const T& t) throw()
        {
        Lock l(cs);
        dcassert(lua_gettop(L) == 0);
        LuaPush(t);
        return MakeCallRaw(table, method, 1 , ret);
        }
        template <typename T,typename T2> bool MakeCall(const string& table, const string& method, int ret , const T& t, const T2& t2) throw()
        {
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

class ScriptManager: public ScriptInstance, public Singleton<ScriptManager>, public Speaker<ScriptManagerListener>,
        private ClientManagerListener, private TimerManagerListener
     {
        Socket s;//xx
        friend class Singleton<ScriptManager>;
        ScriptManager();
        virtual ~ScriptManager() throw() {
            shutdown();
        }
        void shutdown () {
            lua_close(L);

            if(timerEnabled)
                TimerManager::getInstance()->removeListener(this);
        }
    public:
        void load();
        void  SendDebugMessage(const string& s);
        GETSET(bool , timerEnabled, TimerEnabled);
    private:
        friend struct LuaManager;
        friend class ScriptInstance;

        virtual void on(ClientConnected, Client* aClient) throw();
        virtual void on(ClientDisconnected, Client* aClient) throw();
        virtual void on(Second, uint32_t /* ticks */) throw();


};

}//namespace dcpp
#endif







