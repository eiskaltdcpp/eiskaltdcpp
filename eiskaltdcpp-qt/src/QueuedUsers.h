/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QUEUED_USERS_H_
#define QUEUED_USERS_H_

#include <QObject>
#include <QCloseEvent>
#include <QHash>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
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
    QueuedUserItem(const QList<QVariant> &data, QueuedUserItem *parent = 0);
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
    typedef QMap<QString, QVariant> VarMap;
public:

    QueuedUsersModel(QObject *parent = 0);
    ~QueuedUsersModel();

    /** */
    QVariant data(const QModelIndex &, int) const;
    /** */
    Qt::ItemFlags flags(const QModelIndex &) const;
    /** */
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;
    /** */
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const;
    /** */
    QModelIndex parent(const QModelIndex &index) const;
    /** */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /** */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    /** sort list */
    virtual void sort(int column = -1, Qt::SortOrder order = Qt::AscendingOrder);

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
    QWidget *getWidget() { return this; }
    QString getArenaTitle() { return tr("Queued Users"); }
    QString getArenaShortTitle() { return getArenaTitle(); }
    QMenu *getMenu() { return NULL; }
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiUSERS); }
    ArenaWidget::Role role() const { return ArenaWidget::QueuedUsers; }

Q_SIGNALS:
    void coreWaitingAddFile(const VarMap&);
    void coreWaitingRemoved(const VarMap&);

private Q_SLOTS:
    void slotWaitingAddFile(const VarMap&);
    void slotWaitingRemoved(const VarMap&);
    void slotContextMenu();

protected:
    void closeEvent(QCloseEvent *e);

private:
    QueuedUsers();
    virtual ~QueuedUsers();

    virtual void on(WaitingAddFile, const dcpp::HintedUser&, const std::string&) throw();
    virtual void on(WaitingRemoveUser, const dcpp::HintedUser&) throw();

    QueuedUsersModel *model;
};


Q_DECLARE_METATYPE (QueuedUsers*)

#endif
