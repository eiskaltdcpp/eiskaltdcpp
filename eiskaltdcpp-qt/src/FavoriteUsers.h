/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QObject>
#include <QWidget>
#include <QCloseEvent>
#include <QHash>
#include <QTreeWidgetItem>
#include <QMetaType>

#include "dcpp/stdinc.h"
#include "dcpp/FavoriteManager.h"

#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "ui_UIFavoriteUsers.h"

class FavoriteUsersModel;

#include "QtContextAware.h"
#include "QtContext.h"

class FavoriteUsers :
        public QWidget,
        public dcpp::FavoriteManagerListener,
        public ArenaWidget,
        private Ui::UIFavoriteUsers,
        public QtContextAware
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)

friend class QtContext;
typedef QVariantMap VarMap;

public:

    virtual QWidget *getWidget() { return this; }
    virtual QString getArenaTitle() { return tr("Favourite users"); }
    virtual QString getArenaShortTitle() { return getArenaTitle(); }
    virtual QMenu *getMenu() { return nullptr; }
    const QPixmap &getPixmap(){ return qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiFAVUSERS); }
    ArenaWidget::Role role() const { return ArenaWidget::FavoriteUsers; }

Q_SIGNALS:
    void coreUserAdded(VarMap);
    void coreUserRemoved(QString);
    void coreStatusChanged(QString,QString);

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void on(UserAdded, const dcpp::FavoriteUser& aUser) noexcept;
    virtual void on(UserRemoved, const dcpp::FavoriteUser& aUser) noexcept;
    virtual void on(StatusChanged, const dcpp::FavoriteUser& aUser) noexcept;

public Q_SLOTS:
    bool addUserToFav(const QString &id);
    bool remUserFromFav(const QString &id);
    QStringList getUsers() const;

private Q_SLOTS:
    void slotContextMenu();
    void slotHeaderMenu();
    void slotAutoGrant(bool b){ qtCtx()->settings()->setBool(WB_FAVUSERS_AUTOGRANT, b); }
    void slotSettingsChanged(const QString &key, const QString &);

    void addUser(const VarMap &);
    void updateUser(const QString &, const QString &);
    void remUser(const QString &);

public:
    FavoriteUsers(dcpp::DCContext& ctx, QWidget *parent = nullptr);
    ~FavoriteUsers() override;


private:

    void handleRemove(const QString &);
    void handleDesc(const QString &);
    void handleGrant(const QString &);
    void handleBrowseShare(const QString &);

    void getParams(VarMap &map, const dcpp::FavoriteUser &user);
    void getFileList(const VarMap &params);

    FavoriteUsersModel *model;
};

Q_DECLARE_METATYPE (FavoriteUsers*)
