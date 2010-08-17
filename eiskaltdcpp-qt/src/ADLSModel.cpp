/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADLSModel.h"

#include <QList>
#include <QStringList>

ADLSModel::ADLSModel(QObject *parent)
    : QAbstractItemModel(parent), sortColumn(-1), sortOrder(Qt::AscendingOrder)
{
    QList<QVariant> rootData;
    rootData << tr("Checked") << tr("Search String") << tr("Type source")
             << tr("Name directory") << tr("Min. Size") << tr("Max. Size");

    rootItem = new ADLSItem(rootData, NULL);
}

ADLSModel::~ADLSModel()
{
    delete rootItem;
}

int ADLSModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ADLSItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant ADLSModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() > columnCount(QModelIndex()))
        return QVariant();

    ADLSItem *item = static_cast<ADLSItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
            break;
        case Qt::DisplayRole:
            if (index.column() != COLUMN_CHECK)
                return item->data(index.column());
            break;
        case Qt::TextAlignmentRole:
            break;
        case Qt::ForegroundRole:
            break;
        case Qt::ToolTipRole:
            break;
        case Qt::CheckStateRole:
            if (index.column() == COLUMN_CHECK)
                return item->data(COLUMN_CHECK);
            break;
    }

    return QVariant();
}

Qt::ItemFlags ADLSModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    if (index.isValid())
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else
        flags |= Qt::ItemIsDragEnabled;

    if (index.column() == COLUMN_CHECK)
        flags |= Qt::ItemIsUserCheckable;
    return flags;
}

QVariant ADLSModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex ADLSModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ADLSItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ADLSItem*>(parent.internalPointer());

    ADLSItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ADLSModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ADLSItem *childItem = static_cast<ADLSItem*>(index.internalPointer());
    ADLSItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ADLSModel::rowCount(const QModelIndex &parent) const
{
    ADLSItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ADLSItem*>(parent.internalPointer());

    return parentItem->childCount();
}

bool ADLSModel::insertRow(int row, const QModelIndex &parent){
    return true;
}

bool ADLSModel::removeRow(int row, const QModelIndex &parent){
    return true;
}

bool ADLSModel::insertRows(int position, int rows, const QModelIndex &index){
    ADLSItem *from = NULL;

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

bool ADLSModel::removeRows(int position, int rows, const QModelIndex &index){
    return true;
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<ADLSItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<ADLSItem*>& items, ADLSItem* item) {
        QList<ADLSItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const ADLSItem * l, const ADLSItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                 case COLUMN_CHECK:
                     return NumCmp<COLUMN_CHECK>;
                 case COLUMN_SSTRING:
                     return AttrCmp<COLUMN_SSTRING>;
                 case COLUMN_DIRECTORY:
                     return AttrCmp<COLUMN_DIRECTORY>;
                 case COLUMN_MAXSIZE:
                     return AttrCmp<COLUMN_MAXSIZE>;
                 case COLUMN_MINSIZE:
                     return AttrCmp<COLUMN_MINSIZE>;
                 default:
                     break;//return AttrCmp<COLUMN_SSTRING>;
            }

            return 0;
        }
        template <int i>
        bool static AttrCmp(const ADLSItem * l, const ADLSItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const ADLSItem * l, const ADLSItem * r) {
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

void ADLSModel::sort(int column, Qt::SortOrder order) {
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

void ADLSModel::clearModel(){
    QList<ADLSItem*> childs = rootItem->childItems;   //Copying list in another place
    rootItem->childItems.clear();                             //Cleaning root of the model. Do not delete items directly from the root item

    qDeleteAll(childs);

    rootItem->childItems.clear();

    reset();

    emit layoutChanged();
}

bool ADLSModel::removeItem(const QModelIndex &el){
    if (!el.isValid())
        return false;

    ADLSItem *item = (ADLSItem*)el.internalPointer();

    if (!item || !rootItem->childItems.contains(item))
        return false;

    beginRemoveRows(QModelIndex(), item->row(), item->row());
    rootItem->childItems.removeAt(rootItem->childItems.indexOf(item));
    endRemoveRows();

    delete item;

    emit layoutChanged();

    return true;
}

bool ADLSModel::removeItem(const ADLSItem *item){
    if (!item || !rootItem->childItems.contains(const_cast<ADLSItem*>(item)))
        return false;

    QModelIndex i = index(item->row(), 0, QModelIndex());

    removeItem(i);
}

void ADLSModel::addResult(QList<QVariant> &data){

    ADLSItem *item = new ADLSItem(data, rootItem);
    rootItem->appendChild(item);

    emit layoutChanged();
}

QModelIndex ADLSModel::moveDown(const QModelIndex &index){
    if (index.row() >= rootItem->childCount() - 1)
        return QModelIndex();

    ADLSItem *item = NULL;

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

QModelIndex ADLSModel::moveUp(const QModelIndex &index){
    if (index.row() < 1)
        return QModelIndex();

    ADLSItem *item = NULL;

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

const QList<ADLSItem*> &ADLSModel::getItems(){
    return rootItem->childItems;
}

void ADLSModel::repaint(){
    emit layoutChanged();
}

Qt::DropActions ADLSModel::supportedDropActions() const{
    return Qt::MoveAction;
}

ADLSItem::ADLSItem(const QList<QVariant> &data, ADLSItem *parent) :
    itemData(data), parentItem(parent)
{
}

ADLSItem::~ADLSItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void ADLSItem::appendChild(ADLSItem *item) {
    childItems.append(item);
}

ADLSItem *ADLSItem::child(int row) {
    return childItems.value(row);
}

int ADLSItem::childCount() const {
    return childItems.count();
}

int ADLSItem::columnCount() const {
    return itemData.count();
}

QVariant ADLSItem::data(int column) const {
    return itemData.at(column);
}

ADLSItem *ADLSItem::parent() {
    return parentItem;
}

int ADLSItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ADLSItem*>(this));

    return 0;
}

void ADLSItem::updateColumn(unsigned column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}
