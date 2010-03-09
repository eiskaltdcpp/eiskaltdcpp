#include "FavoriteUsers.h"
#include "MainWindow.h"
#include "WulforUtil.h"
#include "FavoriteUsersModel.h"

#include <QMenu>
#include <QInputDialog>
#include <QKeyEvent>
#include <QItemSelectionModel>

#include "dcpp/ClientManager.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/Util.h"

using namespace dcpp;

FavoriteUsers::FavoriteUsers(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    setUnload(false);

    model = new FavoriteUsersModel(this);

    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->installEventFilter(this);
    treeView->setModel(model);
    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->restoreState(QByteArray::fromBase64(WSGET(WS_FAVUSERS_STATE).toAscii()));
    treeView->setSortingEnabled(true);

    connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(checkBox_AUTOGRANT, SIGNAL(toggled(bool)), this, SLOT(slotAutoGrant(bool)));

    FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
    VarMap params;

    for(FavoriteManager::FavoriteMap::iterator i = ul.begin(); i != ul.end(); ++i) {
        dcpp::FavoriteUser &u = i->second;

        if (WBGET(WB_FAVUSERS_AUTOGRANT)){
            u.setFlag(FavoriteUser::FLAG_GRANTSLOT);
            FavoriteManager::getInstance()->setAutoGrant(u.getUser(), true);
        }

        getParams(params, u);
        addUser(params);
    }

    checkBox_AUTOGRANT->setChecked(WBGET(WB_FAVUSERS_AUTOGRANT));

    FavoriteManager::getInstance()->addListener(this);

    MainWindow::getInstance()->addArenaWidget(this);
}

FavoriteUsers::~FavoriteUsers(){
    FavoriteManager::getInstance()->removeListener(this);
    delete model;
}

void FavoriteUsers::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

        WSSET(WS_FAVUSERS_STATE, treeView->header()->saveState().toBase64());

        setAttribute(Qt::WA_DeleteOnClose);

        e->accept();
    }
    else {
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);

        e->ignore();
    }
}

void FavoriteUsers::customEvent(QEvent *e){
    if (e->type() == FavUserEvent::EventAddUser){
        FavUserEvent *u = reinterpret_cast<FavUserEvent*>(e);

        addUser(u->getMap());
    }
    else if (e->type() == FavUserEvent::EventRemUser){
        FavUserEvent *u = reinterpret_cast<FavUserEvent*>(e);

        remUser(_q(u->getCID().toBase32()));
    }
    else if (e->type() == FavUserEvent::EventUpdUser){
        FavUserEvent *u = reinterpret_cast<FavUserEvent*>(e);

        updateUser(_q(u->getCID().toBase32()), u->getStat());
    }

    e->accept();
}

bool FavoriteUsers::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (k_e->key() == Qt::Key_Delete){
            if (treeView == reinterpret_cast<QTreeView*>(obj)){
                QModelIndexList indexes = treeView->selectionModel()->selectedRows(0);
                QList<FavoriteUserItem*> items;

                foreach(QModelIndex i, indexes)
                    items.push_back(reinterpret_cast<FavoriteUserItem*>(i.internalPointer()));

                foreach (FavoriteUserItem *i, items)
                    handleRemove(i->cid);

                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, e);
}

void FavoriteUsers::getParams(VarMap &params, const FavoriteUser &user){
    const UserPtr &u = user.getUser();

    params["CID"]   = _q(u->getCID().toBase32());
    params["NICK"]  = _q(user.getNick());
    params["HUB"]   = u->isOnline()? (WulforUtil::getInstance()->getHubNames(u)) : _q(user.getUrl());
    params["SEEN"]  = u->isOnline()? tr("Online") : _q(Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen()));
    params["DESC"]  = _q(user.getDescription());
    params["SLOT"]  = user.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

void FavoriteUsers::addUser(const VarMap &params){
    model->addUser(params);
}

