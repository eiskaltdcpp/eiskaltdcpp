/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef FAVORITEUSERS_H
#define FAVORITEUSERS_H

#include <QObject>
#include <QWidget>
#include <QCloseEvent>
#include <QHash>
#include <QTreeWidgetItem>
#include <QMetaType>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"

#include "ArenaWidget.h"
#include "Func.h"
#include "WulforUtil.h"

#include "ui_UIFavoriteUsers.h"

class FavoriteUsersModel;

class FavoriteUsers :
        public QWidget,
        public dcpp::Singleton<FavoriteUsers>,
        public dcpp::FavoriteManagerListener,
        public ArenaWidget,
        private Ui::UIFavoriteUsers
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)

friend class dcpp::Singleton<FavoriteUsers>;
typedef QMap<QString, QVariant> VarMap;

class FavUserEvent: public QEvent{
public:
    static const QEvent::Type EventAddUser  = static_cast<QEvent::Type>(1211);
    static const QEvent::Type EventRemUser  = static_cast<QEvent::Type>(1212);
    static const QEvent::Type EventUpdUser  = static_cast<QEvent::Type>(1213);

    FavUserEvent(): QEvent(EventAddUser) {}
    FavUserEvent(const dcpp::CID &cid, const QString &stat): QEvent(EventUpdUser), cid(cid), stat(stat) {}
    FavUserEvent(const dcpp::CID &cid):QEvent(EventRemUser), cid(cid) {}
    virtual ~FavUserEvent() { }

    VarMap &getMap() { return map; }
    QString &getStat() {return stat; }
    dcpp::CID &getCID() {return cid; }
private:
    dcpp::CID cid;
    QString stat;
    VarMap map;
};

public:

    virtual QWidget *getWidget() { return this; }
    virtual QString getArenaTitle() { return tr("Favourite users"); }
    virtual QString getArenaShortTitle() { return getArenaTitle(); }
    virtual QMenu *getMenu() { return NULL; }
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiFAVUSERS); }
    ArenaWidget::Role role() const { return ArenaWidget::FavoriteUsers; }

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void customEvent(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void on(UserAdded, const dcpp::FavoriteUser& aUser) throw();
    virtual void on(UserRemoved, const dcpp::FavoriteUser& aUser) throw();
    virtual void on(StatusChanged, const dcpp::UserPtr& aUser) throw();

public Q_SLOTS:
    bool addUserToFav(const QString &id);
    bool remUserFromFav(const QString &id);
    QStringList getUsers() const;

private Q_SLOTS:
    void slotContextMenu();
    void slotHeaderMenu();
    void slotAutoGrant(bool b){ WBSET(WB_FAVUSERS_AUTOGRANT, b); }

private:
    FavoriteUsers(QWidget *parent = NULL);
    virtual ~FavoriteUsers();

    void handleRemove(const QString &);
    void handleDesc(const QString &);
    void handleGrant(const QString &);

    void getParams(VarMap &map, const dcpp::FavoriteUser &);

    void addUser(const VarMap &);
    void updateUser(const QString &, const QString &);
    void remUser(const QString &);

    FavoriteUsersModel *model;
};

Q_DECLARE_METATYPE (FavoriteUsers*)

#endif // FAVORITEUSERS_H
