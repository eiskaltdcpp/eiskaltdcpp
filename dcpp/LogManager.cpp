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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"

#include "LogManager.h"

#include "File.h"
#include "TimerManager.h"

namespace dcpp {

void LogManager::log(Area area, StringMap& params) noexcept {
    log(getPath(area, params), Util::formatParams(getSetting(area, FORMAT), params, false));
}

void LogManager::message(const string& msg) {
    if(BOOLSETTING(LOG_SYSTEM)) {
        StringMap params;
        params["message"] = msg;
        log(SYSTEM, params);
    }
    time_t t = GET_TIME();
    {
        Lock l(cs);
        // Keep the last 100 messages (completely arbitrary number...)
        while(lastLogs.size() > 100)
            lastLogs.pop_front();
        lastLogs.push_back(make_pair(t, msg));
    }
    fire(LogManagerListener::Message(), t, msg);
}

LogManager::List LogManager::getLastLogs() {
    Lock l(cs);
    return lastLogs;
}

string LogManager::getPath(Area area, StringMap& params) const {
    return SETTING(LOG_DIRECTORY) + Util::formatParams(getSetting(area, FILE), params, true);
}

string LogManager::getPath(Area area) const {
    StringMap params;
    return getPath(area, params);
}

const string& LogManager::getSetting(int area, int sel) const {
    return SettingsManager::getInstance()->get(static_cast<SettingsManager::StrSetting>(options[area][sel]), true);
}

void LogManager::saveSetting(int area, int sel, const string& setting) {
    SettingsManager::getInstance()->set(static_cast<SettingsManager::StrSetting>(options[area][sel]), setting);
}

void LogManager::log(const string& area, const string& msg) noexcept {
    Lock l(cs);
    try {
        string aArea = Util::validateFileName(area);
        File::ensureDirectory(aArea);
        File f(aArea, File::WRITE, File::OPEN | File::CREATE);
        f.setEndPos(0);
        f.write(msg + "\r\n");
    } catch (const FileException&) {
        // ...
    }
}

LogManager::LogManager() {
    options[UPLOAD][FILE]       = SettingsManager::LOG_FILE_UPLOAD;
    options[UPLOAD][FORMAT]     = SettingsManager::LOG_FORMAT_POST_UPLOAD;
    options[DOWNLOAD][FILE]     = SettingsManager::LOG_FILE_DOWNLOAD;
    options[DOWNLOAD][FORMAT]   = SettingsManager::LOG_FORMAT_POST_DOWNLOAD;
    options[FINISHED_DOWNLOAD][FILE] = SettingsManager::LOG_FILE_FINISHED_DOWNLOAD;
    options[FINISHED_DOWNLOAD][FORMAT] = SettingsManager::LOG_FORMAT_POST_FINISHED_DOWNLOAD;
    options[CHAT][FILE]     = SettingsManager::LOG_FILE_MAIN_CHAT;
    options[CHAT][FORMAT]       = SettingsManager::LOG_FORMAT_MAIN_CHAT;
    options[PM][FILE]       = SettingsManager::LOG_FILE_PRIVATE_CHAT;
    options[PM][FORMAT]     = SettingsManager::LOG_FORMAT_PRIVATE_CHAT;
    options[SYSTEM][FILE]       = SettingsManager::LOG_FILE_SYSTEM;
    options[SYSTEM][FORMAT]     = SettingsManager::LOG_FORMAT_SYSTEM;
    options[STATUS][FILE]       = SettingsManager::LOG_FILE_STATUS;
    options[STATUS][FORMAT]     = SettingsManager::LOG_FORMAT_STATUS;
}

LogManager::~LogManager() {
}

} // namespace dcpp
