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

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"
#include "Exception.h"

namespace dcpp {

STANDARD_EXCEPTION(SearchTypeException);

class SimpleXML;

class SettingsManagerListener {
public:
    virtual ~SettingsManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> Load;
    typedef X<1> Save;
    typedef X<2> SearchTypesChanged;

    virtual void on(Load, SimpleXML&) noexcept { }
    virtual void on(Save, SimpleXML&) noexcept { }
    virtual void on(SearchTypesChanged) noexcept { }
};

class SettingsManager : public Singleton<SettingsManager>, public Speaker<SettingsManagerListener>
{
public:
    typedef std::unordered_map<string, StringList> SearchTypes;
    typedef SearchTypes::iterator SearchTypesIter;
    typedef SearchTypes::const_iterator SearchTypesIterC;
    static StringList connectionSpeeds;

    enum StrSetting { STR_FIRST,
        NICK = STR_FIRST, UPLOAD_SPEED, DESCRIPTION, DOWNLOAD_DIRECTORY,
        EMAIL, EXTERNAL_IP, HUBLIST_SERVERS,
        HTTP_PROXY, LOG_DIRECTORY, LOG_FORMAT_POST_DOWNLOAD, LOG_FORMAT_POST_FINISHED_DOWNLOAD,
        LOG_FORMAT_POST_UPLOAD, LOG_FORMAT_MAIN_CHAT,
        LOG_FORMAT_PRIVATE_CHAT, TEMP_DOWNLOAD_DIRECTORY,
        BIND_ADDRESS, SOCKS_SERVER, SOCKS_USER, SOCKS_PASSWORD,
        CONFIG_VERSION, DEFAULT_AWAY_MESSAGE, TIME_STAMPS_FORMAT,
        PRIVATE_ID, LOG_FILE_MAIN_CHAT, LOG_FILE_PRIVATE_CHAT,
        LOG_FILE_STATUS, LOG_FILE_UPLOAD,
        LOG_FILE_DOWNLOAD, LOG_FILE_FINISHED_DOWNLOAD, LOG_FILE_SYSTEM, LOG_FORMAT_SYSTEM,
        LOG_FORMAT_STATUS, LOG_FILE_SPY, LOG_FORMAT_SPY, TLS_PRIVATE_KEY_FILE,
        TLS_CERTIFICATE_FILE, TLS_TRUSTED_CERTIFICATES_PATH,
        LANGUAGE, SKIPLIST_SHARE, INTERNETIP, BIND_IFACE_NAME,
        DHT_KEY, DYNDNS_SERVER, MIME_HANDLER,
        STR_LAST };

