/*
 * Copyright (C) 2009-2019 EiskaltDC++ developers
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

#include "DCContext.h"
#include "TimerManager.h"
#include "LogManager.h"
#include "SettingsManager.h"
#include "DCPlusPlus.h"

namespace dcpp {

class DebugManagerListener {
public:
    template<int I> struct X { enum { TYPE = I };  };

    typedef X<0> DebugCommand;
    typedef X<1> DebugDetection;

    virtual void on(DebugDetection, const string&) { }
    virtual void on(DebugCommand, const string&, int, const string&) { }
};

class DebugManager : public Speaker<DebugManagerListener>, public ContextAware {
public:
    void SendCommandMessage(const string& mess, int typeDir, const string& ip) {
        fire(DebugManagerListener::DebugCommand(), mess, typeDir, ip);
        if (CTX_BOOLSETTING(LOG_CMD_DEBUG)) {
            dcpp::StringMap params;
            params["cmd"] = mess;
            params["ip"] = ip;
            params["type"] = typeDirToString(typeDir);
            CTX_LOG(LogManager::CMD_DEBUG, params);
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

public:
    explicit DebugManager(DCContext& ctx) : ContextAware(ctx) { };
    virtual ~DebugManager() { };

private:
    static string typeDirToString(int typeDir);
};
// Context-aware versions — for use inside ContextAware member functions
#define CTX_COMMAND_DEBUG(a,b,c) if (this->ctx().getDebugManager()) this->ctx().getDebugManager()->SendCommandMessage(a,b,c);
#define CTX_DETECTION_DEBUG(m) if (this->ctx().getDebugManager()) this->ctx().getDebugManager()->SendDetectionMessage(m);

} // namespace dcpp
