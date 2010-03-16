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
#include "dcpp/FavoriteManager.h"
#include "dcpp/Util.h"

#include "WulforUtil.h"

UserListModel::UserListModel(QObject * parent) : QAbstractItemModel(parent) {
    sortColumn = COLUMN_SHARE;
    sortOrder = Qt::DescendingOrder;
    stripper.setPattern("\\[.*\\]");
    stripper.setMinimal(true);

    rootItem = new UserListItem(NULL);

    WU = WulforUtil::getInstance();
}


UserListModel::~UserListModel() {
    delete rootItem;
}


int UserListModel::rowCount(const QModelIndex & ) const {
    return rootItem->childCount();
}

int UserListModel::columnCount(const QModelIndex & ) const {
    return 7;
}

bool UserListModel::hasChildren(const QModelIndex &parent) const{
    return (!parent.isValid());
}

bool UserListModel::canFetchMore(const QModelIndex &parent) const{
    return false;
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
            switch (index.column()) {
                case COLUMN_NICK: return item->nick;
                case COLUMN_COMMENT: return item->comm;
                case COLUMN_TAG: return item->tag;
                case COLUMN_CONN: return item->conn;
                case COLUMN_EMAIL: return item->email;
                case COLUMN_SHARE: return QString::fromStdString(dcpp::Util::formatBytes(item->share));
                case COLUMN_IP: return item->ip;
            }

            break;
        }
        case Qt::DecorationRole:
        {
            if (index.column() != COLUMN_NICK)
                break;
            
            if (item->px)
                return (*item->px);

            break;
        }
        case Qt::ToolTipRole:
        {
            if (index.column() == COLUMN_SHARE)
                return QString::number(item->share);
            else {
                QString ttip = "";

                ttip =  "<b>" + headerData(COLUMN_NICK, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->nick + "<br/>";
                ttip += "<b>" + headerData(COLUMN_COMMENT, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->comm + "<br/>";
                ttip += "<b>" + headerData(COLUMN_EMAIL, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->email + "<br/>";
                ttip += "<b>" + headerData(COLUMN_IP, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->ip + "<br/>";
                ttip += "<b>" + headerData(COLUMN_SHARE, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " +
                        QString::fromStdString(dcpp::Util::formatBytes(item->share)) + "<br/>";
                ttip += "<b>" + headerData(COLUMN_TAG, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->tag + "<br/>";
                ttip += "<b>" + headerData(COLUMN_CONN, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->conn + "<br/>";

                if (item->isOp)
                    ttip += tr("<b>Hub role</b>: Operator");
                else
                    ttip += tr("<b>Hub role</b>: User");

                if (FavoriteManager::getInstance()->isFavoriteUser(item->ptr))
                    ttip += tr("<br/><b>Favorite user</b>");

                return ttip;
            }

            break;
        }
        case Qt::TextAlignmentRole:
        {
            if (index.column() == COLUMN_SHARE)
                return Qt::AlignRight;

            break;
        }
    }

    return QVariant();
}


QVariant UserListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
            case COLUMN_NICK: return tr("Nick");
            case COLUMN_COMMENT: return tr("Comment");
            case COLUMN_TAG: return tr("Tag");
            case COLUMN_CONN: return tr("Connection");
            case COLUMN_EMAIL: return tr("Email");
            case COLUMN_SHARE: return tr("Share");
            case COLUMN_IP: return tr("IP");
        }
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
                case COLUMN_NICK:
                    return AttrCmp<QString, &UserListItem::nick>;
                    break;
                case COLUMN_COMMENT:
                    return AttrCmp<QString, &UserListItem::comm>;
                    break;
                case COLUMN_TAG:
                    return AttrCmp<QString, &UserListItem::tag>;
                    break;
                case COLUMN_CONN:
                    return AttrCmp<QString, &UserListItem::conn>;
                    break;
                case COLUMN_EMAIL:
                    return AttrCmp<QString, &UserListItem::email>;
                    break;
                case COLUMN_SHARE:
                    return AttrCmp<qulonglong, &UserListItem::share>;
                    break;
                case COLUMN_IP:
                    return IPCmp;
                    break;
            }
            Q_ASSERT_X(false, "getAttrComp", QString("Unknown column %1").arg(column).toLocal8Bit().constData());
            return 0;
        }
        template <typename T, T (UserListItem::*attr)>
        bool static AttrCmp(const UserListItem * l, const UserListItem * r) {
            if (l->isOp != r->isOp)
                return l->isOp;
            else
                return Cmp(l->*attr, r->*attr);;
        }

        bool static IPCmp(const UserListItem * l, const UserListItem * r){
            if (l->isOp != r->isOp)
                return l->isOp;
            else {
                QString ip1 = l->ip;
                QString ip2 = r->ip;

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

    if (column < 0) // sorting disabled
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

QModelIndex UserListModel::index(int row, int column, const QModelIndex &) const {
    if (row > (rootItem->childCount() - 1) || row < 0)
        return QModelIndex();

    return createIndex(row, column, rootItem->child(row));
}

QModelIndex UserListModel::parent(const QModelIndex & ) const {
    return QModelIndex();
}

void UserListModel::clear() {
    emit layoutAboutToBeChanged();

    users.clear();
    nicks.clear();

    qDeleteAll(rootItem->childItems);

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
    delete item;

    users.erase(iter);

    endRemoveRows();
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

    item = new UserListItem(rootItem);

    item->px = WU->getUserIcon(ptr, isAway, isOp, speed);
    item->nick = nick;
    item->share = share;
    item->comm = comment;
    item->tag = tag;
    item->conn = conn;
    item->ip = ip;
    item->email = email;
    item->isOp = isOp;
    item->cid = cid;
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

    QList<UserListItem*>::iterator it = rootItem->childItems.end();

    if (sortOrder == Qt::AscendingOrder)
        it = Compare<Qt::AscendingOrder>().insertSorted(sortColumn, rootItem->childItems, item);
    else if (sortOrder == Qt::DescendingOrder)
        it = Compare<Qt::DescendingOrder>().insertSorted(sortColumn, rootItem->childItems, item);

    const int pos = it - rootItem->childItems.begin();

    beginInsertRows(QModelIndex(), pos, pos);

    rootItem->childItems.insert(it, item);

    endInsertRows();
}

UserListItem *UserListModel::itemForPtr(const UserPtr &ptr){
    UserListItem *item = (users.contains(ptr))? (users.value(ptr)) : (NULL);

    return item;
}

UserListItem *UserListModel::itemForNick(const QString &nick){
    if (!nick.isEmpty()){
        QHash<QString, UserPtr>::const_iterator it = nicks.find(nick);

        if (it == nicks.constEnd())
            return NULL;

        USRMap::const_iterator ut = users.find(it.value());

        if (ut != users.constEnd())
            return ut.value();
    }

    return NULL;
}

QString UserListModel::CIDforNick(const QString &nick){
    if (!nick.isEmpty()){
        QHash<QString, UserPtr>::const_iterator it = nicks.find(nick);

        if (it != nicks.constEnd())
            return _q(it.value()->getCID().toBase32());
    }

    return "";
}

QStringList UserListModel::matchNicksContaining(const QString & part, bool stripTags) const {
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (QList<UserListItem*>::const_iterator it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->nick.toLower();

        if (nick_lc.contains(part)) {
                matches << (*it)->nick;
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
        QString nick_lc = (*it)->nick.toLower();

        if (nick_lc.startsWith(part)) {
            matches << (*it)->nick;
        }
    }

    return matches;
}

QStringList UserListModel::matchNicksAny(const QString &part, bool stripTags) const{
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (QList<UserListItem*>::const_iterator it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->nick.toLower();

        if (nick_lc.startsWith(part) || nick_lc.contains(part)) {
            matches << (*it)->nick;
        }
    }

    return matches;
}

UserListItem::UserListItem(UserListItem *parent) :
    isOp(false), px(NULL), parentItem(parent)
{
}

UserListItem::~UserListItem()
{
    qDeleteAll(childItems);
}

void UserListItem::appendChild(UserListItem *item) {
    item->parentItem = this;
    childItems.append(item);
}

UserListItem *UserListItem::child(int row) {
    return childItems.value(row);
}

int UserListItem::childCount() const {
    return childItems.count();
}

int UserListItem::columnCount() const {
    return 7;
}
UserListItem *UserListItem::parent() {
    return parentItem;
}

int UserListItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<UserListItem*>(this));

    return 0;
}
