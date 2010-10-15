/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QStringList>
#include <QRegExp>
#include <QTimer>

#ifndef _WIN32
#include <limits.h>
#endif

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"

using namespace dcpp;

class WulforUtil;

namespace dcpp{
    inline uint qHash(const boost::intrusive_ptr<dcpp::User> &ptr){
        ulong key = (ulong)(void*)ptr.get();

#if ULONG_MAX >= 18446744073709551615UL
        //hash a 64 bit virtual address to a hash table index
        key = (~key) + (key << 18); // key = (key << 18) - key - 1;
        key = key ^ (key >> 31);
        key = key + (key << 2) + (key << 4);
        key = key ^ (key >> 11);
        key = key + (key << 6);
        key = key ^ (key >> 22);
        return uint(key);
#else
        return uint(key);
#endif
    }
}

#include <QHash>

class UserListProxyModel: public QSortFilterProxyModel {
    Q_OBJECT

public:
    virtual void sort(int column, Qt::SortOrder order);
};

#define COLUMN_NICK     0
#define COLUMN_SHARE    1
#define COLUMN_COMMENT  2
#define COLUMN_TAG      3
#define COLUMN_CONN     4
#define COLUMN_IP       5
#define COLUMN_EMAIL    6

typedef QHash<QString, QVariant> UserMap;

class UserListItem{

public:
    UserListItem(UserListItem* = NULL);
    virtual ~UserListItem();

    void appendChild(UserListItem *child);

    UserListItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    UserListItem *parent();
    QList<UserListItem*> childItems;

    bool isOp;
    bool fav;
    QString nick;
    qulonglong share;
    QString comm;
    QString tag;
    QString conn;
    QString ip;
    QString email;
    QString cid;
    QPixmap *px;
    UserPtr ptr;
private:
    UserListItem *parentItem;
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

    void addUser (const UserMap &map, const UserPtr&);
    void addUser (  const QString& nick,
                    const qlonglong share = 0,
                    const QString& comment = QString(),
                    const QString& tag = QString(),
                    const QString& conn = QString(),
                    const QString& ip = QString(),
                    const QString& email = QString(),

                    bool isOp = false,
                    bool isAway = false,
                    const QString& speed = QString(),
                    const QString& cid= QString(),
                    const UserPtr& = UserPtr()
                );

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
    void needResort();

private slots:
    void slotResort(){ sort(sortColumn, sortOrder); _needResort = false; }

private:
    UserListItem *rootItem;

    typedef QHash<UserPtr, UserListItem*> USRMap;

    USRMap users;

    int sortColumn;
    Qt::SortOrder sortOrder;
    QRegExp stripper;

    QTimer *t;
    bool _needResort;

    WulforUtil *WU;
};

#endif // USERLISTMODEL_H
