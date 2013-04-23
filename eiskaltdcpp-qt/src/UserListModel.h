/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QStringList>
#include <QRegExp>

#ifndef _WIN32
#include <limits.h>
#endif

#include "PoolItem.h"

#include "dcpp/stdinc.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"

using namespace dcpp;

class WulforUtil;

namespace dcpp{
    inline uint qHash(const boost::intrusive_ptr<dcpp::User> &ptr){
        ulong key = (ulong)(void*)ptr.get();

        return ::qHash(key);
    }
}

#include <QHash>

class UserListProxyModel: public QSortFilterProxyModel {
    Q_OBJECT

public:
    virtual void sort(int column, Qt::SortOrder order);
};

static const unsigned COLUMN_NICK       = 0;
static const unsigned COLUMN_SHARE      = 1;
static const unsigned COLUMN_COMMENT    = 2;
static const unsigned COLUMN_TAG        = 3;
static const unsigned COLUMN_CONN       = 4;
static const unsigned COLUMN_IP         = 5;
static const unsigned COLUMN_EMAIL      = 6;

typedef QHash<QString, QVariant> UserMap;

class UserListItem: public PoolItem<UserListItem> {

public:
    UserListItem();
    UserListItem(UserListItem*, dcpp::UserPtr, const Identity&, const QString&, bool);
    virtual ~UserListItem();

    void appendChild(UserListItem *child);

    UserListItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    UserListItem *parent();
    QList<UserListItem*> childItems;

    inline const dcpp::Identity &getIdentity() { return id; }
    QString      getNick() const;
    UserPtr      getUser() const;
    qulonglong   getShare() const;
    QString      getComment() const;
    QString      getTag() const;
    QString      getIP() const;
    QString      getConnection() const;
    QString      getEmail() const;
    QString      getCID() const;
    bool         isOP() const;
    bool         isFav() const;
    bool         isAway() const;

	void         updateIdentity(const Identity&, const QString&, bool);

private:
    bool _isOp: 1;
    bool _isFav: 1;
    QString cid;
    UserListItem *parentItem;

    UserPtr ptr;
    dcpp::Identity id;
};

class UserListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    UserListModel(QObject * parent = 0);
    virtual ~UserListModel();

    virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual bool canFetchMore(const QModelIndex &parent) const;

    void clear();

    void removeUser(const UserPtr&);
    UserListItem *addUser (const UserPtr&, const Identity&, const QString&, bool);
    void updateUser(UserListItem *, const Identity&, const QString&, bool);

    UserListItem *itemForPtr(const UserPtr&);
    UserListItem *itemForNick(const QString&, const QString&);

    int getSortColumn() const {
        return sortColumn;
    }

    Qt::SortOrder getSortOrder() const {
        return sortOrder;
    }

    QString CIDforNick(const QString&, const QString&);
    bool hasUser(const UserPtr &usr) { return users.contains(usr); }

    QStringList matchNicksContaining(const QString & part, bool stripTags = false) const;
    QStringList matchNicksStartingWith(const QString & part, bool stripTags = false) const;
    QStringList matchNicksAny(const QString &part, bool stripTags = false) const;

    QStringList findItems(const QString &part, Qt::MatchFlags flags, int column) const;

    void repaint() { emit layoutChanged(); }
    void repaintItem(const UserListItem *item);
    inline void repaintData(const QModelIndex &left, const QModelIndex &right){ emit dataChanged(left, right); }

private:
    UserListItem *rootItem;

    typedef QHash<UserPtr, UserListItem*> USRMap;

    USRMap users;

    int sortColumn;
    Qt::SortOrder sortOrder;
    QRegExp stripper;

    WulforUtil *WU;
};
