#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"

#include "dcpp/forward.h"
#include "dcpp/QueueManager.h"

#include "WulforManager.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "UPnP.h"
#include "UPnPMapper.h"
#include "HubManager.h"

#include <QApplication>
#include <QMainWindow>

#include "MainWindow.h"

void callBack(void* x, const std::string& a)
{
	std::cout << "Loading: " << a << std::endl;
}

int main(int argc, char *argv[])
{
        QApplication app(argc, argv);
        int ret = 0;

	dcpp::startup(callBack, NULL);
        dcpp::TimerManager::getInstance()->start();

        WulforManager::newInstance();
        WulforManager::getInstance()->start();

        WulforUtil::newInstance();

        WulforSettings::newInstance();
        WulforSettings::getInstance()->load();
        WulforSettings::getInstance()->loadTranslation();

        if (WulforUtil::getInstance()->loadUserIcons())
            std::cout << "UserList icons has been loaded" << std::endl;


        if (WulforUtil::getInstance()->loadIcons())
            std::cout << "Application icons has been loaded" << std::endl;

        UPnP::newInstance();
        UPnP::getInstance()->start();
        UPnPMapper::newInstance();

        HubManager::newInstance();
	
        MainWindow::newInstance();
        MainWindow::getInstance()->autoconnect();
        MainWindow::getInstance()->show();

        ret = app.exec();

        WulforSettings::getInstance()->save();
        WulforManager::getInstance()->stop();

        UPnPMapper::deleteInstance();
        UPnP::getInstance()->stop();
        UPnP::deleteInstance();

        MainWindow::deleteInstance();

        HubManager::deleteInstance();

        WulforManager::deleteInstance();
        WulforUtil::deleteInstance();
        WulforSettings::deleteInstance();

        dcpp::shutdown();

        return ret;
}
