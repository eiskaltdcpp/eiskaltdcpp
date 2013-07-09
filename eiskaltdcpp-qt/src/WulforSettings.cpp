/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "WulforSettings.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/File.h"
#include "dcpp/Util.h"
#include "dcpp/SimpleXML.h"

#include <QApplication>
#include <QFile>
#include <QTextCodec>
#include <QLocale>
#include <QFile>
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QStyle>
#include <QBrush>
#include <QUrl>

#include <QtDebug>

#ifndef CLIENT_TRANSLATIONS_DIR
#define CLIENT_TRANSLATIONS_DIR ""
#endif

using namespace dcpp;



WulforSettings::WulforSettings():
        settings(_q(Util::getPath(Util::PATH_USER_CONFIG)) + "EiskaltDC++_Qt.conf", QSettings::IniFormat),
        tor(0)
{
    configFileOld = _q(Util::getPath(Util::PATH_USER_CONFIG)) + "EiskaltDC++.xml";

    QStringList idns = QUrl::idnWhitelist();
    idns.push_back("рф");
    QUrl::setIdnWhitelist(idns);

    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&qtTranslator);

    connect(this, SIGNAL(fontChanged(QString,QString)), this, SLOT(slotFontChanged(QString,QString)));
    connect(this, SIGNAL(fontChanged(QString,QString)), this, SIGNAL(strValueChanged(QString,QString)));
}

WulforSettings::~WulforSettings(){
}

bool WulforSettings::hasKey(const QString &key) const{
    return (intmap.keys().contains(key) || strmap.keys().contains(key));
}

