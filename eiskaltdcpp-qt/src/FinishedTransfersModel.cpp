/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "FinishedTransfersModel.h"

#include <QtGui>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QDir>

#include "FinishedTransfersModel.h"
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

FinishedTransfersModel::FinishedTransfersModel(QObject *parent):
        QAbstractItemModel(parent), sortColumn(0), sortOrder(Qt::AscendingOrder)
{
    QList<QVariant> userData;
    userData << tr("User")<< tr("Files") << tr("Time") << tr("Transferred")
             << tr("Speed") << tr("Elapsed time") << tr("Full");

    userItem = new FinishedTransfersItem(userData);

    QList<QVariant> fileData;
    fileData << tr("Filename") << tr("Path") << tr("Time") << tr("User")
             << tr("Transferred") << tr("Speed")   << tr("Check sum")
             << tr("Target") << tr("Elapsed time") << tr("Full");

    fileItem = new FinishedTransfersItem(fileData);

    rootItem = fileItem;

    file_header_table.insert(COLUMN_FINISHED_NAME,      "FNAME");
    file_header_table.insert(COLUMN_FINISHED_PATH,      "PATH");
    file_header_table.insert(COLUMN_FINISHED_TIME,      "TIME");
    file_header_table.insert(COLUMN_FINISHED_USER,      "USERS");
    file_header_table.insert(COLUMN_FINISHED_TR,        "TR");
    file_header_table.insert(COLUMN_FINISHED_SPEED,     "SPEED");
    file_header_table.insert(COLUMN_FINISHED_CRC32,     "CRC32");
    file_header_table.insert(COLUMN_FINISHED_TARGET,    "TARGET");
    file_header_table.insert(COLUMN_FINISHED_ELAPS,     "ELAP");
    file_header_table.insert(COLUMN_FINISHED_FULL,      "FULL");

    user_header_table.insert(COLUMN_FINISHED_NAME,      "NICK");
    user_header_table.insert(COLUMN_FINISHED_PATH,      "FILES");
    user_header_table.insert(COLUMN_FINISHED_TIME,      "TIME");
    user_header_table.insert(COLUMN_FINISHED_USER,      "TR");
    user_header_table.insert(COLUMN_FINISHED_TR,        "SPEED");
    user_header_table.insert(COLUMN_FINISHED_SPEED,     "ELAP");
    user_header_table.insert(COLUMN_FINISHED_CRC32,     "FULL");
}

FinishedTransfersModel::~FinishedTransfersModel()
{
    delete userItem;
    delete fileItem;
}

int FinishedTransfersModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FinishedTransfersItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant FinishedTransfersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    FinishedTransfersItem *item = static_cast<FinishedTransfersItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
        {
            if (rootItem == fileItem){
                if (index.column() == COLUMN_FINISHED_NAME)
                    return WulforUtil::getInstance()->getPixmapForFile(item->data(COLUMN_FINISHED_TARGET).toString()).scaled(16, 16);
            }

            break;
        }
        case Qt::DisplayRole:
        {
            if (rootItem == fileItem){
                if (index.column() == COLUMN_FINISHED_ELAPS)
                    return _q(Util::formatSeconds(item->data(COLUMN_FINISHED_ELAPS).toLongLong()/1000L));
                else if (index.column() == COLUMN_FINISHED_SPEED)
                    return tr("%1/s").arg(WulforUtil::formatBytes(item->data(COLUMN_FINISHED_SPEED).toLongLong()));
                else if (index.column() == COLUMN_FINISHED_TR)
                    return WulforUtil::formatBytes(item->data(COLUMN_FINISHED_TR).toLongLong());
                else if (index.column() == COLUMN_FINISHED_FULL)
                    return (item->data(COLUMN_FINISHED_FULL).toBool()? tr("Yes") : tr("No"));
            }
            else {
                if (index.column() == COLUMN_FINISHED_SPEED)
                    return _q(Util::formatSeconds(item->data(COLUMN_FINISHED_SPEED).toLongLong()/1000L));
                else if (index.column() == COLUMN_FINISHED_TR)
                    return tr("%1/s").arg(WulforUtil::formatBytes(item->data(COLUMN_FINISHED_TR).toLongLong()));
                else if (index.column() == COLUMN_FINISHED_USER)
                    return WulforUtil::formatBytes(item->data(COLUMN_FINISHED_USER).toLongLong());
                else if (index.column() == COLUMN_FINISHED_CRC32)
                    return (item->data(COLUMN_FINISHED_CRC32).toBool()? tr("Yes") : tr("No"));
            }

            return item->data(index.column());
        }
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

