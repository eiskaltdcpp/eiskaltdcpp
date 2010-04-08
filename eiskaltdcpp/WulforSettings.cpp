#include "WulforSettings.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
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

#ifndef CLIENT_TRANSLATIONS_DIR
#define CLIENT_TRANSLATIONS_DIR ""
#endif

using namespace dcpp;

WulforSettings::WulforSettings():
        configFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "EiskaltDC++.xml"),
        tor(0)
{
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&qtTranslator);

    //Fill the maps with default values
    {
        strmap.insert(WS_CHAT_LOCAL_COLOR, "#078010");
        strmap.insert(WS_CHAT_OP_COLOR, "#000000");
        strmap.insert(WS_CHAT_BOT_COLOR, "#838383");
        strmap.insert(WS_CHAT_FIND_COLOR, "#FFFF00");
        strmap.insert(WS_CHAT_PRIV_LOCAL_COLOR, "#078010");
        strmap.insert(WS_CHAT_PRIV_USER_COLOR, "#ac0000");
        strmap.insert(WS_CHAT_SAY_NICK, "#2344e7");
        strmap.insert(WS_CHAT_CORE_COLOR, "#ff0000");
        strmap.insert(WS_CHAT_STAT_COLOR, "#ac0000");
        strmap.insert(WS_CHAT_USER_COLOR, "#ac0000");
        strmap.insert(WS_CHAT_FAVUSER_COLOR, "#00ff7f");
        strmap.insert(WS_CHAT_TIME_COLOR, qApp->palette().text().color().name());
        strmap.insert(WS_CHAT_MSG_COLOR, qApp->palette().text().color().name());
        strmap.insert(WS_CHAT_USERLIST_STATE, "");
        strmap.insert(WS_CHAT_CMD_ALIASES, "");
        strmap.insert(WS_QCONNECT_HISTORY, "");
        strmap.insert(WS_DEFAULT_LOCALE, "UTF-8");
        strmap.insert(WS_DQUEUE_STATE, "");
        strmap.insert(WS_SEARCH_STATE, "");
        strmap.insert(WS_SEARCH_HISTORY, "");
        strmap.insert(WS_TRANSLATION_FILE, "");
        strmap.insert(WS_TRANSFERS_STATE, "");
        strmap.insert(WS_SHARE_LPANE_STATE, "");
        strmap.insert(WS_SHARE_RPANE_STATE, "");
        strmap.insert(WS_MAINWINDOW_STATE, "");
        strmap.insert(WS_FTRANSFERS_FILES_STATE, "");
        strmap.insert(WS_FTRANSFERS_USERS_STATE, "");
        strmap.insert(WS_FAV_HUBS_STATE, "");
        strmap.insert(WS_APP_THEME, "");
        strmap.insert(WS_APP_FONT, "");
        strmap.insert(WS_APP_ICONTHEME, "default");
        strmap.insert(WS_NOTIFY_SOUNDS, "");
        strmap.insert(WS_NOTIFY_SND_CMD, "");
        strmap.insert(WS_FAVUSERS_STATE, "");
        strmap.insert(WS_SHAREHEADER_STATE, "");
        strmap.insert(WS_DOWNLOADTO_ALIASES, "");
        strmap.insert(WS_DOWNLOADTO_PATHS, "");
        strmap.insert(WS_APP_TOTAL_DOWN, "0");
        strmap.insert(WS_APP_TOTAL_UP, "0");
        strmap.insert(WS_APP_EMOTICON_THEME, "default");
        strmap.insert(WS_APP_ASPELL_LANG, "");
        strmap.insert(WS_APP_DYNDNS_SERVER, "checkip.dyndns.org");
        strmap.insert(WS_APP_DYNDNS_INDEX, "/index.html");
        strmap.insert(WS_PUBLICHUBS_STATE, "");

        intmap.insert(WB_CHAT_SHOW_TIMESTAMP, (int)true);
        intmap.insert(WB_SHOW_FREE_SPACE, (int)true);
        intmap.insert(WB_LAST_STATUS, (int)true);
        intmap.insert(WB_USERS_STATISTICS, (int)true);
        intmap.insert(WB_CHAT_SHOW_JOINS, (int)false);
        intmap.insert(WB_CHAT_REDIRECT_BOT_PMS, (int)true);
        intmap.insert(WB_CHAT_KEEPFOCUS, (int)true);
        intmap.insert(WB_CHAT_SHOW_JOINS_FAV, (int)true);
        intmap.insert(WB_MAINWINDOW_MAXIMIZED, (int)true);
        intmap.insert(WB_MAINWINDOW_HIDE, (int)false);
        intmap.insert(WB_SEARCHFILTER_NOFREE, (int)false);
        intmap.insert(WB_SEARCH_DONTHIDEPANEL, (int)false);
        intmap.insert(WB_ANTISPAM_ENABLED, (int)false);
        intmap.insert(WB_ANTISPAM_AS_FILTER, (int)false);
        intmap.insert(WB_IPFILTER_ENABLED, (int)false);
        intmap.insert(WB_TRAY_ENABLED, (int)true);
        intmap.insert(WB_EXIT_CONFIRM, (int)false);
        intmap.insert(WB_SHOW_HIDDEN_USERS, (int)false);
        intmap.insert(WB_SHOW_JOINS, (int)false);
        intmap.insert(WB_NOTIFY_ENABLED, (int)true);
        intmap.insert(WB_NOTIFY_SND_ENABLED, (int)false);
        intmap.insert(WB_NOTIFY_SND_EXTERNAL, (int)false);
        intmap.insert(WB_NOTIFY_CH_ICON_ALWAYS, (int)false);
        intmap.insert(WB_NOTIFY_SHOW_ON_ACTIVE, (int)false);
        intmap.insert(WB_FAVUSERS_AUTOGRANT, (int)true);
        intmap.insert(WB_APP_ENABLE_EMOTICON, (int)true);
        intmap.insert(WB_APP_FORCE_EMOTICONS, (int)false);
        intmap.insert(WB_APP_ENABLE_ASPELL, (int)true);
        intmap.insert(WB_APP_AUTO_AWAY, (int)false);
        intmap.insert(WB_WIDGETS_PANEL_VISIBLE, (int)true);
        intmap.insert(WB_TOOLS_PANEL_VISIBLE, (int)true);
        intmap.insert(WB_SEARCH_PANEL_VISIBLE, (int)false);
        intmap.insert(WB_MAIN_MENU_VISIBLE, (int)true);
        intmap.insert(WB_USE_CTRL_ENTER, (int)false);
        intmap.insert(WI_CHAT_MAXPARAGRAPHS, 1000);
        intmap.insert(WI_DEF_MAGNET_ACTION, 0);
        intmap.insert(WI_CHAT_WIDTH, -1);
        intmap.insert(WI_CHAT_USERLIST_WIDTH, -1);
        intmap.insert(WI_CHAT_SORT_COLUMN, 0);
        intmap.insert(WI_CHAT_SORT_ORDER, 0);
        intmap.insert(WI_CHAT_DBLCLICK_ACT, 0);//nick in chat
        intmap.insert(WI_CHAT_MDLCLICK_ACT, 1);//browse files
        intmap.insert(WI_CHAT_FIND_COLOR_ALPHA, 127);
        intmap.insert(WI_MAINWINDOW_HEIGHT, -1);
        intmap.insert(WI_MAINWINDOW_WIDTH, -1);
        intmap.insert(WI_MAINWINDOW_X, -1);
        intmap.insert(WI_MAINWINDOW_Y, -1);
        intmap.insert(WI_SEARCH_SORT_COLUMN, 1);
        intmap.insert(WI_SEARCH_SORT_ORDER, 0);
        intmap.insert(WI_SEARCH_SHARED_ACTION, 0);
        intmap.insert(WI_SEARCH_LAST_TYPE, 0);
        intmap.insert(WI_TEXT_EDIT_HEIGHT, 35);
        intmap.insert(WI_TRANSFER_HEIGHT, -1);
        intmap.insert(WI_SHARE_RPANE_WIDTH, -1);
        intmap.insert(WI_SHARE_WIDTH, -1);
        intmap.insert(WI_NOTIFY_EVENTMAP, 0x0B);// 0b00001011, (transfer done = off)
        intmap.insert(WI_NOTIFY_MODULE, 0);//default
        intmap.insert(WI_NOTIFY_SNDMAP, 0x0F);// 0b00001111, all events
        intmap.insert(WI_OUT_IN_HIST, 50);//number of output messages in history
    }
}