void WulforSettings::load(){
#ifdef _DEBUG_MODEL_
    qDebug() << settings.fileName();
#endif
    if (QFile::exists(configFileOld) && settings.value("app/firstrun", true).toBool()){
        loadOldConfig();

        //And load old config into QSettings
        auto it = strmap.begin();

        for (; it != strmap.end(); ++it)
            settings.setValue(it.key(), it.value());

        auto iit = intmap.begin();

        for (; iit != intmap.end(); ++iit)
            settings.setValue(iit.key(), iit.value());

        // QFile(configFileOld).remove();

        intmap.clear();
        strmap.clear();

        settings.setValue("app/firstrun", false);
    }

    if (settings.value("app/firstrun", true).toBool()){
        settings.setValue("app/firstrun", false);
        //Load default values
        {
            settings.setValue(WS_CHAT_LOCAL_COLOR,      "#078010");
            settings.setValue(WS_CHAT_OP_COLOR,         "#000000");
            settings.setValue(WS_CHAT_BOT_COLOR,        "#838383");
            settings.setValue(WS_CHAT_FIND_COLOR,       "#FFFF00");
            settings.setValue(WS_CHAT_PRIV_LOCAL_COLOR, "#078010");
            settings.setValue(WS_CHAT_PRIV_USER_COLOR,  "#ac0000");
            settings.setValue(WS_CHAT_SAY_NICK,         "#2344e7");
            settings.setValue(WS_CHAT_CORE_COLOR,       "#ff0000");
            settings.setValue(WS_CHAT_STAT_COLOR,       "#ac0000");
            settings.setValue(WS_CHAT_USER_COLOR,       "#ac0000");
            settings.setValue(WS_CHAT_FAVUSER_COLOR,    "#00ff7f");
            settings.setValue(WS_CHAT_TIME_COLOR,       qApp->palette().text().color().name());
            settings.setValue(WS_CHAT_MSG_COLOR,        qApp->palette().text().color().name());
            settings.setValue(WS_CHAT_USERLIST_STATE,   "");
            settings.setValue(WS_CHAT_CMD_ALIASES,      "");
            settings.setValue(WS_CHAT_FONT,             "");
            settings.setValue(WS_CHAT_ULIST_FONT,       "");
            settings.setValue(WS_CHAT_PM_FONT,          "");
            settings.setValue(WS_CHAT_SEPARATOR,        ":");
            settings.setValue(WS_CHAT_TIMESTAMP,        "hh:mm:ss");
            settings.setValue(WS_QCONNECT_HISTORY,      "");
            settings.setValue(WS_DEFAULT_LOCALE,        "UTF-8");
            settings.setValue(WS_DOWNLOAD_DIR_HISTORY,  "");
            settings.setValue(WS_DQUEUE_STATE,          "");
            settings.setValue(WS_SEARCH_STATE,          "");
            settings.setValue(WS_SEARCH_HISTORY,        "");
            settings.setValue(WS_TRANSLATION_FILE,      "");
            settings.setValue(WS_TRANSFERS_STATE,       "");
            settings.setValue(WS_SHARE_LPANE_STATE,     "");
            settings.setValue(WS_SHARE_RPANE_STATE,     "");
            settings.setValue(WS_MAINWINDOW_STATE,      "");
            settings.setValue(WS_MAINWINDOW_TOOLBAR_ACTS,"");
            settings.setValue(WS_FTRANSFERS_FILES_STATE,"");
            settings.setValue(WS_FTRANSFERS_USERS_STATE,"");
            settings.setValue(WS_FAV_HUBS_STATE,        "");
            settings.setValue(WS_ADLS_STATE,            "");
            settings.setValue(WS_APP_THEME,             "");
            settings.setValue(WS_APP_FONT,              "");
            settings.setValue(WS_APP_ICONTHEME,         "default");
            settings.setValue(WS_APP_USERTHEME,         "default");
            settings.setValue(WS_APP_SHARED_FILES_COLOR,"#1f8f1f");
            settings.setValue(WS_NOTIFY_SOUNDS,         "");
            settings.setValue(WS_NOTIFY_SND_CMD,        "");
            settings.setValue(WS_FAVUSERS_STATE,        "");
            settings.setValue(WS_SHAREHEADER_STATE,     "");
            settings.setValue(WS_DOWNLOADTO_ALIASES,    "");
            settings.setValue(WS_DOWNLOADTO_PATHS,      "");
            settings.setValue(WS_APP_EMOTICON_THEME,    "default");
            settings.setValue(WS_APP_ASPELL_LANG,       "");
            settings.setValue(WS_APP_ENABLED_SCRIPTS,   "");
            settings.setValue(WS_PUBLICHUBS_STATE,      "");
            settings.setValue(WS_SETTINGS_GUI_FONTS_STATE, "");

            settings.setValue(WB_APP_AUTOAWAY_BY_TIMER, static_cast<int>(false));
            settings.setValue(WB_CHAT_SHOW_TIMESTAMP,   static_cast<int>(true));
            settings.setValue(WB_SHOW_FREE_SPACE,       static_cast<int>(true));
            settings.setValue(WB_LAST_STATUS,           static_cast<int>(true));
            settings.setValue(WB_USERS_STATISTICS,      static_cast<int>(true));
            settings.setValue(WB_CHAT_SHOW_JOINS,       static_cast<int>(false));
            settings.setValue(WB_CHAT_REDIRECT_BOT_PMS, static_cast<int>(true));
            settings.setValue(WB_CHAT_KEEPFOCUS,        static_cast<int>(true));
            settings.setValue(WB_CHAT_SHOW_JOINS_FAV,   static_cast<int>(true));
            settings.setValue(WB_CHAT_HIGHLIGHT_FAVS,   static_cast<int>(true));
            settings.setValue(WB_CHAT_ROTATING_MSGS,    static_cast<int>(true));
            settings.setValue(WB_CHAT_USE_SMILE_PANEL,  static_cast<int>(false));
            settings.setValue(WB_CHAT_HIDE_SMILE_PANEL, static_cast<bool>(true));
            settings.setValue(WB_MAINWINDOW_MAXIMIZED,  static_cast<int>(true));
            settings.setValue(WB_MAINWINDOW_HIDE,       static_cast<int>(false));
            settings.setValue(WB_MAINWINDOW_REMEMBER,   static_cast<int>(true));
            settings.setValue(WB_MAINWINDOW_USE_M_TABBAR, static_cast<int>(true));
            settings.setValue(WB_MAINWINDOW_USE_SIDEBAR, static_cast<int>(true));
            settings.setValue(WB_SEARCHFILTER_NOFREE,   static_cast<int>(false));
            settings.setValue(WB_SEARCH_DONTHIDEPANEL,  static_cast<int>(false));
            settings.setValue(WB_ANTISPAM_ENABLED,      static_cast<int>(false));
            settings.setValue(WB_ANTISPAM_AS_FILTER,    static_cast<int>(false));
            settings.setValue(WB_ANTISPAM_FILTER_OPS,   static_cast<int>(false));
            settings.setValue(WB_IPFILTER_ENABLED,      static_cast<int>(false));
            settings.setValue(WB_TRAY_ENABLED,          static_cast<int>(true));
            settings.setValue(WB_EXIT_CONFIRM,          static_cast<int>(false));
            settings.setValue(WB_SHOW_IP_IN_CHAT,       static_cast<int>(false));
            settings.setValue(WB_SHOW_HIDDEN_USERS,     static_cast<int>(false));
            settings.setValue(WB_SHOW_JOINS,            static_cast<int>(false));
            settings.setValue(WB_NOTIFY_ENABLED,        static_cast<int>(true));
            settings.setValue(WB_NOTIFY_SND_ENABLED,    static_cast<int>(false));
            settings.setValue(WB_NOTIFY_SND_EXTERNAL,   static_cast<int>(false));
            settings.setValue(WB_NOTIFY_CH_ICON_ALWAYS, static_cast<int>(false));
            settings.setValue(WB_NOTIFY_SHOW_ON_ACTIVE, static_cast<int>(false));
            settings.setValue(WB_NOTIFY_SHOW_ON_VISIBLE, static_cast<int>(false));
            settings.setValue(WB_FAVUSERS_AUTOGRANT,    static_cast<int>(true));
            settings.setValue(WB_APP_ENABLE_EMOTICON,   static_cast<int>(true));
            settings.setValue(WB_APP_FORCE_EMOTICONS,   static_cast<int>(false));
            settings.setValue(WB_APP_ENABLE_ASPELL,     static_cast<int>(true));
            settings.setValue(WB_APP_REMOVE_NOT_EX_DIRS,static_cast<int>(false));
            settings.setValue(WB_APP_AUTO_AWAY,         static_cast<int>(false));
            settings.setValue(WB_APP_TBAR_SHOW_CL_BTNS, static_cast<int>(true));
            settings.setValue(WB_WIDGETS_PANEL_VISIBLE, static_cast<int>(true));
            settings.setValue(WB_TOOLS_PANEL_VISIBLE,   static_cast<int>(true));
            settings.setValue(WB_SEARCH_PANEL_VISIBLE,  static_cast<int>(false));
            settings.setValue(WB_MAIN_MENU_VISIBLE,     static_cast<int>(true));
            settings.setValue(WB_USE_CTRL_ENTER,        static_cast<int>(false));
            settings.setValue(WB_SIMPLE_SHARE_MODE,     static_cast<int>(true));
            settings.setValue(WI_APP_UNIT_BASE,         1024);
            settings.setValue(WI_APP_AUTOAWAY_INTERVAL, 60);
            settings.setValue(WI_APP_SHARED_FILES_ALPHA, 127);
            settings.setValue(WI_CHAT_MAXPARAGRAPHS,    1000);
            settings.setValue(WI_DEF_MAGNET_ACTION,     0);
            settings.setValue(WI_CHAT_WIDTH,            -1);
            settings.setValue(WI_CHAT_USERLIST_WIDTH,   -1);
            settings.setValue(WI_CHAT_SORT_COLUMN,      0);
            settings.setValue(WI_CHAT_SORT_ORDER,       0);
            settings.setValue(WI_CHAT_DBLCLICK_ACT,     0);//nick in chat
            settings.setValue(WI_CHAT_MDLCLICK_ACT,     1);//browse files
            settings.setValue(WI_CHAT_FIND_COLOR_ALPHA, 127);
            settings.setValue(WI_CHAT_STATUS_HISTORY_SZ,5);
            settings.setValue(WI_CHAT_STATUS_MSG_MAX_LEN, 128);
            settings.setValue(WI_STATUSBAR_HISTORY_SZ,  5);
            settings.setValue(WI_MAINWINDOW_HEIGHT,     -1);
            settings.setValue(WI_MAINWINDOW_WIDTH,      -1);
            settings.setValue(WI_MAINWINDOW_X,          -1);
            settings.setValue(WI_MAINWINDOW_Y,          -1);
            settings.setValue(WI_SEARCH_SORT_COLUMN,    1);
            settings.setValue(WI_SEARCH_SORT_ORDER,     0);
            settings.setValue(WI_SEARCH_SHARED_ACTION,  0);
            settings.setValue(WI_SEARCH_LAST_TYPE,      0);
            settings.setValue(WI_TRANSFER_HEIGHT,       -1);
            settings.setValue(WI_SHARE_RPANE_WIDTH,     -1);
            settings.setValue(WI_SHARE_WIDTH,           -1);
            settings.setValue(WI_NOTIFY_EVENTMAP,       0x0B);// 0b00001011, (transfer done = off)
            settings.setValue(WI_NOTIFY_MODULE,         1);//default
            settings.setValue(WI_NOTIFY_SNDMAP,         0x0F);// 0b00001111, all events
            settings.setValue(WI_OUT_IN_HIST,           50);//number of output messages in history
        }
    }
}

