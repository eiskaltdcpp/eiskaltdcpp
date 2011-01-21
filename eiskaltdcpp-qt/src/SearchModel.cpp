/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QDir>

#include "SearchModel.h"
#include "SearchFrame.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/ShareManager.h"

//#define _DEBUG_MODEL_


#include <QtDebug>

using namespace dcpp;

#ifdef _DEBUG_MODEL_
#include <QtDebug>
#endif

void SearchProxyModel::sort(int column, Qt::SortOrder order){
    if (sourceModel())
        sourceModel()->sort(column, order);
}

SearchModel::SearchModel(QObject *parent):
        QAbstractItemModel(parent),
        sortColumn(COLUMN_SF_ESIZE),
        sortOrder(Qt::DescendingOrder),
        filterRole(SearchFrame::None)
{
    QList<QVariant> rootData;
    rootData << tr("Count") << tr("File") << tr("Ext") << tr("Size")
             << tr("Exact size") << tr("TTH")   << tr("Path") << tr("Nick")
             << tr("Free slots") << tr("Total slots")
             << tr("IP") << tr("Hub") << tr("Host");

    rootItem = new SearchItem(rootData);

    sortColumn = -1;
}

SearchModel::~SearchModel()
{
    delete rootItem;
}

int SearchModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<SearchItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    SearchItem *item = static_cast<SearchItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
        {
            if (index.column() == COLUMN_SF_FILENAME && !item->isDir)
                return WulforUtil::getInstance()->getPixmapForFile(item->data(COLUMN_SF_FILENAME).toString()).scaled(16, 16);
            else if (index.column() == COLUMN_SF_FILENAME && item->isDir)
                return WICON(WulforUtil::eiFOLDER_BLUE).scaled(16, 16);
            break;
        }
        case Qt::DisplayRole:
            return item->data(index.column());
        case Qt::TextAlignmentRole:
        {
            const int i_column = index.column();
            bool align_center = (i_column == COLUMN_SF_ALLSLOTS) || (i_column == COLUMN_SF_EXTENSION) ||
                                (i_column == COLUMN_SF_FREESLOTS);
            bool align_right  = (i_column == COLUMN_SF_ESIZE) || (i_column == COLUMN_SF_SIZE ) || (i_column == COLUMN_SF_COUNT);

            if (align_center)
                return Qt::AlignCenter;
            else if (align_right)
                return Qt::AlignRight;

            break;
        }
        case Qt::ForegroundRole:
        {
            if (filterRole == static_cast<int>(SearchFrame::Highlight)){
                TTHValue t(_tq(item->data(COLUMN_SF_TTH).toString()));

                if (ShareManager::getInstance()->isTTHShared(t)){
                    static QColor c;

                    c.setNamedColor(WSGET(WS_APP_SHARED_FILES_COLOR));
                    c.setAlpha(WIGET(WI_APP_SHARED_FILES_ALPHA));

                    return c;
                }
            }

            break;
        }
        case Qt::BackgroundColorRole:
            break;
        case Qt::ToolTipRole:
            break;
    }

    return QVariant();
}

void SearchModel::repaint(){
    emit layoutChanged();
}

Qt::ItemFlags SearchModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SearchModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex SearchModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SearchItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SearchItem*>(parent.internalPointer());

    SearchItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SearchModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SearchItem *childItem = static_cast<SearchItem*>(index.internalPointer());
    SearchItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    SearchItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SearchItem*>(parent.internalPointer());

    return parentItem->childCount();
}

