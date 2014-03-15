/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FavoriteHubModel.h"

#include <QList>
#include <QStringList>

FavoriteHubModel::FavoriteHubModel(QObject *parent)
    : QAbstractItemModel(parent), sortColumn(-1), sortOrder(Qt::AscendingOrder)
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

    if (index.isValid())
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else
        flags |= Qt::ItemIsDragEnabled;

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

bool FavoriteHubModel::insertRow(int row, const QModelIndex &parent){
    return true;
}

bool FavoriteHubModel::removeRow(int row, const QModelIndex &parent){
    return true;
}

bool FavoriteHubModel::insertRows(int position, int rows, const QModelIndex &index){
    FavoriteHubItem *from = NULL;

    beginRemoveRows(QModelIndex(), position, position);
    {
        from = rootItem->childItems.takeAt(position);
    }
    endRemoveRows();

    beginInsertRows(QModelIndex(), index.row(), index.row());
    {
        rootItem->childItems.insert(index.row(), from);
    }
    endInsertRows();

    return true;
}

bool FavoriteHubModel::removeRows(int position, int rows, const QModelIndex &index){
    return true;
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<FavoriteHubItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<FavoriteHubItem*>& items, FavoriteHubItem* item) {
        auto it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const FavoriteHubItem * l, const FavoriteHubItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                 case COLUMN_HUB_AUTOCONNECT:
                     return NumCmp<COLUMN_HUB_AUTOCONNECT>;
                 case COLUMN_HUB_ADDRESS:
                     return AttrCmp<COLUMN_HUB_ADDRESS>;
                 case COLUMN_HUB_DESC:
                     return AttrCmp<COLUMN_HUB_DESC>;
                 case COLUMN_HUB_ENCODING:
                     return AttrCmp<COLUMN_HUB_ENCODING>;
                 case COLUMN_HUB_NAME:
                     return AttrCmp<COLUMN_HUB_NAME>;
                 case COLUMN_HUB_NICK:
                     return AttrCmp<COLUMN_HUB_NICK>;
                 case COLUMN_HUB_PASSWORD:
                     return AttrCmp<COLUMN_HUB_PASSWORD>;
                 default:
                     return AttrCmp<COLUMN_HUB_USERDESC>;
            }

            return 0;
        }
        template <int i>
        bool static AttrCmp(const FavoriteHubItem * l, const FavoriteHubItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const FavoriteHubItem * l, const FavoriteHubItem * r) {
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

void FavoriteHubModel::sort(int column, Qt::SortOrder order) {
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

void FavoriteHubModel::clearModel(){
    QList<FavoriteHubItem*> childs = rootItem->childItems;   //Copying list in another place
    rootItem->childItems.clear();                             //Cleaning root of the model. Do not delete items directly from the root item

    qDeleteAll(childs);

    rootItem->childItems.clear();

    reset();

    emit layoutChanged();
}

void FavoriteHubModel::reset() {
    beginResetModel();
    endResetModel();
}

bool FavoriteHubModel::removeItem(const QModelIndex &el){
    if (!el.isValid())
        return false;

    FavoriteHubItem *item = static_cast<FavoriteHubItem*>(el.internalPointer());

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

    return true;
}

void FavoriteHubModel::addResult(QList<QVariant> &data){

    FavoriteHubItem *item = new FavoriteHubItem(data, rootItem);
    rootItem->appendChild(item);

    emit layoutChanged();
}

QModelIndex FavoriteHubModel::moveDown(const QModelIndex &index){
    if (index.row() >= rootItem->childCount() - 1)
        return QModelIndex();

    FavoriteHubItem *item = NULL;

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    {
        item = rootItem->childItems.takeAt(index.row());
    }
    endRemoveRows();

    beginInsertRows(QModelIndex(), index.row()+1, index.row()+1);
    {
        rootItem->childItems.insert(index.row()+1, item);
    }
    endInsertRows();

    return this->index(index.row()+1, index.column(), QModelIndex());
}

QModelIndex FavoriteHubModel::moveUp(const QModelIndex &index){
    if (index.row() < 1)
        return QModelIndex();

    FavoriteHubItem *item = NULL;

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    {
        item = rootItem->childItems.takeAt(index.row());
    }
    endRemoveRows();

    beginInsertRows(QModelIndex(), index.row()-1, index.row()-1);
    {
        rootItem->childItems.insert(index.row()-1, item);
    }
    endInsertRows();

    return this->index(index.row()-1, index.column(), QModelIndex());
}

const QList<FavoriteHubItem*> &FavoriteHubModel::getItems(){
    return rootItem->childItems;
}

void FavoriteHubModel::repaint(){
    emit layoutChanged();
}

Qt::DropActions FavoriteHubModel::supportedDropActions() const{
    return Qt::MoveAction;
}

FavoriteHubItem::FavoriteHubItem(const QList<QVariant> &data, FavoriteHubItem *parent) :
    itemData(data), parentItem(parent)
{
}

FavoriteHubItem::~FavoriteHubItem()
{
    if (!childItems.isEmpty())
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
