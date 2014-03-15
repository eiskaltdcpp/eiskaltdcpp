/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "DownloadQueueModel.h"
#include "WulforUtil.h"

#ifdef USE_QT5
#include <QtWidgets>
#else
#include <QtGui>
#endif

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

	qDebug() << dlmtr.toLatin1().constData() << i->data(COLUMN_DOWNLOADQUEUE_NAME).toString().toLatin1().constData();

    for (const auto &child : i->childItems)
        printRoot(child, dlmtr + "-");
}
#endif

class DownloadQueueModelPrivate{
public:
    /** */
    quint64 total_files;
    /** */
    quint64 total_size;
    /** */
    int sortColumn;
    /** */
    Qt::SortOrder sortOrder;
    /** */
    DownloadQueueItem *rootItem;
    /** */
    bool iconsScaled;
    /** */
    QSize iconsSize;
};

DownloadQueueModel::DownloadQueueModel(QObject *parent)
    : QAbstractItemModel(parent), d_ptr(new DownloadQueueModelPrivate())
{
    Q_D(DownloadQueueModel);

    d->iconsScaled = false;
    d->total_files = 0;
    d->total_size  = 0;

    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Status") << tr("Size") << tr("Downloaded")
             << tr("Priority") << tr("User") << tr("Path") << tr("Exact size")
             << tr("Errors") << tr("Added") << tr("TTH");

    d->rootItem = new DownloadQueueItem(rootData, NULL);

    d->sortColumn = COLUMN_DOWNLOADQUEUE_NAME;
    d->sortOrder = Qt::DescendingOrder;
}

DownloadQueueModel::~DownloadQueueModel()
{
    Q_D(DownloadQueueModel);

    if (d->rootItem)
        delete d->rootItem;

    delete d;
}

int DownloadQueueModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const static DownloadQueueModel);

    if (parent.isValid())
        return static_cast<DownloadQueueItem*>(parent.internalPointer())->columnCount();
    else
        return d->rootItem->columnCount();
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
                return WICON(WulforUtil::eiFOLDER_BLUE).scaled(16, 16);
            else if (index.column() == COLUMN_DOWNLOADQUEUE_NAME)
                return WulforUtil::getInstance()->getPixmapForFile(item->data(COLUMN_DOWNLOADQUEUE_NAME).toString()).scaled(16, 16);
        }
        case Qt::DisplayRole:
        {
            if ((index.column() == COLUMN_DOWNLOADQUEUE_DOWN || index.column() == COLUMN_DOWNLOADQUEUE_SIZE) && !item->dir)
                return WulforUtil::formatBytes(item->data(index.column()).toLongLong());
            else if ((index.column() == COLUMN_DOWNLOADQUEUE_DOWN || index.column() == COLUMN_DOWNLOADQUEUE_SIZE) && item->dir)
                break;
            else if (index.column() == COLUMN_DOWNLOADQUEUE_PRIO && !item->dir){
                QueueItem::Priority prio = static_cast<QueueItem::Priority>(item->data(COLUMN_DOWNLOADQUEUE_PRIO).toInt());

                QString prio_str = "";

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

                return prio_str;
            }

            return item->data(index.column());
        }
        case Qt::TextAlignmentRole:
        {
            if (index.column() == COLUMN_DOWNLOADQUEUE_SIZE ||
                index.column() == COLUMN_DOWNLOADQUEUE_ESIZE ||
                index.column() == COLUMN_DOWNLOADQUEUE_DOWN)
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            else
                return static_cast<int>(Qt::AlignLeft);
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
    typedef bool (*AttrComp)(const DownloadQueueItem * l, const DownloadQueueItem * r);

    void static sort(unsigned column, QList<DownloadQueueItem*>& items) {
        if (column > COLUMN_DOWNLOADQUEUE_TTH)
            return;

        qStableSort(items.begin(), items.end(), attrs[column]);
    }

    void static insertSorted(unsigned column, QList<DownloadQueueItem*>& items, DownloadQueueItem* item) {
        if (column > COLUMN_DOWNLOADQUEUE_TTH){
            items.push_back(item);

            return;
        }

        auto it = qLowerBound(items.begin(),
                                                             items.end(),
                                                             item,
                                                             attrs[column]
                                                             );
        items.insert(it, item);
    }

    private:
        template <int i>
        bool static AttrCmp(const DownloadQueueItem * l, const DownloadQueueItem * r) {
            if (l->dir != r->dir){
                return (l->dir);
            }
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const DownloadQueueItem * l, const DownloadQueueItem * r) {
            if (l->dir != r->dir){
                return (l->dir);
            }
            return Cmp(l->data(column).toULongLong(), r->data(column).toULongLong());
       }
        template <typename T>
        bool static Cmp(const T& l, const T& r);

        static AttrComp attrs[11];
};

