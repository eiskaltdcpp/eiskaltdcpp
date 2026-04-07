/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#include "CriticalSection.h"
#include "SettingsManager.h"
#include "DCContext.h"
#include "Speaker.h"
#include "Util.h"

#include <string>
#include <unordered_map>

namespace dcpp {

using std::string;
using std::unordered_map;

class ConnectivityManagerListener {
public:
    virtual ~ConnectivityManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> Message;
    typedef X<1> Finished;

    virtual void on(Message, const string&) { }
    virtual void on(Finished) { }
};

class ConnectivityManager : public Speaker<ConnectivityManagerListener>, public ContextAware
{
public:
    void detectConnection();
    void setup(bool settingsChanged);
    bool isRunning() const { return running; }
    void updateLast();

public:
    explicit ConnectivityManager(DCContext& ctx);
    virtual ~ConnectivityManager() { }

private:
    friend class MappingManager;

    void mappingFinished(bool success);
    void log(const string& msg);

    void startSocket();
    void listen();
    void disconnect();

    bool autoDetected;
    bool running;

    string lastBind;
};

} // namespace dcpp
