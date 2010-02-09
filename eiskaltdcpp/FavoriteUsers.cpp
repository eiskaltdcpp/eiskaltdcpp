#include "FavoriteUsers.h"
#include "MainWindow.h"
#include "WulforUtil.h"

#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/Util.h"

using namespace dcpp;

FavoriteUsers::FavoriteUsers(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    setUnload(false);

    FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
    VarMap params;

    for(FavoriteManager::FavoriteMap::iterator i = ul.begin(); i != ul.end(); ++i) {
        getParams(params, i->second);
        addUser(params);
    }

    FavoriteManager::getInstance()->addListener(this);

    MainWindow::getInstance()->addArenaWidget(this);
}

FavoriteUsers::~FavoriteUsers(){
    FavoriteManager::getInstance()->removeListener(this);
}

void FavoriteUsers::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

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

void FavoriteUsers::getParams(VarMap &params, const FavoriteUser &user){
    const UserPtr &u = user.getUser();

    params["CID"]   = _q(u->getCID().toBase32());
    params["NICK"]  = _q(user.getNick());
    params["HUB"]   = u->isOnline()? (WulforUtil::getInstance()->getHubNames(u)) : _q(user.getUrl());
    params["SEEN"]  = u->isOnline()? tr("Online") : _q(Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen()));
    params["DESC"]  = _q(user.getDescription());
    params["SLOT"]  = user.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

void FavoriteUsers::updItem(const QString &stat, QTreeWidgetItem *item){
    if (!item)
        return;

    item->setText(2, stat);

    treeWidget->repaint();
}

void FavoriteUsers::addUser(const VarMap &params){
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

    item->setText(0, params["NICK"].toString());
    item->setText(1, params["HUB"].toString());
    item->setText(2, params["SEEN"].toString());
    item->setText(3, params["DESC"].toString());

    if (params["SLOT"].toBool())
        item->setIcon(0, WulforUtil::getInstance()->getPixmap(WulforUtil::eiBALL_GREEN));

    QString cid = params["CID"].toString();

    hash.insert(cid, item);
}

void FavoriteUsers::updateUser(const QString &cid, const QString &stat){
    if (!hash.contains(cid))
        return;

    updItem(stat, hash[cid]);
}

void FavoriteUsers::remUser(const QString &cid){
    if (!hash.contains(cid))
        return;

    QTreeWidgetItem *i = hash[cid];

    delete i;

    hash.remove(cid);

    treeWidget->repaint();
}

void FavoriteUsers::on(UserAdded, const FavoriteUser& aUser) throw() {
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

