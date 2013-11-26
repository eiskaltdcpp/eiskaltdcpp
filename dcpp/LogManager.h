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

#include <deque>
#include <utility>
#include "typedefs.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "Speaker.h"
#include "LogManagerListener.h"

namespace dcpp {

using std::deque;
using std::pair;

class LogManager : public Singleton<LogManager>, public Speaker<LogManagerListener>
{
public:
    typedef pair<time_t, string> Pair;
    typedef deque<Pair> List;

    enum Area { CHAT, PM, DOWNLOAD, FINISHED_DOWNLOAD, UPLOAD, SYSTEM, STATUS, SPY, LAST };
    enum { FILE, FORMAT };

    void log(Area area, StringMap& params) noexcept;
    void message(const string& msg);

    List getLastLogs();
    string getPath(Area area, StringMap& params) const;
    string getPath(Area area) const;

    const string& getSetting(int area, int sel) const;
    void saveSetting(int area, int sel, const string& setting);

private:
    void log(const string& area, const string& msg) noexcept;

    friend class Singleton<LogManager>;
    CriticalSection cs;
    List lastLogs;

    int options[LAST][2];

    LogManager();
    virtual ~LogManager();
};

#define LOG(area, msg) LogManager::getInstance()->log(area, msg)

} // namespace dcpp