    enum IntSetting { INT_FIRST = STR_LAST + 1,
        INCOMING_CONNECTIONS = INT_FIRST, TCP_PORT, SLOTS,
        AUTO_FOLLOW, SHARE_HIDDEN, FILTER_MESSAGES,
        AUTO_SEARCH, AUTO_SEARCH_TIME,
        REPORT_ALTERNATES, TIME_STAMPS,
        IGNORE_HUB_PMS, IGNORE_BOT_PMS, LIST_DUPES, BUFFER_SIZE,
        DOWNLOAD_SLOTS, MAX_DOWNLOAD_SPEED, LOG_MAIN_CHAT,
        LOG_PRIVATE_CHAT, LOG_DOWNLOADS, LOG_FINISHED_DOWNLOADS,
        LOG_UPLOADS, MIN_UPLOAD_SPEED,
        AUTO_AWAY, SOCKS_PORT, SOCKS_RESOLVE,
        KEEP_LISTS, AUTO_KICK, COMPRESS_TRANSFERS,
        SFV_CHECK, MAX_COMPRESSION, NO_AWAYMSG_TO_BOTS, SKIP_ZERO_BYTE,
        ADLS_BREAK_ON_FIRST, HUB_USER_COMMANDS, AUTO_SEARCH_AUTO_MATCH,
        LOG_SYSTEM,
        LOG_FILELIST_TRANSFERS, SEND_UNKNOWN_COMMANDS, MAX_HASH_SPEED,
        GET_USER_COUNTRY, LOG_STATUS_MESSAGES,  SEARCH_PASSIVE,
        ADD_FINISHED_INSTANTLY, DONT_DL_ALREADY_SHARED, UDP_PORT,
        SHOW_LAST_LINES_LOG, ADC_DEBUG,
        SEARCH_HISTORY, SET_MINISLOT_SIZE, MAX_FILELIST_SIZE,
        PRIO_HIGHEST_SIZE, PRIO_HIGH_SIZE, PRIO_NORMAL_SIZE,
        PRIO_LOW_SIZE, PRIO_LOWEST, AUTODROP_SPEED, AUTODROP_INTERVAL,
        AUTODROP_ELAPSED, AUTODROP_INACTIVITY, AUTODROP_MINSOURCES,
        AUTODROP_FILESIZE, AUTODROP_ALL, AUTODROP_FILELISTS,
        AUTODROP_DISCONNECT, OUTGOING_CONNECTIONS, NO_IP_OVERRIDE,
        NO_USE_TEMP_DIR, SHARE_TEMP_FILES, SEARCH_ONLY_FREE_SLOTS,
        LAST_SEARCH_TYPE,
        SOCKET_IN_BUFFER, SOCKET_OUT_BUFFER,
        AUTO_REFRESH_TIME, HASHING_START_DELAY, USE_TLS, AUTO_SEARCH_LIMIT,
        AUTO_KICK_NO_FAVS, PROMPT_PASSWORD,
        DONT_DL_ALREADY_QUEUED,
        MAX_COMMAND_LENGTH, ALLOW_UNTRUSTED_HUBS, ALLOW_UNTRUSTED_CLIENTS,
        TLS_PORT, FAST_HASH, SEGMENTED_DL,
        FOLLOW_LINKS, SEND_BLOOM, CORAL,
        SEARCH_FILTER_SHARED, FINISHED_DL_ONLY_FULL,
        SEARCH_MERGE, HASH_BUFFER_SIZE_MB, HASH_BUFFER_POPULATE,
        HASH_BUFFER_NORESERVE, HASH_BUFFER_PRIVATE,
        USE_DHT, DHT_PORT,
        RECONNECT_DELAY, AUTO_DETECT_CONNECTION, BANDWIDTH_LIMIT_START,
        BANDWIDTH_LIMIT_END, THROTTLE_ENABLE, TIME_DEPENDENT_THROTTLE,
        MAX_DOWNLOAD_SPEED_ALTERNATE, MAX_UPLOAD_SPEED_ALTERNATE,
        MAX_DOWNLOAD_SPEED_MAIN, MAX_UPLOAD_SPEED_MAIN,
        SLOTS_ALTERNATE_LIMITING, SLOTS_PRIMARY, KEEP_FINISHED_FILES,
        SHOW_FREE_SLOTS_DESC, USE_IP, OVERLAP_CHUNKS, CASESENSITIVE_FILELIST,
        IPFILTER, TEXT_COLOR, USE_LUA, ALLOW_NATT, IP_TOS_VALUE, SEGMENT_SIZE,
        BIND_IFACE, MINIMUM_SEARCH_INTERVAL, DYNDNS_ENABLE, ALLOW_UPLOAD_MULTI_HUB,
        USE_ADL_ONLY_OWN_LIST, ALLOW_SIM_UPLOADS, CHECK_TARGETS_PATHS_ON_START,
        NMDC_DEBUG, SHARE_SKIP_ZERO_BYTE, REQUIRE_TLS, LOG_SPY,
        INT_LAST };

    enum Int64Setting { INT64_FIRST = INT_LAST + 1,
        TOTAL_UPLOAD = INT64_FIRST, TOTAL_DOWNLOAD,
        INT64_LAST };

    enum FloatSetting { FLOAT_FIRST = INT64_LAST +1,
        FLOAT_LAST, SETTINGS_LAST = FLOAT_LAST };
    enum {  INCOMING_DIRECT, INCOMING_FIREWALL_UPNP, INCOMING_FIREWALL_NAT,
        INCOMING_FIREWALL_PASSIVE };
    enum {  OUTGOING_DIRECT, OUTGOING_SOCKS5 };

    enum {  MAGNET_AUTO_SEARCH, MAGNET_AUTO_DOWNLOAD };

    const string& get(StrSetting key, bool useDefault = true) const {
        return (isSet[key] || !useDefault) ? strSettings[key - STR_FIRST] : strDefaults[key - STR_FIRST];
    }

    int get(IntSetting key, bool useDefault = true) const {
        return (isSet[key] || !useDefault) ? intSettings[key - INT_FIRST] : intDefaults[key - INT_FIRST];
    }
    int64_t get(Int64Setting key, bool useDefault = true) const {
        return (isSet[key] || !useDefault) ? int64Settings[key - INT64_FIRST] : int64Defaults[key - INT64_FIRST];
    }
    float get(FloatSetting key, bool useDefault = true) const {
        return (isSet[key] || !useDefault) ? floatSettings[key - FLOAT_FIRST] : floatDefaults[key - FLOAT_FIRST];
    }

