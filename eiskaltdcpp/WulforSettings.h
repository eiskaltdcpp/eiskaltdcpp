#ifndef WULFORSETTINGS_H
#define WULFORSETTINGS_H

#include <QObject>
#include <QMap>
#include <QTranslator>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

#define WS_CHAT_OP_COLOR            "chat-op-color"
#define WS_CHAT_USER_COLOR          "chat-us-color"
#define WS_CHAT_CORE_COLOR          "chat-co-color"
#define WS_CHAT_BOT_COLOR           "chat-bt-color"
#define WS_CHAT_LOCAL_COLOR         "chat-lc-color"
#define WS_CHAT_STAT_COLOR          "chat-st-color"
#define WS_CHAT_TIME_COLOR          "chat-ts-color"
#define WS_CHAT_MSG_COLOR           "chat-msg-color"
#define WS_CHAT_PRIV_USER_COLOR     "chat-pr-us-color"
#define WS_CHAT_PRIV_LOCAL_COLOR    "chat-pr-lc-color"
#define WS_CHAT_SAY_NICK            "chat-say-mynick"
#define WS_CHAT_USERLIST_COL_WIDTH  "chat-userlist-col-width"
#define WS_QCONNECT_HISTORY         "qc-history"
#define WS_DEFAULT_LOCALE           "default_locale"
#define WS_DQUEUE_COLUMN_WIDTHS     "dqueue-column-widths"
#define WS_SEARCH_COLUMN_WIDTHS     "search-column-widths"
#define WS_TRANSLATION_FILE         "translation-file"
#define WS_TRANSFERS_COLUMN_WIDTHS  "transfer-col-widths"
#define WS_SHARE_LPANE_COL_WIDTHS   "sharebrowser-lp-col-widths"
#define WS_SHARE_RPANE_COL_WIDTHS   "sharebrowser-rp-col-widths"
#define WB_CHAT_SHOW_TIMESTAMP      "chat-show-timestamp"
#define WB_CHAT_SHOW_JOINS          "chat-show-joins"
#define WB_MAINWINDOW_MAXIMIZED     "mainwindow-maximized"
#define WB_SEARCHFILTER_NOFREE      "search-filter-nofree"
#define WB_ANTISPAM_ENABLED         "antispam-enabled"
#define WB_ANTISPAM_AS_FILTER       "antispam-as-filter"
#define WB_IPFILTER_ENABLED         "ipfilter-enabled"
#define WI_CHAT_MAXPARAGRAPHS       "chat-max-paragraph"
#define WI_CHAT_WIDTH               "chat-width"
#define WI_CHAT_USERLIST_WIDTH      "chat-userlist-width"
#define WI_CHAT_USERLIST_COL_BITMAP "chat-userlist-bitmap"
#define WI_DQUEUE_COL_BITMAP        "dqueue-column-bitmap"
#define WI_SEARCH_COL_BITMAP        "search-column-bitmap"
#define WI_SEARCH_SORT_COLUMN       "search-sort-column"
#define WI_SEARCH_SORT_ORDER        "search-sort-order"
#define WI_SEARCH_SHARED_ACTION     "search-shared-action"
#define WI_TRANSFER_COL_BITMAP      "transferlumn-bitmap"
#define WI_TRANSFER_HEIGHT          "transfer-height"
#define WI_MAINWINDOW_WIDTH         "mainwindow-width"
#define WI_MAINWINDOW_HEIGHT        "mainwindow-height"
#define WI_MAINWINDOW_X             "mainwindow-x"
#define WI_MAINWINDOW_Y             "mainwindow-y"
#define WI_MAINWINDOW_TBAR_X        "mainwindow-tbar-x"
#define WI_MAINWINDOW_TBAR_Y        "mainwindow-tbar-y"
#define WI_MAINWINDOW_FBAR_X        "mainwindow-fbar-x"
#define WI_MAINWINDOW_FBAR_Y        "mainwindow-fbar-y"
#define WI_SHARE_RPANE_COL_BITMAP   "sharebrowser-rp-col-bitmap"
#define WI_SHARE_RPANE_WIDTH        "sharebrowser-rp-width"
#define WI_SHARE_WIDTH              "sharebrowser-width"

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

    QString getStr(QString) throw(BadKey);
    int     getInt(QString) throw(BadKey);
    bool    getBool(QString)throw(BadKey);

    void    setStr (QString, QString) throw(BadKey);
    void    setInt (QString, int) throw(BadKey);
    void    setBool(QString, bool)throw(BadKey);

private:
    WulforSettings();
    virtual ~WulforSettings();

    QString configFile;

    WIntMap intmap;
    WStrMap strmap;

    QTranslator tor;
};

#define WSGET(k)    (WulforSettings::getInstance()->getStr(k))
#define WSSET(k, y) (WulforSettings::getInstance()->setStr(k, y))
#define WIGET(k)    (WulforSettings::getInstance()->getInt(k))
#define WISET(k, y) (WulforSettings::getInstance()->setInt(k, y))
#define WBGET(k)    (WulforSettings::getInstance()->getBool(k))
#define WBSET(k, y) (WulforSettings::getInstance()->setBool(k, y))

#endif // WULFORSETTINGS_H