QModelIndex SearchModel::createIndexForItem(SearchItem *item){
    if (!(rootItem && item) || item == rootItem)
        return QModelIndex();

    return index(item->row(), COLUMN_SF_FILENAME, (item->parent() == rootItem)? QModelIndex() : createIndexForItem(item->parent()));
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<SearchItem*>& items) {
#ifdef _DEBUG_MODEL_
        qDebug() << "Sorting by " << col << " column and " << WulforUtil::getInstance()->sortOrderToInt(order) << " order.";
#endif
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    QList<SearchItem*>::iterator static insertSorted(int col, QList<SearchItem*>& items, SearchItem* item) {
        return qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
    }

    private:
        typedef bool (*AttrComp)(const SearchItem * l, const SearchItem * r);
        AttrComp static getAttrComp(const int column) {
            static AttrComp attrs[13] = {   NumCmp<COLUMN_SF_COUNT>,
                                            AttrCmp<COLUMN_SF_FILENAME>,
                                            AttrCmp<COLUMN_SF_EXTENSION>,
                                            NumCmp<COLUMN_SF_ESIZE>,
                                            NumCmp<COLUMN_SF_ESIZE>,
                                            AttrCmp<COLUMN_SF_TTH>,
                                            AttrCmp<COLUMN_SF_PATH>,
                                            AttrCmp<COLUMN_SF_NICK>,
                                            NumCmp<COLUMN_SF_FREESLOTS>,
                                            NumCmp<COLUMN_SF_ALLSLOTS>,
                                            AttrCmp<COLUMN_SF_IP>,
                                            AttrCmp<COLUMN_SF_HUB>,
                                            AttrCmp<COLUMN_SF_HOST>
                                        };

            return attrs[column];//column number checked in SearchModel::sort
        }
        template <int i>
        bool static AttrCmp(const SearchItem * l, const SearchItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <typename T, T (SearchItem::*attr)>
        bool static AttrCmp(const SearchItem * l, const SearchItem * r) {
            return Cmp(l->*attr, r->*attr);
        }
        template <int i>
        bool static NumCmp(const SearchItem * l, const SearchItem * r) {
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

void SearchModel::sort(int column, Qt::SortOrder order) {
    static Compare<Qt::AscendingOrder>  acomp = Compare<Qt::AscendingOrder>();
    static Compare<Qt::DescendingOrder> dcomp = Compare<Qt::DescendingOrder>();

    sortColumn = column;
    sortOrder = order;

    if (sortColumn < 0 || sortColumn > columnCount()-1)
        return;

    emit layoutAboutToBeChanged();

    try {
        if (order == Qt::AscendingOrder)
            acomp.sort(column, rootItem->childItems);
        else if (order == Qt::DescendingOrder)
            dcomp.sort(column, rootItem->childItems);
    }
    catch (SearchListException &e){
        sort(COLUMN_SF_FILENAME, order);
    }

    emit layoutChanged();
}

bool SearchModel::addResultPtr(const QMap<QString, QVariant> &map){
    try {
        return addResult(map["FILE"].toString(),
                  map["SIZE"].toULongLong(),
                  map["TTH"].toString(),
                  map["PATH"].toString(),
                  map["NICK"].toString(),
                  map["FSLS"].toULongLong(),
                  map["ASLS"].toULongLong(),
                  map["IP"].toString(),
                  map["HUB"].toString(),
                  map["HOST"].toString(),
                  map["CID"].toString(),
                  map["ISDIR"].toBool());
    }
    catch (SearchListException){
        return false;
    }
}

bool SearchModel::addResult
        (
        const QString &file,
        qulonglong size,
        const QString &tth,
        const QString &path,
        const QString &nick,
        const int free_slots,
        const int all_slots,
        const QString &ip,
        const QString &hub,
        const QString &host,
        const QString &cid,
        const bool isDir
        )
{
    static Compare<Qt::AscendingOrder>  acomp = Compare<Qt::AscendingOrder>();
    static Compare<Qt::DescendingOrder> dcomp = Compare<Qt::DescendingOrder>();

    if (file.isEmpty())
        return false;

    SearchItem *item;

    QFileInfo file_info(QDir::toNativeSeparators(file));
    QString ext = "";

    if (size > 0)
        ext = file_info.suffix().toUpper();

    SearchItem * parent = NULL;

    if (!isDir && tths.contains(tth)) {
        parent = tths[tth];
        if (parent->exists(cid))
            return false;
    } else {
        parent = rootItem;
    }

    QList<QVariant> item_data;

    item_data << QVariant() << file << ext << WulforUtil::formatBytes(size)
              << size << tth << path << nick << free_slots
              << all_slots << ip << hub << host,

    item =new SearchItem(item_data, parent);

    if (!item)
        throw SearchListException();

    item->isDir = isDir;
    item->cid = cid;

    if (parent == rootItem && !isDir)
        tths.insert(tth, item);
    else {


        if (sortColumn == COLUMN_SF_COUNT){
            sort(sortColumn, sortOrder);

            return true;
        }

        beginInsertRows(createIndexForItem(parent), parent->childCount(), parent->childCount());
        {
             parent->appendChild(item);
        }
        endInsertRows();

        return true;
    }

    emit layoutAboutToBeChanged();

    QList<SearchItem*>::iterator it = parent->childItems.end();

    if (sortOrder == Qt::AscendingOrder)
        it = acomp.insertSorted(sortColumn, parent->childItems, item);
    else
        it = dcomp.insertSorted(sortColumn, parent->childItems, item);

    const int pos = it - parent->childItems.begin();

    //beginInsertRows(createIndexForItem(parent), pos, pos);
    //{
        parent->childItems.insert(it, item);
    //}
    //endInsertRows();// Crash (???)

    emit layoutChanged();

    return true;
}

int SearchModel::getSortColumn() const {
    return sortColumn;
}

void SearchModel::setSortColumn(int c) {
    sortColumn = c;
}

Qt::SortOrder SearchModel::getSortOrder() const {
    return sortOrder;
}

void SearchModel::setSortOrder(Qt::SortOrder o) {
    sortOrder = o;
}

void SearchModel::clearModel(){
    blockSignals(true);

    qDeleteAll(rootItem->childItems);
    rootItem->childItems.clear();

    tths.clear();

    blockSignals(false);

    reset();
}

void SearchModel::removeItem(const SearchItem *item){
    if (!okToFind(item))
        return;

    QModelIndex i = createIndexForItem(const_cast<SearchItem*>(item));

    beginRemoveRows(i, item->row(), item->row());

    SearchItem *p = const_cast<SearchItem*>(item->parent());
    p->childItems.removeAt(item->row());

    if (tths[item->data(COLUMN_SF_TTH).toString()] == item)
        tths.remove(item->data(COLUMN_SF_TTH).toString());

    endRemoveRows();

    delete item;
}

void SearchModel::setFilterRole(int role){
    filterRole = role;
}

bool SearchModel::okToFind(const SearchItem *item){
    if (!item)
        return false;

    if (!rootItem->childItems.contains(const_cast<SearchItem*>(item))){
        QString tth = item->data(COLUMN_SF_TTH).toString();

        SearchItem *tth_root = tths.value(tth);//try to find item by tth

        foreach (SearchItem *i, tth_root->childItems){
            if (item == i)
                return true;
        }
    }
    else
        return true;

    return false;
}

SearchItem::SearchItem(const QList<QVariant> &data, SearchItem *parent) :
    isDir(false),
    itemData(data),
    parentItem(parent)
{
}

SearchItem::~SearchItem()
{
    qDeleteAll(childItems);
}

void SearchItem::appendChild(SearchItem *item) {
    childItems.append(item);
    count = childItems.size();
}

SearchItem *SearchItem::child(int row) {
    return childItems.value(row);
}

int SearchItem::childCount() const {
    return childItems.count();
}

int SearchItem::columnCount() const {
    return itemData.count();
}

QVariant SearchItem::data(int column) const {
    if (column == COLUMN_SF_COUNT && childItems.size() > 0 && parentItem != 0)
        return childItems.size()+1;

    return itemData.value(column);
}

SearchItem *SearchItem::parent() const{
    return parentItem;
}

int SearchItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<SearchItem*>(this));

    return 0;
}

bool SearchItem::exists(const QString &user_cid) const {
    if (childItems.isEmpty())
        return cid == user_cid;

    foreach(SearchItem *child, childItems) {
        if (child->cid == user_cid)
            return true;
    }
    return false;
}

SearchListException::SearchListException() :
    message("Unknown"), type(Unkn)
{}

SearchListException::SearchListException(const SearchListException &ex) :
    message(ex.message), type(ex.type)
{}

SearchListException::SearchListException(const QString& message, Type type) :
    message(message), type(type)
{}

SearchListException::~SearchListException(){
}

SearchListException &SearchListException::operator =(const SearchListException &ex2) {
    type = ex2.type;
    message = ex2.message;

    return (*this);
}