void FavoriteUsers::updateUser(const QString &cid, const QString &stat){
    model->updateUserStatus(cid, stat);
}

void FavoriteUsers::remUser(const QString &cid){
    model->removeUser(cid);
}

void FavoriteUsers::handleRemove(const QString & _cid){
    dcpp::CID cid(_tq(_cid));
    const dcpp::UserPtr &user = ClientManager::getInstance()->findUser(cid);

    if (user)
        FavoriteManager::getInstance()->removeFavoriteUser(user);
}

void FavoriteUsers::handleDesc(const QString & _cid){
    FavoriteUserItem *item = model->itemForCID(_cid);

    if (!item)
        return;

    dcpp::CID cid(_tq(_cid));
    const dcpp::UserPtr &user = ClientManager::getInstance()->findUser(cid);

    if (user){
        QString desc = QInputDialog::getText(this, item->data(COLUMN_USER_NICK).toString(), tr("Description"));

        if (!desc.isEmpty()){
            item->updateColumn(COLUMN_USER_DESC, desc);
            FavoriteManager::getInstance()->setUserDescription(user, _tq(desc));
        }
    }
}

void FavoriteUsers::handleGrant(const QString &cid){
    FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();

    for(FavoriteManager::FavoriteMap::iterator i = ul.begin(); i != ul.end(); ++i) {
        dcpp::FavoriteUser &u = i->second;

        if (_q(u.getUser()->getCID().toBase32()) == cid){
            if (u.isSet(FavoriteUser::FLAG_GRANTSLOT)){
                u.unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
                FavoriteManager::getInstance()->setAutoGrant(u.getUser(), false);
            }
            else {
                u.setFlag(FavoriteUser::FLAG_GRANTSLOT);
                FavoriteManager::getInstance()->setAutoGrant(u.getUser(), true);
            }

            break;
        }
    }
}

void FavoriteUsers::slotContextMenu(){
    QModelIndexList indexes = treeView->selectionModel()->selectedRows(0);
    QList<FavoriteUserItem*> items;

    foreach(QModelIndex i, indexes)
        items.push_back(reinterpret_cast<FavoriteUserItem*>(i.internalPointer()));

    if (items.size() < 1)
        return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose, true);

    QAction *remove = new QAction(tr("Remove"), menu);
    remove->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE));

    QAction *desc   = new QAction(tr("Description"), menu);
    desc->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDIT));

    QAction *grant  = new QAction(tr("Grant/Remove slot"), menu);
    grant->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiBALL_GREEN));

    menu->addActions(QList<QAction*>() << desc << grant << remove);

    QAction *ret = menu->exec(QCursor::pos());

    if (!ret)
        return;

    if (ret == remove){
        foreach(FavoriteUserItem *i, items)
            handleRemove(i->cid);
    }
    else if (ret == grant){
        foreach(FavoriteUserItem *i, items)
            handleGrant(i->cid);
    }
    else {
        foreach(FavoriteUserItem *i, items)
            handleDesc(i->cid);
    }
}

void FavoriteUsers::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void FavoriteUsers::on(UserAdded, const FavoriteUser& aUser) throw() {
    if (WBGET(WB_FAVUSERS_AUTOGRANT))
        FavoriteManager::getInstance()->setAutoGrant(aUser.getUser(), true);

    FavUserEvent *u_e = new FavUserEvent();

    getParams(u_e->getMap(), aUser);

    QApplication::postEvent(this, u_e);
}

void FavoriteUsers::on(UserRemoved, const FavoriteUser& aUser) throw() {
    FavUserEvent *u_e = new FavUserEvent(aUser.getUser()->getCID());

    QApplication::postEvent(this, u_e);
}

void FavoriteUsers::on(StatusChanged, const UserPtr& u) throw(){
    FavUserEvent *u_e = new FavUserEvent(u->isOnline()?
                                         tr("Online")
                                         :
                                        _q(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(u))));

    QApplication::postEvent(this, u_e);
}

