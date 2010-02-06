/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "UserListModel.h"

#include <QtAlgorithms>
#include <QtGlobal>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"

#include "WulforUtil.h"

UserListModel::UserListModel(QObject * parent) : QAbstractItemModel(parent) {
    sortColumn = COLUMN_SHARE;
    sortOrder = Qt::DescendingOrder;
    stripper.setPattern("\\[.*\\]");
    stripper.setMinimal(true);

    QList<QVariant> rootData;

    rootData << tr("Nick") << tr("Share") << tr("Comment")
             << tr("Tag") << tr("Connection") << tr("IP") << tr("E-Mail");

    rootItem = new UserListItem(rootData);

    WU = WulforUtil::getInstance();
}


UserListModel::~UserListModel() {
    users.clear();
    nicks.clear();

    foreach(UserListItem *i, rootItem->childItems)
        pool.destroy(i);

    rootItem->childItems.clear();

    delete rootItem;
}


int UserListModel::rowCount(const QModelIndex & ) const {
    return rootItem->childCount();
}


int UserListModel::columnCount(const QModelIndex & ) const {
    return rootItem->columnCount();
}


QVariant UserListModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    UserListItem * item = static_cast<UserListItem*>(index.internalPointer());

    if (!item)
        return QVariant();

    switch (role){
        case Qt::DisplayRole:
        {
            if (index.column() == COLUMN_SHARE)
                return QString::fromStdString(dcpp::Util::formatBytes(item->data(COLUMN_SHARE).toULongLong()));
            return item->data(index.column());
        }
        case Qt::DecorationRole:
        {
            if (index.column() != COLUMN_NICK)
                break;
            
            if (!item->px || !item->px_loaded)
                item->px = WU->getUserIcon(item->ptr, item->isAway, item->isOp, item->speed);

            return (*item->px);
        }
    }

    return QVariant();
}


QVariant UserListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        return rootItem->data(section);
    }

    return QVariant();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<UserListItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    QList<UserListItem*>::iterator static insertSorted(int col, QList<UserListItem*>& items, UserListItem* item) {
        return qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
    }

    private:
        typedef bool (*AttrComp)(const UserListItem* l, const UserListItem* r);
        AttrComp static getAttrComp(int column) {
            switch (column) {
                case COLUMN_IP:
                {
                    return IPCmp;
                }
                case COLUMN_COMMENT:
                    return AttrCmp<COLUMN_COMMENT>;
                case COLUMN_CONN:
                    return AttrCmp<COLUMN_CONN>;
                case COLUMN_EMAIL:
                    return AttrCmp<COLUMN_EMAIL>;
                case COLUMN_NICK:
                    return AttrCmp<COLUMN_NICK>;
                case COLUMN_SHARE:
                    return NumCmp<COLUMN_SHARE>;
                case COLUMN_TAG:
                    return AttrCmp<COLUMN_TAG>;
            }

            Q_ASSERT_X(false, "getAttrComp", QString("Unknown column %1").arg(column).toLocal8Bit().constData());
            return 0;
        }

        template <int column>
        bool static NumCmp(const UserListItem * l, const UserListItem * r) {
            if (l->isOp != r->isOp)
                return l->isOp;

            return Cmp(l->data(column).toULongLong(), r->data(column).toULongLong());
       }

        bool static IPCmp(const UserListItem * l, const UserListItem * r){
            if (l->isOp == r->isOp){
                QString ip1 = l->data(COLUMN_IP).toString();
                QString ip2 = r->data(COLUMN_IP).toString();

                quint32 l_ip = ip1.section('.',0,0).toULong();
                l_ip <<= 8;
                l_ip |= ip1.section('.',1,1).toULong();
                l_ip <<= 8;
                l_ip |= ip1.section('.',2,2).toULong();
                l_ip <<= 8;
                l_ip |= ip1.section('.',3,3).toULong();

                quint32 r_ip = ip2.section('.',0,0).toULong();
                r_ip <<= 8;
                r_ip |= ip2.section('.',1,1).toULong();
                r_ip <<= 8;
                r_ip |= ip2.section('.',2,2).toULong();
                r_ip <<= 8;
                r_ip |= ip2.section('.',3,3).toULong();

                return Cmp(l_ip, r_ip);
            }
            else
                return l->isOp;
        }

        template <int i>
        bool static AttrCmp(const UserListItem * l, const UserListItem * r) {
            if (l->isOp != r->isOp)
                return l->isOp;
                
            return Cmp(l->data(i).toString(), r->data(i).toString());
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

template <> template <>
bool inline Compare<Qt::AscendingOrder>::Cmp(const QString& l, const QString& r) {
    return Cmp(QString::localeAwareCompare(l, r), 0);
}

template <> template <>
bool inline Compare<Qt::DescendingOrder>::Cmp(const QString& l, const QString& r) {
    return Cmp(QString::localeAwareCompare(l, r), 0);
}

} //namespace


void UserListModel::sort(int column, Qt::SortOrder order) {
    sortColumn = column;
    sortOrder = order;

    if (column == -1) // sorting disabled
    {
        return;
    }

    emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
        Compare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
    else if (order == Qt::DescendingOrder)
        Compare<Qt::DescendingOrder>().sort(column, rootItem->childItems);

    emit layoutChanged();
}

