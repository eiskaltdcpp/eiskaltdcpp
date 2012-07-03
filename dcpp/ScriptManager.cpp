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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"

#include "ScriptManager.h"
#include "Util.h"
#include "StringTokenizer.h"
#include "Client.h"
#include "UserConnection.h"
#include "ClientManager.h"
#include "DownloadManager.h"
#include "LogManager.h"
#include "NmdcHub.h"
#include "AdcHub.h"
#include "Thread.h"
#include <cstddef>

namespace dcpp {

static void callalert (lua_State *L, int status) {
    if (status != 0) {
        lua_getglobal(L, "_ALERT");
        if (lua_isfunction(L, -1)) {
            lua_insert(L, -2);
            lua_call(L, 1, 0);
        }
        else {  /* no _ALERT function; print it on stderr */
            ScriptManager::getInstance()->SendDebugMessage(Text::acpToUtf8(string("LUA ERROR: ") + lua_tostring(L, -2)));
            lua_pop(L, 2);  /* remove error message and _ALERT */
        }
    }
}


static int aux_do (lua_State *L, int status) {
  if (status == 0) {  /* parse OK? */
    status = lua_pcall(L, 0, LUA_MULTRET, 0);  /* call main */
  }
  callalert(L, status);
  return status;
}

LUALIB_API int lua_dofile (lua_State *L, const char *filename) {
  return aux_do(L, luaL_loadfile(L, filename));
}

LUALIB_API int lua_dobuffer (lua_State *L, const char *buff, size_t size, const char *name) {
  return aux_do(L, luaL_loadbuffer(L, buff, size, name));
}

LUALIB_API int lua_dostring (lua_State *L, const char *str) {
  return lua_dobuffer(L, str, strlen(str), str);
}

const char LuaManager::className[] = "DC";
Lunar<LuaManager>::RegType LuaManager::methods[] = {
        {"SendHubMessage", &LuaManager::SendHubMessage },
        {"SendClientMessage", &LuaManager::SendClientMessage },
        {"SendUDP", &LuaManager::SendUDPPacket},
        {"PrintDebug", &LuaManager::GenerateDebugMessage},
        {"GetClientIp", &LuaManager::GetClientIp},
        {"GetHubIpPort", &LuaManager::GetHubIpPort},
        {"GetHubUrl", &LuaManager::GetHubUrl},
        {"InjectHubMessage", &LuaManager::InjectHubMessageNMDC},
        {"InjectHubMessageADC", &LuaManager::InjectHubMessageADC},
        //{"FindWindowHandle", &LuaManager::FindWindow},
        //{"SendWindowMessage", &LuaManager::PostMessage},
        {"CreateClient", &LuaManager::CreateClient},
        {"DeleteClient", &LuaManager::DeleteClient},
        {"RunTimer", &LuaManager::RunTimer},
        {"GetSetting", &LuaManager::GetSetting},
        {"ToUtf8", &LuaManager::ToUtf8},
        {"FromUtf8", &LuaManager::FromUtf8},
        {"GetAppPath", &LuaManager::GetAppPath},
        {"GetConfigPath", &LuaManager::GetConfigPath},
        {"GetScriptsPath", &LuaManager::GetScriptsPath},
        {"GetConfigScriptsPath", &LuaManager::GetConfigScriptsPath},
        {"DropUserConnection", &LuaManager::DropUserConnection},
        {0}
};

int LuaManager::DeleteClient(lua_State* L){
    if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)){
        Client* client = (Client*) lua_touserdata(L, -1);
        ClientManager::getInstance()->putClient(client);
    }
    return 0;
}

int LuaManager::CreateClient(lua_State* L) {
    if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)){
        Client* client = ClientManager::getInstance()->getClient(lua_tostring(L, -2));
        Identity ident;
        ident.setNick(lua_tostring(L, -1));
        client->setMyIdentity(ident);
        client->setPassword("");
        //this will block?
        client->connect();

        lua_pushlightuserdata(L, client);
        return 1;
    }

    return 0;
}

int LuaManager::InjectHubMessageNMDC(lua_State* L) {
    if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1))
        reinterpret_cast<NmdcHub *>(lua_touserdata(L, -2))->onLine(lua_tostring(L, -1));

    return 0;
}

int LuaManager::InjectHubMessageADC(lua_State* L) {
    if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1))
        reinterpret_cast<AdcHub *>(lua_touserdata(L, -2))->dispatch(lua_tostring(L, -1));

    return 0;
}

