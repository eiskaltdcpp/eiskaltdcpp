/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FileBrowserModel.h"
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
#include <QSize>
#include <QFile>

#include "dcpp/ShareManager.h"
#include "dcpp/UploadManager.h"

using namespace dcpp;

#include <set>

FileBrowserModel::FileBrowserModel(QObject *parent)
    : QAbstractItemModel(parent), iconsScaled(false), restrictionsLoaded(false)
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Size") << tr("Exact size") << tr("TTH");

    rootItem = new FileBrowserItem(rootData, NULL);

    sortColumn = COLUMN_FILEBROWSER_NAME;
    sortOrder = Qt::DescendingOrder;
}

FileBrowserModel::~FileBrowserModel()
{
    if (rootItem)
        delete rootItem;

    if (restrictionsLoaded){
        QFile f(_q(Util::getPath(Util::PATH_USER_CONFIG) + "PerFolderLimit.conf"));

        if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
            QTextStream stream(&f);
            QMap<QString, unsigned>::iterator it = restrict_map.begin();

            for(; it != restrict_map.end(); ++it)
                stream << it.value() << " " << it.key() << '\n';

            stream.flush();

            f.close();

            dcpp::UploadManager::getInstance()->reloadRestrictions();
        }
    }
}

int FileBrowserModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FileBrowserItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant FileBrowserModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    FileBrowserItem *item = static_cast<FileBrowserItem*>(index.internalPointer());
    QString path = "";//virtual path
    QStringList dirs = createRemotePath(item).split("\\");

    if (dirs.size() >= 2){
        dirs.removeFirst();

        path = "/" + dirs.join("/");
    }
    else
        path = "";

    switch(role) {
        case Qt::DecorationRole:
        {
            if (item->dir && index.column() == COLUMN_FILEBROWSER_NAME)
                return WICON(WulforUtil::eiFOLDER_BLUE).scaled(16, 16);
            else if (index.column() == COLUMN_FILEBROWSER_NAME)
                return WulforUtil::getInstance()->getPixmapForFile(item->data(COLUMN_FILEBROWSER_NAME).toString()).scaled(16, 16);
        }
        case Qt::DisplayRole:
        {
            if (restrict_map.contains(path) && index.column() == COLUMN_FILEBROWSER_NAME){
                QString ret = tr("%1 [%2 Gb]").arg(item->data(index.column()).toString()).arg(restrict_map[path]);

                return ret;
            }

            return item->data(index.column());
        }
        case Qt::TextAlignmentRole:
        {
            bool align_right = (index.column() == COLUMN_FILEBROWSER_ESIZE) || (index.column() == COLUMN_FILEBROWSER_SIZE);

            if (align_right)
                return Qt::AlignRight;
            else
                return Qt::AlignLeft;
        }
        case Qt::ForegroundRole:
        {
            if (item->dir)
                break;

            TTHValue t(_tq(item->data(COLUMN_FILEBROWSER_TTH).toString()));

            if (ShareManager::getInstance()->isTTHShared(t)){
                static QColor c;

                c.setNamedColor(WSGET(WS_APP_SHARED_FILES_COLOR));
                c.setAlpha(WIGET(WI_APP_SHARED_FILES_ALPHA));

                return c;
            }

            break;
        }
        case Qt::BackgroundColorRole:
        {
            if (item->isDuplicate){
                QPalette pal = qApp->palette();

                return pal.highlight().color();
            }
        }
        case Qt::ToolTipRole:
        {
            if (item->isDuplicate && item->file){
                const QString &tth = item->data(COLUMN_FILEBROWSER_TTH).toString();
                QHash<QString, dcpp::DirectoryListing::File*>::const_iterator it = hash.find(tth);

                if (it == hash.end())
                    break;

                dcpp::DirectoryListing::File *file = const_cast<dcpp::DirectoryListing::File*>(it.value());
                dcpp::DirectoryListing::Directory *parentDir = file->getParent();

                if (!parentDir)
                    break;

                QString path = "";

                do {
                    path = _q(parentDir->getName()) + "/" + path;
                    parentDir = parentDir->getParent();
                } while (parentDir->getParent());

                return tr("File marked as a duplicate of another file: %1").arg(path+_q(file->getName()));
            }

            break;
        }
        case Qt::FontRole:
        {
            if (restrict_map.contains(path) && index.column() == COLUMN_FILEBROWSER_NAME){
                QFont f;
                f.setBold(true);

                return f;
            }

            break;
        }
        default:
            break;
    }

    return QVariant();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<FileBrowserItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<FileBrowserItem*>& items, FileBrowserItem* item) {
        QList<FileBrowserItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const FileBrowserItem * l, const FileBrowserItem * r);
        AttrComp static getAttrComp(const int column) {
            static AttrComp attrs[4] = {    AttrCmp<COLUMN_FILEBROWSER_NAME>,
                                            NumCmp<COLUMN_FILEBROWSER_ESIZE>,
                                            NumCmp<COLUMN_FILEBROWSER_ESIZE>,
                                            AttrCmp<COLUMN_FILEBROWSER_TTH>
                                       };

            return attrs[column];
        }
        template <int i>
        bool static AttrCmp(const FileBrowserItem * l, const FileBrowserItem * r) {
            if ((l->dir && !r->dir) || (!l->dir && r->dir)){
                return (l->dir != NULL);
            }
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const FileBrowserItem * l, const FileBrowserItem * r) {
            if ((l->dir && !r->dir) || (!l->dir && r->dir)){
                return (l->dir != NULL);
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

QVariant FileBrowserModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    QList<QVariant> rootData;
    rootData << tr("Name") << tr("Size") << tr("Exact size") << tr("TTH");

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootData.at(section);

    return QVariant();
}

QModelIndex FileBrowserModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FileBrowserItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FileBrowserItem*>(parent.internalPointer());

    FileBrowserItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FileBrowserModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    FileBrowserItem *childItem = static_cast<FileBrowserItem*>(index.internalPointer());
    FileBrowserItem *parentItem = childItem->parent();

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FileBrowserModel::rowCount(const QModelIndex &parent) const
{
    FileBrowserItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FileBrowserItem*>(parent.internalPointer());

    return parentItem->childCount();
}

static void sortRecursive(int column, Qt::SortOrder order, FileBrowserItem *i){
    static Compare<Qt::AscendingOrder> acomp = Compare<Qt::AscendingOrder>();
    static Compare<Qt::DescendingOrder> dcomp = Compare<Qt::DescendingOrder>();

    if (column < 0 || !i || i->childCount() == 0)
        return;

    if (order == Qt::AscendingOrder)
        acomp.sort(column, i->childItems);
    else if (order == Qt::DescendingOrder)
        dcomp.sort(column, i->childItems);

    foreach(FileBrowserItem *ii, i->childItems)
        sortRecursive(column, order, ii);
}

void FileBrowserModel::sort(int column, Qt::SortOrder order) {
    sortColumn = column;
    sortOrder = order;

    if (!rootItem || rootItem->childItems.empty() || column < 0 || column > columnCount()-1)
        return;

    emit layoutAboutToBeChanged();

    sortRecursive(column, order, rootItem);

    emit layoutChanged();
}

void FileBrowserModel::setRootElem(FileBrowserItem *root, bool del_old, bool controlNull){
    if (controlNull && !root)
        return;

    FileBrowserItem *from = rootItem, *to = root;

    if (del_old && root != rootItem){//prevent deleting own root element
        delete rootItem;

        rootItem = NULL;
    }

    rootItem = root;

    if (rootItem){
        emit layoutChanged();
    }

    emit rootChanged(from, to);
}

FileBrowserItem *FileBrowserModel::getRootElem() const{
    return rootItem;
}

void FileBrowserModel::setIconsScaled(bool scaled, const QSize &size){
    iconsScaled = scaled;
    iconsSize = size;
}

int FileBrowserModel::getSortColumn() const {
    return sortColumn;
}

void FileBrowserModel::setSortColumn(int c) {
    sortColumn = c;
}

Qt::SortOrder FileBrowserModel::getSortOrder() const {
    return sortOrder;
}

void FileBrowserModel::setSortOrder(Qt::SortOrder o) {
    sortOrder = o;
}

QString FileBrowserModel::createRemotePath(FileBrowserItem *item) const{
    QString s;
    FileBrowserItem * pitem;

    if (!item) {
        return s;
    }

    pitem = item;
    s = pitem->data(COLUMN_FILEBROWSER_NAME).toString();

    while ((pitem = pitem->parent()) != NULL) {
        // check for root entry
        if (pitem->parent() != NULL) {
            s = pitem->data(COLUMN_FILEBROWSER_NAME).toString() + "\\" + s;
        }
    }

    return s;
}

FileBrowserItem *FileBrowserModel::createRootForPath(const QString &path, FileBrowserItem *pathRoot){
    if (path.isEmpty() || path.isNull())
        return NULL;

    QString _path = path;
    _path.replace("\\", "/");

    QStringList list = _path.split("/", QString::SkipEmptyParts);
    FileBrowserItem *root = pathRoot?pathRoot:rootItem;

    if (list.empty() || !root)
        return NULL;

    foreach (QString s, list){
        if (s.isEmpty())
            continue;

        if (!root)
            return NULL;

        if (s == ".." && root->parent()){
            root = root->parent();

            continue;
        }
        else if (s == ".")
            continue;

        bool found = false;

        foreach(FileBrowserItem *item, root->childItems){
            if (!item->dir)
                continue;

            QString name = (item == rootItem?"":item->data(COLUMN_FILEBROWSER_NAME).toString());

            if (!name.compare(s, Qt::CaseInsensitive)){
                root = item;
                found = true;

                break;
            }
        }

        if (!found)
            return root;

    }

    return root;
}

QModelIndex FileBrowserModel::createIndexForItem(FileBrowserItem *item){
    if (!rootItem || !item)
        return QModelIndex();

    return createIndex(item->row(), 0, item);
}

void FileBrowserModel::highlightDuplicates(){
    if (!rootItem || rootItem->childCount() == 0)
        return;

    foreach (FileBrowserItem *i, rootItem->childItems){
        const QString &tth = i->data(COLUMN_FILEBROWSER_TTH).toString();

        if (tth.isEmpty())
            continue;

        QHash<QString, dcpp::DirectoryListing::File*>::iterator it = hash.find(tth);

        if (it != hash.end()){
            if (i->file != it.value())//Found duplicate
                i->isDuplicate = true;
        }
        else if (!i->file->getAdls()){
            hash.insert(tth, i->file);
        }
    }
}

void FileBrowserModel::loadRestrictions(){
    QFile f(_q(Util::getPath(Util::PATH_USER_CONFIG) + "PerFolderLimit.conf"));

    if (f.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&f);
        QString line = "";

        while (!(line = stream.readLine(0)).isNull()){
            QStringList list = line.split(' ');

            if (list.size() < 2)
                continue;

            bool ok = false;
            unsigned size = 0;

            size = list.at(0).toUInt(&ok);

            if (!ok)
                continue;

            QString virt_path = line.remove(0, list.at(0).length() + 1);

            if (!virt_path.startsWith('/'))
                virt_path.prepend('/');

            if (!virt_path.isEmpty() && size > 0 && !restrict_map.contains(virt_path))
                restrict_map.insert(virt_path, size);
        }

        restrictionsLoaded = true;

        f.close();
    }
}

