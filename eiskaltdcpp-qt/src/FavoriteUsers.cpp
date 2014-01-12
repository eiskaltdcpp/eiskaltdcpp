/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "FavoriteUsers.h"
#include "WulforUtil.h"
#include "FavoriteUsersModel.h"

#include <QMenu>
#include <QInputDialog>
#include <QKeyEvent>
#include <QHeaderView>
#include <QItemSelectionModel>

#include "dcpp/ClientManager.h"
#include "dcpp/QueueManager.h"
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
    treeView->header()->restoreState(WVGET(WS_FAVUSERS_STATE, QByteArray()).toByteArray());
    treeView->setSortingEnabled(true);

    connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(checkBox_AUTOGRANT, SIGNAL(toggled(bool)), this, SLOT(slotAutoGrant(bool)));

    connect(this, SIGNAL(coreUserAdded(VarMap)),                this, SLOT(addUser(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUserRemoved(QString)),             this, SLOT(remUser(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreStatusChanged(QString,QString)),   this, SLOT(updateUser(QString,QString)), Qt::QueuedConnection);

    WulforSettings *WS = WulforSettings::getInstance();
    connect(WS, SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));

    FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
    VarMap params;

    for (auto &i : ul) {
        dcpp::FavoriteUser &u = i.second;

        if (WBGET(WB_FAVUSERS_AUTOGRANT)){
            u.setFlag(FavoriteUser::FLAG_GRANTSLOT);
            FavoriteManager::getInstance()->setAutoGrant(u.getUser(), true);
        }

        getParams(params, u);
        addUser(params);
    }

    checkBox_AUTOGRANT->setChecked(WBGET(WB_FAVUSERS_AUTOGRANT));

    FavoriteManager::getInstance()->addListener(this);

    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

FavoriteUsers::~FavoriteUsers(){
    WVSET(WS_FAVUSERS_STATE, treeView->header()->saveState());
    
    FavoriteManager::getInstance()->removeListener(this);
    
    delete model;
}

void FavoriteUsers::closeEvent(QCloseEvent *e){
    isUnload()? e->accept() : e->ignore();
}

bool FavoriteUsers::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (k_e->key() == Qt::Key_Delete){
            if (treeView == reinterpret_cast<QTreeView*>(obj)){
                QModelIndexList indexes = treeView->selectionModel()->selectedRows(0);
                QList<FavoriteUserItem*> items;

                for (const auto &i : indexes)
                    items.push_back(reinterpret_cast<FavoriteUserItem*>(i.internalPointer()));

                for (const auto &i : items)
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

bool FavoriteUsers::addUserToFav(const QString &id){
    if (id.isEmpty())
        return false;

    string cid = id.toStdString();

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user){
        if (user != ClientManager::getInstance()->getMe() && !FavoriteManager::getInstance()->isFavoriteUser(user))
            FavoriteManager::getInstance()->addFavoriteUser(user);
    }

    return true;
}

bool FavoriteUsers::remUserFromFav(const QString &id){
    if (id.isEmpty())
        return false;

    string cid = id.toStdString();

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user){
        if (user != ClientManager::getInstance()->getMe() && FavoriteManager::getInstance()->isFavoriteUser(user))
            FavoriteManager::getInstance()->removeFavoriteUser(user);
    }

    return true;
}

QStringList FavoriteUsers::getUsers() const {
    return model->getUsers();
}

void FavoriteUsers::addUser(const VarMap &params){
    model->addUser(params);
}

void FavoriteUsers::updateUser(const QString &_cid, const QString &stat){
    dcpp::CID cid(_tq(_cid));
    const dcpp::UserPtr &user = ClientManager::getInstance()->findUser(cid);

    QString userUrl = user ? _q(FavoriteManager::getInstance()->getUserURL(user)) : QString();
    model->updateUserStatus(_cid, stat,
        (user && user->isOnline()) ? (WulforUtil::getInstance()->getHubNames(user)) : userUrl);
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
    static QString old = "";

    if (!item)
        return;

    dcpp::CID cid(_tq(_cid));
    const dcpp::UserPtr &user = ClientManager::getInstance()->findUser(cid);

    if (user){
        QString desc = QInputDialog::getText(this, item->data(COLUMN_USER_NICK).toString(), tr("Description"), QLineEdit::Normal, old);

        if (!desc.isEmpty()){
            old = desc;
            item->updateColumn(COLUMN_USER_DESC, desc);
            FavoriteManager::getInstance()->setUserDescription(user, _tq(desc));
        }
    }
}

void FavoriteUsers::getFileList(const VarMap &params){
    string cid  = params["CID"].toString().toStdString();
    string hub  = params["HUB"].toString().toStdString();

    if (cid.empty())
        return;

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
    if (user){
        try {
            QueueManager::getInstance()->addList(HintedUser(user, hub),  QueueItem::FLAG_CLIENT_VIEW, "");
        } catch(const Exception&) {
            // ...
        }
    }
}

void FavoriteUsers::handleBrowseShare(const QString &cid){
    FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();

    auto i = ul.find(CID(_tq(cid)));
    if (i != ul.end()){
        dcpp::FavoriteUser &user = i->second;

        VarMap params;
        getParams(params, user);
        getFileList(params);
    }
}

void FavoriteUsers::handleGrant(const QString &cid){
    FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();

    auto i = ul.find(CID(_tq(cid)));
    if (i != ul.end()){
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
        }
    }
}

