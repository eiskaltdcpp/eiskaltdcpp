#include "DownloadQueueModel.h"
#include "Utils.h"

#include <stdio.h>

using namespace Wt;

DownloadQueueModel::DownloadQueueModel(WObject *parent) : WAbstractItemModel(parent) {
    std::vector<boost::any> rootData;

    rootData.push_back(boost::any(WString("Name")));
    rootData.push_back(boost::any(WString("Status")));
    rootData.push_back(boost::any(WString("Size")));
    rootData.push_back(boost::any(WString("Downloaded")));
    rootData.push_back(boost::any(WString("Priority")));
    rootData.push_back(boost::any(WString("User")));
    rootData.push_back(boost::any(WString("Path")));
    rootData.push_back(boost::any(WString("Exact size")));
    rootData.push_back(boost::any(WString("Errors")));
    rootData.push_back(boost::any(WString("Added")));
    rootData.push_back(boost::any(WString("TTH")));

    rootItem = new DownloadQueueItem(rootData);

    timer = new WTimer(this);
    timer->setInterval(3000);
    timer->setSingleShot(false);
    timer->timeout().connect(this, &DownloadQueueModel::tick);

    timer->start();
}

DownloadQueueModel::~DownloadQueueModel() {
    delete rootItem;
}

int DownloadQueueModel::rowCount(const Wt::WModelIndex& parent) const{
    DownloadQueueItem *par = NULL;

    if (parent.isValid())
        par = reinterpret_cast<DownloadQueueItem*>(parent.internalPointer());
    else
        par = rootItem;

    if (!par)
        return 0;

    return par->childCount();
}

int DownloadQueueModel::columnCount(const WModelIndex& parent) const{
    return 11;
}

WModelIndex DownloadQueueModel::parent(const WModelIndex  &index) const{
    if (!index.isValid())
        return WModelIndex();

    DownloadQueueItem *childItem = reinterpret_cast<DownloadQueueItem*>(index.internalPointer());
    DownloadQueueItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return WModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

boost::any DownloadQueueModel::data(const WModelIndex  &index, int role) const{
    if (!index.isValid())
        return boost::any();

    DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(index.internalPointer());
    const int column = index.column();

    if (role == DisplayRole){
        return item->data(column);
    }
    else if (role == DecorationRole){
        if (column == 0){
            return boost::any();
        }
    }

    return boost::any();
}

boost::any DownloadQueueModel::headerData(int section, Orientation  orientation, int role) const {
    if (orientation == Horizontal && role == DisplayRole)
        return rootItem->data(section);

    return boost::any();
}

WModelIndex DownloadQueueModel::index(int row, int column, const WModelIndex  &parent) const{
    if (!hasIndex(row, column, parent))
        return WModelIndex();

    DownloadQueueItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DownloadQueueItem*>(parent.internalPointer());

    DownloadQueueItem *childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return WModelIndex();
}

/*namespace {

    template <SortOrder order>
    struct Compare {

        void static sort(int col, std::vector<DownloadQueueItem*>& items) {
            std::sort(items.begin(), items.end(), getAttrComp(col));
        }

    private:
        typedef bool (*AttrComp)(const DownloadQueueItem * l, const DownloadQueueItem * r);

        AttrComp static getAttrComp(const int column) {
            static AttrComp attrs[6] = { AttrCmp<WString, &DownloadQueueItem::file>,
                                         AttrCmp<WString, &DownloadQueueItem::file>,
                                         AttrCmp<unsigned long, &DownloadQueueItem::size>,
                                         AttrCmp<WString, &DownloadQueueItem::path>,
                                         AttrCmp<WString, &DownloadQueueItem::tth>,
                                         AttrCmp<WString, &DownloadQueueItem::host>};

            return attrs[column]; //column number checked in DownloadQueueModel::sort
        }

        template <typename T, T(DownloadQueueItem::*attr)>
        bool static AttrCmp(const DownloadQueueItem * l, const DownloadQueueItem * r) {
            return Cmp(l->*attr, r->*attr);
        }

        template <typename T>
                bool static Cmp(const T& l, const T & r);
    };

    template <> template <typename T>
    bool inline Compare<AscendingOrder>::Cmp(const T& l, const T& r) {
        return l < r;
    }

    template <> template <typename T>
    bool inline Compare<DescendingOrder>::Cmp(const T& l, const T& r) {
        return l > r;
    }

} //namespace*/

void DownloadQueueModel::sort(int column, SortOrder order){
    layoutChanged().emit();

    /*if (order == DescendingOrder)
        Compare<DescendingOrder>().sort(column, rootItem->childs);
    else
        Compare<AscendingOrder>().sort(column, rootItem->childs);*/

    layoutChanged().emit();
}

bool DownloadQueueModel::setData(const WModelIndex &index, const boost::any &value, int role){
    if (role != CheckStateRole || index.column() != 0)
        return WAbstractItemModel::setData(index, value, role);

    /*DownloadQueueItem *item = reinterpret_cast<DownloadQueueItem*>(index.internalPointer());
    std::vector<DownloadQueueItem*>::iterator begin = checkedItems.begin();
    std::vector<DownloadQueueItem*>::iterator end   = checkedItems.end();
    std::vector<DownloadQueueItem*>::iterator it = std::find(begin, end, item);

    bool checked = (it != checkedItems.end());

    if (checked){
        std::remove(it, it, *it);
    }
    else {
        checkedItems.push_back(item);
    }*/

    return true;
}

/*WFlags<ItemFlag> DownloadQueueModel::flags(const WModelIndex &index) const{
    WFlags<ItemFlag> _flags = WAbstractItemModel::flags(index);

    if (index.column() == 0)
        _flags |= ItemIsUserCheckable;

    return _flags;
}*/

void DownloadQueueModel::addFile(std::map<Wt::WString, boost::any> &map){
    DownloadQueueItem *child = NULL;
    std::vector<boost::any> childData;

    childData.push_back(map["FNAME"]);
    childData.push_back(map["STATUS"]);
    childData.push_back(map["ESIZE"]);
    childData.push_back(map["DOWN"]);
    childData.push_back(map["PRIO"]);
    childData.push_back(map["USERS"]);
    childData.push_back(map["PATH"]);
    childData.push_back(map["ESIZE"]);
    childData.push_back(map["ERRORS"]);
    childData.push_back(map["ADDED"]);
    childData.push_back(map["TTH"]);

    child = new DownloadQueueItem(childData, rootItem);
    rootItem->appendChild(child);

    items[boost::any_cast<WString>(map["TARGET"])] = child;
}

void DownloadQueueModel::updFile(std::map<Wt::WString, boost::any> &params){
    std::map<Wt::WString, DownloadQueueItem* >::iterator it = items.find(boost::any_cast<WString>(params["TARGET"]));

    if (it == items.end())
        return;

    DownloadQueueItem *item = it->second;

    item->updateColumn(COLUMN_DOWNLOADQUEUE_STATUS, params["STATUS"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_DOWN,  boost::any_cast<long long>(params["DOWN"]) > 0? params["DOWN"] : 0);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_ESIZE, boost::any_cast<long long>(params["ESIZE"]) > 0? params["ESIZE"] : 0);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_SIZE,  boost::any_cast<long long>(params["ESIZE"]) > 0? params["ESIZE"] : 0);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_PRIO, params["PRIO"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_USER, params["USERS"]);
    item->updateColumn(COLUMN_DOWNLOADQUEUE_ERR, params["ERRORS"]);
}