void FileBrowserModel::updateRestriction(QModelIndex &i, unsigned size){
    FileBrowserItem *item = static_cast<FileBrowserItem*>(i.internalPointer());
    QString path = "";//virtual path
    QStringList dirs = createRemotePath(item).split("\\");

    if (dirs.size() >= 2){
        dirs.removeFirst();

        path = "/" + dirs.join("/");
    }
    else
        path = "/";

    if (size == 0 && restrict_map.contains(path))
        restrict_map.remove(path);
    else {
        restrict_map[path] = size;
    }
}

void FileBrowserModel::clear(){
    beginRemoveRows(QModelIndex(), 0, (rowCount() >= 1? rowCount() : 1)-1);
    {
        qDeleteAll(rootItem->childItems);
        rootItem->childItems.clear();
    }
    endRemoveRows();
}

void FileBrowserModel::repaint(){
    emit layoutChanged();
}

FileBrowserItem::FileBrowserItem(const QList<QVariant> &data, FileBrowserItem *parent) :
    itemData(data), parentItem(parent), dir(NULL), file(NULL), isDuplicate(false)
{
}

FileBrowserItem::FileBrowserItem(const FileBrowserItem &item){
    itemData = item.itemData;
    dir = item.dir;
    file = item.file;
    isDuplicate = item.isDuplicate;
}
void FileBrowserItem::operator=(const FileBrowserItem &item){
    itemData = item.itemData;
    dir = item.dir;
    file = item.file;
    isDuplicate = item.isDuplicate;
}

FileBrowserItem::~FileBrowserItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void FileBrowserItem::appendChild(FileBrowserItem *item) {
    childItems.append(item);
}

FileBrowserItem *FileBrowserItem::child(int row) {
    return childItems.value(row);
}

int FileBrowserItem::childCount() const {
    return childItems.count();
}

int FileBrowserItem::columnCount() const {
    return itemData.count();
}

QVariant FileBrowserItem::data(int column) const {
    return itemData.value(column);
}

FileBrowserItem *FileBrowserItem::parent() {
    return parentItem;
}

int FileBrowserItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FileBrowserItem*>(this));

    return 0;
}

void FileBrowserItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}

FileBrowserItem *FileBrowserItem::nextSibling(){
    if (!parent())
        return NULL;

    if (row() == (parent()->childCount()-1))
        return NULL;

    return parent()->child(row()+1);
}
