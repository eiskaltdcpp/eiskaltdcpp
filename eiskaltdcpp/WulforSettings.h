#ifndef WULFORSETTINGS_H
#define WULFORSETTINGS_H

#include <QObject>
#include <QMap>
#include <QTranslator>
#include <QFont>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

const char * const WS_CHAT_OP_COLOR           = "chat-op-color";
const char * const WS_CHAT_USER_COLOR         = "chat-us-color";
const char * const WS_CHAT_CORE_COLOR         = "chat-co-color";
const char * const WS_CHAT_BOT_COLOR          = "chat-bt-color";
const char * const WS_CHAT_FIND_COLOR         = "chat-find-color";
const char * const WS_CHAT_LOCAL_COLOR        = "chat-lc-color";
const char * const WS_CHAT_STAT_COLOR         = "chat-st-color";
const char * const WS_CHAT_TIME_COLOR         = "chat-ts-color";
const char * const WS_CHAT_MSG_COLOR          = "chat-msg-color";
const char * const WS_CHAT_PRIV_USER_COLOR    = "chat-pr-us-color";
const char * const WS_CHAT_PRIV_LOCAL_COLOR   = "chat-pr-lc-color";
const char * const WS_CHAT_FAVUSER_COLOR      = "chat-fu-color";
const char * const WS_CHAT_SAY_NICK           = "chat-say-mynick";
const char * const WS_CHAT_USERLIST_STATE     = "chat-userlist-saved-state";
const char * const WS_CHAT_CMD_ALIASES        = "chat-cmd-aliases";
const char * const WS_QCONNECT_HISTORY        = "qc-history";
const char * const WS_DEFAULT_LOCALE          = "default_locale";
const char * const WS_DQUEUE_STATE            = "dqueue-state";
const char * const WS_SEARCH_STATE            = "search-state";
const char * const WS_TRANSLATION_FILE        = "translation-file";
const char * const WS_TRANSFERS_STATE         = "transfer-state";
const char * const WS_MAINWINDOW_STATE        = "mainwindow-state";
const char * const WS_SHARE_LPANE_STATE       = "share-lpane-state";
const char * const WS_SHARE_RPANE_STATE       = "share-rpane-state";
const char * const WS_FTRANSFERS_USERS_STATE  = "finished-u-state";
const char * const WS_FTRANSFERS_FILES_STATE  = "finished-f-state";
const char * const WS_FAV_HUBS_STATE          = "fav-hubs-state";
const char * const WS_APP_THEME               = "app-theme-name";
const char * const WS_APP_FONT                = "app-font-name";
const char * const WS_NOTIFY_SOUNDS           = "notify-sound-files";
const char * const WS_NOTIFY_SND_CMD          = "notify-sound-cmd";
const char * const WS_FAVUSERS_STATE          = "favoriteusers-state";
const char * const WS_SHAREHEADER_STATE       = "shareheader-state";
const char * const WS_DOWNLOADTO_ALIASES      = "downloadto-aliases";
const char * const WS_DOWNLOADTO_PATHS        = "downloadto-paths";
const char * const WS_APP_ICONTHEME           = "app-theme-icons";
const char * const WS_APP_TOTAL_UP            = "app-stat-total-up";
const char * const WS_APP_TOTAL_DOWN          = "app-stat-total-down";
const char * const WS_APP_ASPELL_LANG         = "app-aspell-lang";
const char * const WS_APP_EMOTICON_THEME      = "app-emoticon-theme";
const char * const WS_APP_DYNDNS_SERVER       = "app-dyndns-server";
const char * const WS_APP_DYNDNS_INDEX        = "app-dyndns-server-index";
const char * const WS_PUBLICHUBS_STATE        = "publichubs-state";
const char * const WB_CHAT_SHOW_TIMESTAMP     = "chat-show-timestamp";
const char * const WB_SHOW_FREE_SPACE         = "show-free-space";
const char * const WB_CHAT_SHOW_JOINS         = "chat-show-joins";
const char * const WB_CHAT_SHOW_JOINS_FAV     = "chat-show-joins-fav-only";
const char * const WB_CHAT_REDIRECT_BOT_PMS   = "chat-redirect-pms-from-bot";
const char * const WB_CHAT_KEEPFOCUS          = "chat-keep-focus";
const char * const WB_MAINWINDOW_MAXIMIZED    = "mainwindow-maximized";
const char * const WB_MAINWINDOW_HIDE         = "mainwindow-autohide";
const char * const WB_SEARCHFILTER_NOFREE     = "search-filter-nofree";
const char * const WB_SEARCH_DONTHIDEPANEL    = "search-panel-dont-hide";
const char * const WB_ANTISPAM_ENABLED        = "antispam-enabled";
const char * const WB_ANTISPAM_AS_FILTER      = "antispam-as-filter";
const char * const WB_IPFILTER_ENABLED        = "ipfilter-enabled";
const char * const WB_TRAY_ENABLED            = "systemtray-enabled";
const char * const WB_EXIT_CONFIRM            = "exit-confirm";
const char * const WB_SHOW_HIDDEN_USERS       = "show-hidden-users";
const char * const WB_SHOW_JOINS              = "show-joins";
const char * const WB_LAST_STATUS             = "last-status";
const char * const WB_USERS_STATISTICS         = "users-statistics";
const char * const WB_NOTIFY_ENABLED          = "notify-enabled";
const char * const WB_NOTIFY_SND_ENABLED      = "notify-sound-enabled";
const char * const WB_NOTIFY_SND_EXTERNAL     = "notify-sound-use-external";
const char * const WB_NOTIFY_SHOW_ON_ACTIVE   = "notify-show-on-active";
const char * const WB_NOTIFY_CH_ICON_ALWAYS   = "notify-change-icon-always";
const char * const WB_FAVUSERS_AUTOGRANT      = "favusers-auto-grant";
const char * const WB_APP_ENABLE_EMOTICON     = "app-enable-emoticon";
const char * const WB_APP_FORCE_EMOTICONS     = "app-force-find-emoticons";
const char * const WB_APP_ENABLE_ASPELL       = "app-enable-aspell";
const char * const WB_APP_AUTO_AWAY           = "app-auto-away";
const char * const WB_WIDGETS_PANEL_VISIBLE   = "widgets-panel-visible";
const char * const WB_TOOLS_PANEL_VISIBLE     = "tools-panel-visible";
const char * const WB_MAIN_MENU_VISIBLE       = "main-menu-visible";
const char * const WI_DEF_MAGNET_ACTION       = "def-magnet-action"; //0-not do any 1-search 2-download
const char * const WI_CHAT_MAXPARAGRAPHS      = "chat-max-paragraph";
const char * const WI_CHAT_WIDTH              = "chat-width";
const char * const WI_CHAT_USERLIST_WIDTH     = "chat-userlist-width";
const char * const WI_CHAT_SORT_COLUMN        = "chat-userlist-sort-column";
const char * const WI_CHAT_SORT_ORDER         = "chat-userlist-sort-order";
const char * const WI_CHAT_DBLCLICK_ACT       = "chat-dbl-click-action";// 0 - nick in chat, 1 - browse files
const char * const WI_CHAT_MDLCLICK_ACT       = "chat-mdl-click-action";//see WI_CHAT_DBLCLICK_ACT
const char * const WI_CHAT_FIND_COLOR_ALPHA   = "chat-find-color-alpha";
const char * const WI_SEARCH_SORT_COLUMN      = "search-sort-column";
const char * const WI_SEARCH_SORT_ORDER       = "search-sort-order";
const char * const WI_SEARCH_SHARED_ACTION    = "search-shared-action";
const char * const WI_SEARCH_LAST_TYPE        = "search-last-search-type";
const char * const WI_TRANSFER_HEIGHT         = "transfer-height";
const char * const WI_MAINWINDOW_WIDTH        = "mainwindow-width";
const char * const WI_MAINWINDOW_HEIGHT       = "mainwindow-height";
const char * const WI_MAINWINDOW_X            = "mainwindow-x";
const char * const WI_MAINWINDOW_Y            = "mainwindow-y";
const char * const WI_SHARE_RPANE_WIDTH       = "sharebrowser-rp-width";
const char * const WI_SHARE_WIDTH             = "sharebrowser-width";
const char * const WI_NOTIFY_EVENTMAP         = "notify-event-map";
const char * const WI_NOTIFY_SNDMAP           = "notify-snd-map";
const char * const WI_NOTIFY_MODULE           = "notify-module";
const char * const WI_OUT_IN_HIST             = "number-of-output-messages-in-history";


