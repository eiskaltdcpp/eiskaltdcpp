/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "DownloadQueueModel.h"
#include "WulforUtil.h"

#include <QtGui>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QFontMetrics>
#include <QStyleOptionProgressBar>
#include <QSize>

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/QueueManager.h>

#define _DEBUG_ 1

#if _DEBUG_
#include <QtDebug>
#endif

#include <set>

#if _DEBUG_
static inline void printRoot(DownloadQueueItem *i, const QString &dlmtr){
    if (!i)
        return;

    qDebug() << dlmtr.toAscii().constData() << i->data(COLUMN_DOWNLOADQUEUE_NAME).toString().toAscii().constData();

    foreach (DownloadQueueItem *child, i->childItems)
        printRoot(child, dlmtr + "-");
}
#endif

DownloadQueueModel::DownloadQueueModel(QObject *parent)
    : QAbstractItemModel(parent), iconsScaled(false)
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Status") << tr("Size") << tr("Downloaded")
             << tr("Priority") << tr("User") << tr("Path") << tr("Exact size")
             << tr("Errors") << tr("Added") << tr("TTH");

    rootItem = new DownloadQueueItem(rootData, NULL);

    sortColumn = COLUMN_DOWNLOADQUEUE_NAME;
    sortOrder = Qt::DescendingOrder;
}

DownloadQueueModel::~DownloadQueueModel()
{
    if (rootItem)
        delete rootItem;
}

int DownloadQueueModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<DownloadQueueItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant DownloadQueueModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DownloadQueueItem *item = static_cast<DownloadQueueItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole:
        {
            if (item->dir && index.column() == COLUMN_DOWNLOADQUEUE_NAME)
                return WulforUtil::getInstance()->getPixmap(WulforUtil::eiFOLDER_BLUE);
            else if (index.column() == COLUMN_DOWNLOADQUEUE_NAME)
                return WulforUtil::getInstance()->getPixmapForFile(item->data(COLUMN_DOWNLOADQUEUE_NAME).toString());
        }
        case Qt::DisplayRole:
        {
            return item->data(index.column());
        }
        case Qt::TextAlignmentRole:
        {
            if (index.column() == COLUMN_DOWNLOADQUEUE_SIZE || index.column())
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            else
                return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::ForegroundRole:
        {
            QString errors = item->data(COLUMN_DOWNLOADQUEUE_ERR).toString();

            if (!errors.isEmpty() && errors != tr("No errors"))
                return QColor(0xff, 0x00, 0x00);

            break;
        }
        case Qt::BackgroundColorRole:
            break;
        case Qt::ToolTipRole:
        {
            if (item->dir)
                break;

            QString added  = item->data(COLUMN_DOWNLOADQUEUE_ADDED).toString();
            QString errors = item->data(COLUMN_DOWNLOADQUEUE_ERR).toString();
            QString path   = item->data(COLUMN_DOWNLOADQUEUE_PATH).toString();

            if (errors.isEmpty())
                errors = tr("No errors");

            QString tooltip = QString(tr("<b>Added: </b> %1\n"
                                         "<b>Path: </b> %2\n"
                                         "<b>Errors: </b> %3\n")).arg(added).arg(path).arg(errors);

            return tooltip;
        }
    }

    return QVariant();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<DownloadQueueItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<DownloadQueueItem*>& items, DownloadQueueItem* item) {
        QList<DownloadQueueItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const DownloadQueueItem * l, const DownloadQueueItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                 case COLUMN_DOWNLOADQUEUE_ESIZE:
                     return NumCmp<COLUMN_DOWNLOADQUEUE_ESIZE>;
                 case COLUMN_DOWNLOADQUEUE_SIZE:
                     return NumCmp<COLUMN_DOWNLOADQUEUE_ESIZE>;
                 case COLUMN_DOWNLOADQUEUE_DOWN:
                     return NumCmp<COLUMN_DOWNLOADQUEUE_DOWN>;
                 case COLUMN_DOWNLOADQUEUE_NAME:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_NAME>;
                 case COLUMN_DOWNLOADQUEUE_ADDED:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_ADDED>;
                 case COLUMN_DOWNLOADQUEUE_ERR:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_ERR>;
                 case COLUMN_DOWNLOADQUEUE_PRIO:
                     return NumCmp<COLUMN_DOWNLOADQUEUE_PRIO>;
                 case COLUMN_DOWNLOADQUEUE_PATH:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_PATH>;
                 case COLUMN_DOWNLOADQUEUE_STATUS:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_STATUS>;
                 case COLUMN_DOWNLOADQUEUE_TTH:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_TTH>;
                 default:
                     return AttrCmp<COLUMN_DOWNLOADQUEUE_USER>;
            }

            Q_ASSERT_X(false, "getAttrComp", QString("Inncorrect column %1").arg(column).toAscii().constData());
            return 0;
        }
        template <int i>
        bool static AttrCmp(const DownloadQueueItem * l, const DownloadQueueItem * r) {
            if (!(l->dir && r->dir)){
                return (l->dir);
            }
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const DownloadQueueItem * l, const DownloadQueueItem * r) {
            if (!(l->dir && r->dir)){
                return (l->dir);
            }
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

QVariant DownloadQueueModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Status") << tr("Size") << tr("Downloaded")
             << tr("Priority") << tr("User") << tr("Path") << tr("Exact size")
             << tr("Errors") << tr("Added") << tr("TTH");

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootData.at(section);

    return QVariant();
}

