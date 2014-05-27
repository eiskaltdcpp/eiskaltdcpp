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

#include "stdinc.h"

#include "SettingsManager.h"

#include "SimpleXML.h"
#include "Util.h"
#include "File.h"
#include "version.h"
#include "AdcHub.h"
#include "CID.h"
#ifdef WITH_DHT
#include "dht/DHT.h"
#endif
#include "SearchManager.h"
#include "StringTokenizer.h"

namespace dcpp {

StringList SettingsManager::connectionSpeeds;

const string SettingsManager::settingTags[] =
{
    // Strings
    "Nick", "UploadSpeed", "Description", "DownloadDirectory", "EMail",
    "ExternalIp", "HublistServers", "HttpProxy",
    "LogDirectory", "LogFormatPostDownload","LogFormatPostFinishedDownload",
    "LogFormatPostUpload", "LogFormatMainChat", "LogFormatPrivateChat",
    "TempDownloadDirectory", "BindAddress", "BindAddress6", "SocksServer",
    "SocksUser", "SocksPassword", "ConfigVersion", "DefaultAwayMessage",
    "TimeStampsFormat", "CID", "LogFileMainChat", "LogFilePrivateChat",
    "LogFileStatus", "LogFileUpload", "LogFileDownload", "LogFileFinishedDownload",
    "LogFileSystem",
    "LogFormatSystem", "LogFormatStatus", "LogFileSpy", "LogFormatSpy", "TLSPrivateKeyFile",
    "TLSCertificateFile", "TLSTrustedCertificatesPath",
    "Language", "SkipListShare", "InternetIp", "BindIfaceName",
    "DHTKey", "DynDNSServer", "MimeHandler",  "BindIfaceName6",
    "SENTRY",
    // Ints
    "IncomingConnections", "InPort", "Slots", "AutoFollow",
    "ShareHidden", "FilterMessages", "AutoSearch",
    "AutoSearchTime", "ReportFoundAlternates", "TimeStamps",
    "IgnoreHubPms", "IgnoreBotPms",
    "ListDuplicates", "BufferSize", "DownloadSlots", "MaxDownloadSpeed",
    "LogMainChat", "LogPrivateChat", "LogDownloads","LogFileFinishedDownload",
    "LogUploads", "MinUploadSpeed", "AutoAway",
    "SocksPort", "SocksResolve", "KeepLists", "AutoKick",
    "CompressTransfers", "SFVCheck",
    "MaxCompression", "NoAwayMsgToBots", "SkipZeroByte", "AdlsBreakOnFirst",
    "HubUserCommands", "AutoSearchAutoMatch","LogSystem",
    "LogFilelistTransfers",
    "SendUnknownCommands", "MaxHashSpeed",
    "GetUserCountry", "LogStatusMessages", "SearchPassiveAlways",
    "AddFinishedInstantly", "DontDLAlreadyShared",
    "UDPPort", "ShowLastLinesLog", "AdcDebug",
    "SearchHistory", "SetMinislotSize",
    "MaxFilelistSize", "HighestPrioSize", "HighPrioSize", "NormalPrioSize",
    "LowPrioSize", "LowestPrio", "AutoDropSpeed", "AutoDropInterval",
    "AutoDropElapsed", "AutoDropInactivity", "AutoDropMinSources",
    "AutoDropFilesize", "AutoDropAll", "AutoDropFilelists",
    "AutoDropDisconnect", "OutgoingConnections", "NoIpOverride", "NoUseTempDir",
    "ShareTempFiles", "SearchOnlyFreeSlots", "LastSearchType",
    "SocketInBuffer", "SocketOutBuffer",
    "AutoRefreshTime", "HashingStartDelay", "UseTLS", "AutoSearchLimit",
    "AutoKickNoFavs", "PromptPassword",
    "DontDlAlreadyQueued", "MaxCommandLength", "AllowUntrustedHubs",
    "AllowUntrustedClients", "TLSPort", "FastHash",
    "SegmentedDL", "FollowLinks", "SendBloom",
    "Coral", "SearchFilterShared", "FinishedDLOnlyFull",
    "SearchMerge", "HashBufferSize", "HashBufferPopulate",
    "HashBufferNoReserve", "HashBufferPrivate",
    "UseDHT", "DHTPort",
    "ReconnectDelay", "AutoDetectIncomingConnection",
    "BandwidthLimitStart", "BandwidthLimitEnd", "EnableThrottle","TimeDependentThrottle",
    "MaxDownloadSpeedAlternate", "MaxUploadSpeedAlternate",
    "MaxDownloadSpeedMain", "MaxUploadSpeedMain",
    "SlotsAlternateLimiting", "SlotsPrimaryLimiting", "KeepFinishedFiles",
    "ShowFreeSlotsDesc", "UseIP", "OverLapChunks", "CaseSensitiveFilelist",
    "IpFilter", "TextColor", "UseLua", "AllowNatt", "IpTOSValue", "SegmentSize",
    "BindIface", "MinimumSearchInterval", "EnableDynDNS", "AllowUploadOverMultiHubs",
    "UseADLOnlyOnOwnList", "AllowSimUploads", "CheckTargetsPathsOnStart", "NmdcDebug",
    "ShareSkipZeroByte", "RequireTLS", "LogSpy",
    "BindIface6",
    "SENTRY",
    // Int64
    "TotalUpload", "TotalDownload",
    "SENTRY",
    // Floats
    "SENTRY"
};

SettingsManager::SettingsManager()
{

    connectionSpeeds.push_back("0.005");
    connectionSpeeds.push_back("0.01");
    connectionSpeeds.push_back("0.02");
    connectionSpeeds.push_back("0.05");
    connectionSpeeds.push_back("0.1");
    connectionSpeeds.push_back("0.2");
    connectionSpeeds.push_back("0.5");
    connectionSpeeds.push_back("1");
    connectionSpeeds.push_back("2");
    connectionSpeeds.push_back("5");
    connectionSpeeds.push_back("10");
    connectionSpeeds.push_back("20");
    connectionSpeeds.push_back("50");
    connectionSpeeds.push_back("100");
    connectionSpeeds.push_back("1000");

    for(int i=0; i<SETTINGS_LAST; i++)
        isSet[i] = false;

    for(int i=0; i<INT_LAST-INT_FIRST; i++) {
        intDefaults[i] = 0;
        intSettings[i] = 0;
    }
    for(int i=0; i<INT64_LAST-INT64_FIRST; i++) {
        int64Defaults[i] = 0;
        int64Settings[i] = 0;
    }
    for(int i=0; i<FLOAT_LAST-FLOAT_FIRST; i++) {
        floatDefaults[i] = 0;
        floatSettings[i] = 0;
    }

    setDefault(DOWNLOAD_DIRECTORY, Util::getPath(Util::PATH_DOWNLOADS));
    setDefault(TEMP_DOWNLOAD_DIRECTORY, Util::getPath(Util::PATH_DOWNLOADS) + "Incomplete" PATH_SEPARATOR_STR);
    setDefault(SLOTS, 5);
    setDefault(TCP_PORT, 3000);
    setDefault(UDP_PORT, 3000);
    setDefault(TLS_PORT, 3001);
    setDefault(INCOMING_CONNECTIONS, INCOMING_DIRECT);
    setDefault(OUTGOING_CONNECTIONS, OUTGOING_DIRECT);
    setDefault(AUTO_FOLLOW, true);
    setDefault(SHARE_HIDDEN, false);
    setDefault(FILTER_MESSAGES, true);
    setDefault(AUTO_SEARCH, true);
    setDefault(AUTO_SEARCH_TIME, 2);
    setDefault(REPORT_ALTERNATES, true);
    setDefault(TIME_STAMPS, true);
    setDefault(IGNORE_HUB_PMS, false);
    setDefault(IGNORE_BOT_PMS, false);
    setDefault(LIST_DUPES, true);
    setDefault(BUFFER_SIZE, 64);
    setDefault(HUBLIST_SERVERS,
               "http://dchublist.com/hublist.xml.bz2;"
               "http://dchublist.ru/hublist.xml.bz2;"
               "http://hublista.hu/hublist.xml.bz2"
               );
    setDefault(DOWNLOAD_SLOTS, 3);
    setDefault(SKIPLIST_SHARE, "*.~*|*.*~");
    setDefault(MAX_DOWNLOAD_SPEED, 0);
    setDefault(LOG_DIRECTORY, Util::getPath(Util::PATH_USER_LOCAL) + "Logs" PATH_SEPARATOR_STR);
    setDefault(LOG_UPLOADS, false);
    setDefault(LOG_DOWNLOADS, false);
    setDefault(LOG_FINISHED_DOWNLOADS, false);
    setDefault(LOG_PRIVATE_CHAT, false);
    setDefault(LOG_MAIN_CHAT, false);
    setDefault(LOG_SPY, false);
    setDefault(UPLOAD_SPEED, connectionSpeeds[11]);
    setDefault(MIN_UPLOAD_SPEED, 0);
    setDefault(LOG_FORMAT_POST_DOWNLOAD, "[%Y-%m-%d %H:%M:%S] %[target] downloaded from %[userNI] (%[userCID]), %[fileSI] (%[fileSIchunk]), %[speed], %[time], %[fileTR]");
    setDefault(LOG_FORMAT_POST_FINISHED_DOWNLOAD, "%Y-%m-%d %H:%M: %[target] " + string(_("downloaded from")) + " %[userNI] (%[userCID]), %[fileSI] (%[fileSIsession]), %[speed], %[time], %[fileTR]");
    setDefault(LOG_FORMAT_POST_UPLOAD,   "[%Y-%m-%d %H:%M:%S] %[source] uploaded to %[userNI] (%[userCID]), %[fileSI] (%[fileSIchunk]), %[speed], %[time], %[fileTR]");
    setDefault(LOG_FORMAT_MAIN_CHAT,     "[%Y-%m-%d %H:%M:%S] %[message]");
    setDefault(LOG_FORMAT_PRIVATE_CHAT,  "[%Y-%m-%d %H:%M:%S] %[message]");
    setDefault(LOG_FORMAT_STATUS,        "[%Y-%m-%d %H:%M:%S] %[message]");
    setDefault(LOG_FORMAT_SYSTEM,        "[%Y-%m-%d %H:%M:%S] %[message]");
    setDefault(LOG_FORMAT_SPY,        "[%Y-%m-%d %H:%M:%S] %[message] (%[count])");
    setDefault(LOG_FILE_MAIN_CHAT,    "CHAT/%B - %Y/%[hubNI] (%[hubURL]).log");
    setDefault(LOG_FILE_STATUS,       "STATUS/%B - %Y/%[hubNI] (%[hubURL]).log");
    setDefault(LOG_FILE_PRIVATE_CHAT, "PM/%B - %Y/%[userNI] (%[userCID]).log");
    setDefault(LOG_FILE_UPLOAD,       "Uploads.log");
    setDefault(LOG_FILE_DOWNLOAD,     "Downloads.log");
    setDefault(LOG_FILE_FINISHED_DOWNLOAD, "Finished_downloads.log");
    setDefault(LOG_FILE_SYSTEM,       "System.log");
    setDefault(LOG_FILE_SPY,       "Spy.log");
    setDefault(AUTO_AWAY, false);
    setDefault(BIND_ADDRESS, "0.0.0.0");
    setDefault(SOCKS_PORT, 1080);
    setDefault(SOCKS_RESOLVE, 1);
    setDefault(CONFIG_VERSION, "0.181");        // 0.181 is the last version missing configversion
    setDefault(KEEP_LISTS, false);
    setDefault(AUTO_KICK, false);
    setDefault(COMPRESS_TRANSFERS, true);
    setDefault(SFV_CHECK, true);
    setDefault(DEFAULT_AWAY_MESSAGE, "I'm away. State your business and I might answer later if you're lucky.");
    setDefault(TIME_STAMPS_FORMAT, "%H:%M");
    setDefault(MAX_COMPRESSION, 6);
    setDefault(NO_AWAYMSG_TO_BOTS, true);
    setDefault(SKIP_ZERO_BYTE, false);
    setDefault(ADLS_BREAK_ON_FIRST, false);
    setDefault(HUB_USER_COMMANDS, true);
    setDefault(AUTO_SEARCH_AUTO_MATCH, false);
    setDefault(LOG_FILELIST_TRANSFERS, false);
    setDefault(LOG_SYSTEM, false);
    setDefault(SEND_UNKNOWN_COMMANDS, true);
    setDefault(MAX_HASH_SPEED, 0);
    setDefault(GET_USER_COUNTRY, true);
    setDefault(LOG_STATUS_MESSAGES, false);
    setDefault(ADD_FINISHED_INSTANTLY, false);
    setDefault(DONT_DL_ALREADY_SHARED, false);
    setDefault(SHOW_LAST_LINES_LOG, 0);
    setDefault(ADC_DEBUG, false);
    setDefault(SEARCH_HISTORY, 10);
    setDefault(SET_MINISLOT_SIZE, 64);
    setDefault(MAX_FILELIST_SIZE, 512);
    setDefault(PRIO_HIGHEST_SIZE, 64);
    setDefault(PRIO_HIGH_SIZE, 0);
    setDefault(PRIO_NORMAL_SIZE, 0);
    setDefault(PRIO_LOW_SIZE, 0);
    setDefault(PRIO_LOWEST, false);
    setDefault(AUTODROP_SPEED, 1024);
    setDefault(AUTODROP_INTERVAL, 10);
    setDefault(AUTODROP_ELAPSED, 15);
    setDefault(AUTODROP_INACTIVITY, 10);
    setDefault(AUTODROP_MINSOURCES, 2);
    setDefault(AUTODROP_FILESIZE, 0);
    setDefault(AUTODROP_ALL, true);
    setDefault(AUTODROP_FILELISTS, true);
    setDefault(AUTODROP_DISCONNECT, true);
    setDefault(NO_IP_OVERRIDE, false);
    setDefault(NO_USE_TEMP_DIR, false);
    setDefault(SHARE_TEMP_FILES, false);
    setDefault(SEARCH_ONLY_FREE_SLOTS, false);
    setDefault(SEARCH_FILTER_SHARED, true);
    setDefault(LAST_SEARCH_TYPE, 0);
    setDefault(SOCKET_IN_BUFFER, 64*1024);
    setDefault(SOCKET_OUT_BUFFER, 64*1024);
    setDefault(TLS_TRUSTED_CERTIFICATES_PATH, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR);
    setDefault(TLS_PRIVATE_KEY_FILE, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR "client.key");
    setDefault(TLS_CERTIFICATE_FILE, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR "client.crt");
    setDefault(AUTO_REFRESH_TIME, 60);  // minutes
    setDefault(HASHING_START_DELAY, 60); // seconds
    setDefault(USE_TLS, true);
    setDefault(AUTO_SEARCH_LIMIT, 5);
    setDefault(AUTO_KICK_NO_FAVS, false);
    setDefault(PROMPT_PASSWORD, false);
    setDefault(DONT_DL_ALREADY_QUEUED, false);
    setDefault(MAX_COMMAND_LENGTH, 16*1024*1024);
    setDefault(ALLOW_UNTRUSTED_HUBS, true);
    setDefault(ALLOW_UNTRUSTED_CLIENTS, true);
    setDefault(FAST_HASH, true);
    setDefault(SEGMENTED_DL, true);
    setDefault(FOLLOW_LINKS, false);
    setDefault(SEND_BLOOM, true);
    setDefault(CORAL, true);
    setDefault(FINISHED_DL_ONLY_FULL, true);
    setDefault(SEARCH_MERGE, true);
    setDefault(HASH_BUFFER_SIZE_MB, 8);
    setDefault(HASH_BUFFER_POPULATE, true);
    setDefault(HASH_BUFFER_NORESERVE, true);
    setDefault(HASH_BUFFER_PRIVATE, true);
    setDefault(RECONNECT_DELAY, 15);
    setDefault(DHT_PORT, 6250);
    setDefault(USE_DHT, false);
    setDefault(SEARCH_PASSIVE, false);
    setDefault(AUTO_DETECT_CONNECTION, false);
    setDefault(MAX_UPLOAD_SPEED_MAIN, 0);
    setDefault(MAX_DOWNLOAD_SPEED_MAIN, 0);
    setDefault(TIME_DEPENDENT_THROTTLE, false);
    setDefault(THROTTLE_ENABLE, false);
    setDefault(MAX_DOWNLOAD_SPEED_ALTERNATE, 0);
    setDefault(MAX_UPLOAD_SPEED_ALTERNATE, 0);
    setDefault(BANDWIDTH_LIMIT_START, 1);
    setDefault(BANDWIDTH_LIMIT_END, 1);
    setDefault(SLOTS_ALTERNATE_LIMITING, 1);
    setDefault(SLOTS_PRIMARY, 3);
    setDefault(KEEP_FINISHED_FILES, false);
    setDefault(USE_IP, true);
    setDefault(SHOW_FREE_SLOTS_DESC, false);
    setDefault(OVERLAP_CHUNKS, true);
    setDefault(CASESENSITIVE_FILELIST, false);
    setDefault(IPFILTER,false);
    setDefault(USE_LUA,false);
    setDefault(ALLOW_NATT, true);
    setDefault(IP_TOS_VALUE, -1);
    setDefault(SEGMENT_SIZE, 0);
    setDefault(BIND_IFACE, false);
    setDefault(BIND_IFACE_NAME, "");
    setDefault(BIND_IFACE6, false);
    setDefault(BIND_IFACE_NAME6, "");
    setDefault(MINIMUM_SEARCH_INTERVAL, 60);
    setDefault(DYNDNS_SERVER, "http://checkip.dyndns.org/index.html");
    setDefault(DYNDNS_ENABLE, false);
    setDefault(ALLOW_UPLOAD_MULTI_HUB, true);
    setDefault(USE_ADL_ONLY_OWN_LIST, false);
    setDefault(ALLOW_SIM_UPLOADS, true);
    setDefault(CHECK_TARGETS_PATHS_ON_START, false);
    setDefault(SHARE_SKIP_ZERO_BYTE, false);
    setDefault(BIND_ADDRESS6, "::");

    setSearchTypeDefaults();
}

void SettingsManager::load(string const& aFileName)
{
    try {
        SimpleXML xml;

        xml.fromXML(File(aFileName, File::READ, File::OPEN).read());

        xml.resetCurrentChild();

        xml.stepIn();

        if(xml.findChild("Settings"))
        {
            xml.stepIn();

            int i;

            for(i=STR_FIRST; i<STR_LAST; i++)
            {
                const string& attr = settingTags[i];
                dcassert(attr.find("SENTRY") == string::npos);

                if(xml.findChild(attr))
                    set(StrSetting(i), xml.getChildData());
                xml.resetCurrentChild();
            }
            for(i=INT_FIRST; i<INT_LAST; i++)
            {
                const string& attr = settingTags[i];
                dcassert(attr.find("SENTRY") == string::npos);

                if(xml.findChild(attr))
                    set(IntSetting(i), Util::toInt(xml.getChildData()));
                xml.resetCurrentChild();
            }
            for(i=FLOAT_FIRST; i<FLOAT_LAST; i++)
            {
                const string& attr = settingTags[i];
                dcassert(attr.find("SENTRY") == string::npos);

                if(xml.findChild(attr))
                    set(FloatSetting(i), Util::toInt(xml.getChildData()) / 1000.);
                xml.resetCurrentChild();
            }
            for(i=INT64_FIRST; i<INT64_LAST; i++)
            {
                const string& attr = settingTags[i];
                dcassert(attr.find("SENTRY") == string::npos);

                if(xml.findChild(attr))
                    set(Int64Setting(i), Util::toInt64(xml.getChildData()));
                xml.resetCurrentChild();
            }

            xml.stepOut();
        }

        xml.resetCurrentChild();
        if(xml.findChild("SearchTypes")) {
            try {
                searchTypes.clear();
                xml.stepIn();
                while(xml.findChild("SearchType")) {
                    const string& extensions = xml.getChildData();
                    if(extensions.empty()) {
                        continue;
                    }
                    const string& name = xml.getChildAttrib("Id");
                    if(name.empty()) {
                        continue;
                    }
                    searchTypes[name] = StringTokenizer<string>(extensions, ';').getTokens();
                }
                xml.stepOut();
            } catch(const SimpleXMLException&) {
                setSearchTypeDefaults();
            }
        }

        if(SETTING(PRIVATE_ID).length() != 39 || CID(SETTING(PRIVATE_ID)).isZero()) {
            set(PRIVATE_ID, CID::generate().toBase32());
        }

        double v = Util::toDouble(SETTING(CONFIG_VERSION));
        // if(v < 0.x) { // Fix old settings here }

        if(v <= 0.674) {

            // Formats changed, might as well remove these...
            unset(LOG_FORMAT_POST_DOWNLOAD);
            unset(LOG_FORMAT_POST_UPLOAD);
            unset(LOG_FORMAT_MAIN_CHAT);
            unset(LOG_FORMAT_PRIVATE_CHAT);
            unset(LOG_FORMAT_STATUS);
            unset(LOG_FORMAT_SYSTEM);
            unset(LOG_FILE_MAIN_CHAT);
            unset(LOG_FILE_STATUS);
            unset(LOG_FILE_PRIVATE_CHAT);
            unset(LOG_FILE_UPLOAD);
            unset(LOG_FILE_DOWNLOAD);
            unset(LOG_FILE_SYSTEM);
        }

        if(SETTING(SET_MINISLOT_SIZE) < 64)
            set(SET_MINISLOT_SIZE, 64);
        if(SETTING(AUTODROP_INTERVAL) < 1)
            set(AUTODROP_INTERVAL, 1);
        if(SETTING(AUTODROP_ELAPSED) < 1)
            set(AUTODROP_ELAPSED, 1);
        if(SETTING(AUTO_SEARCH_LIMIT) > 5)
            set(AUTO_SEARCH_LIMIT, 5);
        else if(SETTING(AUTO_SEARCH_LIMIT) < 1)
            set(AUTO_SEARCH_LIMIT, 1);

#ifdef _DEBUG
        set(PRIVATE_ID, CID::generate().toBase32());
#endif
        setDefault(UDP_PORT, SETTING(TCP_PORT));

        File::ensureDirectory(SETTING(TLS_TRUSTED_CERTIFICATES_PATH));

        fire(SettingsManagerListener::Load(), xml);

        xml.stepOut();

    } catch(const Exception&) {
        if(CID(SETTING(PRIVATE_ID)).isZero())
            set(PRIVATE_ID, CID::generate().toBase32());
    }
    if (SETTING(DHT_KEY).length() != 39 || CID(SETTING(DHT_KEY)).isZero())
        set(DHT_KEY, CID::generate().toBase32());
}

void SettingsManager::save(string const& aFileName) {

    SimpleXML xml;
    xml.addTag("DCPlusPlus");
    xml.stepIn();
    xml.addTag("Settings");
    xml.stepIn();

    int i;
    string type("type"), curType("string");

    for(i=STR_FIRST; i<STR_LAST; i++)
    {
        if(i == CONFIG_VERSION) {
            xml.addTag(settingTags[i], VERSIONSTRING);
            xml.addChildAttrib(type, curType);
        } else if(isSet[i]) {
            xml.addTag(settingTags[i], get(StrSetting(i), false));
            xml.addChildAttrib(type, curType);
        }
    }

    curType = "int";
    for(i=INT_FIRST; i<INT_LAST; i++)
    {
        if(isSet[i]) {
            xml.addTag(settingTags[i], get(IntSetting(i), false));
            xml.addChildAttrib(type, curType);
        }
    }
    for(i=FLOAT_FIRST; i<FLOAT_LAST; i++)
    {
        if(isSet[i]) {
            xml.addTag(settingTags[i], static_cast<int>(get(FloatSetting(i), false) * 1000.));
            xml.addChildAttrib(type, curType);
        }
    }
    curType = "int64";
    for(i=INT64_FIRST; i<INT64_LAST; i++)
    {
        if(isSet[i])
        {
            xml.addTag(settingTags[i], get(Int64Setting(i), false));
            xml.addChildAttrib(type, curType);
        }
    }
    xml.stepOut();

    xml.addTag("SearchTypes");
    xml.stepIn();
    for(SearchTypesIterC i = searchTypes.begin(); i != searchTypes.end(); ++i) {
        xml.addTag("SearchType", Util::toString(";", i->second));
        xml.addChildAttrib("Id", i->first);
    }
    xml.stepOut();

    fire(SettingsManagerListener::Save(), xml);

    try {
        File out(aFileName + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
        BufferedOutputStream<false> f(&out);
        f.write(SimpleXML::utf8Header);
        xml.toXML(&f);
        f.flush();
        out.close();
        File::deleteFile(aFileName);
        File::renameFile(aFileName + ".tmp", aFileName);
    } catch(const FileException&) {
        // ...
    }
}

void SettingsManager::validateSearchTypeName(const string& name) const {
    if(name.empty() || (name.size() == 1 && name[0] >= '1' && name[0] <= '6')) {
        throw SearchTypeException(_("Invalid search type name"));
    }
    for(int type = SearchManager::TYPE_ANY; type != SearchManager::TYPE_LAST; ++type) {
        if(SearchManager::getTypeStr(type) == name) {
            throw SearchTypeException(_("This search type already exists"));
        }
    }
}

void SettingsManager::setSearchTypeDefaults() {
    searchTypes.clear();

    // for conveniency, the default search exts will be the same as the ones defined by SEGA.
    const vector<StringList>& searchExts = AdcHub::getSearchExts();
    for(size_t i = 0, n = searchExts.size(); i < n; ++i)
        searchTypes[string(1, '1' + i)] = searchExts[i];

    fire(SettingsManagerListener::SearchTypesChanged());
}

void SettingsManager::addSearchType(const string& name, const StringList& extensions, bool validated) {
    if(!validated) {
        validateSearchTypeName(name);
    }

    if(searchTypes.find(name) != searchTypes.end()) {
        throw SearchTypeException(_("This search type already exists"));
    }

    searchTypes[name] = extensions;
    fire(SettingsManagerListener::SearchTypesChanged());
}

void SettingsManager::delSearchType(const string& name) {
    validateSearchTypeName(name);
    searchTypes.erase(name);
    fire(SettingsManagerListener::SearchTypesChanged());
}

void SettingsManager::renameSearchType(const string& oldName, const string& newName) {
    validateSearchTypeName(newName);
    StringList exts = getSearchType(oldName)->second;
    addSearchType(newName, exts, true);
    searchTypes.erase(oldName);
}

void SettingsManager::modSearchType(const string& name, const StringList& extensions) {
    getSearchType(name)->second = extensions;
    fire(SettingsManagerListener::SearchTypesChanged());
}

const StringList& SettingsManager::getExtensions(const string& name) {
    return getSearchType(name)->second;
}

SettingsManager::SearchTypesIter SettingsManager::getSearchType(const string& name) {
    SearchTypesIter ret = searchTypes.find(name);
    if(ret == searchTypes.end()) {
        throw SearchTypeException(_("No such search type"));
    }
    return ret;
}

bool SettingsManager::getType(const char* name, int& n, int& type) const {
    for(n = 0; n < INT64_LAST; n++) {
        if (strcmp(settingTags[n].c_str(), name) == 0) {
            if (n < STR_LAST) {
                type = TYPE_STRING;
                return true;
            } else if (n < INT_LAST) {
                type= TYPE_INT;
                return true;
            } else {
                type = TYPE_INT64;
                return true;
            }
        }
    }
    return false;
}

const std::string SettingsManager::parseCoreCmd(const std::string& cmd) {
    StringTokenizer<string> sl(cmd, ' ');
    if (sl.getTokens().size() == 1) {
        string ret;
        bool b = parseCoreCmd(ret, sl.getTokens().at(0), Util::emptyString);
        return (!b ? _("Error: setting not found!") : _("Core setting ") + sl.getTokens().at(0) + ": " + ret);
    } else if (sl.getTokens().size() >= 2) {
        string tmp = cmd.substr(sl.getTokens().at(0).size()+1);
        string ret;
        bool b = parseCoreCmd(ret, sl.getTokens().at(0), tmp);
        return (!b ? _("Error: setting not found!") : _("Change core setting ") + sl.getTokens().at(0) + _(" to ") + tmp);
    }
    return Util::emptyString;
}

bool SettingsManager::parseCoreCmd(string& ret, const std::string& key, const string& value) {
    if (key.empty()) {
        return false;
    }

    int n,type;
    getType(key.c_str(),n,type);
    if (type == SettingsManager::TYPE_INT) {
        if (!value.empty()) {
            int i = atoi(value.c_str());
            set((SettingsManager::IntSetting)n,i);
        } else if (value.empty())
            ret = Util::toString(get((SettingsManager::IntSetting)n,false));
    }
    else if (type == SettingsManager::TYPE_STRING) {
        if (!value.empty())
            set((SettingsManager::StrSetting)n, value);
        else if (value.empty())
            ret = get((SettingsManager::StrSetting)n,false);
    } else
        return false;
    return true;
}

} // namespace dcpp
