/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"

#include "dcpp/forward.h"
#include "dcpp/QueueManager.h"
#include "dcpp/HashManager.h"
#include "dcpp/Thread.h"
#include "MainWindow.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "HubManager.h"
#include "Notification.h"
#include "VersionGlobal.h"
#include "IPFilter.h"
#include "EmoticonFactory.h"
#include "FinishedTransfers.h"
#include "QueuedUsers.h"

#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif

#ifdef USE_JS
#include "ScriptEngine.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QRegExp>
#include <QObject>

#ifdef DBUS_NOTIFY
#include <QtDBus>
#endif

void callBack(void* x, const std::string& a)
{
    std::cout << QObject::tr("Loading: ").toStdString() << a << std::endl;
}

void parseCmdLine(const QStringList &);

#ifndef Q_WS_WIN
#include <unistd.h>
#include <signal.h>
#ifndef __HAIKU__
#include <execinfo.h>

#ifdef ENABLE_STACKTRACE
#include "extra/stacktrace.h"
#endif // ENABLE_STACKTRACE

void installHandlers();
#endif

#ifdef FORCE_XDG
#include <QTextStream>
void migrateConfig();
#endif

#else//WIN32
#include <locale.h>
#endif

int main(int argc, char *argv[])
{
    EiskaltApp app(argc, argv);
    int ret = 0;

    parseCmdLine(app.arguments());

#if !defined (Q_WS_HAIKU)
    if (app.isRunning()){
        app.sendMessage(app.arguments().join("\n"));

        return 0;
    }
#endif

    setlocale(LC_ALL, "");

#if !defined (Q_WS_WIN) && !defined (Q_WS_HAIKU)
    installHandlers();
#endif

#if defined(FORCE_XDG) && !defined(Q_WS_WIN)
    migrateConfig();
#endif

    dcpp::startup(callBack, NULL);
    dcpp::TimerManager::getInstance()->start();

    HashManager::getInstance()->setPriority(Thread::IDLE);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    app.setOrganizationName("EiskaltDC++ Team");
    app.setApplicationName("EiskaltDC++ Qt");
    app.setApplicationVersion(EISKALTDCPP_VERSION);

    WulforSettings::newInstance();
    WulforSettings::getInstance()->load();
    WulforSettings::getInstance()->loadTranslation();
    WulforSettings::getInstance()->loadTheme();

    WulforUtil::newInstance();

    Text::hubDefaultCharset = WulforUtil::getInstance()->qtEnc2DcEnc(WSGET(WS_DEFAULT_LOCALE)).toStdString();

    if (WulforUtil::getInstance()->loadUserIcons())
        std::cout << QObject::tr("UserList icons has been loaded").toStdString() << std::endl;

    if (WulforUtil::getInstance()->loadIcons())
        std::cout << QObject::tr("Application icons has been loaded").toStdString() << std::endl;

    app.setWindowIcon(WICON(WulforUtil::eiICON_APPL));

    MainWindow::newInstance();
    MainWindow::getInstance()->setUnload(!WBGET(WB_TRAY_ENABLED));

    app.connect(&app, SIGNAL(messageReceived(QString)), MainWindow::getInstance(), SLOT(parseInstanceLine(QString)));
    app.setActivationWindow(MainWindow::getInstance(), true);

    HubManager::newInstance();

    WulforSettings::getInstance()->loadTheme();

    if (WBGET(WB_APP_ENABLE_EMOTICON)){
        EmoticonFactory::newInstance();
        EmoticonFactory::getInstance()->load();
    }

#ifdef USE_ASPELL
    if (WBGET(WB_APP_ENABLE_ASPELL))
        SpellCheck::newInstance();
#endif

    Notification::newInstance();

    if (!WBGET(WB_MAINWINDOW_HIDE) || !WBGET(WB_TRAY_ENABLED))
        MainWindow::getInstance()->show();

    MainWindow::getInstance()->autoconnect();
    MainWindow::getInstance()->parseCmdLine();

#ifdef USE_JS
    ScriptEngine::newInstance();
#endif

    FinishedUploads::newInstance();
    FinishedDownloads::newInstance();
    QueuedUsers::newInstance();

    ret = app.exec();

    std::cout << QObject::tr("Shutting down libdcpp...").toStdString() << std::endl;

    WulforSettings::getInstance()->save();

    EmoticonFactory::deleteInstance();

#ifdef USE_ASPELL
    if (SpellCheck::getInstance())
        SpellCheck::deleteInstance();
#endif
    Notification::deleteInstance();

#ifdef USE_JS
    ScriptEngine::deleteInstance();
#endif

    MainWindow::deleteInstance();

    WulforUtil::deleteInstance();
    WulforSettings::deleteInstance();

    dcpp::shutdown();

    if (IPFilter::getInstance()){
        IPFilter::getInstance()->saveList();
        IPFilter::deleteInstance();
    }

    std::cout << QObject::tr("Quit...").toStdString() << std::endl;

    return ret;
}