WulforSettings::~WulforSettings(){
}

void WulforSettings::load(){
    if (!QFile::exists(configFile))
        return;

    try{
        SimpleXML xml;
        xml.fromXML(File(configFile.toStdString(), File::READ, File::OPEN).read());
        xml.resetCurrentChild();
        xml.stepIn();

        if (xml.findChild("Settings")){

            xml.stepIn();

            WStrMap::iterator it = strmap.begin();

            for (; it != strmap.end(); ++it){
                if (xml.findChild(it.key().toStdString()))
                    setStr(it.key(), QString::fromStdString(xml.getChildData()));

                xml.resetCurrentChild();
            }

            WIntMap::iterator iit = intmap.begin();

            for (; iit != intmap.end(); ++iit){
                if (xml.findChild(iit.key().toStdString()))
                    setInt(iit.key(), Util::toInt(xml.getChildData()));

                xml.resetCurrentChild();
            }
        }
    }
    catch (Exception ex){
    }
}

void WulforSettings::save(){
    SimpleXML xml;
    xml.addTag("EiskaltDC++");
    xml.stepIn();
    xml.addTag("Settings");
    xml.stepIn();

    WStrMap::iterator it = strmap.begin();

    for (; it != strmap.end(); ++it){
        xml.addTag(it.key().toStdString(), it.value().toStdString());
        xml.addChildAttrib(string("type"), string("string"));
    }

    WIntMap::iterator iit = intmap.begin();

    for (; iit != intmap.end(); ++iit){
        xml.addTag(iit.key().toStdString(), QString().setNum(iit.value()).toStdString());
        xml.addChildAttrib(string("type"), string("int"));
    }

    xml.stepOut();

    try{
        File out(QString(configFile + ".tmp").toStdString(), File::WRITE, File::CREATE | File::TRUNCATE);
        BufferedOutputStream<false> f(&out);

        f.write(SimpleXML::utf8Header);
        xml.toXML(&f);
        f.flush();

        out.close();

        File::deleteFile(configFile.toStdString());
        File::renameFile(configFile.toStdString() + ".tmp", configFile.toStdString());
    }
    catch (const FileException &){}
}

