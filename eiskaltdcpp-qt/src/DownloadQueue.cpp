/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "DownloadQueue.h"

#include <QMap>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QClipboard>
#include <QHeaderView>
#include <QDir>

#include "DownloadQueueModel.h"
#include "MainWindow.h"
#include "SearchFrame.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "Magnet.h"

#include "dcpp/ClientManager.h"
#include "dcpp/User.h"

#define _DEBUG_ 1

#if _DEBUG_
#include <QtDebug>
#endif

using namespace dcpp;

DownloadQueue::Menu::Menu(){
    menu = new QMenu();
    QMenu *menu_magnet = new QMenu(tr("Magnet"), DownloadQueue::getInstance());

    QAction *search_alt  = new QAction(tr("Search for alternates"), menu);
    QAction *copy_magnet = new QAction(tr("Copy magnet"), menu_magnet);
    QAction *copy_magnet_web = new QAction(tr("Copy web-magnet"), menu_magnet);
    QAction *magnet_info = new QAction(tr("Properties of magnet"), menu_magnet);
    QAction *ren_move    = new QAction(tr("Rename/Move"), menu);

    QAction *sep1 = new QAction(menu);
    sep1->setSeparator(true);

    set_prio = new QMenu(tr("Set priority"), menu);
    {
        QAction *paused = new QAction(tr("Paused"), set_prio);
        paused->setData(static_cast<int>(QueueItem::PAUSED));

        QAction *lowest = new QAction(tr("Lowest"), set_prio);
        lowest->setData(static_cast<int>(QueueItem::LOWEST));

        QAction *low    = new QAction(tr("Low"), set_prio);
        low->setData(static_cast<int>(QueueItem::LOW));

        QAction *normal = new QAction(tr("Normal"), set_prio);
        normal->setData(static_cast<int>(QueueItem::NORMAL));

        QAction *high   = new QAction(tr("High"), set_prio);
        high->setData(static_cast<int>(QueueItem::HIGH));

        QAction *highest= new QAction(tr("Highest"), set_prio);
        highest->setData(static_cast<int>(QueueItem::HIGHEST));

        set_prio->addActions(QList<QAction*>() << paused << lowest << low << normal << high << highest);
    }

    browse = new QMenu(tr("Browse files"), menu);
    send_pm = new QMenu(tr("Send private message"), menu);

    QAction *sep2 = new QAction(menu);
    sep2->setSeparator(true);

    rem_src  = new QMenu(tr("Remove source"), menu);
    rem_usr  = new QMenu(tr("Remove user"), menu);

    QAction *remove   = new QAction(tr("Remove"), menu);

    QAction *sep3 = new QAction(menu);
    sep3->setSeparator(true);

    map[search_alt] = Alternates;
    map[copy_magnet] = Magnet;
    map[copy_magnet_web] = MagnetWeb;
    map[magnet_info] = MagnetInfo;
    map[ren_move] = RenameMove;
    map[remove] = Remove;

    menu_magnet->addActions(QList<QAction*>()
            << copy_magnet << copy_magnet_web << sep3 << magnet_info);

    menu->addAction(search_alt);
    menu->addMenu(menu_magnet);
    menu->addAction(ren_move);
    menu->addAction(sep1);
    menu->addMenu(set_prio);
    menu->addMenu(browse);
    menu->addMenu(send_pm);
    menu->addAction(sep2);
    menu->addMenu(rem_src);
    menu->addMenu(rem_usr);
    menu->addAction(remove);
}

DownloadQueue::Menu::~Menu(){
    delete menu;
}

void DownloadQueue::Menu::clearMenu(QMenu *m){
    if (!m)
        return;

    QList<QAction*> actions = m->actions();

   foreach (QAction *a, actions)
       m->removeAction(a);

   qDeleteAll(actions);
}

