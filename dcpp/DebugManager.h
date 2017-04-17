/*
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
#include "TimerManager.h"
#include "LogManager.h"
#include "SettingsManager.h"

namespace dcpp {

class DebugManagerListener {
public:
template<int I> struct X { enum { TYPE = I };  };

    typedef X<0> DebugCommand;
    typedef X<1> DebugDetection;

    virtual void on(DebugDetection, const string&) noexcept { }
    virtual void on(DebugCommand, const string&, int, const string&) noexcept { }
};

class DebugManager : public Singleton<DebugManager>, public Speaker<DebugManagerListener> {
public:
    void SendCommandMessage(const string& mess, int typeDir, const string& ip) {
        fire(DebugManagerListener::DebugCommand(), mess, typeDir, ip);
        if (BOOLSETTING(LOG_CMD_DEBUG)) {
            dcpp::StringMap params;
            params["cmd"] = mess;
            params["ip"] = ip;
            params["type"] = typeDirToString(typeDir);
            LOG(LogManager::CMD_DEBUG, params);
        }
    }
    void SendDetectionMessage(const string& mess) {
        fire(DebugManagerListener::DebugDetection(), mess);
    }
    enum {
        HUB_IN, HUB_OUT, CLIENT_IN, CLIENT_OUT
#ifdef WITH_DHT
, DHT_IN, DHT_OUT
#endif
    };

private:
    friend class Singleton<DebugManager>;
    DebugManager() noexcept { };
    ~DebugManager() noexcept { };
    static string typeDirToString(int typeDir);
};
#define COMMAND_DEBUG(a,b,c) if (DebugManager::getInstance()) DebugManager::getInstance()->SendCommandMessage(a,b,c);
#define DETECTION_DEBUG(m) if (DebugManager::getInstance()) DebugManager::getInstance()->SendDetectionMessage(m);

} // namespace dcpp