void WulforSettings::loadOldConfig(){
    //Load default values into maps
    {
        strmap.insert(WS_CHAT_LOCAL_COLOR,      "#078010");
        strmap.insert(WS_CHAT_OP_COLOR,         "#000000");
        strmap.insert(WS_CHAT_BOT_COLOR,        "#838383");
        strmap.insert(WS_CHAT_FIND_COLOR,       "#FFFF00");
        strmap.insert(WS_CHAT_PRIV_LOCAL_COLOR, "#078010");
        strmap.insert(WS_CHAT_PRIV_USER_COLOR,  "#ac0000");
        strmap.insert(WS_CHAT_SAY_NICK,         "#2344e7");
        strmap.insert(WS_CHAT_CORE_COLOR,       "#ff0000");
        strmap.insert(WS_CHAT_STAT_COLOR,       "#ac0000");
        strmap.insert(WS_CHAT_USER_COLOR,       "#ac0000");
        strmap.insert(WS_CHAT_FAVUSER_COLOR,    "#00ff7f");
        strmap.insert(WS_CHAT_TIME_COLOR,       qApp->palette().text().color().name());
        strmap.insert(WS_CHAT_MSG_COLOR,        qApp->palette().text().color().name());
        strmap.insert(WS_CHAT_USERLIST_STATE,   "");
        strmap.insert(WS_CHAT_CMD_ALIASES,      "");
        strmap.insert(WS_CHAT_FONT,             "");
        strmap.insert(WS_CHAT_ULIST_FONT,       "");
        strmap.insert(WS_CHAT_PM_FONT,          "");
        strmap.insert(WS_CHAT_SEPARATOR,        ":");
        strmap.insert(WS_CHAT_TIMESTAMP,        "hh:mm:ss");
        strmap.insert(WS_QCONNECT_HISTORY,      "");
        strmap.insert(WS_DEFAULT_LOCALE,        "UTF-8");
        strmap.insert(WS_DOWNLOAD_DIR_HISTORY,  "");
        strmap.insert(WS_DQUEUE_STATE,          "");
        strmap.insert(WS_SEARCH_STATE,          "");
        strmap.insert(WS_SEARCH_HISTORY,        "");
        strmap.insert(WS_TRANSLATION_FILE,      "");
        strmap.insert(WS_TRANSFERS_STATE,       "");
        strmap.insert(WS_SHARE_LPANE_STATE,     "");
        strmap.insert(WS_SHARE_RPANE_STATE,     "");
        strmap.insert(WS_MAINWINDOW_STATE,      "");
        strmap.insert(WS_MAINWINDOW_TOOLBAR_ACTS,"");
        strmap.insert(WS_FTRANSFERS_FILES_STATE,"");
        strmap.insert(WS_FTRANSFERS_USERS_STATE,"");
        strmap.insert(WS_FAV_HUBS_STATE,        "");
        strmap.insert(WS_ADLS_STATE,            "");
        strmap.insert(WS_APP_THEME,             "");
        strmap.insert(WS_APP_FONT,              "");
        strmap.insert(WS_APP_ICONTHEME,         "default");
        strmap.insert(WS_APP_USERTHEME,         "default");
        strmap.insert(WS_APP_SHARED_FILES_COLOR,"#1f8f1f");
        strmap.insert(WS_NOTIFY_SOUNDS,         "");
        strmap.insert(WS_NOTIFY_SND_CMD,        "");
        strmap.insert(WS_FAVUSERS_STATE,        "");
        strmap.insert(WS_SHAREHEADER_STATE,     "");
        strmap.insert(WS_DOWNLOADTO_ALIASES,    "");
        strmap.insert(WS_DOWNLOADTO_PATHS,      "");
        strmap.insert(WS_APP_EMOTICON_THEME,    "default");
        strmap.insert(WS_APP_ASPELL_LANG,       "");
        strmap.insert(WS_APP_ENABLED_SCRIPTS,   "");
        strmap.insert(WS_PUBLICHUBS_STATE,      "");
        strmap.insert(WS_SETTINGS_GUI_FONTS_STATE, "");

        intmap.insert(WB_APP_AUTOAWAY_BY_TIMER, static_cast<int>(false));
        intmap.insert(WB_CHAT_SHOW_TIMESTAMP,   static_cast<int>(true));
        intmap.insert(WB_SHOW_FREE_SPACE,       static_cast<int>(true));
        intmap.insert(WB_LAST_STATUS,           static_cast<int>(true));
        intmap.insert(WB_USERS_STATISTICS,      static_cast<int>(true));
        intmap.insert(WB_CHAT_SHOW_JOINS,       static_cast<int>(false));
        intmap.insert(WB_CHAT_REDIRECT_BOT_PMS, static_cast<int>(true));
        intmap.insert(WB_CHAT_KEEPFOCUS,        static_cast<int>(true));
        intmap.insert(WB_CHAT_SHOW_JOINS_FAV,   static_cast<int>(true));
        intmap.insert(WB_CHAT_HIGHLIGHT_FAVS,   static_cast<int>(true));
        intmap.insert(WB_CHAT_ROTATING_MSGS,    static_cast<int>(true));
        intmap.insert(WB_CHAT_USE_SMILE_PANEL,  static_cast<int>(false));
        intmap.insert(WB_CHAT_HIDE_SMILE_PANEL, static_cast<bool>(true));
        intmap.insert(WB_MAINWINDOW_MAXIMIZED,  static_cast<int>(true));
        intmap.insert(WB_MAINWINDOW_HIDE,       static_cast<int>(false));
        intmap.insert(WB_MAINWINDOW_REMEMBER,   static_cast<int>(true));
        intmap.insert(WB_MAINWINDOW_USE_M_TABBAR, static_cast<int>(true));
        intmap.insert(WB_MAINWINDOW_USE_SIDEBAR, static_cast<int>(true));
        intmap.insert(WB_SEARCHFILTER_NOFREE,   static_cast<int>(false));
        intmap.insert(WB_SEARCH_DONTHIDEPANEL,  static_cast<int>(false));
        intmap.insert(WB_ANTISPAM_ENABLED,      static_cast<int>(false));
        intmap.insert(WB_ANTISPAM_AS_FILTER,    static_cast<int>(false));
        intmap.insert(WB_ANTISPAM_FILTER_OPS,   static_cast<int>(false));
        intmap.insert(WB_IPFILTER_ENABLED,      static_cast<int>(false));
        intmap.insert(WB_TRAY_ENABLED,          static_cast<int>(true));
        intmap.insert(WB_EXIT_CONFIRM,          static_cast<int>(false));
        intmap.insert(WB_SHOW_IP_IN_CHAT,       static_cast<int>(false));
        intmap.insert(WB_SHOW_HIDDEN_USERS,     static_cast<int>(false));
        intmap.insert(WB_SHOW_JOINS,            static_cast<int>(false));
        intmap.insert(WB_NOTIFY_ENABLED,        static_cast<int>(true));
        intmap.insert(WB_NOTIFY_SND_ENABLED,    static_cast<int>(false));
        intmap.insert(WB_NOTIFY_SND_EXTERNAL,   static_cast<int>(false));
        intmap.insert(WB_NOTIFY_CH_ICON_ALWAYS, static_cast<int>(false));
        intmap.insert(WB_NOTIFY_SHOW_ON_ACTIVE, static_cast<int>(false));
        intmap.insert(WB_NOTIFY_SHOW_ON_VISIBLE, static_cast<int>(false));
        intmap.insert(WB_FAVUSERS_AUTOGRANT,    static_cast<int>(true));
        intmap.insert(WB_APP_ENABLE_EMOTICON,   static_cast<int>(true));
        intmap.insert(WB_APP_FORCE_EMOTICONS,   static_cast<int>(false));
        intmap.insert(WB_APP_ENABLE_ASPELL,     static_cast<int>(true));
        intmap.insert(WB_APP_REMOVE_NOT_EX_DIRS,static_cast<int>(false));
        intmap.insert(WB_APP_AUTO_AWAY,         static_cast<int>(false));
        intmap.insert(WB_APP_TBAR_SHOW_CL_BTNS, static_cast<int>(true));
        intmap.insert(WB_WIDGETS_PANEL_VISIBLE, static_cast<int>(true));
        intmap.insert(WB_TOOLS_PANEL_VISIBLE,   static_cast<int>(true));
        intmap.insert(WB_SEARCH_PANEL_VISIBLE,  static_cast<int>(false));
        intmap.insert(WB_MAIN_MENU_VISIBLE,     static_cast<int>(true));
        intmap.insert(WB_USE_CTRL_ENTER,        static_cast<int>(false));
        intmap.insert(WB_SIMPLE_SHARE_MODE,     static_cast<int>(false));
        intmap.insert(WI_APP_UNIT_BASE,         1024);
        intmap.insert(WI_APP_AUTOAWAY_INTERVAL, 60);
        intmap.insert(WI_APP_SHARED_FILES_ALPHA, 127);
        intmap.insert(WI_CHAT_MAXPARAGRAPHS,    1000);
        intmap.insert(WI_DEF_MAGNET_ACTION,     0);
        intmap.insert(WI_CHAT_WIDTH,            -1);
        intmap.insert(WI_CHAT_USERLIST_WIDTH,   -1);
        intmap.insert(WI_CHAT_SORT_COLUMN,      0);
        intmap.insert(WI_CHAT_SORT_ORDER,       0);
        intmap.insert(WI_CHAT_DBLCLICK_ACT,     0);//nick in chat
        intmap.insert(WI_CHAT_MDLCLICK_ACT,     1);//browse files
        intmap.insert(WI_CHAT_FIND_COLOR_ALPHA, 127);
        intmap.insert(WI_CHAT_STATUS_HISTORY_SZ,5);
        intmap.insert(WI_CHAT_STATUS_MSG_MAX_LEN, 128);
        intmap.insert(WI_STATUSBAR_HISTORY_SZ,  5);
        intmap.insert(WI_MAINWINDOW_HEIGHT,     -1);
        intmap.insert(WI_MAINWINDOW_WIDTH,      -1);
        intmap.insert(WI_MAINWINDOW_X,          -1);
        intmap.insert(WI_MAINWINDOW_Y,          -1);
        intmap.insert(WI_SEARCH_SORT_COLUMN,    1);
        intmap.insert(WI_SEARCH_SORT_ORDER,     0);
        intmap.insert(WI_SEARCH_SHARED_ACTION,  0);
        intmap.insert(WI_SEARCH_LAST_TYPE,      0);
        intmap.insert(WI_TRANSFER_HEIGHT,       -1);
        intmap.insert(WI_SHARE_RPANE_WIDTH,     -1);
        intmap.insert(WI_SHARE_WIDTH,           -1);
        intmap.insert(WI_NOTIFY_EVENTMAP,       0x0B);// 0b00001011, (transfer done = off)
        intmap.insert(WI_NOTIFY_MODULE,         1);//default
        intmap.insert(WI_NOTIFY_SNDMAP,         0x0F);// 0b00001111, all events
        intmap.insert(WI_OUT_IN_HIST,           50);//number of output messages in history
    }

    if (!QFile::exists(configFileOld))
        return;

    try{
        SimpleXML xml;
        xml.fromXML(File(configFileOld.toStdString(), File::READ, File::OPEN).read());
        xml.resetCurrentChild();
        xml.stepIn();

        if (xml.findChild("Settings")){

            xml.stepIn();

            auto it = strmap.begin();

            for (; it != strmap.end(); ++it){
                if (xml.findChild(it.key().toStdString()))
                        strmap.insert(it.key(), QTextCodec::codecForCStrings()->fromUnicode(xml.getChildData().c_str()));

                xml.resetCurrentChild();
            }

            auto iit = intmap.begin();

            for (; iit != intmap.end(); ++iit){
                if (xml.findChild(iit.key().toStdString()))
                    intmap.insert(iit.key(), Util::toInt(xml.getChildData()));

                xml.resetCurrentChild();
            }
        }
    }
    catch (Exception &ex){}
}

