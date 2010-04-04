/*
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SettingsManager.h"

#include "SimpleXML.h"
#include "Util.h"
#include "File.h"
#include "version.h"
#include "CID.h"

namespace dcpp {

StringList SettingsManager::connectionSpeeds;

const string SettingsManager::settingTags[] =
{
    // Strings
    "Nick", "UploadSpeed", "Description", "DownloadDirectory", "EMail", "ExternalIp",
    "Font", "ConnectionsOrder", "ConnectionsWidths", "HubFrameOrder", "HubFrameWidths",
    "SearchFrameOrder", "SearchFrameWidths", "FavHubsFrameOrder", "FavHubsFrameWidths",
    "HublistServers", "QueueFrameOrder", "QueueFrameWidths", "PublicHubsFrameOrder", "PublicHubsFrameWidths",
    "FinishedDLFilesOrder", "FinishedDLFilesWidths", "FinishedDLUsersOrder", "FinishedDLUsersWidths",
    "FinishedULFilesOrder", "FinishedULFilesWidths", "FinishedULUsersOrder", "FinishedULUsersWidths",
    "UsersFrameOrder", "UsersFrameWidths", "HttpProxy", "LogDirectory", "LogFormatPostDownload",
    "LogFormatPostUpload", "LogFormatMainChat", "LogFormatPrivateChat",
    "TempDownloadDirectory", "BindAddress", "SocksServer", "SocksUser", "SocksPassword", "ConfigVersion",
    "DefaultAwayMessage", "TimeStampsFormat", "ADLSearchFrameOrder", "ADLSearchFrameWidths",
    "CID", "SpyFrameWidths", "SpyFrameOrder", "LogFileMainChat",
    "LogFilePrivateChat", "LogFileStatus", "LogFileUpload", "LogFileDownload", "LogFileSystem",
    "LogFormatSystem", "LogFormatStatus", "DirectoryListingFrameOrder", "DirectoryListingFrameWidths",
    "TLSPrivateKeyFile", "TLSCertificateFile", "TLSTrustedCertificatesPath",
    "Language", "DownloadsOrder", "DownloadsWidth", "SkipListShare",
    "SoundMainChat", "SoundPM", "SoundPMWindow", "InternetIp",
    "SENTRY",
    // Ints
    "IncomingConnections", "InPort", "Slots", "AutoFollow", "ClearSearch",
    "BackgroundColor", "TextColor", "UseOemMonoFont", "ShareHidden", "FilterMessages", "MinimizeToTray", "AlwaysTray",
    "AutoSearch", "TimeStamps", "PopupHubPms", "PopupBotPms", "IgnoreHubPms", "IgnoreBotPms",
    "ListDuplicates", "BufferSize", "DownloadSlots", "MaxDownloadSpeed", "LogMainChat", "LogPrivateChat",
    "LogDownloads", "LogUploads", "StatusInChat", "ShowJoins",
    "UseSystemIcons", "PopupPMs", "MinUploadSpeed", "GetUserInfo", "UrlHandler", "MainWindowState",
    "MainWindowSizeX", "MainWindowSizeY", "MainWindowPosX", "MainWindowPosY", "AutoAway",
    "SocksPort", "SocksResolve", "KeepLists", "AutoKick", "QueueFrameShowTree",
    "CompressTransfers", "SFVCheck",
    "MaxCompression", "NoAwayMsgToBots", "SkipZeroByte", "AdlsBreakOnFirst",
    "HubUserCommands", "AutoSearchAutoMatch", "DownloadBarColor", "UploadBarColor", "LogSystem",
    "LogFilelistTransfers", "SendUnknownCommands", "MaxHashSpeed", "OpenUserCmdHelp",
    "GetUserCountry", "FavShowJoins", "LogStatusMessages", "ShowStatusbar",
    "ShowToolbar", "ShowTransferview", "PopunderPm", "PopunderFilelist", "MagnetAsk", "MagnetAction", "MagnetRegister",
    "AddFinishedInstantly", "DontDLAlreadyShared", "UseCTRLForLineHistory",
    "OpenNewWindow", "UDPPort", "ShowLastLinesLog",
    "AdcDebug", "ToggleActiveWindow", "SearchHistory", "SetMinislotSize", "MaxFilelistSize",
    "HighestPrioSize", "HighPrioSize", "NormalPrioSize", "LowPrioSize", "LowestPrio",
    "AutoDropSpeed", "AutoDropInterval", "AutoDropElapsed", "AutoDropInactivity", "AutoDropMinSources", "AutoDropFilesize",
    "AutoDropAll", "AutoDropFilelists", "AutoDropDisconnect",
    "OpenPublic", "OpenFavoriteHubs", "OpenFavoriteUsers", "OpenQueue", "OpenFinishedDownloads",
    "OpenFinishedUploads", "OpenSearchSpy", "OpenNetworkStatistics", "OpenNotepad", "OutgoingConnections",
    "NoIpOverride","NoUseTempDir","ShareTempFiles", "SearchOnlyFreeSlots", "LastSearchType", "BoldFinishedDownloads", "BoldFinishedUploads", "BoldQueue",
    "BoldHub", "BoldPm", "BoldSearch", "BoldSearchSpy",
    "ThrottleEnable","UploadLimit","DownloadLimit", "UploadLimitNormal","DownloadLimitNormal",
    "UploadLimitTime", "DownloadLimitTime", "TimeThrottle", "TimeLimitStart", "TimeLimitEnd",
    "SocketInBuffer", "SocketOutBuffer",
    "OpenWaitingUsers", "BoldWaitingUsers", "OpenSystemLog", "BoldSystemLog", "AutoRefreshTime",
    "UseTLS", "AutoSearchLimit", "AltSortOrder", "AutoKickNoFavs", "PromptPassword", "SpyFrameIgnoreTthSearches",
    "DontDlAlreadyQueued", "MaxCommandLength", "AllowUntrustedHubs", "AllowUntrustedClients",
    "TLSPort", "FastHash", "SortFavUsersFirst", "SegmentedDL", "FollowLinks",
    "SendBloom", "OwnerDrawnMenus", "Coral", "SearchFilterShared", "MaxTabChars", "FinishedDLOnlyFull",
    "ConfirmExit", "ConfirmHubClosing", "ConfirmHubRemoval", "ConfirmUserRemoval", "ConfirmItemRemoval", "ConfirmADLSRemoval",
    "SearchMerge",
    "SENTRY",
    // Int64
    "TotalUpload", "TotalDownload",
    "SENTRY",
    // Floats
    "TransfersPanedPos", "QueuePanedPos", "SearchPanedPos",
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
    setDefault(TEMP_DOWNLOAD_DIRECTORY, Util::getPath(Util::PATH_USER_LOCAL) + "Incomplete" PATH_SEPARATOR_STR);
    setDefault(SLOTS, 5);
    setDefault(TCP_PORT, 0);
    setDefault(UDP_PORT, 0);
    setDefault(TLS_PORT, 0);
    setDefault(INCOMING_CONNECTIONS, INCOMING_DIRECT);
    setDefault(OUTGOING_CONNECTIONS, OUTGOING_DIRECT);
    setDefault(AUTO_FOLLOW, true);
    setDefault(CLEAR_SEARCH, true);
    setDefault(SHARE_HIDDEN, false);
    setDefault(FILTER_MESSAGES, true);
    setDefault(MINIMIZE_TRAY, true);
    setDefault(ALWAYS_TRAY, true);
    setDefault(AUTO_SEARCH, true);
    setDefault(TIME_STAMPS, true);
    setDefault(POPUP_HUB_PMS, true);
    setDefault(POPUP_BOT_PMS, true);
    setDefault(IGNORE_HUB_PMS, false);
    setDefault(IGNORE_BOT_PMS, false);
    setDefault(LIST_DUPES, true);
    setDefault(BUFFER_SIZE, 64);
    setDefault(HUBLIST_SERVERS, "http://hublist.openhublist.org/hublist.xml.bz2;http://dchublist.com/hublist.xml.bz2;http://adchublist.com/hublist.xml.bz2;http://www.hublist.org/PublicHubList.xml.bz2;http://dclist.eu/hublist.xml.bz2;http://download.hublist.cz/hublist.xml.bz2;http://hublist.awenet.info/PublicHubList.xml.bz2");
    setDefault(DOWNLOAD_SLOTS, 3);
    setDefault(SKIPLIST_SHARE, "*.~*|*.*~");
    setDefault(MAX_DOWNLOAD_SPEED, 0);
    setDefault(LOG_DIRECTORY, Util::getPath(Util::PATH_USER_LOCAL) + "Logs" PATH_SEPARATOR_STR);
    setDefault(LOG_UPLOADS, false);
    setDefault(LOG_DOWNLOADS, false);
    setDefault(LOG_PRIVATE_CHAT, false);
    setDefault(LOG_MAIN_CHAT, false);
    setDefault(STATUS_IN_CHAT, true);
    setDefault(SHOW_JOINS, false);
    setDefault(UPLOAD_SPEED, connectionSpeeds[11]);
    setDefault(USE_SYSTEM_ICONS, true);
    setDefault(USE_OEM_MONOFONT, false);
    setDefault(POPUP_PMS, true);
    setDefault(MIN_UPLOAD_SPEED, 0);
        setDefault(LOG_FORMAT_POST_DOWNLOAD, "%H:%M:%S: %[target] " + string(_("downloaded from")) + " %[userNI] (%[userCID]), %[fileSI] (%[fileSIchunk]), %[speed], %[time], %[fileTR]");
        setDefault(LOG_FORMAT_POST_UPLOAD, "%H:%M:%S: %[source] " + string(_("uploaded to")) + " %[userNI] (%[userCID]), %[fileSI] (%[fileSIchunk]), %[speed], %[time], %[fileTR]");
        setDefault(LOG_FORMAT_MAIN_CHAT, "[%H:%M:%S] %[message]");
        setDefault(LOG_FORMAT_PRIVATE_CHAT, "[%H:%M:%S] %[message]");
        setDefault(LOG_FORMAT_STATUS, "[%H:%M:%S] %[message]");
        setDefault(LOG_FORMAT_SYSTEM, "[%H:%M:%S] %[message]");
        setDefault(LOG_FILE_MAIN_CHAT, "%Y-%m-%d chat on hub %[hubURL] (%[hubNI]).log");
        setDefault(LOG_FILE_STATUS, "%Y-%m-%d status %[hubURL].log");
        setDefault(LOG_FILE_PRIVATE_CHAT, "%Y-%m-%d chat with %[userNI] (%[userCID]).log");
        setDefault(LOG_FILE_UPLOAD, "%Y-%m-%d uploads.log");
        setDefault(LOG_FILE_DOWNLOAD, "%Y-%m-%d downloads.log");
        setDefault(LOG_FILE_SYSTEM, "%Y-%m-%d system.log");
    setDefault(GET_USER_INFO, true);
    setDefault(URL_HANDLER, false);
    setDefault(AUTO_AWAY, false);
    setDefault(BIND_ADDRESS, "0.0.0.0");
    setDefault(SOCKS_PORT, 1080);
    setDefault(SOCKS_RESOLVE, 1);
    setDefault(CONFIG_VERSION, "0.181");        // 0.181 is the last version missing configversion
    setDefault(KEEP_LISTS, false);
    setDefault(AUTO_KICK, false);
    setDefault(MAX_UPLOAD_SPEED_LIMIT_NORMAL, 0);
    setDefault(MAX_DOWNLOAD_SPEED_LIMIT_NORMAL, 0);
    setDefault(MAX_UPLOAD_SPEED_LIMIT, 0);
    setDefault(MAX_DOWNLOAD_SPEED_LIMIT, 0);
    setDefault(MAX_UPLOAD_SPEED_LIMIT_TIME, 0);
    setDefault(MAX_DOWNLOAD_SPEED_LIMIT_TIME, 0);
    setDefault(TIME_DEPENDENT_THROTTLE, false);
    setDefault(BANDWIDTH_LIMIT_START, 0);
    setDefault(BANDWIDTH_LIMIT_END, 0);
    setDefault(THROTTLE_ENABLE, false);
   // setDefault(BWSETTING_MODE, BWSETTING_DEFAULT);
    setDefault(QUEUEFRAME_SHOW_TREE, true);
    setDefault(COMPRESS_TRANSFERS, true);
    setDefault(SFV_CHECK, true);
    setDefault(DEFAULT_AWAY_MESSAGE, "I'm away. State your business and I might answer later if you're lucky.");
    setDefault(TIME_STAMPS_FORMAT, "%H:%M");
    setDefault(MAX_COMPRESSION, 6);
    setDefault(NO_AWAYMSG_TO_BOTS, true);
    setDefault(SKIP_ZERO_BYTE, false);
    setDefault(ADLS_BREAK_ON_FIRST, false);
    setDefault(HUB_USER_COMMANDS, true);
    setDefault(AUTO_SEARCH_AUTO_MATCH, true);
    setDefault(LOG_FILELIST_TRANSFERS, false);
    setDefault(LOG_SYSTEM, false);
    setDefault(SEND_UNKNOWN_COMMANDS, true);
    setDefault(MAX_HASH_SPEED, 0);
    setDefault(OPEN_USER_CMD_HELP, true);
    setDefault(GET_USER_COUNTRY, true);
    setDefault(FAV_SHOW_JOINS, false);
    setDefault(LOG_STATUS_MESSAGES, false);
    setDefault(SHOW_TRANSFERVIEW, true);
    setDefault(SHOW_STATUSBAR, true);
    setDefault(SHOW_TOOLBAR, true);
    setDefault(POPUNDER_PM, false);
    setDefault(POPUNDER_FILELIST, false);
    setDefault(MAGNET_REGISTER, true);
    setDefault(MAGNET_ASK, true);
    setDefault(MAGNET_ACTION, MAGNET_AUTO_SEARCH);
    setDefault(ADD_FINISHED_INSTANTLY, false);
    setDefault(DONT_DL_ALREADY_SHARED, false);
    setDefault(USE_CTRL_FOR_LINE_HISTORY, true);
    setDefault(JOIN_OPEN_NEW_WINDOW, false);
    setDefault(SHOW_LAST_LINES_LOG, 0);
    setDefault(ADC_DEBUG, false);
    setDefault(TOGGLE_ACTIVE_WINDOW, true);
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
    setDefault(OPEN_PUBLIC, false);
    setDefault(OPEN_FAVORITE_HUBS, false);
    setDefault(OPEN_FAVORITE_USERS, false);
    setDefault(OPEN_QUEUE, false);
    setDefault(OPEN_FINISHED_DOWNLOADS, false);
    setDefault(OPEN_FINISHED_UPLOADS, false);
    setDefault(OPEN_SEARCH_SPY, false);
    setDefault(OPEN_NETWORK_STATISTICS, false);
    setDefault(OPEN_NOTEPAD, false);
    setDefault(NO_IP_OVERRIDE, false);
    setDefault(NO_USE_TEMP_DIR, false);
    setDefault(SHARE_TEMP_FILES, false);
    setDefault(SEARCH_ONLY_FREE_SLOTS, false);
    setDefault(SEARCH_FILTER_SHARED, true);
    setDefault(LAST_SEARCH_TYPE, 0);
    setDefault(SOCKET_IN_BUFFER, 64*1024);
    setDefault(SOCKET_OUT_BUFFER, 64*1024);
    setDefault(OPEN_WAITING_USERS, false);
    setDefault(OPEN_SYSTEM_LOG, true);
    setDefault(TLS_TRUSTED_CERTIFICATES_PATH, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR);
    setDefault(TLS_PRIVATE_KEY_FILE, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR "client.key");
    setDefault(TLS_CERTIFICATE_FILE, Util::getPath(Util::PATH_USER_CONFIG) + "Certificates" PATH_SEPARATOR_STR "client.crt");
    setDefault(BOLD_FINISHED_DOWNLOADS, true);
    setDefault(BOLD_FINISHED_UPLOADS, true);
    setDefault(BOLD_QUEUE, true);
    setDefault(BOLD_HUB, true);
    setDefault(BOLD_PM, true);
    setDefault(BOLD_SEARCH, true);
    setDefault(BOLD_SEARCH_SPY, true);
    setDefault(BOLD_WAITING_USERS, true);
    setDefault(BOLD_SYSTEM_LOG, true);
    setDefault(AUTO_REFRESH_TIME, 60);
    setDefault(USE_TLS, true);
    setDefault(AUTO_SEARCH_LIMIT, 5);
    setDefault(ALT_SORT_ORDER, false);
    setDefault(AUTO_KICK_NO_FAVS, false);
    setDefault(PROMPT_PASSWORD, false);
    setDefault(SPY_FRAME_IGNORE_TTH_SEARCHES, false);
    setDefault(DONT_DL_ALREADY_QUEUED, false);
    setDefault(MAX_COMMAND_LENGTH, 16*1024*1024);
    setDefault(ALLOW_UNTRUSTED_HUBS, true);
    setDefault(ALLOW_UNTRUSTED_CLIENTS, true);
    setDefault(FAST_HASH, true);
    setDefault(SORT_FAVUSERS_FIRST, false);
    setDefault(SEGMENTED_DL, true);
    setDefault(FOLLOW_LINKS, false);
    setDefault(SEND_BLOOM, true);
    setDefault(OWNER_DRAWN_MENUS, true);
    setDefault(CORAL, true);
    setDefault(MAX_TAB_CHARS, 20);
    setDefault(FINISHED_DL_ONLY_FULL, true);
    setDefault(CONFIRM_EXIT, true);
    setDefault(CONFIRM_HUB_CLOSING, true);
    setDefault(CONFIRM_HUB_REMOVAL, true);
    setDefault(CONFIRM_USER_REMOVAL, true);
    setDefault(CONFIRM_ITEM_REMOVAL, true);
    setDefault(CONFIRM_ADLS_REMOVAL, true);
    setDefault(SEARCH_MERGE, true);
    setDefault(TRANSFERS_PANED_POS, .7);
    setDefault(QUEUE_PANED_POS, .3);
    setDefault(SEARCH_PANED_POS, .2);

#ifdef _WIN32
    setDefault(MAIN_WINDOW_STATE, SW_SHOWNORMAL);
    setDefault(MAIN_WINDOW_SIZE_X, CW_USEDEFAULT);
    setDefault(MAIN_WINDOW_SIZE_Y, CW_USEDEFAULT);
    setDefault(MAIN_WINDOW_POS_X, CW_USEDEFAULT);
    setDefault(MAIN_WINDOW_POS_Y, CW_USEDEFAULT);
    setDefault(UPLOAD_BAR_COLOR, RGB(205, 60, 55));
    setDefault(DOWNLOAD_BAR_COLOR, RGB(55, 170, 85));

#endif
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

        if(SETTING(PRIVATE_ID).length() != 39 || CID(SETTING(PRIVATE_ID)).isZero()) {
            set(PRIVATE_ID, CID::generate().toBase32());
        }

        double v = Util::toDouble(SETTING(CONFIG_VERSION));
        // if(v < 0.x) { // Fix old settings here }

        if(v <= 0.674) {

            // Formats changed, might as well remove these...
            set(LOG_FORMAT_POST_DOWNLOAD, Util::emptyString);
            set(LOG_FORMAT_POST_UPLOAD, Util::emptyString);
            set(LOG_FORMAT_MAIN_CHAT, Util::emptyString);
            set(LOG_FORMAT_PRIVATE_CHAT, Util::emptyString);
            set(LOG_FORMAT_STATUS, Util::emptyString);
            set(LOG_FORMAT_SYSTEM, Util::emptyString);
            set(LOG_FILE_MAIN_CHAT, Util::emptyString);
            set(LOG_FILE_STATUS, Util::emptyString);
            set(LOG_FILE_PRIVATE_CHAT, Util::emptyString);
            set(LOG_FILE_UPLOAD, Util::emptyString);
            set(LOG_FILE_DOWNLOAD, Util::emptyString);
            set(LOG_FILE_SYSTEM, Util::emptyString);
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

} // namespace dcpp
