#include "HubManager.h"
#include "HubFrame.h"
#include "MainWindow.h"

#include <QtDebug>

HubManager::HubManager():
        active(NULL)
{
    setupUi(this);

    MainWindow *MW = MainWindow::getInstance();

    MW->addArenaWidget(this);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
}

HubManager::~HubManager(){
}

void HubManager::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

        e->accept();
    }
    else {
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);

        e->ignore();
    }
}

void HubManager::registerHubUrl(const QString &url, HubFrame *hub){
    HubHash::const_iterator it = hubs.find(url);

    if (it != hubs.constEnd() || !hub)
        return;

    hubs.insert(url, hub);

    QTreeWidgetItem *item = (items.contains(hub)? (items[hub]) : (new QTreeWidgetItem(treeWidget)));

    quint64 users = 0, share = 0;

    hub->getStatistic(users, share);

    item->setText(0, hub->getArenaTitle());
    item->setText(1, url);
    item->setText(2, QString("%1").arg(users));
    item->setText(3, WulforUtil::formatBytes(share));

    items[hub] = item;

    connect(hub, SIGNAL(closeRequest()), this, SLOT(slotHubClosed()));
    connect(hub, SIGNAL(newMessage(HubFrame*,QString,QString,QString,QString)), this, SIGNAL(newMessage(HubFrame*,QString,QString,QString,QString)));
    connect(hub, SIGNAL(coreUserUpdated(VarMap,dcpp::UserPtr,bool)), this, SLOT(slotHubUpdated()));
    connect(hub, SIGNAL(coreUserRemoved(dcpp::UserPtr,qlonglong)), this, SLOT(slotHubUpdated()));
    connect(hub, SIGNAL(coreConnected(QString)), this, SLOT(slotHubUpdated()));
    connect(hub, SIGNAL(coreFailed()), this, SLOT(slotHubUpdated()));
}

void HubManager::unregisterHubUrl(const QString &url){
    HubHash::iterator it = hubs.find(url);

    if (it != hubs.end()){
        hubs.erase(it);

        QTreeWidgetItem *item = items[(*it)];
        quint64 users = 0, share = 0;

        (*it)->getStatistic(users, share);

        item->setText(0, (*it)->getArenaShortTitle());
        item->setText(2, QString("%1").arg(users));
        item->setText(3, WulforUtil::formatBytes(share));
    }
}

void HubManager::setActiveHub(HubFrame *f){
    active = f;
}

HubFrame *HubManager::getHub(const QString &url){
    HubHash::const_iterator it = hubs.find(url);

    if (it != hubs.constEnd()){
        return it.value();
    }

    return NULL;
}

QList<HubFrame*> HubManager::getHubs() const {
    QList<HubFrame*> list;

    HubHash::const_iterator it = hubs.constBegin();

    for(; it != hubs.constEnd(); ++it)
        list << const_cast<HubFrame*>(it.value());

    return list;
}

HubFrame *HubManager::activeHub() const {
    return active;
}

QObject *HubManager::getHubObject(){
    return qobject_cast<QObject*>(activeHub());
}

void HubManager::slotHubUpdated(){
    HubFrame *hub = qobject_cast<HubFrame* >(sender());
    QMap<HubFrame*,QTreeWidgetItem*>::iterator it = items.find(hub);

    if (it == items.end())
        return;

    QTreeWidgetItem *item = it.value();

    quint64 users = 0, share = 0;

    hub->getStatistic(users, share);

    item->setText(0, hub->getArenaShortTitle());
    item->setText(2, QString("%1").arg(users));
    item->setText(3, WulforUtil::formatBytes(share));
}

void HubManager::slotContextMenu(){
    QList<QTreeWidgetItem*> itemList = treeWidget->selectedItems();

    if (itemList.size() != 1)
        return;

    QTreeWidgetItem *item = itemList.first();
    QMap<HubFrame*,QTreeWidgetItem*>::iterator it = items.begin();

    for (; it != items.end(); ++it){
        if (it.value() != item)
            continue;

        HubFrame *fr = it.key();

        if (!fr->getMenu())
            return;

        QMenu *m = fr->getMenu();

        m->exec(QCursor::pos());

        return;
    }
}

void HubManager::slotHubClosed(){
    HubFrame *hub = qobject_cast<HubFrame* >(sender());

    if (!hub)
        return;

    QTreeWidgetItem *item = items[hub];
    items.remove(hub);

    delete item;
}