Qt::ItemFlags FinishedTransfersModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant FinishedTransfersModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex FinishedTransfersModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FinishedTransfersItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FinishedTransfersItem*>(parent.internalPointer());

    FinishedTransfersItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FinishedTransfersModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int FinishedTransfersModel::rowCount(const QModelIndex &parent) const
{
    FinishedTransfersItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FinishedTransfersItem*>(parent.internalPointer());

    return parentItem->childCount();
}

namespace {

template <Qt::SortOrder order>
struct FileCompare {
    void static sort(int col, QList<FinishedTransfersItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<FinishedTransfersItem*>& items, FinishedTransfersItem* item) {
        QList<FinishedTransfersItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const FinishedTransfersItem * l, const FinishedTransfersItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                case COLUMN_FINISHED_NAME:
                    return AttrCmp<COLUMN_FINISHED_NAME>;
                case COLUMN_FINISHED_PATH:
                    return AttrCmp<COLUMN_FINISHED_PATH>;
                case COLUMN_FINISHED_TIME:
                    return AttrCmp<COLUMN_FINISHED_TIME>;
                case COLUMN_FINISHED_USER:
                    return AttrCmp<COLUMN_FINISHED_USER>;
                case COLUMN_FINISHED_TR:
                    return NumCmp<COLUMN_FINISHED_TR>;
                case COLUMN_FINISHED_SPEED:
                    return NumCmp<COLUMN_FINISHED_SPEED>;
                case COLUMN_FINISHED_CRC32:
                    return NumCmp<COLUMN_FINISHED_CRC32>;
                case COLUMN_FINISHED_TARGET:
                    return AttrCmp<COLUMN_FINISHED_TARGET>;
                case COLUMN_FINISHED_FULL:
                    return NumCmp<COLUMN_FINISHED_FULL>;
                default:
                    return NumCmp<COLUMN_FINISHED_ELAPS>;
            }

            return NULL;
        }
        template <int i>
        bool static AttrCmp(const FinishedTransfersItem * l, const FinishedTransfersItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <typename T, T (FinishedTransfersItem::*attr)>
        bool static AttrCmp(const FinishedTransfersItem * l, const FinishedTransfersItem * r) {
            return Cmp(l->*attr, r->*attr);
        }
        template <int i>
        bool static NumCmp(const FinishedTransfersItem * l, const FinishedTransfersItem * r) {
            return Cmp(l->data(i).toULongLong(), r->data(i).toULongLong());
        }
        template <typename T>
        bool static Cmp(const T& l, const T& r);
};

template <> template <typename T>
bool inline FileCompare<Qt::AscendingOrder>::Cmp(const T& l, const T& r) {
    return l < r;
}

template <> template <typename T>
bool inline FileCompare<Qt::DescendingOrder>::Cmp(const T& l, const T& r) {
    return l > r;
}

template <Qt::SortOrder order>
struct UserCompare {
    void static sort(int col, QList<FinishedTransfersItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<FinishedTransfersItem*>& items, FinishedTransfersItem* item) {
        QList<FinishedTransfersItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const FinishedTransfersItem * l, const FinishedTransfersItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                case COLUMN_FINISHED_NAME:
                    return AttrCmp<COLUMN_FINISHED_NAME>;
                case COLUMN_FINISHED_PATH:
                    return AttrCmp<COLUMN_FINISHED_PATH>;
                case COLUMN_FINISHED_TIME:
                    return AttrCmp<COLUMN_FINISHED_TIME>;
                case COLUMN_FINISHED_USER:
                    return NumCmp<COLUMN_FINISHED_USER>;
                case COLUMN_FINISHED_TR:
                    return NumCmp<COLUMN_FINISHED_TR>;
                case COLUMN_FINISHED_FULL:
                    return NumCmp<COLUMN_FINISHED_FULL>;
                default:
                    return AttrCmp<COLUMN_FINISHED_ELAPS>;
            }