//int LuaManager::PostMessage(lua_State* L) {
    //if (lua_gettop(L) == 4 && lua_islightuserdata(L, -4) && lua_isnumber(L, -3) &&
            //lua_islightuserdata(L, -2) && lua_islightuserdata(L, -1)) {
        //::SendMessage(reinterpret_cast<HWND>(lua_touserdata(L, -4)), static_cast<UINT>(lua_tonumber(L, -3)),
            //reinterpret_cast<WPARAM>(lua_touserdata(L, -2)), reinterpret_cast<LPARAM>(lua_touserdata(L, -1)));
    //}

    //return 0;
//}

//int LuaManager::FindWindow(lua_State* L) {
    //if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
        //lua_pushlightuserdata(L, ::FindWindow(Text::toT(string(lua_tostring(L, -2))).c_str(), Text::toT(string(lua_tostring(L, -1))).c_str()));
        //return 1;
    //}

    //return 0;
//}

int LuaManager::SendClientMessage(lua_State* L) {
    if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
        reinterpret_cast<UserConnection *>(lua_touserdata(L, -2))->sendRaw(lua_tostring(L, -1));
    }

    return 0;
}

int LuaManager::SendHubMessage(lua_State* L) {
    if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
        reinterpret_cast<Client*>(lua_touserdata(L, -2))->send(lua_tostring(L, -1));
    }

    return 0;
}

int LuaManager::GenerateDebugMessage(lua_State* L) {
    /* arguments: socket, buffer, address */
    if (lua_gettop(L) == 1 && lua_isstring(L, -1))
        ScriptManager::getInstance()->SendDebugMessage(lua_tostring(L, -1));

    return 0;
}

int LuaManager::SendUDPPacket(lua_State* L) {
    /* arguments: ip:port, data */
    if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
        StringList sl = StringTokenizer<string>(lua_tostring(L, -2), ':').getTokens();
        ScriptManager::getInstance()->s.writeTo(sl[0], static_cast<short>(Util::toInt(sl[1])), lua_tostring(L, -1), lua_strlen(L, -1));
    }

    return 0;
}

int LuaManager::DropUserConnection(lua_State* L) {
    /* arguments: userconnection to drop */
    if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
        reinterpret_cast<UserConnection *>(lua_touserdata(L, -1))->disconnect();
    }

    return 0;
}

int LuaManager::GetSetting(lua_State* L) {
    /* arguments: string */
    int n, type;
    if(lua_gettop(L) == 1 && lua_isstring(L, -1) && SettingsManager::getInstance()->getType(lua_tostring(L, -1), n, type)) {
        if(type == SettingsManager::TYPE_STRING) {
            lua_pushstring(L, SettingsManager::getInstance()->get((SettingsManager::StrSetting)n).c_str());
            return 1;
        } else if(type == SettingsManager::TYPE_INT) {
            lua_pushnumber(L, SettingsManager::getInstance()->get((SettingsManager::IntSetting)n));
            return 1;
        } else if(type == SettingsManager::TYPE_INT64) {
            lua_pushnumber(L, static_cast<lua_Number>(SettingsManager::getInstance()->get((SettingsManager::Int64Setting)n)));
            return 1;
        }
    }
    lua_pushliteral(L, "GetSetting: setting not found");
    lua_error(L);
    return 0;
}

int LuaManager::ToUtf8(lua_State* L) {
    /* arguments: string */
    if(lua_gettop(L) == 1 && lua_isstring(L, -1) ) {
        lua_pushstring(L, Text::acpToUtf8(lua_tostring(L, -1)).c_str());
        return 1;
    } else {
        lua_pushliteral(L, "ToUtf8: string needed as argument");
        lua_error(L);
    }
    return 0;
}

int LuaManager::FromUtf8(lua_State* L) {
    /* arguments: string */
    if(lua_gettop(L) == 1 && lua_isstring(L, -1) ) {
        lua_pushstring(L, Text::utf8ToAcp(lua_tostring(L, -1)).c_str());
        return 1;
    } else {
        lua_pushliteral(L, "FromUtf8: string needed as argument");
        lua_error(L);
    }
    return 0;
}

int LuaManager::GetAppPath(lua_State* L) {
    lua_pushstring(L, Text::utf8ToAcp(Util::getPath(Util::PATH_RESOURCES)).c_str());
    return 1;
}

int LuaManager::GetConfigPath(lua_State* L) {
    lua_pushstring(L, (Text::utf8ToAcp(Util::getPath(Util::PATH_USER_CONFIG)) + PATH_SEPARATOR).c_str());
    return 1;
}

