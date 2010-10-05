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

#include "WulforUtil.h"

Settings::Settings(): is_dirty(false)
{
    setupUi(this);

    init();

    setWindowTitle(tr("Options"));
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
    widgets.insert(item, 0);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiCONNECT), tr("Connection"), listWidget);
    SettingsConnection *connection = new SettingsConnection(this);
    connect(this, SIGNAL(timeToDie()), connection, SLOT(ok()));
    widgets.insert(item, 1);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiDOWNLOAD), tr("Downloads"), listWidget);
    SettingsDownloads *downloads = new SettingsDownloads(this);
    connect(this, SIGNAL(timeToDie()), downloads, SLOT(ok()));
    widgets.insert(item, 2);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Sharing"), listWidget);
    SettingsSharing *sharing = new SettingsSharing(this);
    connect(this, SIGNAL(timeToDie()), sharing, SLOT(ok()));
    widgets.insert(item, 3);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiGUI), tr("GUI"), listWidget);
    SettingsGUI *gui = new SettingsGUI(this);
    connect(this, SIGNAL(timeToDie()), gui, SLOT(ok()));
    widgets.insert(item, 4);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiMESSAGE), tr("Notifications"), listWidget);
    SettingsNotification *notify = new SettingsNotification(this);
    connect(this, SIGNAL(timeToDie()), notify, SLOT(ok()));
    widgets.insert(item, 5);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE), tr("Logs"), listWidget);
    SettingsLog *logs = new SettingsLog(this);
    connect(this, SIGNAL(timeToDie()), logs, SLOT(ok()));
    widgets.insert(item, 6);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiUSERS), tr("User Commands"), listWidget);
    SettingsUC *ucs = new SettingsUC(this);
    connect(this, SIGNAL(timeToDie()), ucs, SLOT(ok()));
    widgets.insert(item, 7);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiEDIT), tr("Shortcuts"), listWidget);
    SettingsShortcuts *sshs = new SettingsShortcuts(this);
    connect(this, SIGNAL(timeToDie()), sshs, SLOT(ok()));
    widgets.insert(item, 8);

    stackedWidget->insertWidget(0, personal);
    stackedWidget->insertWidget(1, connection);
    stackedWidget->insertWidget(2, downloads);
    stackedWidget->insertWidget(3, sharing);
    stackedWidget->insertWidget(4, gui);
    stackedWidget->insertWidget(5, notify);
    stackedWidget->insertWidget(6, logs);
    stackedWidget->insertWidget(7, ucs);
    stackedWidget->insertWidget(8, sshs);

    stackedWidget->setCurrentIndex(0);

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
}
