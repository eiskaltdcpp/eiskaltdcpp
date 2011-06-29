/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef WULFORSETTINGS_H
#define WULFORSETTINGS_H

#include <QObject>
#include <QMap>
#include <QTranslator>
#include <QFont>
#include <QSettings>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"

static const QString & WS_CHAT_OP_COLOR           = "chat-op-color";
static const QString & WS_CHAT_USER_COLOR         = "chat-us-color";
static const QString & WS_CHAT_CORE_COLOR         = "chat-co-color";
static const QString & WS_CHAT_BOT_COLOR          = "chat-bt-color";
static const QString & WS_CHAT_FIND_COLOR         = "chat-find-color";
static const QString & WS_CHAT_LOCAL_COLOR        = "chat-lc-color";
static const QString & WS_CHAT_STAT_COLOR         = "chat-st-color";
static const QString & WS_CHAT_TIME_COLOR         = "chat-ts-color";
static const QString & WS_CHAT_MSG_COLOR          = "chat-msg-color";
static const QString & WS_CHAT_PRIV_USER_COLOR    = "chat-pr-us-color";
static const QString & WS_CHAT_PRIV_LOCAL_COLOR   = "chat-pr-lc-color";
static const QString & WS_CHAT_FAVUSER_COLOR      = "chat-fu-color";
static const QString & WS_CHAT_SAY_NICK           = "chat-say-mynick";
static const QString & WS_CHAT_USERLIST_STATE     = "chat-userlist-saved-state";
static const QString & WS_CHAT_CMD_ALIASES        = "chat-cmd-aliases";
static const QString & WS_CHAT_FONT               = "chat-font";
static const QString & WS_CHAT_ULIST_FONT         = "chat-userlist-font";
static const QString & WS_CHAT_PM_FONT            = "chat-pm-font";
static const QString & WS_CHAT_SEPARATOR          = "chat-separator";
static const QString & WS_CHAT_TIMESTAMP          = "chat-timestamp";
static const QString & WS_QCONNECT_HISTORY        = "qc-history";
static const QString & WS_DEFAULT_LOCALE          = "default_locale";
static const QString & WS_DOWNLOAD_DIR_HISTORY    = "download-directory-history";
static const QString & WS_DQUEUE_STATE            = "dqueue-state";
static const QString & WS_SEARCH_STATE            = "search-state";
static const QString & WS_SEARCH_HISTORY          = "search-history";
static const QString & WS_TRANSLATION_FILE        = "translation-file";
static const QString & WS_TRANSFERS_STATE         = "transfer-state";
static const QString & WS_MAINWINDOW_STATE        = "mainwindow-state";
static const QString & WS_MAINWINDOW_TOOLBAR_ACTS = "mainwindow-toolbar-enabled-actions";
static const QString & WS_SHARE_LPANE_STATE       = "share-lpane-state";
static const QString & WS_SHARE_RPANE_STATE       = "share-rpane-state";
static const QString & WS_FTRANSFERS_USERS_STATE  = "finished-u-state";
static const QString & WS_FTRANSFERS_FILES_STATE  = "finished-f-state";
static const QString & WS_FAV_HUBS_STATE          = "fav-hubs-state";
static const QString & WS_ADLS_STATE              = "adls-state";
static const QString & WS_APP_THEME               = "app-theme-name";
static const QString & WS_APP_FONT                = "app-font-name";
static const QString & WS_APP_ENABLED_SCRIPTS     = "app-enabled-scripts";
static const QString & WS_NOTIFY_SOUNDS           = "notify-sound-files";
static const QString & WS_NOTIFY_SND_CMD          = "notify-sound-cmd";
static const QString & WS_FAVUSERS_STATE          = "favoriteusers-state";
static const QString & WS_SHAREHEADER_STATE       = "shareheader-state";
static const QString & WS_DOWNLOADTO_ALIASES      = "downloadto-aliases";
static const QString & WS_DOWNLOADTO_PATHS        = "downloadto-paths";
static const QString & WS_APP_ICONTHEME           = "app-theme-icons";
static const QString & WS_APP_USERTHEME           = "app-theme-users";
static const QString & WS_APP_ASPELL_LANG         = "app-aspell-lang";
static const QString & WS_APP_EMOTICON_THEME      = "app-emoticon-theme";
static const QString & WS_APP_SHARED_FILES_COLOR  = "app-already-shared-files-color";
static const QString & WS_PUBLICHUBS_STATE        = "publichubs-state";
static const QString & WS_SETTINGS_GUI_FONTS_STATE= "settings-gui-fonts-state";
static const QString & WB_APP_AUTOAWAY_BY_TIMER   = "app-autoaway";
static const QString & WB_CHAT_SHOW_TIMESTAMP     = "chat-show-timestamp";
static const QString & WB_SHOW_FREE_SPACE         = "show-free-space";
static const QString & WB_CHAT_SHOW_JOINS         = "chat-show-joins";
static const QString & WB_CHAT_SHOW_JOINS_FAV     = "chat-show-joins-fav-only";
static const QString & WB_CHAT_REDIRECT_BOT_PMS   = "chat-redirect-pms-from-bot";
static const QString & WB_CHAT_KEEPFOCUS          = "chat-keep-focus";
static const QString & WB_CHAT_HIGHLIGHT_FAVS     = "chat-highlight-favs";
static const QString & WB_CHAT_ROTATING_MSGS      = "chat-support-rotating-messages";
static const QString & WB_CHAT_USE_SMILE_PANEL    = "chat-use-smile-panel";
static const QString & WB_CHAT_HIDE_SMILE_PANEL   = "chat-autohide-smile-panel";
static const QString & WB_MAINWINDOW_MAXIMIZED    = "mainwindow-maximized";
static const QString & WB_MAINWINDOW_HIDE         = "mainwindow-autohide";
static const QString & WB_MAINWINDOW_REMEMBER     = "mainwindow-remember-position-on-exit";
static const QString & WB_MAINWINDOW_USE_M_TABBAR = "mainwindow-use-multi-line-tabbar";
static const QString & WB_MAINWINDOW_USE_SIDEBAR  = "mainwindow-use-sidebar";
static const QString & WB_SEARCHFILTER_NOFREE     = "search-filter-nofree";
static const QString & WB_SEARCH_DONTHIDEPANEL    = "search-panel-dont-hide";
static const QString & WB_ANTISPAM_ENABLED        = "antispam-enabled";
static const QString & WB_ANTISPAM_AS_FILTER      = "antispam-as-filter";
static const QString & WB_ANTISPAM_FILTER_OPS     = "antispam-filter-ops";
static const QString & WB_IPFILTER_ENABLED        = "ipfilter-enabled";
static const QString & WB_TRAY_ENABLED            = "systemtray-enabled";
static const QString & WB_EXIT_CONFIRM            = "exit-confirm";
static const QString & WB_SHOW_IP_IN_CHAT         = "show-ip-in-chat";
static const QString & WB_SHOW_HIDDEN_USERS       = "show-hidden-users";
static const QString & WB_SHOW_JOINS              = "show-joins";
static const QString & WB_LAST_STATUS             = "last-status";
static const QString & WB_USERS_STATISTICS        = "users-statistics";
static const QString & WB_NOTIFY_ENABLED          = "notify-enabled";
static const QString & WB_NOTIFY_SND_ENABLED      = "notify-sound-enabled";
static const QString & WB_NOTIFY_SND_EXTERNAL     = "notify-sound-use-external";
static const QString & WB_NOTIFY_SHOW_ON_ACTIVE   = "notify-show-on-active";
static const QString & WB_NOTIFY_SHOW_ON_VISIBLE  = "notify-show-on-visible";
static const QString & WB_NOTIFY_CH_ICON_ALWAYS   = "notify-change-icon-always";
static const QString & WB_FAVUSERS_AUTOGRANT      = "favusers-auto-grant";
static const QString & WB_APP_ENABLE_EMOTICON     = "app-enable-emoticon";
static const QString & WB_APP_FORCE_EMOTICONS     = "app-force-find-emoticons";
static const QString & WB_APP_ENABLE_ASPELL       = "app-enable-aspell";
static const QString & WB_APP_AUTO_AWAY           = "app-auto-away";
static const QString & WB_APP_TBAR_SHOW_CL_BTNS   = "app-toolbar-show-close-buttons";
static const QString & WB_APP_REMOVE_NOT_EX_DIRS  = "app-auto-rem-not-exsisting-dirs";
static const QString & WB_WIDGETS_PANEL_VISIBLE   = "widgets-panel-visible";
static const QString & WB_TOOLS_PANEL_VISIBLE     = "tools-panel-visible";
static const QString & WB_SEARCH_PANEL_VISIBLE    = "search-panel-visible";
static const QString & WB_MAIN_MENU_VISIBLE       = "main-menu-visible";
static const QString & WB_USE_CTRL_ENTER          = "use-ctrl-enter";
static const QString & WB_SIMPLE_SHARE_MODE       = "use-simple-share-mode";
static const QString & WI_APP_UNIT_BASE           = "app-unit-base";
static const QString & WI_APP_AUTOAWAY_INTERVAL   = "app-autoaway-interval";
static const QString & WI_APP_SHARED_FILES_ALPHA  = "app-already-shared-files-alpha";
//
static const QString & WI_DEF_MAGNET_ACTION       = "def-magnet-action";
// 0 - ask, 1 - search, 2 - download
//
static const QString & WI_CHAT_MAXPARAGRAPHS      = "chat-max-paragraph";
static const QString & WI_CHAT_WIDTH              = "chat-width";
static const QString & WI_CHAT_USERLIST_WIDTH     = "chat-userlist-width";
static const QString & WI_CHAT_SORT_COLUMN        = "chat-userlist-sort-column";
static const QString & WI_CHAT_SORT_ORDER         = "chat-userlist-sort-order";
//
static const QString & WI_CHAT_DBLCLICK_ACT       = "chat-dbl-click-action";
static const QString & WI_CHAT_MDLCLICK_ACT       = "chat-mdl-click-action";
// 0 - insert nick into input widget, 1 - get file list, 2 - private message
//
static const QString & WI_CHAT_FIND_COLOR_ALPHA   = "chat-find-color-alpha";
static const QString & WI_CHAT_STATUS_HISTORY_SZ  = "chat-status-history-size";
static const QString & WI_STATUSBAR_HISTORY_SZ    = "statusbar-history-size";
static const QString & WI_CHAT_STATUS_MSG_MAX_LEN = "chat-status-msg-max-len";
static const QString & WI_SEARCH_SORT_COLUMN      = "search-sort-column";
static const QString & WI_SEARCH_SORT_ORDER       = "search-sort-order";
static const QString & WI_SEARCH_SHARED_ACTION    = "search-shared-action";
static const QString & WI_SEARCH_LAST_TYPE        = "search-last-search-type";
static const QString & WI_TRANSFER_HEIGHT         = "transfer-height";
static const QString & WI_MAINWINDOW_WIDTH        = "mainwindow-width";
static const QString & WI_MAINWINDOW_HEIGHT       = "mainwindow-height";
static const QString & WI_MAINWINDOW_X            = "mainwindow-x";
static const QString & WI_MAINWINDOW_Y            = "mainwindow-y";
static const QString & WI_SHARE_RPANE_WIDTH       = "sharebrowser-rp-width";
static const QString & WI_SHARE_WIDTH             = "sharebrowser-width";
static const QString & WI_NOTIFY_EVENTMAP         = "notify-event-map";
static const QString & WI_NOTIFY_SNDMAP           = "notify-snd-map";
static const QString & WI_NOTIFY_MODULE           = "notify-module";
static const QString & WI_OUT_IN_HIST             = "number-of-output-messages-in-history";

