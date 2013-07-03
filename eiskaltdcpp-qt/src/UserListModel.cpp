/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "UserListModel.h"

#include <QtAlgorithms>
#include <QtGlobal>

#include "dcpp/stdinc.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/ClientManager.h"
#include "dcpp/Util.h"

#include "WulforUtil.h"

void UserListProxyModel::sort(int column, Qt::SortOrder order){
    if (sourceModel())
        sourceModel()->sort(column, order);
}

UserListModel::UserListModel(QObject * parent) : QAbstractItemModel(parent) {
    sortColumn = COLUMN_SHARE;
    sortOrder = Qt::DescendingOrder;
    stripper.setPattern("\\[.*\\]");
    stripper.setMinimal(true);

    rootItem = new UserListItem();

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
                case COLUMN_NICK: return item->getNick();
                case COLUMN_COMMENT: return item->getComment();
                case COLUMN_TAG: return item->getTag();
                case COLUMN_CONN: return item->getConnection();
                case COLUMN_EMAIL: return item->getEmail();
                case COLUMN_SHARE: return WulforUtil::formatBytes(item->getShare());
                case COLUMN_IP: return item->getIP();
            }

            break;
        }
        case Qt::DecorationRole:
        {
            if (index.column() != COLUMN_NICK)
                break;

            return (*WU->getUserIcon(item->getUser(), item->isAway(), item->isOP(), item->getConnection()));

            break;
        }
        case Qt::ToolTipRole:
        {
            if (index.column() == COLUMN_SHARE)
                return QString::number(item->getShare());
            else {

                QString ttip  = "<b>" + headerData(COLUMN_NICK, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->getNick() + "<br/>";
                ttip += "<b>" + headerData(COLUMN_COMMENT, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->getComment() + "<br/>";
                ttip += "<b>" + headerData(COLUMN_EMAIL, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->getEmail() + "<br/>";
                ttip += "<b>" + headerData(COLUMN_IP, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->getIP() + "<br/>";
                ttip += "<b>" + headerData(COLUMN_SHARE, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " +
                        WulforUtil::formatBytes(item->getShare()) + "<br/>";

                QString tag = item->getTag();
                WulforUtil::getInstance()->textToHtml(tag, true);

                ttip += "<b>" + headerData(COLUMN_TAG, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + tag + "<br/>";
                ttip += "<b>" + headerData(COLUMN_CONN, Qt::Horizontal, Qt::DisplayRole).toString() + "</b>: " + item->getConnection() + "<br/>";

                if (item->isOP())
                    ttip += tr("<b>Hub role</b>: Operator");
                else
                    ttip += tr("<b>Hub role</b>: User");

                if (item->isFav())
                    ttip += tr("<br/><b>Favorite user</b>");

                return ttip;
            }

            break;
        }
        case Qt::TextAlignmentRole:
        {
            if (index.column() == COLUMN_SHARE)
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);

            break;
        }
        case Qt::FontRole:
        {
            if (item->isFav() && WBGET(WB_CHAT_HIGHLIGHT_FAVS)) {
                QFont font;
                font.setBold(true);
                return font;
            }

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
            case COLUMN_EMAIL: return tr("E-mail");
            case COLUMN_SHARE: return tr("Share");
            case COLUMN_IP: return tr("IP");
        }
    }

    return QVariant();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    typedef bool (*AttrComp)(const UserListItem* l, const UserListItem* r);
    
    void static sort(unsigned column, QList<UserListItem*>& items) {
        if (column > COLUMN_EMAIL)
            return;

        qStableSort(items.begin(), items.end(), attrs[column]);
    }

    QList<UserListItem*>::iterator static insertSorted(unsigned column, QList<UserListItem*>& items, UserListItem* item) {
        if (column > COLUMN_EMAIL)
            return items.end();

        return qLowerBound(items.begin(), items.end(), item, attrs[column] );
    }

    private:
        template <typename T, T (UserListItem::*attr)() const >
        bool static AttrCmp(const UserListItem * l, const UserListItem * r) {
            if (l->isOP() != r->isOP())
                return l->isOP();
            else if (l->isFav() != r->isFav())
                return l->isFav();
            else
                return Cmp((const_cast<UserListItem*>(l)->*attr)(), (const_cast<UserListItem*>(r)->*attr)());
        }

        bool static IPCmp(const UserListItem * l, const UserListItem * r) {
            if (l->isOP() != r->isOP())
                return l->isOP();
            else if (!(l->getIP().isEmpty() || r->getIP().isEmpty())){
                QString ip1 = l->getIP();
                QString ip2 = r->getIP();

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

            return false;
        }

        template <typename T>
        inline bool static Cmp(const T& l, const T& r) __attribute__((always_inline));
        
        static AttrComp attrs[7];
};

template <Qt::SortOrder order>
typename Compare<order>::AttrComp Compare<order>::attrs[7]  = {     AttrCmp<QString, &UserListItem::getNick>,
                                                                    AttrCmp<qulonglong, &UserListItem::getShare>,
                                                                    AttrCmp<QString, &UserListItem::getComment>,
                                                                    AttrCmp<QString, &UserListItem::getTag>,
                                                                    AttrCmp<QString, &UserListItem::getConnection>,
                                                                    IPCmp,
                                                                    AttrCmp<QString, &UserListItem::getEmail> };

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

typedef Compare<Qt::AscendingOrder>     AscendingCompare;
typedef Compare<Qt::DescendingOrder>    DescendingCompare;

void UserListModel::sort(int column, Qt::SortOrder order) {
    static AscendingCompare  acomp = AscendingCompare();
    static DescendingCompare dcomp = DescendingCompare();

    sortColumn = column;
    sortOrder = order;

    if (column < 0 || column > columnCount() - 1)
        return;

    if (rootItem->childItems.size() <= 0)
        return;

    emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
        acomp.sort(column, rootItem->childItems);
    else if (order == Qt::DescendingOrder)
        dcomp.sort(column, rootItem->childItems);

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

    qDeleteAll(rootItem->childItems);

    rootItem->childItems.clear();

    emit layoutChanged();
}

void UserListModel::removeUser(const UserPtr &ptr) {
    auto iter = users.find(ptr);

    if (iter == users.end())
        return;

    const int index = (iter.value())->row();

    beginRemoveRows(QModelIndex(), index, index);

    UserListItem *item = iter.value();

    rootItem->childItems.removeAt(index);
    delete item;

    users.erase(iter);

    endRemoveRows();
}


void UserListModel::updateUser(UserListItem *item, const Identity& _id, const QString& _cid, bool _fav) {
    if (!item || item->parent() != rootItem)
        return;

    bool needSorted = (item->getIdentity().isOp() != _id.isOp()) || (item->isFav() != _fav);

    if (sortColumn != -1) {
        switch (sortColumn) {
            case COLUMN_NICK:
                needSorted = needSorted || (item->getIdentity().getNick() != _id.getNick());
                break;
            case COLUMN_SHARE:
                needSorted = needSorted || (item->getIdentity().getBytesShared() != _id.getBytesShared());
                break;
            case COLUMN_COMMENT:
                needSorted = needSorted || (item->getIdentity().getDescription() != _id.getDescription());
                break;
            case COLUMN_TAG:
                needSorted = needSorted || (item->getIdentity().getTag() != _id.getTag());
                break;
            case COLUMN_CONN:
                needSorted = needSorted || (item->getIdentity().getConnection() != _id.getConnection());
                break;
            case COLUMN_IP:
                needSorted = needSorted || (item->getIdentity().getIp() != _id.getIp());
                break;
            case COLUMN_EMAIL:
                needSorted = needSorted || (item->getIdentity().getEmail() != _id.getEmail());
                break;
        }
    }

    item->updateIdentity(_id, _cid, _fav);

    if (needSorted) {

        static AscendingCompare  acomp = AscendingCompare();
        static DescendingCompare dcomp = DescendingCompare();

        const int oldRow = item->row();

        beginRemoveRows(QModelIndex(), oldRow, oldRow);
        {
            rootItem->childItems.removeAt(oldRow);
        }
        endRemoveRows();

        auto it = rootItem->childItems.end();

        if (sortOrder == Qt::AscendingOrder)
            it = acomp.insertSorted(sortColumn, rootItem->childItems, item);
        else if (sortOrder == Qt::DescendingOrder)
            it = dcomp.insertSorted(sortColumn, rootItem->childItems, item);

        const int newRow = it - rootItem->childItems.begin();

        beginInsertRows(QModelIndex(), newRow, newRow);
        {
            rootItem->childItems.insert(it, item);
        }
        endInsertRows();
    } else {
        repaintData(index(item->row(), COLUMN_NICK), index(item->row(), COLUMN_EMAIL));
    }

    return;
}

UserListItem *UserListModel::addUser(const UserPtr& _ptr, const Identity& _id, const QString& _cid, bool _fav) {

    if (users.contains(_ptr))
        return itemForPtr(_ptr);

    static AscendingCompare  acomp = AscendingCompare();
    static DescendingCompare dcomp = DescendingCompare();

    UserListItem *item = new UserListItem(rootItem, _ptr, _id, _cid, _fav);

    users.insert(_ptr, item);

    if (sortColumn == -1) // if sorting disabled
    {
        const int row = rootItem->childCount();

        beginInsertRows(QModelIndex(), row, row);
        {
            rootItem->appendChild(item);
        }
        endInsertRows();

    } else {
        auto it = rootItem->childItems.end();

        if (sortOrder == Qt::AscendingOrder)
            it = acomp.insertSorted(sortColumn, rootItem->childItems, item);
        else if (sortOrder == Qt::DescendingOrder)
            it = dcomp.insertSorted(sortColumn, rootItem->childItems, item);

        const int row = it - rootItem->childItems.begin();

        beginInsertRows(QModelIndex(), row, row);
        {
            rootItem->childItems.insert(it, item);
        }
        endInsertRows();
    }

    return item;
}

UserListItem *UserListModel::itemForPtr(const UserPtr &ptr){
    auto iter = users.find(ptr);

    UserListItem *item = (iter != users.end())? (iter.value()) : (NULL);

    return item;
}

UserListItem *UserListModel::itemForNick(const QString &nick, const QString &){   
    if (nick.isEmpty())
        return NULL;
    
    auto it = std::find_if(rootItem->childItems.begin(), rootItem->childItems.end(),
                                                     [&nick] (const UserListItem *i) {
                                                        return (i->getNick() == nick);
                                                     }
                                                    );

    return (it == rootItem->childItems.end()? NULL : *it);
}

QString UserListModel::CIDforNick(const QString &nick, const QString &){
    UserListItem *item = itemForNick(nick, "");

    return (item? item->getCID() : "");
}

QStringList UserListModel::matchNicksContaining(const QString & part, bool stripTags) const {
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (auto it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->getNick().toLower();

        if (nick_lc.contains(part)) {
                matches << (*it)->getNick();
        }
    }

    return matches;
}

QStringList UserListModel::matchNicksStartingWith(const QString & part, bool stripTags) const {
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (auto it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->getNick().toLower();

        if (nick_lc.startsWith(part)) {
            matches << (*it)->getNick();
        }
    }

    return matches;
}

QStringList UserListModel::matchNicksAny(const QString &part, bool stripTags) const{
    QStringList matches;

    if (part.isEmpty()) {
        return matches;
    }

    for (auto it = rootItem->childItems.constBegin(); it != rootItem->childItems.constEnd(); ++it) {
        QString nick_lc = (*it)->getNick().toLower();

        if (nick_lc.startsWith(part) || nick_lc.contains(part)) {
            matches << (*it)->getNick();
        }
    }

    return matches;
}

QStringList UserListModel::findItems(const QString &part, Qt::MatchFlags flags, int column) const
{
    QModelIndexList indexes = match(index(0, column, QModelIndex()),
                                    Qt::DisplayRole, part, -1, flags);
    QStringList items;
    for (int i = 0; i < indexes.size(); ++i) {
        QModelIndex index = indexes.at(i);
        if (index.isValid())
            items.append( index.data().toString() );
    }
    return items;
}

void UserListModel::repaintItem(const UserListItem *item){
    int r = rootItem->childItems.indexOf(const_cast<UserListItem*>(item));

    if (!(item && r >= 0))
        return;

    repaintData(createIndex(r, COLUMN_NICK, const_cast<UserListItem*>(item)), createIndex(r, COLUMN_EMAIL, const_cast<UserListItem*>(item)));
}

UserListItem::UserListItem() : ptr(NULL), parentItem(NULL) { }

UserListItem::UserListItem(UserListItem *parent, dcpp::UserPtr _ptr, const Identity& _id, const QString& _cid, bool _fav) :
    parentItem(parent), ptr(_ptr)
{
    updateIdentity(_id, _cid, _fav);
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

qulonglong UserListItem::getShare()  const{
    return id.getBytesShared();
}

QString UserListItem::getComment()  const{
    return _q(id.getDescription());
}

QString UserListItem::getConnection()  const{
    return _q(id.getConnection());
}

QString UserListItem::getEmail()  const{
    return _q(id.getEmail());
}

QString UserListItem::getIP()  const{
    return _q(id.getIp());
}   

QString UserListItem::getNick()  const{
    return _q(id.getNick());
}

QString UserListItem::getTag()  const{
    return _q(id.getTag());
}

QString UserListItem::getCID()  const{
    return cid;
}

dcpp::UserPtr UserListItem::getUser()  const{
    return ptr;
}

bool UserListItem::isOP()  const{
    return _isOp;
}

bool UserListItem::isFav()  const{
    return _isFav;
}

bool UserListItem::isAway() const {
    return id.isAway();
}

void UserListItem::updateIdentity(const Identity& _id, const QString& _cid, bool _fav) {
    if (ptr) {
        id = _id;
        cid = _cid;
        _isOp   = id.isOp();
        _isFav  = _fav;
    }
}
