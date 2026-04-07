/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#ifdef BUILD_STATIC
#include <QtPlugin>
#if defined(_WIN32)
Q_IMPORT_PLUGIN (QWindowsAudioPlugin);
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
Q_IMPORT_PLUGIN (QSQLiteDriverPlugin);
Q_IMPORT_PLUGIN (QWindowsVistaStylePlugin);
#elif defined(__linux) // defined(_WIN32)
Q_IMPORT_PLUGIN (QXcbIntegrationPlugin);
Q_IMPORT_PLUGIN (QSQLiteDriverPlugin);
#endif // defined(_WIN32)
#endif // BUILD_STATIC

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
#include "QtContext.h"
#include "QtContextAware.h"
#include "HubManager.h"
#include "Notification.h"
#include "VersionGlobal.h"
#include "EmoticonFactory.h"
#include "FinishedTransfers.h"
#include "QueuedUsers.h"
#include "ArenaWidgetManager.h"
#include "ArenaWidgetFactory.h"
#include "MainWindow.h"
#include "GlobalTimer.h"

#if defined(Q_OS_HAIKU)
#include "EiskaltApp_haiku.h"
#elif defined(Q_OS_MAC)
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
#include <QRegularExpression>
#include <QObject>
#include <QScopeGuard>

#ifdef DBUS_NOTIFY
#include <QtDBus>
#endif

void callBack(void *, const std::string &a)
{
    std::cout << QObject::tr("Loading: ").toStdString() << a << std::endl;
}

void parseCmdLine(const QStringList &);

#if !defined(Q_OS_WIN)
#include <unistd.h>
#include <signal.h>
#if !defined (Q_OS_HAIKU) && defined (__GLIBC__)
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
#include <windows.h>
#include <string>
#include <sstream>

/**
 * Show a diagnostic MessageBox when the Qt GUI app fails to start.
 * WIN32 subsystem apps have no console, so a missing DLL or early
 * crash is completely silent without explicit error reporting.
 */
