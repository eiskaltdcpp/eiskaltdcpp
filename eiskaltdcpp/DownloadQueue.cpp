#include "DownloadQueue.h"

#include <QMap>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QClipboard>
#include <QHeaderView>

#include "DownloadQueueModel.h"
#include "MainWindow.h"
#include "SearchFrame.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "WulforUtil.h"
#include "WulforSettings.h"

#include "dcpp/ClientManager.h"
#include "dcpp/User.h"

#define _DEBUG_ 1

#if _DEBUG_
#include <QtDebug>
#endif

using namespace dcpp;

DownloadQueue::Menu::Menu(){
    menu = new QMenu();

    QAction *search_alt  = new QAction(tr("Search for alternates"), menu);
    QAction *copy_magnet = new QAction(tr("Copy magnet"), menu);
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
    map[ren_move] = RenameMove;
    map[remove] = Remove;

    menu->addActions(QList<QAction*>() << search_alt << copy_magnet << ren_move << sep1);
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
}

DownloadQueue::~DownloadQueue(){
    QueueManager::getInstance()->removeListener(this);

    delete menu;
}

void DownloadQueue::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

        //setAttribute(Qt::WA_DeleteOnClose);

        save();

        e->accept();
    }
    else {
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);

        e->ignore();
    }
}

void DownloadQueue::customEvent(QEvent *e){
    if (e->type() == DownloadQueueCustomEvent::Event){
        DownloadQueueCustomEvent *c_e = reinterpret_cast<DownloadQueueCustomEvent*>(e);

        c_e->func()->call();
    }

    e->accept();
}