DownloadQueue::Menu::Action DownloadQueue::Menu::exec(const DownloadQueue::SourceMap &sources, const QString &target, bool multiselect){
    if (target.isEmpty() || sources.isEmpty() || !sources.contains(target))
        return None;

    arg = QVariant();

    clearMenu(browse), clearMenu(send_pm), clearMenu(rem_src), clearMenu(rem_usr);

    browse->setDisabled(multiselect);
    send_pm->setDisabled(multiselect);
    rem_src->setDisabled(multiselect);
    rem_usr->setDisabled(multiselect);

    QMap<QString, QString>  users = sources[target];
    QMap<QString, QString>::const_iterator it = users.constBegin();

    for (; it != users.constEnd(); ++it){
        QAction *act = new QAction(it.key(), menu);
        act->setStatusTip(it.value());

        browse->addAction(act);
        send_pm->addAction(act);
        rem_src->addAction(act);
        rem_usr->addAction(act);
    }

    QAction *ret = menu->exec(QCursor::pos());
    DownloadQueue::VarMap rmap;

    if (!ret)
        return None;
    else if (map.contains(ret))
        return map[ret];
    else if (set_prio->actions().contains(ret)){
        arg = ret->data();

        return SetPriority;
    }

    rmap.insert(ret->text(), ret->statusTip());
    arg = rmap;

    if (browse->actions().contains(ret))
        return Browse;
    else if (send_pm->actions().contains(ret))
        return SendPM;
    else if (rem_src->actions().contains(ret))
        return RemoveSource;
    else if (rem_usr->actions().contains(ret))
        return RemoveUser;
    else
        arg = QVariant();

    return None;
}

QVariant DownloadQueue::Menu::getArg(){
    return arg;
}

DownloadQueue::DownloadQueue(QWidget *parent):
        QWidget(parent)
{
    setupUi(this);

    init();

    QueueManager::getInstance()->addListener(this);

    setUnload(false);
    
    registerThis();
}

DownloadQueue::~DownloadQueue(){
    QueueManager::getInstance()->removeListener(this);

    delete menu;
}

void DownloadQueue::closeEvent(QCloseEvent *e){
    if (isUnload()){
        save();

        e->accept();
    }
    else {
        e->ignore();
    }
}

void DownloadQueue::requestDelete(){
    if (!treeView_TARGET->hasFocus())
        return;

    QModelIndexList list = treeView_TARGET->selectionModel()->selectedRows(0);

    if (list.isEmpty())
        return;

    QList<DownloadQueueItem*> items;

    foreach (QModelIndex i, list){
        DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(i.internalPointer());

        if (!item)
            continue;

        if (item->dir)
            getChilds(item, items);
        else if (!items.contains(item))
            items.push_front(item);
    }

    QueueManager *QM = QueueManager::getInstance();
    foreach (DownloadQueueItem *i, items){
        QString target = i->data(COLUMN_DOWNLOADQUEUE_PATH).toString() + i->data(COLUMN_DOWNLOADQUEUE_NAME).toString();

        try {
            QM->remove(target.toStdString());
        }
        catch (const Exception &){}
    }
}