static LONG WINAPI earlyExceptionHandler(EXCEPTION_POINTERS *ep)
{
    char buf[512];
    snprintf(buf, sizeof(buf),
             "EiskaltDC++ crashed during startup.\n\n"
             "Exception code: 0x%08lX\nAddress: %p\n\n"
             "This may indicate a missing DLL or incompatible library.\n"
             "Please report this to the developers.",
             ep->ExceptionRecord->ExceptionCode,
             ep->ExceptionRecord->ExceptionAddress);
    MessageBoxA(nullptr, buf, "EiskaltDC++ — Fatal Error",
                MB_OK | MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#if defined(Q_OS_MAC)
#include <objc/objc.h>
#include <objc/message.h>

bool dockClickHandler(id self,SEL _cmd,...)
{
    Q_UNUSED(self)
    Q_UNUSED(_cmd)
    Notification *N = qtCtx()->notification();
    if (N)
        N->slotShowHide();
    return true;
}
#endif

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    // Install an early crash handler so WIN32 subsystem apps don't
    // die silently when a DLL is missing or initialization fails.
    SetUnhandledExceptionFilter(earlyExceptionHandler);
#endif

    setlocale(LC_ALL, "");

    EiskaltApp app(argc, argv, _q(dcpp::Util::getLoginName()+"EDCPP"));
    int ret = 0;

    parseCmdLine(app.arguments());

    if (app.isRunning()){
        QStringList args = app.arguments();
        args.removeFirst();//remove path to executable
#if !defined (Q_OS_HAIKU)
        app.sendMessage(args.join("\n"));
#endif
        return 0;
    }

#if !defined (Q_OS_WIN) && !defined (Q_OS_HAIKU) && defined (__GLIBC__)
    installHandlers();
#endif

#if defined(FORCE_XDG) && !defined(Q_OS_WIN)
    migrateConfig();
#endif

    auto dcContext = dcpp::startup(callBack, nullptr);
    dcContext->getTimerManager()->start();

    dcContext->getHashManager()->setPriority(Thread::IDLE);

    app.setOrganizationName("EiskaltDC++ Team");
    app.setApplicationName("EiskaltDC++ Qt");
    app.setApplicationVersion(QString::fromStdString(eiskaltdcppVersionString));
    
    { // Begin Qt-widget scope: everything inside is destroyed before dcpp shutdown
    QtContext ctx(*dcContext);
    // Guard: ensure settings are saved on scope exit.
    // The guard runs before ~QtContext.
    auto cleanupGuard = qScopeGuard([&]() {
        qtCtx()->settings()->save();
    });

    ctx.createGlobalTimer();
    ctx.createSettings();

    ctx.settings()->load();
    ctx.settings()->loadTheme();

    ctx.createWulforUtil();
    ctx.settings()->loadTranslation();
#if defined(Q_OS_MAC)
    // Disable system tray functionality in Mac OS X:
    qtCtx()->settings()->setBool(WB_TRAY_ENABLED, false);
#endif

    Text::hubDefaultCharset = qtCtx()->wulforUtil()->qtEnc2DcEnc(qtCtx()->settings()->getStr(WS_DEFAULT_LOCALE)).toStdString();
    // Safety: if the conversion returned an empty string (should not happen
    // after the qtEnc2DcEnc fix, but guard against it), fall back to the
    // system charset so NMDC hubs get a real encoding for iconv.
    if (Text::hubDefaultCharset.empty())
        Text::hubDefaultCharset = Text::systemCharset;

    if (qtCtx()->wulforUtil()->loadUserIcons())
        std::cout << QObject::tr("UserList icons has been loaded").toStdString() << std::endl;

    if (qtCtx()->wulforUtil()->loadIcons())
        std::cout << QObject::tr("Application icons has been loaded").toStdString() << std::endl;

    app.setWindowIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiICON_APPL));

    ctx.createArenaWidgetManager();

    ctx.createMainWindow();
#if defined(Q_OS_MAC)
    qtCtx()->mainWindow()->setUnload(false);
    QObject::connect(&app, &EiskaltApp::clickedOnDock,
                     qtCtx()->mainWindow(), &MainWindow::show);
#else // defined(Q_OS_MAC)
    qtCtx()->mainWindow()->setUnload(!qtCtx()->settings()->getBool(WB_TRAY_ENABLED));
#endif // defined(Q_OS_MAC)

    QObject::connect(&app, &QtSingleCoreApplication::messageReceived, qtCtx()->mainWindow(), &MainWindow::parseInstanceLine);

    ctx.createHubManager();

    qtCtx()->settings()->loadTheme();

    if (qtCtx()->settings()->getBool(WB_APP_ENABLE_EMOTICON)){
        ctx.createEmoticonFactory();
        qtCtx()->emoticonFactory()->load();
    }

#ifdef USE_ASPELL
    if (qtCtx()->settings()->getBool(WB_APP_ENABLE_ASPELL))
        ctx.createSpellCheck();
#endif

    ctx.createNotification();

#ifdef USE_JS
    ctx.createScriptEngine();
    QObject::connect(qtCtx()->scriptEngine(), SIGNAL(scriptChanged(QString)), qtCtx()->mainWindow(), SLOT(slotJSFileChanged(QString)));
#endif

    ctx.createFinishedUploads();
    qtCtx()->arenaWidgetManager()->add(ctx.finishedUploads());
    ctx.createFinishedDownloads();
    qtCtx()->arenaWidgetManager()->add(ctx.finishedDownloads());
    ctx.createQueuedUsers();
    qtCtx()->arenaWidgetManager()->add(ctx.queuedUsers());

    qtCtx()->mainWindow()->autoconnect();
    qtCtx()->mainWindow()->parseCmdLine(app.arguments());

    if (!qtCtx()->settings()->getBool(WB_MAINWINDOW_HIDE) || !qtCtx()->settings()->getBool(WB_TRAY_ENABLED))
        qtCtx()->mainWindow()->show();

    ret = app.exec();

    std::cout << QObject::tr("Shutting down libeiskaltdcpp...").toStdString() << std::endl;

    // Destruction order (reverse of declaration):
    //   1. cleanupGuard → saves settings
    //   2. ~QtContext   → destroys ScriptEngine then all other
    //      Qt widgets, and deregisters the process-wide context.
    }

    dcContext->shutdown();
    dcContext.reset();
    dcpp::setContext(nullptr);

    std::cout << QObject::tr("Quit...").toStdString() << std::endl;

    return ret;
}

void parseCmdLine(const QStringList &args){
    for (const auto &arg : args){
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

#if !defined (Q_OS_WIN) && !defined (Q_OS_HAIKU)

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

    if (sigaction(SIGPIPE, &sa, nullptr) == -1)
        printf("Cannot handle SIGPIPE\n");
    else {
        sigset_t set;
        sigemptyset (&set);
        sigaddset (&set, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &set, nullptr);
    }

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

    for (const auto &s : from.entryList(QDir::Dirs)){
        QDir new_dir(to_path+s);

        if (new_dir.exists())
            continue;
        else{
            if (!new_dir.mkpath(new_dir.absolutePath()))
                continue;

            copy(QDir(from_path+s), new_dir);
        }
    }

    for (const auto &f : from.entryList(QDir::Files)){
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

        QRegularExpression replace_str("/(\\S+)/\\.eiskaltdc\\+\\+/");
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
