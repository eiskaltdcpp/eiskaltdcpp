/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "QueuedUsers.h"
#include "WulforSettings.h"

#include "dcpp/UploadManager.h"
#include "dcpp/ClientManager.h"

#include <QAbstractItemModel>
#include <QMenu>

QueuedUsers::QueuedUsers(){
    setupUi(this);

    setUnload(false);

    model = new QueuedUsersModel(this);
    treeView_USERS->setModel(model);
    treeView_USERS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_USERS->header()->restoreState(WVGET("queued-users/headerstate", QByteArray()).toByteArray());

    connect(this, SIGNAL(coreWaitingAddFile(VarMap)), this, SLOT(slotWaitingAddFile(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreWaitingRemoved(VarMap)), this, SLOT(slotWaitingRemoved(VarMap)), Qt::QueuedConnection);
    connect(treeView_USERS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));

    UploadManager::getInstance()->addListener(this);
    
    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

QueuedUsers::~QueuedUsers(){
    UploadManager::getInstance()->removeListener(this);
}

void QueuedUsers::closeEvent(QCloseEvent *e){
    if (isUnload()){
        WVSET("queued-users/headerstate", treeView_USERS->header()->saveState());

        e->accept();
    }
    else {
        e->ignore();
    }
}

void QueuedUsers::slotWaitingAddFile(const VarMap &map){
    model->addResult(map);
}

void QueuedUsers::slotWaitingRemoved(const VarMap &map){
    model->remResult(map);
}

void QueuedUsers::slotContextMenu(){
    QModelIndexList indexes = treeView_USERS->selectionModel()->selectedRows(0);

    if (indexes.isEmpty())
        return;

    QMenu *m = new QMenu(this);
    m->addAction(tr("Grant slot"));

    if (m->exec(QCursor::pos())){
        foreach (const QModelIndex &i, indexes){
            QueuedUserItem *item = reinterpret_cast<QueuedUserItem*>(i.internalPointer());

            if (!item)
                continue;

            QString id = item->cid;

            if (!id.isEmpty()){
                UserPtr user = ClientManager::getInstance()->findUser(CID(id.toStdString()));

                if (user){
                    try { UploadManager::getInstance()->reserveSlot(HintedUser(user, _tq(item->hub))); }
                    catch ( ... ) {}
                }
            }

        }
    }

    m->deleteLater();
}

void QueuedUsers::on(WaitingAddFile, const dcpp::HintedUser &user, const std::string &file) noexcept {
    VarMap map;
    map["CID"]  = _q(user.user->getCID().toBase32());
    map["FILE"] = _q(file);
    map["HUB"]  = _q(user.hint);

    emit coreWaitingAddFile(map);
}

void QueuedUsers::on(WaitingRemoveUser, const dcpp::HintedUser &user) noexcept {
    VarMap map;
    map["CID"]  = _q(user.user->getCID().toBase32());
    map["FILE"] = "";
    map["HUB"]  = _q(user.hint);

    emit coreWaitingRemoved(map);
}

QueuedUsersModel::QueuedUsersModel(QObject *parent):
        QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("User") << tr("File");

    rootItem = new QueuedUserItem(rootData);
}

QueuedUsersModel::~QueuedUsersModel()
{
    delete rootItem;
}

int QueuedUsersModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant QueuedUsersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QueuedUserItem *item = static_cast<QueuedUserItem*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole:
            return item->data(index.column());
        default:
            break;
    }

    return QVariant();
}

