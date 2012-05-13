/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SpyModel.h"

#include <QtDebug>

SpyModel::SpyModel(QObject *parent):
        QAbstractItemModel(parent), isSort(false), sortColumn(0), sortOrder(Qt::DescendingOrder)
{
    QList<QVariant> rootData;
    rootData << tr("Count") << tr("Search string");

    rootItem = new SpyItem(rootData);
}

SpyModel::~SpyModel()
{
    foreach(SpyItem *i, rootItem->childItems)
        pool.destroy(i);

    rootItem->childItems.clear();

    delete rootItem;
}

int SpyModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<SpyItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant SpyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    SpyItem *item = static_cast<SpyItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
        {
            break;
        }
        case Qt::DisplayRole:
            return item->data(index.column());
        case Qt::TextAlignmentRole:
        {
            break;
        }
        case Qt::ForegroundRole:
        {
            break;
        }
        case Qt::BackgroundColorRole:
            break;
        case Qt::ToolTipRole:
            break;
    }

    return QVariant();
}

Qt::ItemFlags SpyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SpyModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex SpyModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SpyItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SpyItem*>(parent.internalPointer());

    SpyItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SpyModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SpyItem *childItem = static_cast<SpyItem*>(index.internalPointer());
    SpyItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int SpyModel::rowCount(const QModelIndex &parent) const
{
    SpyItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SpyItem*>(parent.internalPointer());

    return parentItem->childCount();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<SpyItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<SpyItem*>& items, SpyItem* item) {
        auto it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
#ifdef _DEBUG_MODEL_
        qDebug() << "Item inserted at " << items.indexOf(*it) << " position";
#endif
    }

    private:
        typedef bool (*AttrComp)(const SpyItem * l, const SpyItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                case COLUMN_SPY_COUNT:
                    return NumCmp<COLUMN_SPY_COUNT>;
                default:
                    return AttrCmp<COLUMN_SPY_STRING>;
            }
        }
        template <int i>
        bool static AttrCmp(const SpyItem * l, const SpyItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <typename T, T (SpyItem::*attr)>
        bool static AttrCmp(const SpyItem * l, const SpyItem * r) {
            return Cmp(l->*attr, r->*attr);
        }
        template <int i>
        bool static NumCmp(const SpyItem * l, const SpyItem * r) {
            return Cmp(l->data(i).toULongLong(), r->data(i).toULongLong());
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

} //namespace

void SpyModel::sort(int column, Qt::SortOrder order) {

    static int c = 0;

    if (column < 0)
        column = c;

    emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
        Compare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
    else if (order == Qt::DescendingOrder)
        Compare<Qt::DescendingOrder>().sort(column, rootItem->childItems);

    c = column;

    emit layoutChanged();
}

void SpyModel::sort(){
    sort(sortColumn, sortOrder);
}

void SpyModel::addResult(const QString &file, bool isTTH)
{
    QString _temp;

    foreach (const QChar &ch, file)
        _temp += ((ch.isPrint() || ch == ' ')? ch : ' ');//remove all non-printable chars except space

    QString &_file = _temp;

    if (_file.trimmed().isEmpty())
        return;

    SpyItem *item;
    SpyItem * parent = NULL;

    if (hashes.contains(_file))
        parent = hashes[_file];
    else
        parent = rootItem;

    QList<QVariant> item_data;

    item_data << "" << _file;
    item = pool.construct(item_data, parent);

    item->isTTH = isTTH;

    if (parent == rootItem)
        hashes.insert(_file, item);

    if(parent != rootItem){
        parent->appendChild(item);
        rootItem->moveUp(parent);
    }else{
        parent->insertChild(item, 0);
    }

    if(isSort)
        sort();

    emit layoutChanged();
}

void SpyModel::clearModel(){
    QList<SpyItem*> &childs = rootItem->childItems;

    foreach(SpyItem *i, childs)
        pool.destroy(i);

    rootItem->childItems.clear();

    hashes.clear();

    reset();

    emit layoutChanged();
}

void SpyModel::setSort(bool sort){
    isSort = sort;
}

SpyItem::SpyItem(const QList<QVariant> &data, SpyItem *parent) :
    count(0),
    isTTH(false),
    itemData(data),
    parentItem(parent)
{
}

SpyItem::~SpyItem()
{
    //All items allocated in pool that have auto delete
}

void SpyItem::appendChild(SpyItem *item) {
    childItems.append(item);
}

void SpyItem::insertChild(SpyItem *item, int pos) {
    childItems.insert(pos, item);
}

void SpyItem::moveUp(SpyItem *item) {
    childItems.move(childItems.indexOf(item), 0);
}

SpyItem *SpyItem::child(int row) {
    return childItems.value(row);
}

int SpyItem::childCount() const {
    return childItems.count();
}

int SpyItem::columnCount() const {
    return itemData.count();
}

QVariant SpyItem::data(int column) const {
    if (column == COLUMN_SPY_COUNT && childItems.size() > 0 && parent())
        return childItems.size()+1;

    return itemData.value(column);
}

SpyItem *SpyItem::parent() const{
    return parentItem;
}

int SpyItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<SpyItem*>(this));

    return 0;
}
