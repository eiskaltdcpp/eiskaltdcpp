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
#include "dcpp/Singleton.h"

#include "WulforUtil.h"
#include "WulforSettings.h"
#include "HubManager.h"
#include "Notification.h"
#include "VersionGlobal.h"
#include "IPFilter.h"
#include "EmoticonFactory.h"
#include "FinishedTransfers.h"
#include "QueuedUsers.h"
#include "ArenaWidgetManager.h"
#include "ArenaWidgetFactory.h"
#include "MainWindow.h"
#include "GlobalTimer.h"

#if defined (__HAIKU__)
#include "EiskaltApp_haiku.h"
#elif defined(Q_WS_MAC)
#include "EiskaltApp_mac.h"
#else
#include "EiskaltApp.h"
#endif

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
#include <QTextCodec>

#ifdef DBUS_NOTIFY
#include <QtDBus>
#endif

void callBack(void* x, const std::string& a)
{
    std::cout << QObject::tr("Loading: ").toStdString() << a << std::endl;
}

void parseCmdLine(const QStringList &);

#if !defined(Q_WS_WIN)
#include <unistd.h>
#include <signal.h>
#if !defined (__HAIKU__)
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

#else //WIN32
#include <locale.h>
#endif

#if defined(Q_WS_MAC)
#include <objc/objc.h>
#include <objc/message.h>

bool dockClickHandler(id self,SEL _cmd,...)
{
    Q_UNUSED(self)
    Q_UNUSED(_cmd)
    Notification *N = Notification::getInstance();
    if (N)
        N->slotShowHide();
    return true;
}
#endif

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    setlocale(LC_ALL, "");

    EiskaltApp app(argc, argv, _q(dcpp::Util::getLoginName()+"EDCPP"));
    int ret = 0;

    parseCmdLine(app.arguments());

    if (app.isRunning()){
        QStringList args = app.arguments();
        args.removeFirst();//remove path to executable
#if !defined (__HAIKU__)
        app.sendMessage(args.join("\n"));
#endif
        return 0;
    }

#if !defined (Q_WS_WIN) && !defined (__HAIKU__)
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
    
    GlobalTimer::newInstance();

    WulforSettings::newInstance();
    WulforSettings::getInstance()->load();
    WulforSettings::getInstance()->loadTranslation();
    WulforSettings::getInstance()->loadTheme();

    WulforUtil::newInstance();
#if defined(Q_WS_MAC)
    // Disable system tray functionality in Mac OS X:
    WBSET(WB_TRAY_ENABLED, false);
#endif

    Text::hubDefaultCharset = WulforUtil::getInstance()->qtEnc2DcEnc(WSGET(WS_DEFAULT_LOCALE)).toStdString();

    if (WulforUtil::getInstance()->loadUserIcons())
        std::cout << QObject::tr("UserList icons has been loaded").toStdString() << std::endl;

    if (WulforUtil::getInstance()->loadIcons())
        std::cout << QObject::tr("Application icons has been loaded").toStdString() << std::endl;

    app.setWindowIcon(WICON(WulforUtil::eiICON_APPL));
    
    ArenaWidgetManager::newInstance();

    MainWindow::newInstance();
#if defined(Q_WS_MAC)
    MainWindow::getInstance()->setUnload(false);
#else // defined(Q_WS_MAC)
    MainWindow::getInstance()->setUnload(!WBGET(WB_TRAY_ENABLED));
#endif // defined(Q_WS_MAC)

    app.connect(&app, SIGNAL(messageReceived(QString)), MainWindow::getInstance(), SLOT(parseInstanceLine(QString)));

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

#ifdef USE_JS
    ScriptEngine::newInstance();
    QObject::connect(ScriptEngine::getInstance(), SIGNAL(scriptChanged(QString)), MainWindow::getInstance(), SLOT(slotJSFileChanged(QString)));
#endif

    ArenaWidgetFactory().create< dcpp::Singleton, FinishedUploads >();
    ArenaWidgetFactory().create< dcpp::Singleton, FinishedDownloads >();
    ArenaWidgetFactory().create< dcpp::Singleton, QueuedUsers >();

    MainWindow::getInstance()->autoconnect();
    MainWindow::getInstance()->parseCmdLine(app.arguments());

    if (!WBGET(WB_MAINWINDOW_HIDE) || !WBGET(WB_TRAY_ENABLED))
        MainWindow::getInstance()->show();

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

    GlobalTimer::deleteInstance();
    
    ArenaWidgetManager::deleteInstance();
    
    HubManager::getInstance()->release();

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
    foreach (const QString &arg, args){
        if (arg == "-h" || arg == "--help"){
            About().printHelp();

            exit(0);
        }
        else if (arg == "-V" || arg == "--version"){
            About().printVersion();

            exit(0);
        }
    }
}

#if !defined (Q_WS_WIN) && !defined (__HAIKU__)

void catchSIG(int sigNum) {
    psignal(sigNum, "Catching signal ");

#ifdef ENABLE_STACKTRACE
    printBacktrace(sigNum);
#endif // ENABLE_STACKTRACE
    
    EiskaltApp *eapp = dynamic_cast<EiskaltApp*>(qApp);
    
    if (eapp) {
        eapp->getSharedMemory().unlock();
        eapp->getSharedMemory().detach();
    }
    
    raise(SIGINT);
    
    std::abort();
}

template <int sigNum = 0, int ... Params>
void catchSignals() {
    if (!sigNum)
        return;

    psignal(sigNum, "Installing handler for");

    signal(sigNum, catchSIG);

    catchSignals<Params ... >();
}

void installHandlers(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
        printf("Cannot handle SIGPIPE\n");

    catchSignals<SIGSEGV, SIGABRT, SIGBUS, SIGTERM>();

    printf("Signal handlers installed.\n");
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

    foreach (const QString &s, from.entryList(QDir::Dirs)){
        QDir new_dir(to_path+s);

        if (new_dir.exists())
            continue;
        else{
            if (!new_dir.mkpath(new_dir.absolutePath()))
                continue;

            copy(QDir(from_path+s), new_dir);
        }
    }

    foreach (const QString &f, from.entryList(QDir::Files)){
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