template <Qt::SortOrder order>
typename Compare<order>::AttrComp Compare<order>::attrs[11] = { AttrCmp<COLUMN_DOWNLOADQUEUE_NAME>,
                                                                AttrCmp<COLUMN_DOWNLOADQUEUE_DOWN>,
                                                                NumCmp<COLUMN_DOWNLOADQUEUE_ESIZE>,
                                                                NumCmp<COLUMN_DOWNLOADQUEUE_DOWN>,
                                                                NumCmp<COLUMN_DOWNLOADQUEUE_PRIO>,
                                                                AttrCmp<COLUMN_DOWNLOADQUEUE_USER>,
                                                                AttrCmp<COLUMN_DOWNLOADQUEUE_PATH>,
                                                                NumCmp<COLUMN_DOWNLOADQUEUE_ESIZE>,
                                                                AttrCmp<COLUMN_DOWNLOADQUEUE_ERR>,
                                                                AttrCmp<COLUMN_DOWNLOADQUEUE_ADDED>,
                                                                AttrCmp<COLUMN_DOWNLOADQUEUE_TTH>
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

QModelIndex DownloadQueueModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const static DownloadQueueModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DownloadQueueItem *parentItem;

    if (!parent.isValid())
        parentItem = d->rootItem;
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
    Q_D(const static DownloadQueueModel);

    if (!index.isValid())
        return QModelIndex();

    DownloadQueueItem *childItem = static_cast<DownloadQueueItem*>(index.internalPointer());
    DownloadQueueItem *parentItem = childItem->parent();

    if (parentItem == d->rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DownloadQueueModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const static DownloadQueueModel);
    DownloadQueueItem *parentItem;

    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = d->rootItem;
    else
        parentItem = static_cast<DownloadQueueItem*>(parent.internalPointer());

    return parentItem->childCount();
}

static void sortRecursive(int column, Qt::SortOrder order, DownloadQueueItem *i){
    if (column == -1 || !i || !i->childCount())
        return;

    static Compare<Qt::AscendingOrder> acomp;
    static Compare<Qt::DescendingOrder> dcomp;

    if (order == Qt::AscendingOrder)
        acomp.sort(column, i->childItems);
    else if (order == Qt::DescendingOrder)
        dcomp.sort(column, i->childItems);

    for (const auto &ii : i->childItems)
        sortRecursive(column, order, ii);
}

void DownloadQueueModel::sort(int column, Qt::SortOrder order) {
    Q_D(DownloadQueueModel);

    d->sortColumn = column;
    d->sortOrder = order;

    if (!d->rootItem || d->rootItem->childItems.empty() || column < 0 || column > columnCount()-1)
        return;

    emit layoutAboutToBeChanged();

    sortRecursive(column, order, d->rootItem);

    emit layoutChanged();
}

DownloadQueueItem *DownloadQueueModel::addItem(const QMap<QString, QVariant> &map){
    static quint64 counter = 0;

    DownloadQueueItem *droot = createPath(map["PATH"].toString());

    if (!droot)
        return NULL;

    DownloadQueueItem *child = NULL;
    QList<QVariant> childData;

    childData << map["FNAME"]
              << map["STATUS"]
              << (map["ESIZE"].toLongLong() > 0? map["ESIZE"] : 0)
              << (map["DOWN"].toLongLong() > 0? map["DOWN"] : 0)
              << map["PRIO"]
              << map["USERS"]
              << map["PATH"]
              << (map["ESIZE"].toLongLong() > 0? map["ESIZE"] : 0)
              << map["ERRORS"]
              << map["ADDED"]
              << map["TTH"];

    child = new DownloadQueueItem(childData, droot);
    droot->appendChild(child);

    Q_D(static DownloadQueueModel);

    d->total_files++;
    d->total_size += childData.at(COLUMN_DOWNLOADQUEUE_ESIZE).toULongLong();

    emit updateStats(d->total_files, d->total_size);

    counter++;

    repaint();

    if ((counter % 100) == 0)
        QApplication::processEvents();

    return child;
}