            return NULL;
        }
        template <int i>
        bool static AttrCmp(const FinishedTransfersItem * l, const FinishedTransfersItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <typename T, T (FinishedTransfersItem::*attr)>
        bool static AttrCmp(const FinishedTransfersItem * l, const FinishedTransfersItem * r) {
            return Cmp(l->*attr, r->*attr);
        }
        template <int i>
        bool static NumCmp(const FinishedTransfersItem * l, const FinishedTransfersItem * r) {
            return Cmp(l->data(i).toULongLong(), r->data(i).toULongLong());
        }
        template <typename T>
        bool static Cmp(const T& l, const T& r);
};

template <> template <typename T>
bool inline UserCompare<Qt::AscendingOrder>::Cmp(const T& l, const T& r) {
    return l < r;
}

template <> template <typename T>
bool inline UserCompare<Qt::DescendingOrder>::Cmp(const T& l, const T& r) {
    return l > r;
}

} //namespace

void FinishedTransfersModel::sort(int column, Qt::SortOrder order) {
    emit layoutAboutToBeChanged();

    sortColumn = column;
    sortOrder = order;

    if (rootItem == fileItem){
        if (order == Qt::AscendingOrder)
            FileCompare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
        else if (order == Qt::DescendingOrder)
            FileCompare<Qt::DescendingOrder>().sort(column, rootItem->childItems);
    }
    else {
        if (order == Qt::AscendingOrder)
            UserCompare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
        else if (order == Qt::DescendingOrder)
            UserCompare<Qt::DescendingOrder>().sort(column, rootItem->childItems);
    }

    emit layoutChanged();
}

void FinishedTransfersModel::clearModel(){
    blockSignals(true);
    {
        qDeleteAll(userItem->childItems);
        qDeleteAll(fileItem->childItems);

        userItem->childItems.clear();
        fileItem->childItems.clear();

        file_hash.clear();
        user_hash.clear();
    }
    blockSignals(false);

    emit layoutChanged();
}

void FinishedTransfersModel::addFile(const QMap<QString, QVariant> &params){
    FinishedTransfersItem *item = findFile(params["TARGET"].toString());

    if (!item)
        return;

    for (int i = 0; i < fileItem->columnCount(); i++){
        if (file_header_table[i] == "USERS"){
            QStringList users = params[file_header_table[i]].toString().split(" ");
            QStringList old_users = item->data(i).toString().split(" ");

            if (users.isEmpty())
                continue;
            else{
                foreach (QString nick, users){
                    if (!old_users.contains(nick))
                        old_users.push_back(nick);
                }

                item->updateColumn(i, old_users.join(" "));
            }
        }
        else
            item->updateColumn(i, params[file_header_table[i]]);
    }

    emit dataChanged(createIndex(item->row(), COLUMN_FINISHED_NAME, item), createIndex(item->row(), COLUMN_FINISHED_FULL, item));
}

void FinishedTransfersModel::addUser(const QMap<QString, QVariant> &params){
    FinishedTransfersItem *item = findUser(params["CID"].toString());

    if (!item)
        return;

    for (int i = 0; i < userItem->columnCount(); i++){
        if (user_header_table[i] == "NICK"){
            QString user = params[user_header_table[i]].toString();

            if (user.trimmed().isEmpty() || user.trimmed().isNull())
                continue;
            else
                item->updateColumn(i, user);
        }
        else
            item->updateColumn(i, params[user_header_table[i]]);
    }

    emit dataChanged(createIndex(item->row(), COLUMN_FINISHED_NAME, item), createIndex(item->row(), COLUMN_FINISHED_CRC32, item));
}

