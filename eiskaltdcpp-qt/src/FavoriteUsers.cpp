/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "FavoriteUsers.h"
#include "WulforUtil.h"
#include "FavoriteUsersModel.h"
#include "QtContext.h"
#include "QtContextAware.h"

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
#include "dcpp/DCPlusPlus.h"


using namespace dcpp;

FavoriteUsers::FavoriteUsers(dcpp::DCContext& ctx, QWidget *parent) :
    QtContextAware(ctx),
    QWidget(parent)
{
    setupUi(this);

    setUnload(false);

    model = new FavoriteUsersModel(this);

    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->installEventFilter(this);
    treeView->setModel(model);
    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->restoreState(qtCtx()->settings()->getVar(WS_FAVUSERS_STATE, QByteArray()).toByteArray());
    treeView->setSortingEnabled(true);

    connect(treeView, &QWidget::customContextMenuRequested, this, &FavoriteUsers::slotContextMenu);
    connect(treeView->header(), &QWidget::customContextMenuRequested, this, &FavoriteUsers::slotHeaderMenu);
    connect(checkBox_AUTOGRANT, &QCheckBox::toggled, this, &FavoriteUsers::slotAutoGrant);

    connect(this, &FavoriteUsers::coreUserAdded, this, &FavoriteUsers::addUser, Qt::QueuedConnection);
    connect(this, &FavoriteUsers::coreUserRemoved, this, &FavoriteUsers::remUser, Qt::QueuedConnection);
    connect(this, &FavoriteUsers::coreStatusChanged, this, &FavoriteUsers::updateUser, Qt::QueuedConnection);

    WulforSettings *WS = qtCtx()->settings();
    connect(WS, &WulforSettings::strValueChanged, this, &FavoriteUsers::slotSettingsChanged);

    FavoriteManager::FavoriteMap ul = dcCtx().getFavoriteManager()->getFavoriteUsers();
    VarMap params;

    for (auto &i : ul) {
        dcpp::FavoriteUser &u = i.second;

        if (qtCtx()->settings()->getBool(WB_FAVUSERS_AUTOGRANT)){
            u.setFlag(FavoriteUser::FLAG_GRANTSLOT);
            dcCtx().getFavoriteManager()->setAutoGrant(u.getUser(), true);
        }

        getParams(params, u);
        addUser(params);
    }

    checkBox_AUTOGRANT->setChecked(qtCtx()->settings()->getBool(WB_FAVUSERS_AUTOGRANT));

    dcCtx().getFavoriteManager()->addListener(this);

    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

FavoriteUsers::~FavoriteUsers(){
    qtCtx()->settings()->setVar(WS_FAVUSERS_STATE, treeView->header()->saveState());
    
    dcCtx().getFavoriteManager()->removeListener(this);
    
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
    params["HUB"]   = u->isOnline()? (qtCtx()->wulforUtil()->getHubNames(u)) : _q(user.getUrl());
    params["SEEN"]  = u->isOnline()? tr("Online") : _q(Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen()));
    params["DESC"]  = _q(user.getDescription());
    params["SLOT"]  = user.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

bool FavoriteUsers::addUserToFav(const QString &id){
    if (id.isEmpty())
        return false;

    string cid = id.toStdString();

    UserPtr user = dcCtx().getClientManager()->findUser(CID(cid));

    if (user){
        if (user != dcCtx().getClientManager()->getMe() && !dcCtx().getFavoriteManager()->isFavoriteUser(user))
            dcCtx().getFavoriteManager()->addFavoriteUser(user);
    }

    return true;
}

bool FavoriteUsers::remUserFromFav(const QString &id){
    if (id.isEmpty())
        return false;

    string cid = id.toStdString();

    UserPtr user = dcCtx().getClientManager()->findUser(CID(cid));

    if (user){
        if (user != dcCtx().getClientManager()->getMe() && dcCtx().getFavoriteManager()->isFavoriteUser(user))
            dcCtx().getFavoriteManager()->removeFavoriteUser(user);
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
    const dcpp::UserPtr &user = dcCtx().getClientManager()->findUser(cid);

    QString userUrl = user ? _q(dcCtx().getFavoriteManager()->getUserURL(user)) : QString();
    model->updateUserStatus(_cid, stat,
        (user && user->isOnline()) ? (qtCtx()->wulforUtil()->getHubNames(user)) : userUrl);
}

void FavoriteUsers::remUser(const QString &cid){
    model->removeUser(cid);
}

void FavoriteUsers::handleRemove(const QString & _cid){
    dcpp::CID cid(_tq(_cid));
    const dcpp::UserPtr &user = dcCtx().getClientManager()->findUser(cid);

    if (user)
        dcCtx().getFavoriteManager()->removeFavoriteUser(user);
}

void FavoriteUsers::handleDesc(const QString & _cid){
    FavoriteUserItem *item = model->itemForCID(_cid);
    static QString old = "";

    if (!item)
        return;

    dcpp::CID cid(_tq(_cid));
    const dcpp::UserPtr &user = dcCtx().getClientManager()->findUser(cid);

    if (user){
        QString desc = QInputDialog::getText(this, item->data(COLUMN_USER_NICK).toString(), tr("Description"), QLineEdit::Normal, old);

        if (!desc.isEmpty()){
            old = desc;
            item->updateColumn(COLUMN_USER_DESC, desc);
            dcCtx().getFavoriteManager()->setUserDescription(user, _tq(desc));
        }
    }
}

void FavoriteUsers::getFileList(const VarMap &params){
    string cid  = params["CID"].toString().toStdString();
    string hub  = params["HUB"].toString().toStdString();

    if (cid.empty())
        return;

    UserPtr user = dcCtx().getClientManager()->findUser(CID(cid));
    if (user){
        try {
            dcCtx().getQueueManager()->addList(HintedUser(user, hub),  QueueItem::FLAG_CLIENT_VIEW, "");
        } catch(const Exception&) {
            // ...
        }
    }
}

void FavoriteUsers::handleBrowseShare(const QString &cid){
    FavoriteManager::FavoriteMap ul = dcCtx().getFavoriteManager()->getFavoriteUsers();

    auto i = ul.find(CID(_tq(cid)));
    if (i != ul.end()){
        dcpp::FavoriteUser &user = i->second;

        VarMap params;
        getParams(params, user);
        getFileList(params);
    }
}

void FavoriteUsers::handleGrant(const QString &cid){
    FavoriteManager::FavoriteMap ul = dcCtx().getFavoriteManager()->getFavoriteUsers();

    auto i = ul.find(CID(_tq(cid)));
    if (i != ul.end()){
        dcpp::FavoriteUser &u = i->second;

        if (_q(u.getUser()->getCID().toBase32()) == cid){
            if (u.isSet(FavoriteUser::FLAG_GRANTSLOT)){
                u.unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
                dcCtx().getFavoriteManager()->setAutoGrant(u.getUser(), false);
            }
            else {
                u.setFlag(FavoriteUser::FLAG_GRANTSLOT);
                dcCtx().getFavoriteManager()->setAutoGrant(u.getUser(), true);
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
    remove->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEDITDELETE));

    QAction *desc   = new QAction(tr("Description"), menu);
    desc->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEDIT));

    QAction *grant  = new QAction(tr("Grant/Remove slot"), menu);
    grant->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiBALL_GREEN));

    QAction *browse  = new QAction(tr("Browse Files"), menu);
    browse->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiFOLDER_BLUE));

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
    if (qtCtx()->settings()->getBool(WB_FAVUSERS_AUTOGRANT))
        dcCtx().getFavoriteManager()->setAutoGrant(aUser.getUser(), true);

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
                               _q(Util::formatTime("%Y-%m-%d %H:%M", dcCtx().getFavoriteManager()->getLastSeen(u.getUser()))));

}