void DownloadQueue::init(){
    queue_model = new DownloadQueueModel(this);

    delegate = new DownloadQueueDelegate(queue_model);

    treeView_TARGET->setItemDelegate(delegate);
    treeView_TARGET->setModel(queue_model);
    treeView_TARGET->setItemsExpandable(true);
    treeView_TARGET->setRootIsDecorated(true);
    treeView_TARGET->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_TARGET->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    label_STATS->hide();

    deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    deleteShortcut->setContext(Qt::WidgetWithChildrenShortcut);

    connect(this, SIGNAL(coreAdded(VarMap)),            this, SLOT(addFile(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreRemoved(VarMap)),          this, SLOT(remFile(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreSourcesUpdated(VarMap)),   this, SLOT(updateFile(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreStatusUpdated(VarMap)),    this, SLOT(updateFile(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreMoved(VarMap)),            this, SLOT(remFile(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreMoved(VarMap)),            this, SLOT(addFile(VarMap)), Qt::QueuedConnection);

    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(requestDelete()));
    connect(treeView_TARGET, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(treeView_TARGET->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(queue_model, SIGNAL(needExpand(QModelIndex)), treeView_TARGET, SLOT(expand(QModelIndex)));
    connect(queue_model, SIGNAL(rowRemoved(QModelIndex)), this, SLOT(slotCollapseRow(QModelIndex)));
    connect(queue_model, SIGNAL(updateStats(quint64,quint64)), this, SLOT(slotUpdateStats(quint64,quint64)));
    connect(pushButton_EXPAND,      SIGNAL(clicked()), treeView_TARGET, SLOT(expandAll()));
    connect(pushButton_COLLAPSE,    SIGNAL(clicked()), treeView_TARGET, SLOT(collapseAll()));

    menu = new Menu();

    setAttribute(Qt::WA_DeleteOnClose);

    load();

    loadList();

    treeView_TARGET->expandAll();

    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton) );
}

void DownloadQueue::load(){
    treeView_TARGET->header()->restoreState(QByteArray::fromBase64(WSGET(WS_DQUEUE_STATE).toAscii()));
    treeView_TARGET->setSortingEnabled(true);
}

void DownloadQueue::save(){
    QString ustate = treeView_TARGET->header()->saveState().toBase64();

    WSSET(WS_DQUEUE_STATE, ustate);
}

void DownloadQueue::getParams(DownloadQueue::VarMap &params, const QueueItem *item){
    QString nick = "";
    QMap<QString, QString> source;
    int online = 0;

    if (!item)
        return;

    params["FNAME"]     = _q(item->getTargetFileName());
    params["PATH"]      = _q(Util::getFilePath(item->getTarget()));
    params["TARGET"]    = _q(item->getTarget());

    params["USERS"] = QString("");

    QStringList user_list;

    QueueItem::SourceConstIter it = item->getSources().begin();

    for (; it != item->getSources().end(); ++it){
        UserPtr usr = it->getUser();

        if (usr->isOnline())
            ++online;

        nick = WulforUtil::getInstance()->getNicks(usr->getCID());

        if (!nick.isEmpty()){
            source[nick] = _q(usr->getCID().toBase32());
            user_list.push_back(nick);
        }
    }

    if (!user_list.isEmpty())
        params["USERS"] = user_list.join(", ");
    else
        params["USERS"] = tr("No users...");

    sources[_q(item->getTarget())] = source;

    if (item->isWaiting())
        params["STATUS"] = tr("%1 of %2 user(s) online").arg(online).arg(item->getSources().size());
    else
        params["STATUS"] = tr("Running...");

    params["ESIZE"] = (qlonglong)item->getSize();
    params["DOWN"]  = (qlonglong)item->getDownloadedBytes();
    params["PRIO"]  = static_cast<int>(item->getPriority());

    source.clear();

    params["ERRORS"] = QString("");

    it = item->getBadSources().begin();

    for (; it != item->getBadSources().end(); ++it){
        QString errors = params["ERRORS"].toString();
        UserPtr usr = it->getUser();

        nick = WulforUtil::getInstance()->getNicks(usr->getCID());
        source[nick] = _q(usr->getCID().toBase32());

        if (!it->isSet(QueueItem::Source::FLAG_REMOVED)){
            if (!errors.isEmpty())
                errors += ", ";

            errors += nick + " (";

            if (it->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
                errors += tr("File not available");
            else if (it->isSet(QueueItem::Source::FLAG_PASSIVE))
                errors += tr("Passive user");
            else if (it->isSet(QueueItem::Source::FLAG_CRC_FAILED))
                errors += tr("Checksum mismatch");
            else if (it->isSet(QueueItem::Source::FLAG_BAD_TREE))
                errors += tr("Full tree does not match TTH root");
            else if (it->isSet(QueueItem::Source::FLAG_SLOW_SOURCE))
                errors += tr("Source too slow");
            else if (it->isSet(QueueItem::Source::FLAG_NO_TTHF))
                errors += tr("Remote client does not fully support TTH - cannot download");

            params["ERRORS"] = errors + ")";
        }
    }

    if (params["ERRORS"].toString().isEmpty())
        params["ERRORS"] = tr("No errors");

    badSources[_q(item->getTarget())] = source;

    params["ADDED"] = _q(Util::formatTime("%Y-%m-%d %H:%M", item->getAdded()));
    params["TTH"] = _q(item->getTTH().toBase32());

}

QStringList DownloadQueue::getSources(){
    SourceMap::iterator s_it = sources.begin();
    QStringList ret;

    for (; s_it != sources.end(); ++s_it){
        QString target = s_it.key();
        QString users;
        QMap<QString, QString>::iterator it = s_it.value().begin();

        for (; it != s_it.value().end(); ++it){
            users += it.key() + "(" + it.value() + ") ";
        }

        ret.push_back(target + "::" + users);
    }

    return ret;
}

void DownloadQueue::removeTarget(const QString &target){
    QueueManager *QM = QueueManager::getInstance();

    try {
        QM->remove(target.toStdString());
    }
    catch (const Exception&){}
}

void DownloadQueue::removeSource(const QString &cid, const QString &target){
    QueueManager *QM = QueueManager::getInstance();

    if (sources.contains(target) && !cid.isEmpty()){
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

        if (user){
            try {
                QM->removeSource(user, QueueItem::Source::FLAG_REMOVED);
            }
            catch (const Exception&){}
        }
    }
}

void DownloadQueue::loadList(){
    VarMap params;

    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

    for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it){
        getParams(params, it->second);

        addFile(params);
    }

    QueueManager::getInstance()->unlockQueue();
}

void DownloadQueue::addFile(const DownloadQueue::VarMap &map){
    queue_model->addItem(map);
}

void DownloadQueue::remFile(const VarMap &map){
    if (queue_model->remItem(map)){
        SourceMap::iterator it = sources.find(map["TARGET"].toString());

        if (it != sources.end())
            sources.erase(it);

        it = badSources.find(map["TARGET"].toString());

        if (it != badSources.end())
            badSources.erase(it);
    }
}

void DownloadQueue::updateFile(const DownloadQueue::VarMap &map){
    queue_model->updItem(map);
}

QString DownloadQueue::getCID(const VarMap &map){
    if (map.size() < 1)
        return "";

    VarMap::const_iterator it = map.constBegin();

    return ((++it).value()).toString();
}

void DownloadQueue::getChilds(DownloadQueueItem *i, QList<DownloadQueueItem *> &list){
    if (!i)
        return;

    if (!i->dir && !list.contains(i)){
        list.push_back(i);

        return;
    }

    if (i->childCount() < 1)
        return;

    foreach(DownloadQueueItem *ii, i->childItems)
        getChilds(ii, list);
}

void DownloadQueue::getItems(const QModelIndexList &list, QList<DownloadQueueItem*> &items){
    items.clear();

    if (list.isEmpty())
        return;

    foreach (QModelIndex i, list){
        DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(i.internalPointer());

        getChilds(item, items);
    }
}

void DownloadQueue::slotContextMenu(const QPoint &){
    QModelIndexList list = treeView_TARGET->selectionModel()->selectedRows(0);
    QList<DownloadQueueItem*> items;

    if (list.isEmpty())
        return;

    getItems(list, items);

    if (items.isEmpty())
        return;

    DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(items.at(0));

    QString target = item->data(COLUMN_DOWNLOADQUEUE_PATH).toString() + item->data(COLUMN_DOWNLOADQUEUE_NAME).toString();

    if (target.isEmpty())
        return;

    Menu::Action act = menu->exec(sources, target, items.size() > 1);
    QueueManager *QM = QueueManager::getInstance();
    QVariant arg = menu->getArg();
    VarMap rmap;

    /** Now re-read selected indexes and remove broken items */
    list = treeView_TARGET->selectionModel()->selectedRows(0);

    getItems(list, items);

    if (items.isEmpty())
        return;

    switch (act){
        case Menu::Alternates:
        {
            SearchFrame *sf = new SearchFrame();

            foreach (DownloadQueueItem *i, items)
                sf->searchAlternates(i->data(COLUMN_DOWNLOADQUEUE_TTH).toString());

            break;
        }
        case Menu::Magnet:
        {
            QString magnet = "";

            foreach (DownloadQueueItem *i, items)
                magnet += WulforUtil::getInstance()->makeMagnet(
                        i->data(COLUMN_DOWNLOADQUEUE_NAME).toString(),
                        i->data(COLUMN_DOWNLOADQUEUE_ESIZE).toLongLong(),
                        i->data(COLUMN_DOWNLOADQUEUE_TTH).toString()) + "\n";

            if (!magnet.isEmpty())
                qApp->clipboard()->setText(magnet, QClipboard::Clipboard);

            break;
        }
        case Menu::MagnetWeb:
        {
            QString magnet = "";

            foreach (DownloadQueueItem *i, items)
                magnet += "[magnet=\"" +
                    WulforUtil::getInstance()->makeMagnet(
                        i->data(COLUMN_DOWNLOADQUEUE_NAME).toString(),
                        i->data(COLUMN_DOWNLOADQUEUE_ESIZE).toLongLong(),
                        i->data(COLUMN_DOWNLOADQUEUE_TTH).toString()) +
                    "\"]"+i->data(COLUMN_DOWNLOADQUEUE_NAME).toString()+"[/magnet]\n";

            if (!magnet.isEmpty())
                qApp->clipboard()->setText(magnet, QClipboard::Clipboard);

            break;
        }
        case Menu::MagnetInfo:
        {
            QString magnet = "";

            foreach (DownloadQueueItem *i, items){
                magnet = WulforUtil::getInstance()->makeMagnet(
                    i->data(COLUMN_DOWNLOADQUEUE_NAME).toString(),
                    i->data(COLUMN_DOWNLOADQUEUE_ESIZE).toLongLong(),
                    i->data(COLUMN_DOWNLOADQUEUE_TTH).toString()) + "\n";

                if (!magnet.isEmpty()){
                    Magnet m(this);
                    m.setLink(magnet);
                    m.exec();
                }
            }

            break;
        }
        case Menu::RenameMove:
        {
            foreach (DownloadQueueItem *i, items){
                QString target = i->data(COLUMN_DOWNLOADQUEUE_PATH).toString() +
                                 i->data(COLUMN_DOWNLOADQUEUE_NAME).toString();
                QString new_target = QFileDialog::getSaveFileName(this, tr("Choose filename"), QDir::homePath(), tr("All files (*.*)"));

                if (!new_target.isEmpty()){
                    new_target = QDir::toNativeSeparators(new_target);
                    try {
                        QM->move(target.toStdString(), new_target.toStdString());
                    }
                    catch (const Exception &){}
                }
            }

            break;
        }
        case Menu::SetPriority:
        {
            foreach (DownloadQueueItem *i, items){
                QString target = i->data(COLUMN_DOWNLOADQUEUE_PATH).toString() + i->data(COLUMN_DOWNLOADQUEUE_NAME).toString();

                try {
                    QM->setPriority(target.toStdString(), static_cast<QueueItem::Priority>(arg.toInt()));
                }
                catch (const Exception&) {}
            }

            break;
        }
        case Menu::Browse:
        {
            rmap = arg.toMap();
            QString cid = getCID(rmap);

            if (sources.contains(target) && !cid.isEmpty()){
                UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

                if (user){
                    try {
                        QM->addList(HintedUser(user, ""), QueueItem::FLAG_CLIENT_VIEW, "");
                    }
                    catch (const Exception&){}
                }
            }

            break;
        }
        case Menu::SendPM:
        {
            rmap = arg.toMap();
            VarMap::const_iterator it = rmap.constBegin();
            dcpp::CID cid(_tq(getCID(rmap)));
            QString nick = ((++it).key());
            QList<QObject*> list = HubManager::getInstance()->getHubs();

            foreach (QObject *obj, list){
                HubFrame *fr = qobject_cast<HubFrame*>(obj);
                
                if (!fr)
                    continue;
                
                if (fr->hasCID(cid, nick)){
                    fr->createPMWindow(cid);

                    break;
                }
            }

            break;
        }
        case Menu::RemoveSource:
        {
            rmap = arg.toMap();
            QString cid = getCID(rmap);

            if (sources.contains(target) && !cid.isEmpty()){
                UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

                if (user){
                    try {
                        QM->removeSource(target.toStdString(), user, QueueItem::Source::FLAG_REMOVED);
                    }
                    catch (const Exception&){}
                }
            }

            break;
        }
        case Menu::RemoveUser:
        {
            rmap = arg.toMap();
            QString cid = getCID(rmap);

            if (sources.contains(target) && !cid.isEmpty()){
                UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

                if (user){
                    try {
                        QM->removeSource(user, QueueItem::Source::FLAG_REMOVED);
                    }
                    catch (const Exception&){}
                }
            }

            break;
        }
        case Menu::Remove:
        {
            foreach (DownloadQueueItem *i, items){
                QString target = i->data(COLUMN_DOWNLOADQUEUE_PATH).toString() + i->data(COLUMN_DOWNLOADQUEUE_NAME).toString();

                try {
                    QM->remove(target.toStdString());
                }
                catch (const Exception &){}
            }

            break;
        }
        default:
            break;
    }
}

void DownloadQueue::slotCollapseRow(const QModelIndex &row){
    if (row.isValid())
        treeView_TARGET->collapse(row);
}

void DownloadQueue::slotHeaderMenu(const QPoint&){
    WulforUtil::headerMenu(treeView_TARGET);
}

void DownloadQueue::slotUpdateStats(quint64 files, quint64 size){
    if (static_cast<qint64>(size) < 0)
        size = 0;

    if (files == size && size == 0)
        label_STATS->hide();

    if (label_STATS->isHidden())
        label_STATS->show();

    label_STATS->setText(QString("Total files: <b>%1</b> Total size: <b>%2</b>").arg(files).arg(WulforUtil::formatBytes(size)));
}

void DownloadQueue::slotSettingsChanged(const QString &key, const QString &value){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void DownloadQueue::on(QueueManagerListener::Added, QueueItem *item) noexcept{
    VarMap params;
    getParams(params, item);

    emit coreAdded(params);
    emit added(_q(item->getTargetFileName()));
}

void DownloadQueue::on(QueueManagerListener::Moved, QueueItem *item, const std::string &oldTarget) noexcept{
    VarMap params;
    getParams(params, item);

    emit coreMoved(params);
    emit moved(_q(oldTarget), _q(item->getTargetFileName()));
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem *item) noexcept{
    VarMap params;
    getParams(params, item);

    emit coreRemoved(params);
    emit removed(_q(item->getTargetFileName()));
}

void DownloadQueue::on(QueueManagerListener::SourcesUpdated, QueueItem *item) noexcept{
    VarMap params;
    getParams(params, item);

    emit coreSourcesUpdated(params);
}

void DownloadQueue::on(QueueManagerListener::StatusUpdated, QueueItem *item) noexcept{
    VarMap params;
    getParams(params, item);

    emit coreStatusUpdated(params);
}
