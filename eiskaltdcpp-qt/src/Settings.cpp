/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "Settings.h"
#include "SettingsPersonal.h"
#include "SettingsConnection.h"
#include "SettingsDownloads.h"
#include "SettingsSharing.h"
#include "SettingsGUI.h"
#include "SettingsNotification.h"
#include "SettingsLog.h"
#include "SettingsUC.h"
#include "SettingsShortcuts.h"
#include "SettingsHistory.h"

#include "WulforUtil.h"

#include <QTableWidget>

Settings::Settings(): is_dirty(false)
{
    setupUi(this);

    init();

    setWindowTitle(tr("Preferences"));
}

Settings::~Settings(){
    if (is_dirty){
        WulforSettings::getInstance()->save();
        SettingsManager::getInstance()->save();
    }
}

void Settings::init(){
    WulforUtil *WU = WulforUtil::getInstance();

    QListWidgetItem *item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiUSERS), tr("Personal"), listWidget);
    SettingsPersonal *personal = new SettingsPersonal(this);
    connect(this, SIGNAL(timeToDie()), personal, SLOT(ok()));
    widgets.insert(item, (int)Page::Personal);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiCONNECT), tr("Connection"), listWidget);
    SettingsConnection *connection = new SettingsConnection(this);
    connect(this, SIGNAL(timeToDie()), connection, SLOT(ok()));
    widgets.insert(item, (int)Page::Connection);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiDOWNLOAD), tr("Downloads"), listWidget);
    SettingsDownloads *downloads = new SettingsDownloads(this);
    connect(this, SIGNAL(timeToDie()), downloads, SLOT(ok()));
    widgets.insert(item, (int)Page::Downloads);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Sharing"), listWidget);
    SettingsSharing *sharing = new SettingsSharing(this);
    connect(this, SIGNAL(timeToDie()), sharing, SLOT(ok()));
    widgets.insert(item, (int)Page::Sharing);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiGUI), tr("GUI"), listWidget);
    SettingsGUI *gui = new SettingsGUI(this);
    connect(this, SIGNAL(timeToDie()), gui, SLOT(ok()));
    widgets.insert(item, (int)Page::GUI);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiMESSAGE), tr("Notifications"), listWidget);
    SettingsNotification *notify = new SettingsNotification(this);
    connect(this, SIGNAL(timeToDie()), notify, SLOT(ok()));
    widgets.insert(item, (int)Page::Notifications);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE), tr("Logs"), listWidget);
    SettingsLog *logs = new SettingsLog(this);
    connect(this, SIGNAL(timeToDie()), logs, SLOT(ok()));
    widgets.insert(item, (int)Page::Logs);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiUSERS), tr("User Commands"), listWidget);
    SettingsUC *ucs = new SettingsUC(this);
    connect(this, SIGNAL(timeToDie()), ucs, SLOT(ok()));
    widgets.insert(item, (int)Page::UserCommands);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiEDIT), tr("Shortcuts"), listWidget);
    SettingsShortcuts *sshs = new SettingsShortcuts(this);
    connect(this, SIGNAL(timeToDie()), sshs, SLOT(ok()));
    widgets.insert(item, (int)Page::Shortcuts);
    
    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiEDIT), tr("History"), listWidget);
    SettingsHistory *shist = new SettingsHistory(this);
    connect(this, SIGNAL(timeToDie()), shist, SLOT(ok()));
    widgets.insert(item, (int)Page::History);

    stackedWidget->insertWidget((int)Page::Personal, personal);
    stackedWidget->insertWidget((int)Page::Connection, connection);
    stackedWidget->insertWidget((int)Page::Downloads, downloads);
    stackedWidget->insertWidget((int)Page::Sharing, sharing);
    stackedWidget->insertWidget((int)Page::GUI, gui);
    stackedWidget->insertWidget((int)Page::Notifications, notify);
    stackedWidget->insertWidget((int)Page::Logs, logs);
    stackedWidget->insertWidget((int)Page::UserCommands, ucs);
    stackedWidget->insertWidget((int)Page::Shortcuts, sshs);
    stackedWidget->insertWidget((int)Page::History, shist);

    stackedWidget->setCurrentIndex(0);

    if (WVGET("settings/dialog-size").isValid())
        resize(WVGET("settings/dialog-size").toSize());

    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotItemActivated(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemActivated(QListWidgetItem*)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(this, SIGNAL(accepted()), this, SIGNAL(timeToDie()));
    connect(this, SIGNAL(accepted()), this, SLOT(dirty()));
}

void Settings::slotItemActivated(QListWidgetItem *item){
    if (widgets.contains(item)){
        stackedWidget->setCurrentIndex(widgets[item]);
    }
}

void Settings::dirty(){
    is_dirty = true;

    WVSET("settings/dialog-size", size());
}

void Settings::navigate(enum Settings::Page pg, int tab){
    listWidget->setCurrentRow((int)pg);
    stackedWidget->setCurrentIndex((int)pg);

    if (tab < 0)
        return;

    QWidget *wgt = stackedWidget->currentWidget();
    QTabWidget *tabwgt = wgt->findChild<QTabWidget*>("tabWidget");

    if (tabwgt)
        tabwgt->setCurrentIndex(tab);
}
