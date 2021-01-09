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
#include <QCloseEvent>
#include <QHash>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"
#include "dcpp/UploadManager.h"
#include "dcpp/UploadManagerListener.h"
#include "dcpp/User.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "ui_UIQueuedUsers.h"

class QueuedUserItem
{

public:
    QueuedUserItem(const QList<QVariant> &data, QueuedUserItem *parent = nullptr);
    virtual ~QueuedUserItem();

    void appendChild(QueuedUserItem *child);

    QueuedUserItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    QueuedUserItem *parent() const;

    QString cid;
    QString file;
    QString hub;

    QList<QueuedUserItem*> childItems;
private:

    QList<QVariant> itemData;
    QueuedUserItem *parentItem;
};

class QueuedUsersModel : public QAbstractItemModel
{
    Q_OBJECT
    typedef QVariantMap VarMap;
public:

    QueuedUsersModel(QObject *parent = nullptr);
    ~QueuedUsersModel() override;

    /** */
    QVariant data(const QModelIndex &, int) const override;
    /** */
    Qt::ItemFlags flags(const QModelIndex &) const override;
    /** */
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    /** */
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const override;
    /** */
    QModelIndex parent(const QModelIndex &index) const override;
    /** */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /** */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /** sort list */
    void sort(int column = -1, Qt::SortOrder order = Qt::AscendingOrder) override;

    void addResult(const VarMap& map);
    void remResult(const VarMap& map);

private:
    QueuedUserItem *rootItem;
    /** */
    QHash<QString, QueuedUserItem*> cids;
};

class QueuedUsers:
        public QWidget,
        public ArenaWidget,
        private Ui::UIQueuedUsers,
        public dcpp::Singleton<QueuedUsers>,
        private dcpp::UploadManagerListener
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    friend class dcpp::Singleton<QueuedUsers>;

public:
    QWidget *getWidget() override { return this; }
    QString getArenaTitle() override { return tr("Queued Users"); }
    QString getArenaShortTitle() override { return getArenaTitle(); }
    QMenu *getMenu() override { return nullptr; }
    const QPixmap &getPixmap() override { return WulforUtil::getInstance()->getPixmap(WulforUtil::eiUSERS); }
    ArenaWidget::Role role() const override { return ArenaWidget::QueuedUsers; }

Q_SIGNALS:
    void coreWaitingAddFile(const VarMap&);
    void coreWaitingRemoved(const VarMap&);

private Q_SLOTS:
    void slotWaitingAddFile(const VarMap&);
    void slotWaitingRemoved(const VarMap&);
    void slotContextMenu();

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    QueuedUsers();
    ~QueuedUsers() override;

    void on(WaitingAddFile, const dcpp::HintedUser&, const std::string&) noexcept override;
    void on(WaitingRemoveUser, const dcpp::HintedUser&) noexcept override;

    QueuedUsersModel *model;
};

Q_DECLARE_METATYPE (QueuedUsers*)
