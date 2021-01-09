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
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"

#include "ArenaWidget.h"
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
typedef QVariantMap VarMap;

public:

    QWidget *getWidget() override { return this; }
    QString getArenaTitle() override { return tr("Favourite users"); }
    QString getArenaShortTitle() override { return getArenaTitle(); }
    QMenu *getMenu() override { return nullptr; }
    const QPixmap &getPixmap() override { return WICON(WulforUtil::eiFAVUSERS); }
    ArenaWidget::Role role() const override { return ArenaWidget::FavoriteUsers; }

Q_SIGNALS:
    void coreUserAdded(VarMap);
    void coreUserRemoved(QString);
    void coreStatusChanged(QString,QString);

protected:
    void closeEvent(QCloseEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

    void on(UserAdded, const dcpp::FavoriteUser& aUser) noexcept override;
    void on(UserRemoved, const dcpp::FavoriteUser& aUser) noexcept override;
    void on(StatusChanged, const dcpp::FavoriteUser& aUser) noexcept override;

public Q_SLOTS:
    bool addUserToFav(const QString &id);
    bool remUserFromFav(const QString &id);
    QStringList getUsers() const;

private Q_SLOTS:
    void slotContextMenu();
    void slotHeaderMenu();
    void slotAutoGrant(bool b){ WBSET(WB_FAVUSERS_AUTOGRANT, b); }
    void slotSettingsChanged(const QString &key, const QString &);

    void addUser(const VarMap &);
    void updateUser(const QString &, const QString &);
    void remUser(const QString &);

private:
    FavoriteUsers(QWidget *parent = nullptr);
    ~FavoriteUsers() override;

    void handleRemove(const QString &);
    void handleDesc(const QString &);
    void handleGrant(const QString &);
    void handleBrowseShare(const QString &);

    void getParams(VarMap &map, const dcpp::FavoriteUser &user);
    void getFileList(const VarMap &params);

    FavoriteUsersModel *model;
};

Q_DECLARE_METATYPE (FavoriteUsers*)
