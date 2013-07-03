/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "FavoriteUsersModel.h"

#include "WulforUtil.h"

#include <QList>
#include <QStringList>

#include "dcpp/stdinc.h"
#include "dcpp/ClientManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/CID.h"


FavoriteUsersModel::FavoriteUsersModel(QObject *parent)
    : QAbstractItemModel(parent), sortColumn(-1)
{
    QList<QVariant> rootData;
    rootData << tr("Nick") << tr("Hub") << tr("Last seen") << tr("Description");

    rootItem = new FavoriteUserItem(rootData, NULL);
}

FavoriteUsersModel::~FavoriteUsersModel()
{
    delete rootItem;
}

int FavoriteUsersModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FavoriteUserItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant FavoriteUsersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() > columnCount(QModelIndex()))
        return QVariant();

    FavoriteUserItem *item = static_cast<FavoriteUserItem*>(index.internalPointer());

    switch (role) {
        case Qt::DecorationRole: // icon
        {
            if (!index.column()){
                FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();

                for (auto i = ul.begin(); i != ul.end(); ++i) {
                    const dcpp::FavoriteUser &u = i->second;

                    if (_q(u.getUser()->getCID().toBase32()) == item->cid){
                        if (u.isSet(FavoriteUser::FLAG_GRANTSLOT))
                            return WICON(WulforUtil::eiBALL_GREEN).scaled(16, 16);
                    }
                }
            }

            break;
        }
        case Qt::DisplayRole:
            return item->data(index.column());
        case Qt::TextAlignmentRole:
            break;
        case Qt::ForegroundRole:
            break;
        case Qt::ToolTipRole:
            break;
        case Qt::CheckStateRole:
            break;
    }

    return QVariant();
}

Qt::ItemFlags FavoriteUsersModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

QVariant FavoriteUsersModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex FavoriteUsersModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FavoriteUserItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FavoriteUserItem*>(parent.internalPointer());

    FavoriteUserItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FavoriteUsersModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FavoriteUserItem *childItem = static_cast<FavoriteUserItem*>(index.internalPointer());
    FavoriteUserItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FavoriteUsersModel::rowCount(const QModelIndex &parent) const
{
    FavoriteUserItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FavoriteUserItem*>(parent.internalPointer());

    return parentItem->childCount();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<FavoriteUserItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<FavoriteUserItem*>& items, FavoriteUserItem* item) {
        auto it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const FavoriteUserItem * l, const FavoriteUserItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                 case COLUMN_USER_NICK:
                     return AttrCmp<COLUMN_USER_NICK>;
                 case COLUMN_USER_HOST:
                     return AttrCmp<COLUMN_USER_HOST>;
                 case COLUMN_USER_SEEN:
                     return AttrCmp<COLUMN_USER_SEEN>;
                 default:
                     return AttrCmp<COLUMN_USER_DESC>;
            }

            return 0;
        }
        template <int i>
        bool static AttrCmp(const FavoriteUserItem * l, const FavoriteUserItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const FavoriteUserItem * l, const FavoriteUserItem * r) {
            return Cmp(l->data(column).toULongLong(), r->data(column).toULongLong());
       }
        template <typename T>
        bool static Cmp(const T& l, const T& r);
};

template <> template <typename T>
bool inline Compare<Qt::AscendingOrder>::Cmp(const T& l, const T& r) {
    return l < r;
}

template <> template <typename T>
bool inline Compare<Qt::DescendingOrder>::Cmp(const T& l, const T& r) {
    return l > r;
}
}

void FavoriteUsersModel::sort(int column, Qt::SortOrder order) {
    sortColumn = column;
    sortOrder = order;

    if (!rootItem || rootItem->childItems.empty())
        return;

    if (column == -1)
        return;

    emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
        Compare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
    else if (order == Qt::DescendingOrder)
        Compare<Qt::DescendingOrder>().sort(column, rootItem->childItems);

    emit layoutChanged();
}

void FavoriteUsersModel::addUser(const FavoriteUsersModel::VarMap &params){
    if (itemHash.contains(params["CID"].toString()))
        return;

    FavoriteUserItem *item = new FavoriteUserItem(QList<QVariant>() << params["NICK"].toString()
                                                                    << params["HUB"].toString()
                                                                    << params["SEEN"].toString()
                                                                    << params["DESC"].toString(),
                                                                    rootItem);
    item->cid = params["CID"].toString();

    rootItem->appendChild(item);

    itemHash.insert(item->cid, item);

    emit layoutChanged();
}

void FavoriteUsersModel::updateUserStatus(const QString &cid, const QString &stat, const QString &hubUrl){
    if (cid.isEmpty() || !itemHash.contains(cid))
        return;

    FavoriteUserItem *i = itemHash.value(cid);

    i->updateColumn(COLUMN_USER_SEEN, stat);
    i->updateColumn(COLUMN_USER_HOST, hubUrl);
}

QStringList FavoriteUsersModel::getUsers() const{
    QStringList list;

    for (int i = 0; i < rootItem->childCount(); i++){
        FavoriteUserItem *item = rootItem->child(i);
        QString nick = item->data(COLUMN_USER_NICK).toString();
        QString cid  = item->cid;
        QString hub  = item->data(COLUMN_USER_HOST).toString();

        list.push_back(nick + ";" + cid + ";" + hub);
    }

    return list;
}

void FavoriteUsersModel::removeUser(const QString &cid){
    if (cid.isEmpty() || !itemHash.contains(cid))
        return;

    FavoriteUserItem *i = itemHash.value(cid);
    int r = i->row();

    beginRemoveRows(QModelIndex(), r, r);
    {
        rootItem->childItems.removeAt(r);
        itemHash.remove(cid);

        delete i;
    }
    endRemoveRows();
}

void FavoriteUsersModel::repaint(){
    emit layoutChanged();
}

FavoriteUserItem::FavoriteUserItem(const QList<QVariant> &data, FavoriteUserItem *parent) :
    itemData(data), parentItem(parent)
{
}

FavoriteUserItem::~FavoriteUserItem()
{
    if (!childItems.isEmpty())
        qDeleteAll(childItems);
}

void FavoriteUserItem::appendChild(FavoriteUserItem *item) {
    childItems.append(item);
}

FavoriteUserItem *FavoriteUserItem::child(int row) {
    return childItems.value(row);
}

int FavoriteUserItem::childCount() const {
    return childItems.count();
}

int FavoriteUserItem::columnCount() const {
    return itemData.count();
}

QVariant FavoriteUserItem::data(int column) const {
    return itemData.at(column);
}

FavoriteUserItem *FavoriteUserItem::parent() {
    return parentItem;
}

int FavoriteUserItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FavoriteUserItem*>(this));

    return 0;
}

void FavoriteUserItem::updateColumn(unsigned column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}
