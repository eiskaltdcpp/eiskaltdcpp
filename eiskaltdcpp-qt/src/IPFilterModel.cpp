/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "IPFilterModel.h"
#include "IPFilter.h"

#include <QList>
#include <QStringList>

//#define _DEBUG_MODEL_

#ifdef _DEBUG_MODEL_
#include <QtDebug>
#endif

IPFilterModel::IPFilterModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("IP/Mask") << tr("Direction");

    rootItem = new IPFilterModelItem(rootData, NULL);
}

IPFilterModel::~IPFilterModel()
{
    delete rootItem;
}

int IPFilterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<IPFilterModelItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant IPFilterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    IPFilterModelItem *item = static_cast<IPFilterModelItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
            break;
        case Qt::DisplayRole:
            return item->data(index.column());
        case Qt::TextAlignmentRole:
            break;
        case Qt::ForegroundRole:
            break;
        case Qt::ToolTipRole:
            break;
    }

    return QVariant();
}

Qt::ItemFlags IPFilterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant IPFilterModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex IPFilterModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    IPFilterModelItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<IPFilterModelItem*>(parent.internalPointer());

    IPFilterModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex IPFilterModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    IPFilterModelItem *childItem = static_cast<IPFilterModelItem*>(index.internalPointer());
    IPFilterModelItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int IPFilterModel::rowCount(const QModelIndex &parent) const
{
    IPFilterModelItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<IPFilterModelItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void IPFilterModel::sort(int column, Qt::SortOrder order) {
    return;
}

void IPFilterModel::addResult(const QString &ip_mask, const QString &direction){
    IPFilterModelItem *item = new IPFilterModelItem(QList<QVariant>() << ip_mask << direction, rootItem);

    rootItem->appendChild(item);

    emit layoutChanged();
}

void IPFilterModel::clearModel(){
    QList<IPFilterModelItem*> childs = rootItem->childItems;

    rootItem->childItems.clear();

    qDeleteAll(childs);

    reset();

    emit layoutChanged();
}

void IPFilterModel::repaint(){
    emit layoutChanged();
}

void IPFilterModel::removeItem(IPFilterModelItem *el){
    if (!el || !rootItem->childItems.contains(el))
        return;

    beginRemoveRows(QModelIndex(), el->row(), el->row());
    rootItem->childItems.removeAt(rootItem->childItems.indexOf(el));
    endRemoveRows();

    //reset();

    emit layoutChanged();
}

void IPFilterModel::moveIndex(const QModelIndex &i, bool down){
    if (!i.isValid())
        return;

    IPFilterModelItem *it = (IPFilterModelItem*)i.internalPointer();

    if (!it || !rootItem->childItems.contains(it))
        return;

    int index = rootItem->childItems.indexOf(it);
    int control = (down?(rootItem->childItems.size()-1):0);
    int inc = (down?1:-1);

    int new_index = index+inc;

    if (index == control || index < 0)
        return;

    IPFilterModelItem *old_el = rootItem->childItems.at(new_index);

    rootItem->childItems[index]    = old_el;
    rootItem->childItems[new_index]= it;
}

void IPFilterModel::moveUp(const QModelIndex &i){
    moveIndex(i, false);
}

void IPFilterModel::moveDown(const QModelIndex &i){
    moveIndex(i, true);
}

IPFilterModelItem::IPFilterModelItem(const QList<QVariant> &data, IPFilterModelItem *parent) :
    itemData(data), parentItem(parent)
{
}

IPFilterModelItem::~IPFilterModelItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void IPFilterModelItem::appendChild(IPFilterModelItem *item) {
    childItems.append(item);
}

IPFilterModelItem *IPFilterModelItem::child(int row) {
    return childItems.value(row);
}

int IPFilterModelItem::childCount() const {
    return childItems.count();
}

int IPFilterModelItem::columnCount() const {
    return itemData.count();
}

QVariant IPFilterModelItem::data(int column) const {
    return itemData.value(column);
}

IPFilterModelItem *IPFilterModelItem::parent() {
    return parentItem;
}

int IPFilterModelItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<IPFilterModelItem*>(this));

    return 0;
}

void IPFilterModelItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}