void parseCmdLine(const QStringList &args){
    foreach (QString arg, args){
        if (arg == "-h" || arg == "--help"){
            About().printHelp();

            exit(0);
        }
        else if (arg == "-v" || arg == "--version"){
            About().printVersion();

            exit(0);
        }
    }
}

#if !defined (Q_WS_WIN) && !defined (Q_WS_HAIKU)

void installHandlers(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1){
        std::cout << QObject::tr("Cannot handle SIGPIPE").toStdString() << std::endl;
    }

#ifdef ENABLE_STACKTRACE
    signal(SIGSEGV, printBacktrace);
#endif // ENABLE_STACKTRACE

    std::cout << QObject::tr("Signal handlers installed.").toStdString() << std::endl;
}

#endif

#ifdef FORCE_XDG

void copy(const QDir &from, const QDir &to){
    if (!from.exists() || to.exists())
        return;

    QString to_path = to.absolutePath();
    QString from_path = from.absolutePath();

    if (!to_path.endsWith(QDir::separator()))
        to_path += QDir::separator();

    if (!from_path.endsWith(QDir::separator()))
        from_path += QDir::separator();

    foreach (QString s, from.entryList(QDir::Dirs)){
        QDir new_dir(to_path+s);

        if (new_dir.exists())
            continue;
        else{
            if (!new_dir.mkpath(new_dir.absolutePath()))
                continue;

            copy(QDir(from_path+s), new_dir);
        }
    }

    foreach (QString f, from.entryList(QDir::Files)){
        QFile orig(from_path+f);

        if (!orig.copy(to_path+f))
            continue;
    }
}

void migrateConfig(){
    const char* home_ = getenv("HOME");
    string home = home_ ? Text::toUtf8(home_) : "/tmp/";
    string old_config = home + "/.eiskaltdc++/";

    const char *xdg_config_home_ = getenv("XDG_CONFIG_HOME");
    string xdg_config_home = xdg_config_home_? Text::toUtf8(xdg_config_home_) : (home+"/.config");
    string new_config = xdg_config_home + "/eiskaltdc++/";

    if (!QDir().exists(old_config.c_str()) || QDir().exists(new_config.c_str())){
        if (!QDir().exists(new_config.c_str())){
            old_config = _DATADIR + string("/config/");

            if (!QDir().exists(old_config.c_str()))
                return;
        }
        else
            return;
    }

    try{
        printf("Migrating to XDG paths...\n");

        copy(QDir(old_config.c_str()), QDir(new_config.c_str()));

        QFile orig(new_config.c_str()+QString("DCPlusPlus.xml"));
        QFile new_file(new_config.c_str()+QString("DCPlusPlus.xml.new"));

        if (!(orig.open(QIODevice::ReadOnly | QIODevice::Text) && new_file.open(QIODevice::WriteOnly | QIODevice::Text))){
            orig.close();
            new_file.close();

            printf("Migration failed.\n");

            return;
        }

        QTextStream rstream(&orig);
        QTextStream wstream(&new_file);

        QRegExp replace_str("/(\\S+)/\\.eiskaltdc\\+\\+/");
        QString line = "";

        while (!rstream.atEnd()){
            line = rstream.readLine();

            line.replace(replace_str, QString(new_config.c_str()));

            wstream << line << "\n";
        }

        wstream.flush();

        orig.close();
        new_file.close();

        orig.remove();
        new_file.rename(orig.fileName());

        printf("Ok. Migrated.\n");
    }
    catch(const std::exception&){
        printf("Migration failed.\n");
    }
}
#endif