class WulforSettings :
        public QObject,
        public dcpp::Singleton<WulforSettings>
{
    Q_OBJECT

    typedef QMap<QString, int> WIntMap;
    typedef QMap<QString, QString> WStrMap;

friend class dcpp::Singleton<WulforSettings>;

public:
    void load();
    void save();

    void loadTranslation();
    void loadTheme();

    void    parseCmd(const QString &);

    bool hasKey(const QString&) const;

public Q_SLOTS:
    QString  getStr(const QString&, const QString &default_value = "") ;
    int      getInt(const QString&, const int &default_value = -1) ;
    bool     getBool(const QString&, const bool &default_value = false);
    QVariant getVar(const QString&, const QVariant &default_value = QVariant());

    void    setStr (const QString&, const QString&);
    void    setInt (const QString&, int) ;
    void    setBool(const QString&, bool);
    void    setVar (const QString&, const QVariant &);

Q_SIGNALS:
    void fontChanged(const QString &key, const QString &value);
    void intValueChanged(const QString &key, int value);
    void strValueChanged(const QString &key, const QString &value);
    void varValueChanged(const QString &key, const QVariant &value);

private Q_SLOTS:
    void slotFontChanged(const QString &key, const QString &value);

private:
    WulforSettings();
    virtual ~WulforSettings();

    void loadOldConfig();//load old version of config

    QSettings settings;

    QString configFileOld;

    QFont f;

    WIntMap intmap;
    WStrMap strmap;

    QTranslator tor;
    QTranslator qtTranslator;
};

inline QString WSGET(const QString &key, const QString &default_value = ""){ return WulforSettings::getInstance()->getStr(key, default_value); }
inline void WSSET(const QString &key, const QString &value) { WulforSettings::getInstance()->setStr(key, value); }

inline int WIGET(const QString &key, const int &default_value = -1){ return WulforSettings::getInstance()->getInt(key, default_value);}
inline void WISET(const QString &key, const int &value){ WulforSettings::getInstance()->setInt(key, value); }

inline bool WBGET(const QString &key, const bool &default_value = false) { return WulforSettings::getInstance()->getBool(key, default_value);}
inline void WBSET(const QString &key, const bool &value){ WulforSettings::getInstance()->setBool(key, value); }

inline QVariant WVGET(const QString &key, const QVariant &default_value = QVariant()) { return WulforSettings::getInstance()->getVar(key, default_value);}
inline void WVSET(const QString &key, const QVariant &value){ WulforSettings::getInstance()->setVar(key, value); }

inline void WSCMD(const QString &cmd){ WulforSettings::getInstance()->parseCmd(cmd); }

#endif // WULFORSETTINGS_H
