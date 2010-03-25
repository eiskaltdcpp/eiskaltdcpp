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

#include "WulforUtil.h"
#include "WulforSettings.h"
#include "UPnP.h"
#include "UPnPMapper.h"
#include "HubManager.h"
#include "Notification.h"
#include "SingleInstanceRunner.h"
#include "Version.h"
#include "EmoticonFactory.h"

#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QtDBus>

#include "MainWindow.h"

void callBack(void* x, const std::string& a)
{
    std::cout << "Loading: " << a << std::endl;
}

void parseCmdLine(const QStringList &);

#ifndef WIN32
#include <unistd.h>
#include <signal.h>

void installHandlers();
#endif

int main(int argc, char *argv[])
{
    EiskaltApp app(argc, argv);
    int ret = 0;

    parseCmdLine(qApp->arguments());

    SingleInstanceRunner runner;

    if (runner.isServerRunning(qApp->arguments()))
        return 0;

    dcpp::startup(callBack, NULL);
    dcpp::TimerManager::getInstance()->start();

    installHandlers();

    HashManager::getInstance()->setPriority(Thread::IDLE);

    WulforSettings::newInstance();
    WulforSettings::getInstance()->load();
    WulforSettings::getInstance()->loadTranslation();
    WulforSettings::getInstance()->loadTheme();

    WulforUtil::newInstance();

    if (WulforUtil::getInstance()->loadUserIcons())
        std::cout << "UserList icons has been loaded" << std::endl;

    if (WulforUtil::getInstance()->loadIcons())
        std::cout << "Application icons has been loaded" << std::endl;

    UPnP::newInstance();
    UPnP::getInstance()->start();
    UPnPMapper::newInstance();

    HubManager::newInstance();

    MainWindow::newInstance();
    MainWindow::getInstance()->setUnload(!WBGET(WB_TRAY_ENABLED));

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
    Notification::getInstance()->enableTray(WBGET(WB_TRAY_ENABLED));

    MainWindow::getInstance()->autoconnect();
    MainWindow::getInstance()->show();
    MainWindow::getInstance()->parseCmdLine();

    if (WBGET(WB_MAINWINDOW_HIDE))
        MainWindow::getInstance()->hide();

    ret = app.exec();

    std::cout << "Shutting down..." << std::endl;

    WulforSettings::getInstance()->save();

    EmoticonFactory::deleteInstance();

#ifdef USE_ASPELL
    if (SpellCheck::getInstance())
        SpellCheck::deleteInstance();
#endif

    UPnPMapper::deleteInstance();
    UPnP::getInstance()->stop();
    UPnP::deleteInstance();

    Notification::deleteInstance();

    MainWindow::deleteInstance();

    HubManager::deleteInstance();

    WulforUtil::deleteInstance();
    WulforSettings::deleteInstance();

    dcpp::shutdown();

    runner.servStop();

    return ret;
}

void parseCmdLine(const QStringList &args){
    foreach (QString arg, args){
        if (arg == "-h" || arg == "--help"){
            QString msg = QApplication::tr("Using:\n"
                            "  eiskaltdcpp <magnet link> <dchub://link> <adc(s)://link>\n"
                            "  eiskaltdcpp <Key>\n"
                            "EiskaltDC++ is a program for UNIX-like systems that uses the Direct Connect and ADC protocol.\n"
                            "\n"
                            "Keys:\n"
                            "  -h, --help\t Show this message\n"
                            "  -v, --version\t Show version string"
                            );

            std::cout << msg.toAscii().constData() << std::endl;

            exit(0);
        }
        else if (arg == "-v" || arg == "--version"){
#ifndef DCPP_REVISION
            printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
#else
            printf("%s - %s %s \n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX, DCPP_REVISION);
#endif
            exit(0);
        }
    }
}

#ifndef WIN32
void installHandlers(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1){
        std::cout << "Cannot handle SIGPIPE" << std::endl;
    }

    std::cout << "Signal handlers installed." << std::endl;
}

#endif