Qt::ItemFlags QueuedUsersModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QueuedUsersModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex QueuedUsersModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    QueuedUserItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QueuedUserItem*>(parent.internalPointer());

    QueuedUserItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex QueuedUsersModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    QueuedUserItem *childItem = static_cast<QueuedUserItem*>(index.internalPointer());
    QueuedUserItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int QueuedUsersModel::rowCount(const QModelIndex &parent) const
{
    QueuedUserItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QueuedUserItem*>(parent.internalPointer());

    return parentItem->childCount();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<QueuedUserItem*>& items) {
#ifdef _DEBUG_MODEL_
        qDebug() << "Sorting by " << col << " column and " << WulforUtil::getInstance()->sortOrderToInt(order) << " order.";
#endif
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    QList<QueuedUserItem*>::iterator static insertSorted(int col, QList<QueuedUserItem*>& items, QueuedUserItem* item) {
        return qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
    }

    private:
        typedef bool (*AttrComp)(const QueuedUserItem * l, const QueuedUserItem * r);
        AttrComp static getAttrComp(const int column) {
            static AttrComp attrs[2] = {    AttrCmp<0>,
                                            AttrCmp<1>
                                       };

            return attrs[column];//column number checked in SearchModel::sort
        }
        template <int i>
        bool static AttrCmp(const QueuedUserItem * l, const QueuedUserItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <typename T, T (QueuedUserItem::*attr)>
        bool static AttrCmp(const QueuedUserItem * l, const QueuedUserItem * r) {
            return Cmp(l->*attr, r->*attr);
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

void QueuedUsersModel::sort(int column, Qt::SortOrder order) {
    static Compare<Qt::AscendingOrder>  acomp = Compare<Qt::AscendingOrder>();
    static Compare<Qt::DescendingOrder> dcomp = Compare<Qt::DescendingOrder>();
    static int sortColumn = 0;
    static Qt::SortOrder sortOrder = Qt::AscendingOrder;

    sortColumn = (column < 0 || column > 1)? sortColumn : column;
    sortOrder = (column < 0 || column > 1)? sortOrder : order;

    emit layoutAboutToBeChanged();

    if (sortOrder == Qt::AscendingOrder)
        acomp.sort(sortColumn, rootItem->childItems);
    else if (sortOrder == Qt::DescendingOrder)
        dcomp.sort(sortColumn, rootItem->childItems);

    emit layoutChanged();
}

void QueuedUsersModel::addResult(const VarMap &map)
{
    if (!map.contains("CID"))
        return;

    const QString &cid  = map["CID"].toString();
    const QString &file = map["FILE"].toString();
    const QString &hub  = map["HUB"].toString();
    QHash<QString, QueuedUserItem*>::iterator it = cids.find(cid);
    QueuedUserItem *parentItem = NULL;

    if (it != cids.end())
        parentItem = it.value();
    else {
        parentItem = new QueuedUserItem(QList<QVariant>() << WulforUtil::getInstance()->getNicks(cid) << "", rootItem);
        parentItem->cid     = cid;
        parentItem->file    = file;
        parentItem->hub     = hub;

        rootItem->appendChild(parentItem);

        cids.insert(cid, parentItem);
    }

    if (!parentItem)
        return;

    QueuedUserItem *item = new QueuedUserItem(QList<QVariant>() << "" << file, parentItem);
    item->cid     = cid;
    item->file    = file;
    item->hub     = hub;

    parentItem->appendChild(item);

    sort();
}

void QueuedUsersModel::remResult(const VarMap &map){
    if (!map.contains("CID"))
        return;

    const QString &cid  = map["CID"].toString();
    QHash<QString, QueuedUserItem*>::iterator it = cids.find(cid);
    QueuedUserItem *parentItem = NULL;

    if (it != cids.end())
        parentItem = it.value();
    else
        return;

    if (!parentItem)
        return;

    emit layoutChanged();

    parentItem->parent()->childItems.removeAt(parentItem->row());
    cids.remove(cid);

    delete parentItem;

    emit layoutChanged();
}

QueuedUserItem::QueuedUserItem(const QList<QVariant> &data, QueuedUserItem *parent) :
    itemData(data),
    parentItem(parent)
{
}

QueuedUserItem::~QueuedUserItem()
{
    qDeleteAll(childItems);
}

void QueuedUserItem::appendChild(QueuedUserItem *item) {
    childItems.append(item);
}

QueuedUserItem *QueuedUserItem::child(int row) {
    return childItems.value(row);
}

int QueuedUserItem::childCount() const {
    return childItems.count();
}

int QueuedUserItem::columnCount() const {
    return itemData.count();
}

QVariant QueuedUserItem::data(int column) const {
    return itemData.value(column);
}

QueuedUserItem *QueuedUserItem::parent() const{
    return parentItem;
}

int QueuedUserItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<QueuedUserItem*>(this));

    return 0;
}