void WulforSettings::parseCmd(const QString &cmd){
    QStringList args = cmd.split(" ", QString::SkipEmptyParts);

    if (args.size() != 2)
        return;

    QString sname   = args.at(0);
    QString svalue  = args.at(1);

    if (intmap.contains(sname)){
        bool ok = false;

        int val = svalue.toInt(&ok);

        if (ok)
            intmap[sname] = val;
    }
    else if (strmap.contains(sname)){
        strmap[sname] = svalue;
    }
}

void WulforSettings::loadTranslation(){
    QString file = getStr(WS_TRANSLATION_FILE);

    if (file.isEmpty() || !QFile::exists(file)){
        QString lc_prefix = QLocale::system().name();

        file = QString(CLIENT_TRANSLATIONS_DIR) + QDir::separator();
        lc_prefix = lc_prefix.left(lc_prefix.indexOf("_"));
        file += "eiskaltdcpp." + lc_prefix + ".qm";

        if (!QFile::exists(file))
            return;
    }

    if (tor.load(file))
        qApp->installTranslator(&tor);
    else
        WSSET(WS_TRANSLATION_FILE, "");
}

void WulforSettings::loadTheme(){
    if (!getStr(WS_APP_THEME).isEmpty()){
        printf("Setting up new theme: %s\n", getStr(WS_APP_THEME).toAscii().constData());
        qApp->setStyle(getStr(WS_APP_THEME));
    }

    if (!getStr(WS_APP_FONT).isEmpty() && f.fromString(getStr(WS_APP_FONT))){
        printf("Setting up new font: %s\n", f.toString().toAscii().constData());
        qApp->setFont(f);
    }
}

QString WulforSettings::getStr(QString key) throw (WulforSettings::BadKey){
    if (!strmap.contains(key))
        throw BadKey();

    return strmap.value(key);
}

int WulforSettings::getInt(QString key) throw (WulforSettings::BadKey){
    if (!intmap.contains(key))
       throw BadKey();

    return intmap.value(key);
}

bool WulforSettings::getBool(QString key) throw (WulforSettings::BadKey){
    return ((bool)getInt(key));
}

void WulforSettings::setStr(QString key, QString value) throw (WulforSettings::BadKey){
    if (!strmap.contains(key))
        throw BadKey();

    strmap[key] = value;
}

void WulforSettings::setInt(QString key, int value) throw (WulforSettings::BadKey){
    if (!intmap.contains(key))
       throw BadKey();

    intmap[key] = value;
}

void WulforSettings::setBool(QString key, bool value) throw (WulforSettings::BadKey){
    setInt(key, (int)value);
}