void DownloadQueue::DEL_pressed(){
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

    connect(treeView_TARGET, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(treeView_TARGET->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(queue_model, SIGNAL(needExpand(QModelIndex)), treeView_TARGET, SLOT(expand(QModelIndex)));
    connect(queue_model, SIGNAL(rowRemoved(QModelIndex)), this, SLOT(slotCollapseRow(QModelIndex)));

    menu = new Menu();

    setAttribute(Qt::WA_DeleteOnClose);

    load();

    loadList();

    MainWindow *MW = MainWindow::getInstance();

    MW->addArenaWidget(this);
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

    QueueItem::SourceConstIter it = item->getSources().begin();

    for (; it != item->getSources().end(); ++it){
        UserPtr usr = it->getUser();

        if (usr->isOnline())
            ++online;

        if (!params["USERS"].toString().isEmpty())
            params["USERS"] = params["USERS"].toString() +", ";

        nick = WulforUtil::getInstance()->getNicks(usr->getCID());

        source[nick] = _q(usr->getCID().toBase32());
        
        params["USERS"] = params["USERS"].toString() + nick;
    }

    if (params["USERS"].toString().isEmpty())
        params["USERS"] = tr("No users...");

    sources[_q(item->getTarget())] = source;

    if (item->isWaiting())
        params["STATUS"] = QString("%1 of %2 user(s) online").arg(online)
                                                             .arg(item->getSources().size());
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
                errors += tr("CRC32 inconsistency (SFV-Check)");
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

void DownloadQueue::loadList(){
    VarMap params;

    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

    for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it){
        getParams(params, it->second);

        addFile(params);
    }

    QueueManager::getInstance()->unlockQueue();
}

void DownloadQueue::addFile(DownloadQueue::VarMap map){
    queue_model->addItem(map);
}

void DownloadQueue::remFile(VarMap map){
    if (queue_model->remItem(map)){
        SourceMap::iterator it = sources.find(map["TARGET"].toString());

        if (it != sources.end())
            sources.erase(it);

        it = badSources.find(map["TARGET"].toString());

        if (it != badSources.end())
            badSources.erase(it);
    }
}

void DownloadQueue::updateFile(DownloadQueue::VarMap map){
    queue_model->updItem(map);
}

QString DownloadQueue::getCID(const VarMap &map){
    if (map.size() < 1)
        return "";

    VarMap::const_iterator it = map.constBegin();

    return ((++it).value()).toString();
}

void DownloadQueue::getChilds(DownloadQueueItem *i, QList<DownloadQueueItem *> &list){
    if (!i || i->childCount() < 1)
        return;

    foreach(DownloadQueueItem *ii, i->childItems){
        if (ii->dir)
            getChilds(ii, list);
        else
            list.push_front(ii);
    }
}

void DownloadQueue::slotContextMenu(const QPoint &){
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

    DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(list.at(0).internalPointer());
    DownloadQueueItem *par = item->parent();

    QString target = item->data(COLUMN_DOWNLOADQUEUE_PATH).toString() + item->data(COLUMN_DOWNLOADQUEUE_NAME).toString();

    if (target.isEmpty())
        return;

    Menu::Action act = menu->exec(sources, target, items.size() > 1);
    QueueManager *QM = QueueManager::getInstance();
    QVariant arg = menu->getArg();
    VarMap rmap;
    QueueItem::Priority prio;

    if (!par->childItems.contains(item))
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
                magnet += WulforUtil::getInstance()->makeMagnet(i->data(COLUMN_DOWNLOADQUEUE_NAME).toString(),
                                                                i->data(COLUMN_DOWNLOADQUEUE_ESIZE).toLongLong(),
                                                                i->data(COLUMN_DOWNLOADQUEUE_TTH).toString()) + "\n";

            if (!magnet.isEmpty())
                qApp->clipboard()->setText(magnet, QClipboard::Clipboard);

            break;
        }
        case Menu::RenameMove:
        {
            foreach (DownloadQueueItem *i, items){
                QString target = i->data(COLUMN_DOWNLOADQUEUE_PATH).toString() + i->data(COLUMN_DOWNLOADQUEUE_NAME).toString();
                QString new_target = QFileDialog::getSaveFileName(this, tr("Choose filename"), QDir::homePath(), tr("All files (*.*)"));

                if (!new_target.isEmpty()){
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
                        QM->addList(user, "", QueueItem::FLAG_CLIENT_VIEW);
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
            QList<HubFrame*> list = HubManager::getInstance()->getHubs();

            foreach (HubFrame *fr, list){
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

void DownloadQueue::on(QueueManagerListener::Added, QueueItem *item) throw(){
    VarMap params;
    getParams(params, item);

    AddFileFunc *f = new AddFileFunc(this, &DownloadQueue::addFile, params);

    QApplication::postEvent(this, new DownloadQueueCustomEvent(f));
}

void DownloadQueue::on(QueueManagerListener::Moved, QueueItem *item, const std::string &oldTarget) throw(){
    VarMap params;
    getParams(params, item);

    RemFileFunc *rmf  = new RemFileFunc(this, &DownloadQueue::remFile, params);
    AddFileFunc *addf = new AddFileFunc(this, &DownloadQueue::addFile, params);

    QApplication::postEvent(this, new DownloadQueueCustomEvent(rmf));
    QApplication::postEvent(this, new DownloadQueueCustomEvent(addf));
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem *item) throw(){
    VarMap params;
    getParams(params, item);

    RemFileFunc *rmf  = new RemFileFunc(this, &DownloadQueue::remFile, params);

    QApplication::postEvent(this, new DownloadQueueCustomEvent(rmf));
}

void DownloadQueue::on(QueueManagerListener::SourcesUpdated, QueueItem *item) throw(){
    VarMap params;
    getParams(params, item);

    UpdateFileFunc *upf = new UpdateFileFunc(this, &DownloadQueue::updateFile, params);

    QApplication::postEvent(this, new DownloadQueueCustomEvent(upf));
}

void DownloadQueue::on(QueueManagerListener::StatusUpdated, QueueItem *item) throw(){
    VarMap params;
    getParams(params, item);

    UpdateFileFunc *upf = new UpdateFileFunc(this, &DownloadQueue::updateFile, params);

    QApplication::postEvent(this, new DownloadQueueCustomEvent(upf));
}