void WulforSettings::save(){
    //Do nothing
}

void WulforSettings::parseCmd(const QString &cmd){
    QStringList args = cmd.split(" ", QString::SkipEmptyParts);

    if (args.size() != 2)
        return;

    QString sname   = args.at(0);
    QString svalue  = args.at(1);

    setStr(sname, svalue);
}

void WulforSettings::loadTranslation(){
    QString file = getStr(WS_TRANSLATION_FILE);

    if (file.isEmpty() || !QFile::exists(file)){
        QString lc_prefix = QLocale::system().name();

#if !defined(Q_WS_WIN)
        file = QString(CLIENT_TRANSLATIONS_DIR) + QDir::separator();
#else
        file = qApp->applicationDirPath()+QDir::separator()+QString(CLIENT_TRANSLATIONS_DIR)+QDir::separator();
#endif
        lc_prefix = lc_prefix.left(lc_prefix.indexOf("_"));
        file += lc_prefix + ".qm";

        if (!QFile::exists(file))
            return;
    }
    else if (!file.isEmpty() && (file.size() >= 5) && QFile::exists(file)){
        QString lc_prefix = file.mid(file.size()-5, 2);
        dcpp::Util::setLang(lc_prefix.toStdString());
#ifdef _DEBUG_MODEL_
        qDebug() << QString("LANGUAGE=%1").arg(lc_prefix);
#endif
    }

    if (tor.load(file))
        qApp->installTranslator(&tor);
    else
        WSSET(WS_TRANSLATION_FILE, "");
}