class WulforSettings :
        public QObject,
        public dcpp::Singleton<WulforSettings>
{
    Q_OBJECT

    typedef QMap<QString, int> WIntMap;
    typedef QMap<QString, QString> WStrMap;

friend class dcpp::Singleton<WulforSettings>;

public:
    class BadKey{
    };

    void load();
    void save();

    void loadTranslation();
    void loadTheme();

    QString getStr(QString) throw(BadKey);
    int     getInt(QString) throw(BadKey);
    bool    getBool(QString)throw(BadKey);

    void    setStr (QString, QString) throw(BadKey);
    void    setInt (QString, int) throw(BadKey);
    void    setBool(QString, bool)throw(BadKey);

    void    parseCmd(const QString &);

private:
    WulforSettings();
    virtual ~WulforSettings();

    QString configFile;

    QFont f;

    WIntMap intmap;
    WStrMap strmap;

    QTranslator tor;
    QTranslator qtTranslator;
};

#define WSGET(k)    (WulforSettings::getInstance()->getStr(k))
#define WSSET(k, y) (WulforSettings::getInstance()->setStr(k, y))
#define WIGET(k)    (WulforSettings::getInstance()->getInt(k))
#define WISET(k, y) (WulforSettings::getInstance()->setInt(k, y))
#define WBGET(k)    (WulforSettings::getInstance()->getBool(k))
#define WBSET(k, y) (WulforSettings::getInstance()->setBool(k, y))
#define WSCMD(k)    (WulforSettings::getInstance()->parseCmd(k))

#endif // WULFORSETTINGS_H