void DownloadQueueModel::remFile(std::map<Wt::WString, boost::any> &params){
    std::map<Wt::WString, DownloadQueueItem* >::iterator it = items.find(boost::any_cast<WString>(params["TARGET"]));

    if (it == items.end())
        return;

    DownloadQueueItem *item = it->second;

    rootItem->childItems.erase(std::find(rootItem->childItems.begin(), rootItem->childItems.end(), item));
    items.erase(it);

    delete item;
}

void DownloadQueueModel::tick(){
    layoutChanged().emit();
}

DownloadQueueItem::DownloadQueueItem(const std::vector<boost::any> &data, DownloadQueueItem *parent) :
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
    for (long i = 0; i < childItems.size(); i++)
        delete const_cast<DownloadQueueItem*>(childItems.at(i));
}

void DownloadQueueItem::appendChild(DownloadQueueItem *item) {
    childItems.push_back(item);

    item->parentItem = this;
}

DownloadQueueItem *DownloadQueueItem::child(int row) {
    if (row >= childItems.size())
        return NULL;

    return childItems.at(row);
}

int DownloadQueueItem::childCount() const {
    return childItems.size();
}

int DownloadQueueItem::columnCount() const {
    return itemData.size();
}

boost::any DownloadQueueItem::data(int column) const {
    return itemData.at(column);
}

DownloadQueueItem *DownloadQueueItem::parent() {
    return parentItem;
}

int DownloadQueueItem::row() const {
    if (parentItem){
        std::vector<DownloadQueueItem*>::iterator begin = parentItem->childItems.begin();
        std::vector<DownloadQueueItem*>::iterator end   = parentItem->childItems.end();

        int index = std::distance(begin, std::find(end, begin, this));

        return index;
    }
}

void DownloadQueueItem::updateColumn(int column, boost::any var){
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