int LuaManager::GetConfigScriptsPath(lua_State* L) {
    lua_pushstring(L, (Text::utf8ToAcp(Util::getPath(Util::PATH_USER_CONFIG)) + PATH_SEPARATOR).c_str());
    return 1;
}

int LuaManager::GetScriptsPath(lua_State* L) {
    string scripts_path;
    scripts_path = Text::utf8ToAcp(Util::getPath(Util::PATH_USER_CONFIG)) + "luascripts" + PATH_SEPARATOR;

    if(!Util::fileExists(scripts_path)) {
#ifdef WIN32
        scripts_path = Text::utf8ToAcp(Util::getPath(Util::PATH_GLOBAL_CONFIG)) + "resources" + PATH_SEPARATOR + "luascripts" + PATH_SEPARATOR;
#else //WIN32
        scripts_path = string(_DATADIR) + PATH_SEPARATOR + "luascripts" + PATH_SEPARATOR;
#endif //WIN32
    }
    lua_pushstring(L, scripts_path.c_str());
    return 1;
}

int LuaManager::GetClientIp(lua_State* L) {
    /* arguments: client */
    UserConnection* uc = (UserConnection*)lua_touserdata(L, 1);
    if(uc == NULL) {
        lua_pushliteral(L, "GetClientIpPort: missing client pointer");
        lua_error(L);
        return 0;
    }
    lua_pushstring(L, uc->getRemoteIp().c_str());
    return 1;
}

int LuaManager::GetHubIpPort(lua_State* L) {
    /* arguments: client */
    Client* c = (Client*)lua_touserdata(L, 1);
    if(c == NULL) {
        lua_pushliteral(L, "GetHubIpPort: missing hub pointer");
        lua_error(L);
        return 0;
    }
    lua_pushstring(L, c->getIpPort().c_str());
    return 1;
}

int LuaManager::GetHubUrl(lua_State* L) {
    /* arguments: client */
    Client* c = (Client*)lua_touserdata(L, 1);
    if(c == NULL) {
        lua_pushliteral(L, "GetHubUrl: missing hub pointer");
        lua_error(L);
        return 0;
    }
    lua_pushstring(L, c->getHubUrl().c_str());
    return 1;
}

int LuaManager::RunTimer(lua_State* L) {
    /* arguments: bool:on/off */
    if(lua_gettop(L) == 1 && lua_isnumber(L, -1)) {
                bool on = lua_tonumber(L, 1) != 0;      //shut VC++ up
        ScriptManager* sm = ScriptManager::getInstance();
        if(on != sm->getTimerEnabled()) {
            if(on)
                TimerManager::getInstance()->addListener(sm);
            else
                TimerManager::getInstance()->removeListener(sm);
            sm->setTimerEnabled(on);
        }
    } else {
        lua_pushliteral(L, "RunTimer: missing integer (0=off,!0=on)");
        lua_error(L);
        return 0;
    }
    return 1;
}

lua_State* ScriptInstance::L = 0;       //filled in by scriptmanager.
CriticalSection ScriptInstance::cs;

ScriptManager::ScriptManager() : timerEnabled(false) {
}

void ScriptManager::load() {
    L = lua_open();
    luaL_openlibs(L);

    Lunar<LuaManager>::Register(L);

    //create default text formatting function, in case startup.lua or formatting.lua isn't present.
    uint32_t color = SETTING(TEXT_COLOR);
    //this create a dcpp namespace. However, if startup.lua executes, if first clobbers this.
    string function =
            "dcpp = {_init_me_anyway = true}\n"
            "function dcpp.FormatChatText(hub, text)\n"
            "   text = string.gsub(text, \"([{}\\\\])\", \"\\%1\")\n"
            "   text = string.gsub(text, \"\\n\", \"\\\\line\\n\")\n"
            "text = string.gsub( text, \".\", function( c )\n"
            "   if string.byte( c ) >= 128 then\n"
            "       return string.format( \"\\\\'%02x\", string.byte( c ) )\n"
            "   else\n"
            "       return c\n"
            "   end\n"
            "end )"
            "   return \"{\\\\urtf1\\n\"..\n"
            "           \"{\\\\colortbl ;"
                    "\\\\red" + Util::toString(color & 0xFF) +
                    "\\\\green" + Util::toString((color >> 8) & 0xFF) +
                    "\\\\blue" + Util::toString((color >> 16) & 0xFF) +
                    ";}\\n\"..\n"
            "           \"\\\\cf1 \"..text..\"}\\n\"\n"
            "end\n";


    lua_dostring(L, function.c_str());


    lua_pop(L, lua_gettop(L));      //hm. starts at 8 or so for me. I have no idea why...

    s.create(Socket::TYPE_UDP);

    ClientManager::getInstance()->addListener(this);
}