void FavoriteUsers::slotContextMenu(){
    QModelIndexList indexes = treeView->selectionModel()->selectedRows(0);
    QList<FavoriteUserItem*> items;

    for (const auto &i : indexes)
        items.push_back(reinterpret_cast<FavoriteUserItem*>(i.internalPointer()));

    if (items.size() < 1)
        return;

    QMenu *menu = new QMenu(this);
    menu->deleteLater();

    QAction *remove = new QAction(tr("Remove"), menu);
    remove->setIcon(WICON(WulforUtil::eiEDITDELETE));

    QAction *desc   = new QAction(tr("Description"), menu);
    desc->setIcon(WICON(WulforUtil::eiEDIT));

    QAction *grant  = new QAction(tr("Grant/Remove slot"), menu);
    grant->setIcon(WICON(WulforUtil::eiBALL_GREEN));

    QAction *browse  = new QAction(tr("Browse Files"), menu);
    browse->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

    menu->addActions(QList<QAction*>() << browse << desc << grant << remove);

    QAction *ret = menu->exec(QCursor::pos());

    if (!ret)
        return;

    if (ret == remove){
        for (const auto &i : items)
            handleRemove(i->cid);
    }
    else if (ret == grant){
        for (const auto &i : items)
            handleGrant(i->cid);
    }
    else if(ret == browse){
        for (const auto &i : items)
            handleBrowseShare(i->cid);
    }
    else {
        for (const auto &i : items)
            handleDesc(i->cid);
    }
}

void FavoriteUsers::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void FavoriteUsers::slotSettingsChanged(const QString &key, const QString &){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void FavoriteUsers::on(UserAdded, const FavoriteUser& aUser) noexcept {
    if (WBGET(WB_FAVUSERS_AUTOGRANT))
        FavoriteManager::getInstance()->setAutoGrant(aUser.getUser(), true);

    VarMap params;

    getParams(params, aUser);

    emit coreUserAdded(params);
}

void FavoriteUsers::on(UserRemoved, const FavoriteUser& aUser) noexcept {
    emit coreUserRemoved(_q(aUser.getUser()->getCID().toBase32()));
}

void FavoriteUsers::on(StatusChanged, const FavoriteUser& u) noexcept{
    emit coreStatusChanged(_q(u.getUser()->getCID().toBase32()), u.getUser()->isOnline()?
                                                                    tr("Online")
                                                                    :
                               _q(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(u.getUser()))));

}
