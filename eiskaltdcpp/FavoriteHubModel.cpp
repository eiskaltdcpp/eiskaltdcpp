/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FavoriteHubModel.h"

#include <QList>
#include <QStringList>

FavoriteHubModel::FavoriteHubModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("Autoconnect") << tr("Name") << tr("Description")
             << tr("Address") << tr("Nick") << tr("Password") << tr("User description")
             << tr("Remote encoding");

    rootItem = new FavoriteHubItem(rootData, NULL);
}

FavoriteHubModel::~FavoriteHubModel()
{
    delete rootItem;
}

int FavoriteHubModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FavoriteHubItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant FavoriteHubModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() > columnCount(QModelIndex()))
        return QVariant();

    FavoriteHubItem *item = static_cast<FavoriteHubItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
            break;
        case Qt::DisplayRole:
            if (index.column() == COLUMN_HUB_AUTOCONNECT)
                break;
            else if (index.column() != COLUMN_HUB_PASSWORD)
                return item->data(index.column());
            else
                return tr("******");

            break;
        case Qt::TextAlignmentRole:
            break;
        case Qt::ForegroundRole:
            break;
        case Qt::ToolTipRole:
            break;
        case Qt::CheckStateRole:
            if (index.column() == COLUMN_HUB_AUTOCONNECT)
                return item->data(COLUMN_HUB_AUTOCONNECT);
            break;
    }

    return QVariant();
}

Qt::ItemFlags FavoriteHubModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    if (index.column() == COLUMN_HUB_AUTOCONNECT)
        flags |= Qt::ItemIsUserCheckable;
    else if (index.column() == COLUMN_HUB_PASSWORD || index.column() == COLUMN_HUB_ENCODING)
        flags &= ~Qt::ItemIsEditable;

    return flags;
}

QVariant FavoriteHubModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex FavoriteHubModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FavoriteHubItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FavoriteHubItem*>(parent.internalPointer());

    FavoriteHubItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FavoriteHubModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FavoriteHubItem *childItem = static_cast<FavoriteHubItem*>(index.internalPointer());
    FavoriteHubItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FavoriteHubModel::rowCount(const QModelIndex &parent) const
{
    FavoriteHubItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FavoriteHubItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void FavoriteHubModel::sort(int column, Qt::SortOrder order) {
    return;
}

void FavoriteHubModel::clearModel(){
    QList<FavoriteHubItem*> childs = rootItem->childItems;   //Copying list in another place
    rootItem->childItems.clear();                             //Cleaning root of the model. Do not delete items directly from the root item

    qDeleteAll(childs);

    rootItem->childItems.clear();

    reset();

    emit layoutChanged();
}

bool FavoriteHubModel::removeItem(const QModelIndex &el){
    if (!el.isValid())
        return false;

    FavoriteHubItem *item = (FavoriteHubItem*)el.internalPointer();

    if (!item || !rootItem->childItems.contains(item))
        return false;

    beginRemoveRows(QModelIndex(), item->row(), item->row());
    rootItem->childItems.removeAt(rootItem->childItems.indexOf(item));
    endRemoveRows();

    delete item;

    emit layoutChanged();

    return true;
}

bool FavoriteHubModel::removeItem(const FavoriteHubItem *item){
    if (!item || !rootItem->childItems.contains(const_cast<FavoriteHubItem*>(item)))
        return false;

    QModelIndex i = index(item->row(), 0, QModelIndex());

    removeItem(i);
}

void FavoriteHubModel::addResult(QList<QVariant> &data){

    FavoriteHubItem *item = new FavoriteHubItem(data, rootItem);
    rootItem->appendChild(item);

    emit layoutChanged();
}

const QList<FavoriteHubItem*> &FavoriteHubModel::getItems(){
    return rootItem->childItems;
}

void FavoriteHubModel::repaint(){
    emit layoutChanged();
}

FavoriteHubItem::FavoriteHubItem(const QList<QVariant> &data, FavoriteHubItem *parent) :
    itemData(data), parentItem(parent)
{
}

FavoriteHubItem::~FavoriteHubItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void FavoriteHubItem::appendChild(FavoriteHubItem *item) {
    childItems.append(item);
}

FavoriteHubItem *FavoriteHubItem::child(int row) {
    return childItems.value(row);
}

int FavoriteHubItem::childCount() const {
    return childItems.count();
}

int FavoriteHubItem::columnCount() const {
    return itemData.count();
}

QVariant FavoriteHubItem::data(int column) const {
    return itemData.at(column);
}

FavoriteHubItem *FavoriteHubItem::parent() {
    return parentItem;
}

int FavoriteHubItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FavoriteHubItem*>(this));

    return 0;
}

void FavoriteHubItem::updateColumn(unsigned column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}