QModelIndex UserListModel::index(int row, int column, const QModelIndex & parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    UserListItem *childItem = rootItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex UserListModel::parent(const QModelIndex & ) const {
    return QModelIndex();
}

void UserListModel::clear() {
    emit layoutAboutToBeChanged();

    users.clear();
    nicks.clear();

    foreach(UserListItem *i, rootItem->childItems)
        pool.destroy(i);

    rootItem->childItems.clear();

    emit layoutChanged();
}

void UserListModel::removeUser(const UserPtr &ptr) {
    USRMap::iterator iter = users.find(ptr);

    if (iter == users.end())
        return;

    const int index = (iter.value())->row();

    beginRemoveRows(QModelIndex(), index, index);

    UserListItem *item = users.value(ptr);

    rootItem->childItems.removeAt(index);
    pool.destroy(item);
    //delete item;

    users.erase(iter);

    endRemoveRows();

    emit listUpdated();
}

void UserListModel::addUser(const UserMap &map, const UserPtr &ptr){
    addUser(map["NICK"].toString(), map["SHARE"].toULongLong(),
            map["COMM"].toString(), map["TAG"].toString(),
            map["CONN"].toString(), map["IP"].toString(),
            map["EMAIL"].toString(), map["ISOP"].toBool(),
            map["AWAY"].toBool(), map["SPEED"].toString(),
            map["CID"].toString(), ptr);
}

void UserListModel::addUser(const QString& nick,
                            const qlonglong share,
                            const QString& comment,
                            const QString& tag,
                            const QString& conn,
                            const QString& ip,
                            const QString& email,

                            bool isOp,
                            bool isAway,
                            const QString& speed,
                            const QString& cid,
                            const UserPtr &ptr)
{
    if (users.contains(ptr))
        return;

    UserListItem *item;

    QList<QVariant> rootData;

    rootData << nick << share << comment << tag << conn << ip << email;

    item = pool.construct(rootData, rootItem);

    item->isOp = isOp;
    item->cid = cid;
    item->isAway = isAway;
    item->speed = speed;
    item->ptr = ptr;

    users.insert(ptr, item);
    nicks.insert(nick, ptr);

    if (sortColumn == -1) // if sorting disabled
    {
        const int i = rootItem->childCount();

        beginInsertRows(QModelIndex(), i, i);
        rootItem->appendChild(item);
        endInsertRows();

        return;
    }

#ifdef _DEBUG_MODEL_
    qDebug() << "Inserting new user item";
#endif

    QList<UserListItem*>::iterator it = rootItem->childItems.end();

    if (sortOrder == Qt::AscendingOrder)
        it = Compare<Qt::AscendingOrder>().insertSorted(sortColumn, rootItem->childItems, item);
    else if (sortOrder == Qt::DescendingOrder)
        it = Compare<Qt::DescendingOrder>().insertSorted(sortColumn, rootItem->childItems, item);

    const int pos = it - rootItem->childItems.begin();

    beginInsertRows(QModelIndex(), pos, pos);
    rootItem->childItems.insert(it, item);

    endInsertRows();

    emit layoutChanged();;
}

UserListItem *UserListModel::itemForPtr(const UserPtr &ptr){
    UserListItem *item = (users.contains(ptr))? (users.value(ptr)) : (NULL);

    return item;
}

QString UserListModel::CIDforNick(const QString &nick){
    if (!nick.isEmpty()){
        QHash<QString, UserPtr>::const_iterator it = nicks.find(nick);

        if (it != nicks.constEnd())
            return it.value()->getCID().toBase32().c_str();
    }

    return "";
}

QStringList UserListModel::matchNicksContaining(const QString & part, bool stripTags) const {
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (QList<UserListItem*>::const_iterator it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->data(COLUMN_NICK).toString().toLower();

        if (nick_lc.contains(part)) {
                matches << (*it)->data(COLUMN_NICK).toString();
        }
    }

    return matches;
}

QStringList UserListModel::matchNicksStartingWith(const QString & part, bool stripTags) const {
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (QList<UserListItem*>::const_iterator it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->data(COLUMN_NICK).toString().toLower();

        if (nick_lc.startsWith(part)) {
            matches << (*it)->data(COLUMN_NICK).toString();
        }
    }

    return matches;
}

UserListItem::UserListItem(const QList<QVariant> &data, UserListItem *parent) :
    itemData(data), parentItem(parent), isOp(false), cid(""), px(NULL), px_loaded(false)
{
}

UserListItem::UserListItem(const UserListItem &item){
    itemData = item.itemData;
    isOp = item.isOp;
    cid = item.cid;
    px = item.px;
    px_loaded = item.px_loaded;
}
void UserListItem::operator=(const UserListItem &item){
    itemData = item.itemData;
    isOp = item.isOp;
    cid = item.cid;
    px = item.px;
    px_loaded = item.px_loaded;
}

UserListItem::~UserListItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void UserListItem::appendChild(UserListItem *item) {
    childItems.append(item);
}

UserListItem *UserListItem::child(int row) {
    return childItems.value(row);
}

int UserListItem::childCount() const {
    return childItems.count();
}

int UserListItem::columnCount() const {
    return itemData.count();
}

QVariant UserListItem::data(int column) const {
    return itemData.value(column);
}

UserListItem *UserListItem::parent() {
    return parentItem;
}

int UserListItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<UserListItem*>(this));

    return 0;
}

void UserListItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}

