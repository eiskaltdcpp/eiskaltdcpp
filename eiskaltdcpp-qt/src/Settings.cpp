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
#include "SettingsAdvanced.h"

#include "WulforUtil.h"

#include <QTableWidget>
#include <QTabWidget>

#if QT_VERSION >= 0x050000
#include <QScroller>
#endif

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

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiCONSOLE), tr("Advanced"), listWidget);
    SettingsAdvanced *sadv = new SettingsAdvanced(this);
    connect(this, SIGNAL(timeToDie()), sadv, SLOT(ok()));
    widgets.insert(item, (int)Page::Advanced);

    listWidget->setMinimumWidth(listWidget->sizeHintForColumn(0) + 6);

    stackedWidget->insertWidget((int)Page::Personal, prepareWidget(personal));
    stackedWidget->insertWidget((int)Page::Connection, prepareWidget(connection));
    stackedWidget->insertWidget((int)Page::Downloads, prepareWidget(downloads));
    stackedWidget->insertWidget((int)Page::Sharing, prepareWidget(sharing));
    stackedWidget->insertWidget((int)Page::GUI, prepareWidget(gui));
    stackedWidget->insertWidget((int)Page::Notifications, prepareWidget(notify));
    stackedWidget->insertWidget((int)Page::Logs, prepareWidget(logs));
    stackedWidget->insertWidget((int)Page::UserCommands, prepareWidget(ucs));
    stackedWidget->insertWidget((int)Page::Shortcuts, prepareWidget(sshs));
    stackedWidget->insertWidget((int)Page::History, prepareWidget(shist));
    stackedWidget->insertWidget((int)Page::Advanced, prepareWidget(sadv));

    stackedWidget->setCurrentIndex(0);

    if (WVGET("settings/dialog-size").isValid())
        resize(WVGET("settings/dialog-size").toSize());

    // Convenient scrolling of widgets with the mouse, as well as from the touchpad and from the touchscreen:
    for (auto &asa : stackedWidget->findChildren<QAbstractScrollArea*>()) {
        setMouseScroller(asa->viewport());
    }
    // Smooth scrolling. See: http://stackoverflow.com/questions/19298486/qscroller-kinetic-scrolling-is-not-smooth)
    for (auto &aiv : stackedWidget->findChildren<QAbstractItemView*>()) {
        aiv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }

    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotItemActivated(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemActivated(QListWidgetItem*)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(this, SIGNAL(accepted()), this, SIGNAL(timeToDie()));
    connect(this, SIGNAL(accepted()), this, SLOT(dirty()));
}

void Settings::setMouseScroller(QWidget *w){
#if QT_VERSION >= 0x050000
    QScroller::grabGesture(w, QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(w);
    QScrollerProperties properties = scroller->scrollerProperties();
    QVariant overshootPolicy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff);
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, overshootPolicy);
    properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, overshootPolicy);
    scroller->setScrollerProperties(properties);
#endif
}

QWidget *Settings::prepareWidget(QWidget *w)
{
    const bool containsTabs = !w->findChildren<QTabWidget*>().isEmpty();
    if (containsTabs) {
        for (auto *tw : w->findChildren<QTabWidget*>()) {
            // Content of each page should placed to independent QScrollArea
            for (int k = 0; k < tw->count(); ++k) {
                const QString &&title = tw->tabText(k);
                QWidget *page = tw->widget(k);
                QScrollArea *scrollArea = new QScrollArea(this);
                scrollArea->setWidget(page);
                scrollArea->setWidgetResizable(true);
                scrollArea->setFrameShape(QFrame::NoFrame);
                tw->insertTab(k, scrollArea, title);
            }
            tw->setCurrentIndex(0);
        }
    }
    else { // Single widget may be placed directly to QScrollArea
        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidget(w);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        w->layout()->setMargin(0);
        return scrollArea;
    }

    w->layout()->setMargin(0);
    return w;
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