    bool getBool(IntSetting key, bool useDefault = true) const {
        return (get(key, useDefault) != 0);
    }

    void set(StrSetting key, string const& value) {
        if(((key == DESCRIPTION) || (key == NICK)) && (value.size() > 35)) {
            strSettings[key - STR_FIRST] = value.substr(0, 35);
        } else {
            strSettings[key - STR_FIRST] = value;
        }
        isSet[key] = !value.empty();
    }

    void set(IntSetting key, int value) {
        if((key == SLOTS) && (value <= 0)) {
            value = 1;
        }
        intSettings[key - INT_FIRST] = value;
        isSet[key] = true;
    }

    void set(IntSetting key, const string& value) {
        if(value.empty()) {
            intSettings[key - INT_FIRST] = 0;
            isSet[key] = false;
        } else {
            intSettings[key - INT_FIRST] = Util::toInt(value);
            isSet[key] = true;
        }
    }

    void set(Int64Setting key, int64_t value) {
        int64Settings[key - INT64_FIRST] = value;
        isSet[key] = true;
    }

    void set(Int64Setting key, const string& value) {
        if(value.empty()) {
            int64Settings[key - INT64_FIRST] = 0;
            isSet[key] = false;
        } else {
            int64Settings[key - INT64_FIRST] = Util::toInt64(value);
            isSet[key] = true;
        }
    }

    void set(IntSetting key, bool value) { set(key, (int)value); }

    void set(FloatSetting key, float value) {
        floatSettings[key - FLOAT_FIRST] = value;
        isSet[key] = true;
    }
    void set(FloatSetting key, double value) {
        // yes, we loose precision here, but we're gonna loose even more when saving to the XML file...
        floatSettings[key - FLOAT_FIRST] = static_cast<float>(value);
        isSet[key] = true;
    }

    void setDefault(StrSetting key, string const& value) {
        strDefaults[key - STR_FIRST] = value;
    }

    void setDefault(IntSetting key, int value) {
        intDefaults[key - INT_FIRST] = value;
    }
    void setDefault(Int64Setting key, int64_t value) {
        int64Defaults[key - INT64_FIRST] = value;
    }
    void setDefault(FloatSetting key, float value) {
        floatDefaults[key - FLOAT_FIRST] = value;
    }

    bool isDefault(size_t key) { return !isSet[key]; }

    void unset(size_t key) { isSet[key] = false; }

    const std::string parseCoreCmd(const std::string& cmd);

    void load() {
        Util::migrate(getConfigFile());
        load(getConfigFile());
    }
    void save() {
        save(getConfigFile());
    }

    void load(const string& aFileName);
    void save(const string& aFileName);

    enum Types {
        TYPE_STRING,
        TYPE_INT,
        TYPE_INT64
    };

    bool getType(const char* name, int& n, int& type) const;
    // Search types
    void validateSearchTypeName(const string& name) const;
    void setSearchTypeDefaults();
    void addSearchType(const string& name, const StringList& extensions, bool validated = false);
    void delSearchType(const string& name);
    void renameSearchType(const string& oldName, const string& newName);
    void modSearchType(const string& name, const StringList& extensions);

    const SearchTypes& getSearchTypes() const {
        return searchTypes;
    }
    const StringList& getExtensions(const string& name);
private:
    friend class Singleton<SettingsManager>;
    SettingsManager();
    virtual ~SettingsManager() noexcept { }

    static const string settingTags[SETTINGS_LAST+1];

    string strSettings[STR_LAST - STR_FIRST];
    int    intSettings[INT_LAST - INT_FIRST];
    int64_t int64Settings[INT64_LAST - INT64_FIRST];
    float floatSettings[FLOAT_LAST - FLOAT_FIRST];

    string strDefaults[STR_LAST - STR_FIRST];
    int    intDefaults[INT_LAST - INT_FIRST];
    int64_t int64Defaults[INT64_LAST - INT64_FIRST];
    float floatDefaults[FLOAT_LAST - FLOAT_FIRST];

    bool isSet[SETTINGS_LAST];

    string getConfigFile() { return Util::getPath(Util::PATH_USER_CONFIG) + "DCPlusPlus.xml"; }

    // Search types
    SearchTypes searchTypes; // name, extlist

    SearchTypesIter getSearchType(const string& name);
};

// Shorthand accessor macros
#define SETTING(k) (SettingsManager::getInstance()->get(SettingsManager::k, true))
#define BOOLSETTING(k) (SettingsManager::getInstance()->getBool(SettingsManager::k, true))

} // namespace dcpp
