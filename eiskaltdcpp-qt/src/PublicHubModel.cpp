/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "PublicHubModel.h"
#include "WulforUtil.h"

#include "dcpp/Util.h"

#include <QList>
#include <QStringList>

void PublicHubProxyModel::sort(int column, Qt::SortOrder order){
    if (sourceModel())
        sourceModel()->sort(column, order);
}

PublicHubModel::PublicHubModel(QObject *parent)
    : QAbstractItemModel(parent), sortColumn(-1), sortOrder(Qt::AscendingOrder)
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Description") << tr("Users") << tr("Address")
             << tr("Country") << tr("Shared") << tr("Min share") << tr("Min slots")
             << tr("Max hubs") << tr("Max users") << tr("Reliability") << tr("Rating");

    rootItem = new PublicHubItem(rootData, NULL);
}

PublicHubModel::~PublicHubModel()
{
    delete rootItem;
}

int PublicHubModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<PublicHubItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant PublicHubModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() > columnCount(QModelIndex()))
        return QVariant();

    PublicHubItem *item = static_cast<PublicHubItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
            break;
        case Qt::DisplayRole:
            if (index.column() == COLUMN_PHUB_SHARED || index.column() == COLUMN_PHUB_MINSHARE)
                return WulforUtil::formatBytes(item->data(index.column()).toULongLong());

            return item->data(index.column());
        case Qt::TextAlignmentRole:
        {
            int i = index.column();

            if (i == COLUMN_PHUB_SHARED || i == COLUMN_PHUB_RATING || i == COLUMN_PHUB_MAXHUBS || i == COLUMN_PHUB_MAXUSERS ||
                i == COLUMN_PHUB_MINSHARE || i == COLUMN_PHUB_MINSLOTS || i == COLUMN_PHUB_REL)
                return Qt::AlignRight;

            break;
        }
        case Qt::ForegroundRole:
            break;
        case Qt::ToolTipRole:
            break;
    }

    return QVariant();
}

Qt::ItemFlags PublicHubModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

QVariant PublicHubModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex PublicHubModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    PublicHubItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PublicHubItem*>(parent.internalPointer());

    PublicHubItem *childItem = parentItem->child(row);

    if (childItem && rootItem->childItems.contains(childItem))
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex PublicHubModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int PublicHubModel::rowCount(const QModelIndex &parent) const
{
    PublicHubItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PublicHubItem*>(parent.internalPointer());

    return parentItem->childCount();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<PublicHubItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<PublicHubItem*>& items, PublicHubItem* item) {
        QList<PublicHubItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const PublicHubItem * l, const PublicHubItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                case COLUMN_PHUB_MAXHUBS:
                     return NumCmp<COLUMN_PHUB_MAXHUBS>;
                case COLUMN_PHUB_MAXUSERS:
                     return NumCmp<COLUMN_PHUB_MAXUSERS>;
                case COLUMN_PHUB_MINSHARE:
                     return NumCmp<COLUMN_PHUB_MINSHARE>;
                case COLUMN_PHUB_MINSLOTS:
                     return NumCmp<COLUMN_PHUB_MINSLOTS>;
                case COLUMN_PHUB_REL:
                     return DblCmp<COLUMN_PHUB_REL>;
                case COLUMN_PHUB_SHARED:
                     return NumCmp<COLUMN_PHUB_SHARED>;
                case COLUMN_PHUB_ADDRESS:
                     return AttrCmp<COLUMN_PHUB_ADDRESS>;
                case COLUMN_PHUB_COUNTRY:
                     return AttrCmp<COLUMN_PHUB_COUNTRY>;
                case COLUMN_PHUB_DESC:
                     return AttrCmp<COLUMN_PHUB_DESC>;
                case COLUMN_PHUB_NAME:
                     return AttrCmp<COLUMN_PHUB_NAME>;
                case COLUMN_PHUB_RATING:
                     return AttrCmp<COLUMN_PHUB_RATING>;
                default:
                     return NumCmp<COLUMN_PHUB_USERS>;
            }

            return 0;
        }
        template <int i>
        bool static AttrCmp(const PublicHubItem * l, const PublicHubItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const PublicHubItem * l, const PublicHubItem * r) {
            return Cmp(l->data(column).toULongLong(), r->data(column).toULongLong());
        }
        template <int column>
        bool static DblCmp(const PublicHubItem * l, const PublicHubItem * r) {
            return Cmp(l->data(column).toDouble(), r->data(column).toDouble());
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

void PublicHubModel::sort(int column, Qt::SortOrder order) {
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

void PublicHubModel::clearModel(){
    emit layoutAboutToBeChanged();

    beginRemoveRows(QModelIndex(), 0, rootItem->childCount()-1);
    qDeleteAll(rootItem->childItems);
    rootItem->childItems.clear();
    endRemoveRows();

    emit layoutChanged();
}

void PublicHubModel::addResult(const QList<QVariant> &data, dcpp::HubEntry *entry){

    PublicHubItem *item = new PublicHubItem(data, rootItem);
    item->entry = entry;

    beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
    rootItem->appendChild(item);
    endInsertRows();

    emit layoutChanged();
}


PublicHubItem::PublicHubItem(const QList<QVariant> &data, PublicHubItem *parent) :
    itemData(data), parentItem(parent), entry(NULL)
{
}

PublicHubItem::~PublicHubItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void PublicHubItem::appendChild(PublicHubItem *item) {
    childItems.append(item);
}

PublicHubItem *PublicHubItem::child(int row) {
    return childItems.value(row);
}

int PublicHubItem::childCount() const {
    return childItems.count();
}

int PublicHubItem::columnCount() const {
    return itemData.count();
}

QVariant PublicHubItem::data(int column) const {
    return itemData.at(column);
}

PublicHubItem *PublicHubItem::parent() {
    return parentItem;
}

int PublicHubItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<PublicHubItem*>(this));

    return 0;
}

void PublicHubItem::updateColumn(unsigned column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}