QModelIndex DownloadQueueModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DownloadQueueItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DownloadQueueItem*>(parent.internalPointer());

    DownloadQueueItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DownloadQueueModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    DownloadQueueItem *childItem = static_cast<DownloadQueueItem*>(index.internalPointer());
    DownloadQueueItem *parentItem = childItem->parent();

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DownloadQueueModel::rowCount(const QModelIndex &parent) const
{
    DownloadQueueItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DownloadQueueItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void DownloadQueueModel::sort(int column, Qt::SortOrder order) {
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

DownloadQueueItem *DownloadQueueModel::addItem(const QMap<QString, QVariant> &map){
    DownloadQueueItem *droot = createPath(map["PATH"].toString());

    if (!droot)
        return NULL;

    DownloadQueueItem *child = NULL;
    QList<QVariant> childData;

    qlonglong size = map["ESIZE"].toLongLong();
    qlonglong down = map["DOWN"].toLongLong();

    QString size_str = "";
    QString down_str = "";
    QString prio_str = "";

    if (size > 0){
        size_str = _q(Util::formatBytes(size));

        double percent = ((double)down * 100.0/(double)size);

        down_str = _q(Util::formatBytes(down)) + " (" + _q(Util::toString(percent)) + "%)";
    }
    else {
        size_str = tr("Unknown");
        down_str = tr("0 B (0.00%)");
    }

    QueueItem::Priority prio = static_cast<QueueItem::Priority>(map["PRIO"].toInt());

    switch (prio){
        case QueueItem::PAUSED:
            prio_str = tr("Paused");
        case QueueItem::LOWEST:
            prio_str = tr("Lowest");
            break;
        case QueueItem::LOW:
            prio_str = tr("Low");
            break;
        case QueueItem::HIGH:
            prio_str = tr("High");
            break;
        case QueueItem::HIGHEST:
            prio_str = tr("Highest");
            break;
        default:
            prio_str = tr("Normal");
    }

    childData << map["FNAME"]
              << map["STATUS"]
              << size_str
              << down_str
              << prio_str
              << map["USERS"]
              << map["PATH"]
              << map["ESIZE"]
              << map["ERRORS"]
              << map["ADDED"]
              << map["TTH"];

    child = new DownloadQueueItem(childData, droot);
    droot->appendChild(child);

    return child;
}

void DownloadQueueModel::updItem(const QMap<QString, QVariant> &map){
    DownloadQueueItem *item = createPath(map["PATH"].toString());;

    QString target_name = map["FNAME"].toString();
    DownloadQueueItem *target = findTarget(item, target_name);

    if (!target && !(target = addItem(map)))
        return;

    item = target;

    qlonglong size = map["ESIZE"].toLongLong();
    qlonglong down = map["DOWN"].toLongLong();
    QueueItem::Priority prio = static_cast<QueueItem::Priority>(map["PRIO"].toInt());

    QString down_str = "";
    QString prio_str = "";

    if (size > 0){
        double percent = ((double)down * 100.0/(double)size);

        down_str = _q(Util::formatBytes(down)) + " (" + _q(Util::toString(percent)) + "%)";
    }
    else {
        down_str = tr("0 B (0.00%)");
    }

    switch (prio){
        case QueueItem::PAUSED:
            prio_str = tr("Paused");
            break;
        case QueueItem::LOWEST:
            prio_str = tr("Lowest");
            break;
        case QueueItem::LOW:
            prio_str = tr("Low");
            break;
        case QueueItem::HIGH:
            prio_str = tr("High");
            break;
        case QueueItem::HIGHEST:
            prio_str = tr("Highest");
            break;
        default:
            prio_str = tr("Normal");
    }

    item->updateColumn(COLUMN_DOWNLOADQUEUE_STATUS, map["STATUS"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_DOWN, down_str);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_PRIO, prio_str);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_USER, map["USERS"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_ERR, map["ERRORS"]);
}

bool DownloadQueueModel::remItem(const QMap<QString, QVariant> &map){
    DownloadQueueItem *item = createPath(map["PATH"].toString());;

    if (item->childItems.size() < 1)
        return false;

    QString target_name = map["FNAME"].toString();
    DownloadQueueItem *target = findTarget(item, target_name);

    if (!target)
        return false;

    if (item->childCount() > 1){
        beginRemoveRows(createIndexForItem(item), target->row(), target->row());
        {
            int r = target->row();

            item->childItems.removeAt(r);

            delete target;
        }
        endRemoveRows();
    }
    else {

        DownloadQueueItem *p = item;
        DownloadQueueItem *_t = NULL;

        while (true){
            if ((p == rootItem) || (p->childCount() > 1) || !p->parent())
                break;

            beginRemoveRows(createIndexForItem(p->parent()), p->row(), p->row());
            {
                p->parent()->childItems.removeAt(p->row());

                _t = p;
            }
            endRemoveRows();

            if (p->parent()->childCount() > 0)
                break;

            p = p->parent();

            delete _t;
        }
    }

    return true;
}

void DownloadQueueModel::setRootElem(DownloadQueueItem *root, bool del_old, bool controlNull){
    if (controlNull && !root)
        return;

    if (del_old && root != rootItem){//prevent deleting own root element
        delete rootItem;

        rootItem = NULL;
    }

    rootItem = root;

    if (rootItem){
        emit layoutChanged();
    }
}

DownloadQueueItem *DownloadQueueModel::getRootElem() const{
    return rootItem;
}

void DownloadQueueModel::setIconsScaled(bool scaled, const QSize &size){
    iconsScaled = scaled;
    iconsSize = size;
}

int DownloadQueueModel::getSortColumn() const {
    return sortColumn;
}

void DownloadQueueModel::setSortColumn(int c) {
    sortColumn = c;
}

Qt::SortOrder DownloadQueueModel::getSortOrder() const {
    return sortOrder;
}

void DownloadQueueModel::setSortOrder(Qt::SortOrder o) {
    sortOrder = o;
}

QModelIndex DownloadQueueModel::createIndexForItem(DownloadQueueItem *item){
    if (!rootItem || !item || item == rootItem)
        return QModelIndex();

    QStack<DownloadQueueItem*> stack;
    DownloadQueueItem *root = item->parent();

    while (root && (root != rootItem)){
        stack.push(root);

        root = root->parent();
    }

    QModelIndex parent = QModelIndex();
    QModelIndex child;

    while (!stack.empty()){
        DownloadQueueItem *el = stack.pop();

        parent = index(el->row(), COLUMN_DOWNLOADQUEUE_NAME, parent);
    }

    return index(item->row(), COLUMN_DOWNLOADQUEUE_NAME, parent);
}

DownloadQueueItem *DownloadQueueModel::createPath(const QString & path){
    if (!rootItem)
        return NULL;

    QString _path = path;
    _path.replace("\\", "/");

    QStringList list = _path.split("/", QString::SkipEmptyParts);

    DownloadQueueItem *root = rootItem;

    bool found = false;

    for (int i = 0; i < list.size(); i++){
        found = false;

        foreach(DownloadQueueItem *item, root->childItems){
            if (!item->dir)
                continue;

            QString name = item->data(COLUMN_DOWNLOADQUEUE_NAME).toString();

            if (name == list.at(i)){
                found = true;
                root = item;

                break;
            }
        }

        if (!found){
            for (int j = i; j < list.size(); j++){
                QList<QVariant> rootData;
                rootData << list.at(j)  << QString("") << QString("") << QString("")
                         << QString("") << QString("") << QString("") << QString("")
                         << QString("") << QString("") << QString("");

                DownloadQueueItem *item = new DownloadQueueItem(rootData);
                item->dir = true;

                root->appendChild(item);

                root = item;
            }

            emit layoutChanged();

            return root;
        }
    }

    return root;
}

void DownloadQueueModel::clear(){
    blockSignals(true);

    qDeleteAll(rootItem->childItems);
    rootItem->childItems.clear();

    blockSignals(false);

    emit layoutChanged();
}

void DownloadQueueModel::repaint(){
    emit layoutChanged();
}

DownloadQueueItem *DownloadQueueModel::findTarget(const DownloadQueueItem *item, const QString &name){
    DownloadQueueItem *target = NULL;

    foreach(DownloadQueueItem *i, item->childItems){
        if (i->data(COLUMN_DOWNLOADQUEUE_NAME).toString() == name){
            target = i;

            break;
        }
    }

    return target;
}

DownloadQueueItem::DownloadQueueItem(const QList<QVariant> &data, DownloadQueueItem *parent) :
    itemData(data), parentItem(parent), dir(false)
{
}

DownloadQueueItem::DownloadQueueItem(const DownloadQueueItem &item){
    itemData = item.itemData;
    dir = item.dir;
}
void DownloadQueueItem::operator=(const DownloadQueueItem &item){
    itemData = item.itemData;
    dir = item.dir;
}

DownloadQueueItem::~DownloadQueueItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void DownloadQueueItem::appendChild(DownloadQueueItem *item) {
    childItems.append(item);

    item->parentItem = this;
}

DownloadQueueItem *DownloadQueueItem::child(int row) {
    return childItems.value(row);
}

int DownloadQueueItem::childCount() const {
    return childItems.count();
}

int DownloadQueueItem::columnCount() const {
    return itemData.count();
}

QVariant DownloadQueueItem::data(int column) const {
    return itemData.value(column);
}

DownloadQueueItem *DownloadQueueItem::parent() {
    return parentItem;
}

int DownloadQueueItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<DownloadQueueItem*>(this));

    return 0;
}

void DownloadQueueItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}

DownloadQueueItem *DownloadQueueItem::nextSibling(){
    if (!parent())
        return NULL;

    if (row() == (parent()->childCount()-1))
        return NULL;

    return parent()->child(row()+1);
}

DownloadQueueDelegate::DownloadQueueDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{
}

DownloadQueueDelegate::~DownloadQueueDelegate(){
}

void DownloadQueueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.column() != COLUMN_DOWNLOADQUEUE_STATUS){
        QStyledItemDelegate::paint(painter, option, index);

        return;
    }

    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = option.rect;
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(index.internalPointer());

    if (!item || item->dir){
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QString size_str = item->data(COLUMN_DOWNLOADQUEUE_SIZE).toString();
    QString down_str = item->data(COLUMN_DOWNLOADQUEUE_DOWN).toString();
    QString temp = down_str;

    down_str = down_str.left(down_str.indexOf("("));
    down_str = down_str.trimmed();

    temp.remove(0, temp.indexOf("(")+1);
    temp = temp.left(temp.indexOf("%)"));

    double percent = temp.toDouble();

    QString status = QString("%1% (%2 of %3)").arg(temp).arg(down_str).arg(size_str);

    progressBarOption.text = status;
    progressBarOption.progress = static_cast<int>(percent);

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