void FinishedTransfersModel::remFile(const QString &file){
    FinishedTransfersItem *item = findFile(file);

    if (!item)
        return;

    file_hash.remove(file);

    beginRemoveRows(QModelIndex(), item->row(), item->row());
    {
        fileItem->childItems.removeAt(item->row());

        delete item;
    }
    endRemoveRows();
}

void FinishedTransfersModel::remUser(const QString &cid){
    FinishedTransfersItem *item = findUser(cid);

    if (!item)
        return;

    user_hash.remove(cid);

    beginRemoveRows(QModelIndex(), item->row(), item->row());
    {
        userItem->childItems.removeAt(item->row());

        delete item;
    }
    endRemoveRows();
}

void FinishedTransfersModel::switchViewType(FinishedTransfersModel::ViewType t){
    switch (t){
        case FileView:
            rootItem = fileItem;
            break;
        case UserView:
            rootItem = userItem;
            break;
    }

    reset();
}

FinishedTransfersItem *FinishedTransfersModel::findFile(const QString &fname){
    if (fname.isEmpty())
        return NULL;

    QHash<QString, FinishedTransfersItem* >::const_iterator it = file_hash.find(fname);

    if (it != file_hash.constEnd())
        return const_cast<FinishedTransfersItem*>(it.value());

    FinishedTransfersItem *item = new FinishedTransfersItem(QList<QVariant>() << "" << "" << "" << ""
                                                                              << "" << "" << "" << ""
                                                                              << "" << false,
                                                            fileItem);
    if (fileItem == rootItem){
        emit beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
        {
            fileItem->appendChild(item);
        }
        emit endInsertRows();
    }
    else
        fileItem->appendChild(item);

    file_hash.insert(fname, item);

    if (rootItem == fileItem)
        sort();

    return item;
}

FinishedTransfersItem *FinishedTransfersModel::findUser(const QString &cid){
    if (cid.isEmpty())
        return NULL;

    QHash<QString, FinishedTransfersItem* >::const_iterator it = user_hash.find(cid);

    if (it != user_hash.constEnd())
        return const_cast<FinishedTransfersItem*>(it.value());

    FinishedTransfersItem *item = new FinishedTransfersItem(QList<QVariant>() << "" << "" << ""
                                                                              << "" << "" << ""
                                                                              << false,
                                                            userItem);
    if (fileItem == rootItem){
        emit beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
        {
            userItem->appendChild(item);
        }
        emit endInsertRows();
    }
    else
        userItem->appendChild(item);;

    user_hash.insert(cid, item);

    if (rootItem == userItem)
        sort();

    return item;
}

void FinishedTransfersModel::repaint(){
    emit layoutChanged();
}

FinishedTransfersItem::FinishedTransfersItem(const QList<QVariant> &data, FinishedTransfersItem *parent) :
    itemData(data),
    parentItem(parent)
{
}

FinishedTransfersItem::~FinishedTransfersItem()
{
    qDeleteAll(childItems);
}

void FinishedTransfersItem::appendChild(FinishedTransfersItem *item) {
    childItems.append(item);
}

FinishedTransfersItem *FinishedTransfersItem::child(int row) {
    return ((row >= 0 && row <= childItems.count()-1)? childItems.value(row) : NULL);
}

int FinishedTransfersItem::childCount() const {
    return childItems.count();
}

int FinishedTransfersItem::columnCount() const {
    return itemData.count();
}

QVariant FinishedTransfersItem::data(int column) const {
    return itemData.value(column);
}

FinishedTransfersItem *FinishedTransfersItem::parent() const{
    return parentItem;
}

int FinishedTransfersItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FinishedTransfersItem*>(this));

    return 0;
}

void FinishedTransfersItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}