void WulforSettings::loadTheme(){
    if (!getStr(WS_APP_THEME).isEmpty())
        qApp->setStyle(getStr(WS_APP_THEME));

    if (!getStr(WS_APP_FONT).isEmpty() && f.fromString(getStr(WS_APP_FONT)))
        qApp->setFont(f);
}

QString WulforSettings::getStr(const QString & key, const QString &default_value) {
    return settings.value(key, default_value).toString();
}

int WulforSettings::getInt(const QString & key, const int &default_value) {
    return settings.value(key, default_value).toInt();
}

bool WulforSettings::getBool(const QString & key, const bool &default_value) {
    return (static_cast<bool>(getInt(key, default_value)));
}

QVariant WulforSettings::getVar(const QString &key, const QVariant &default_value){
    return settings.value(key, default_value);
}

void WulforSettings::setStr(const QString & key, const QString &value) {
    settings.setValue(key, value);

    emit strValueChanged(key, value);
}

void WulforSettings::setInt(const QString & key, int value) {
    settings.setValue(key, value);

    emit intValueChanged(key, value);
}

void WulforSettings::setBool(const QString & key, bool value) {
    setInt(key, static_cast<int>(value));
}

void WulforSettings::setVar(const QString &key, const QVariant &value){
    settings.setValue(key, value);

    emit varValueChanged(key, value);
}

void WulforSettings::slotFontChanged(const QString &key, const QString &value){
    if (key == WS_APP_FONT){
        QFont f;

        if (f.fromString(value))
            qApp->setFont(f);
    }
}