void DownloadQueueModel::updItem(const QMap<QString, QVariant> &map){
    DownloadQueueItem *item = createPath(map["PATH"].toString());
    Q_D(static DownloadQueueModel);

    QString target_name = map["FNAME"].toString();
    DownloadQueueItem *target = findTarget(item, target_name);

    if (!target && !(target = addItem(map)))
        return;

    item = target;

    d->total_size -= item->data(COLUMN_DOWNLOADQUEUE_ESIZE).toULongLong();

    item->updateColumn(COLUMN_DOWNLOADQUEUE_STATUS, map["STATUS"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_DOWN, (map["DOWN"].toLongLong() > 0? map["DOWN"] : 0));
    item->updateColumn(COLUMN_DOWNLOADQUEUE_ESIZE, map["ESIZE"].toULongLong() > 0? map["ESIZE"] : 0);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_SIZE, map["ESIZE"].toULongLong() > 0? map["ESIZE"] : 0);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_PRIO, map["PRIO"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_USER, map["USERS"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_ERR, map["ERRORS"]);

    d->total_size += item->data(COLUMN_DOWNLOADQUEUE_ESIZE).toULongLong();

    emit updateStats(d->total_files, d->total_size);
    emit layoutChanged();
}

bool DownloadQueueModel::remItem(const QMap<QString, QVariant> &map){
    DownloadQueueItem *item = createPath(map["PATH"].toString());

    if (item->childItems.size() < 1)
        return false;

    QString target_name = map["FNAME"].toString();
    DownloadQueueItem *target = findTarget(item, target_name);

    if (!target)
        return false;

    Q_D(static DownloadQueueModel);

    d->total_size -= target->data(COLUMN_DOWNLOADQUEUE_ESIZE).toULongLong();
    d->total_files--;

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
            if ((p == d->rootItem) || (p->childCount() > 1) || !p->parent())
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

    emit updateStats(d->total_files, d->total_size);

    return true;
}

void DownloadQueueModel::setRootElem(DownloadQueueItem *root, bool del_old, bool controlNull){
    if (controlNull && !root)
        return;

    Q_D(DownloadQueueModel);

    if (del_old && root != d->rootItem){//prevent deleting own root element
        delete d->rootItem;

        d->rootItem = NULL;
    }

    if (d->rootItem == root)
        emit layoutChanged();
}

DownloadQueueItem *DownloadQueueModel::getRootElem() const{
    Q_D(const DownloadQueueModel);

    return d->rootItem;
}

void DownloadQueueModel::setIconsScaled(bool scaled, const QSize &size){
    Q_D(DownloadQueueModel);

    d->iconsScaled = scaled;
    d->iconsSize = size;
}

int DownloadQueueModel::getSortColumn() const {
    Q_D(const DownloadQueueModel);

    return d->sortColumn;
}

void DownloadQueueModel::setSortColumn(int c) {
    Q_D(DownloadQueueModel);

    d->sortColumn = c;
}

Qt::SortOrder DownloadQueueModel::getSortOrder() const {
    Q_D(const DownloadQueueModel);

    return d->sortOrder;
}

void DownloadQueueModel::setSortOrder(Qt::SortOrder o) {
    Q_D(DownloadQueueModel);

    d->sortOrder = o;
}

QModelIndex DownloadQueueModel::createIndexForItem(DownloadQueueItem *item){
    Q_D(DownloadQueueModel);

    if (!d->rootItem || !item || item == d->rootItem)
        return QModelIndex();

    return createIndex(item->row(), 0, item);
}

DownloadQueueItem *DownloadQueueModel::createPath(const QString & path){
    Q_D(static DownloadQueueModel);

    if (!d->rootItem)
        return NULL;

    QString _path = path;
    _path.replace("\\", "/");

    QStringList list = _path.split("/", QString::SkipEmptyParts);

    DownloadQueueItem *root = d->rootItem;

    bool found = false;

    for (int i = 0; i < list.size(); i++){
        found = false;

        for (const auto &item : root->childItems){
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
            static QString data = "";

            for (int j = i; j < list.size(); j++){
                QList<QVariant> rootData;
                rootData << list.at(j)  << data << data << data
                         << data << data << data << data
                         << data << data << data;

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

    Q_D(DownloadQueueModel);

    qDeleteAll(d->rootItem->childItems);
    d->rootItem->childItems.clear();

    blockSignals(false);

    emit layoutChanged();
}

void DownloadQueueModel::repaint(){
    emit layoutChanged();
}

DownloadQueueItem *DownloadQueueModel::findTarget(const DownloadQueueItem *item, const QString &name){
    DownloadQueueItem *target = NULL;

    for (const auto &i : item->childItems){
        if (i->data(COLUMN_DOWNLOADQUEUE_NAME).toString() == name){
            target = i;

            break;
        }
    }

    return target;
}

DownloadQueueItem::DownloadQueueItem(const QList<QVariant> &data, DownloadQueueItem *parent) :
    dir(false), itemData(data), parentItem(parent)
{
}

DownloadQueueItem::DownloadQueueItem(const DownloadQueueItem &item){
    itemData = item.itemData;
    dir = item.dir;
    parentItem = NULL;
    childItems = QList<DownloadQueueItem*> ();
}
void DownloadQueueItem::operator=(const DownloadQueueItem &item){
    parentItem = item.parentItem;
    childItems = item.childItems;
    itemData = item.itemData;
    dir = item.dir;
}

DownloadQueueItem::~DownloadQueueItem()
{
    if (!childItems.isEmpty())
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

const QVariant &DownloadQueueItem::data(int column) const {
    return itemData.at(column);
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

    QStyleOptionProgressBarV2 progressBarOption;
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

    qulonglong esize = item->data(COLUMN_DOWNLOADQUEUE_ESIZE).toLongLong();
    double percent = ((double)item->data(COLUMN_DOWNLOADQUEUE_DOWN).toLongLong() * 100.0);

    percent = esize > 0? (percent/(double)esize) : 0.0;

    QString status = QString("%1%").arg(percent, 0, 'f', 1);

    progressBarOption.text = status;
    progressBarOption.progress = static_cast<int>(percent);

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