void ScriptInstance::EvaluateChunk(const string& chunk) {
    Lock l(cs);
    lua_dostring(L, chunk.c_str());
}

void ScriptInstance::EvaluateFile(const string& fn) {
    Lock l(cs);
    string script_full_name;
    if(Util::fileExists(fn)) {
        script_full_name = fn;
    } else {
        string test_path_0;
        string test_path_1;
#ifdef WIN32
        test_path_0 = Text::utf8ToAcp(Util::getPath(Util::PATH_USER_CONFIG)) + "luascripts" + PATH_SEPARATOR + fn;
        test_path_1 = Text::utf8ToAcp(Util::getPath(Util::PATH_GLOBAL_CONFIG)) + "resources" + PATH_SEPARATOR + "luascripts" + PATH_SEPARATOR + fn;

        if(Util::fileExists(test_path_0))
            script_full_name = test_path_0;
        else if(Util::fileExists(test_path_1))
            script_full_name = test_path_1;
        else {
            LogManager::getInstance()->message("File '" + fn + "' not found!");
            dcdebug("File '%s' not found!",fn.c_str()); // temporary
            return;
        }
#else //WIN32
        test_path_0 = Text::utf8ToAcp(Util::getPath(Util::PATH_USER_CONFIG)) + "luascripts" + PATH_SEPARATOR + fn;
        test_path_1 = string(_DATADIR) + PATH_SEPARATOR + "luascripts" + PATH_SEPARATOR + fn;

        if(Util::fileExists(test_path_0))
            script_full_name = test_path_0;
        else if(Util::fileExists(test_path_1))
            script_full_name = test_path_1;
        else {
            LogManager::getInstance()->message("File '" + fn + "' not found!");
            printf("File '%s' not found!",fn.c_str()); fflush(stdout);// temporary
            return;
        }
#endif //WIN32
    }
    lua_dofile(L, script_full_name.c_str());
}

void ScriptManager::SendDebugMessage(const string &mess) {
    //LogManager::getInstance()->message(mess);
    dcdebug("%s\n", mess.c_str()); // temporary
}

bool ScriptInstance::GetLuaBool() {
    //get value from top of stack, check if should cancel message.
    bool ret = false;
    if (lua_gettop(L) > 0) {
        ret = !lua_isnil(L, -1);
        lua_pop(L, 1);
    }
    return ret;
}

string ScriptInstance::GetClientType(Client* aClient) {
    return dynamic_cast<AdcHub *>(aClient)?"adch":"nmdch";
}

void ScriptManager::on(ClientDisconnected, Client* aClient) noexcept {
    MakeCall(GetClientType(aClient), "OnHubRemoved", 0, aClient);
}

void ScriptManager::on(ClientConnected, Client* aClient) noexcept {
    MakeCall(GetClientType(aClient), "OnHubAdded", 0, aClient);
}

void ScriptManager::on(Second, uint64_t /* ticks */) noexcept {
    MakeCall("dcpp", "OnTimer", 0, 0);
}

void ScriptInstance::LuaPush(int i) { lua_pushnumber(L, i); }
void ScriptInstance::LuaPush(const string& s) { lua_pushlstring(L, s.data(), s.size()); }

bool ScriptInstance::MakeCallRaw(const string& table, const string& method, int args, int ret) noexcept {
    lua_getglobal(L, table.c_str());        // args + 1
    lua_pushstring(L, method.c_str());      // args + 2
    if (lua_istable(L, -2)) {
        lua_gettable(L, -2);                // args + 2
        lua_remove(L, -2);                  // args + 1
        lua_insert(L, 1);                   // args + 1
        if(lua_pcall(L, args, ret, 0) == 0) {
            dcassert(lua_gettop(L) == ret);
            return true;
        }
        const char *msg = lua_tostring(L, -1);
        string formatted_msg = (msg != NULL)?string("LUA Error: ") + msg:string("LUA Error: (unknown)");
        ScriptManager::getInstance()->SendDebugMessage(formatted_msg);
        dcassert(lua_gettop(L) == 1);
        lua_pop(L, 1);
    } else {
        lua_settop(L, 0);
    }
    return false;
}

} // namespace dcpp

/**
 * @file ScriptManager.cpp
 * $Id: ScriptManager.cpp,v 1.4 2010/01/01 04:37:23 cologic Exp $
 */
