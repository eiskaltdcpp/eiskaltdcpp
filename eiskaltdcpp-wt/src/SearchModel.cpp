/* 
 * File:   SearchModel.cpp
 * Author: negativ
 * 
 * Created on 17 Июнь 2010 г., 17:22
 */

#include "SearchModel.h"
#include "Utils.h"

#include <stdio.h>

using namespace Wt;

SearchModel::SearchModel(WObject *parent) : WAbstractItemModel(parent) {
    rootItem = new SearchModelItem();

    timer = new WTimer(this);
    timer->setInterval(3000);
    timer->setSingleShot(false);
    timer->timeout().connect(this, &SearchModel::tick);

    timer->start();
}

SearchModel::~SearchModel() {
    delete rootItem;
}

int SearchModel::rowCount(const Wt::WModelIndex& parent) const{
    SearchModelItem *par = NULL;

    if (parent.isValid())
        par = reinterpret_cast<SearchModelItem*>(parent.internalPointer());
    else
        par = rootItem;

    if (!par)
        return 0;

    return par->count();
}

int SearchModel::columnCount(const WModelIndex& parent) const{
    return 6;
}

WModelIndex SearchModel::parent(const WModelIndex  &index) const{
    if (!index.isValid())
        return WModelIndex();

    SearchModelItem *childItem = reinterpret_cast<SearchModelItem*>(index.internalPointer());
    SearchModelItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return WModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

boost::any SearchModel::data(const WModelIndex  &index, int role) const{
    if (!index.isValid())
        return boost::any();

    SearchModelItem *item = reinterpret_cast<SearchModelItem*>(index.internalPointer());
    const int column = index.column();

    if (role == DisplayRole){
        if (column == 1)
            return boost::any(item->file);
        else if (column == 2)
            return boost::any(Utils::formatBytes(item->size));
        else if (column == 3)
            return boost::any(item->path);
        else if (column == 4)
            return boost::any(item->tth);
        else if (column == 5)
            return boost::any(item->host);
    }
    else if (role == CheckStateRole){
        std::vector<SearchModelItem*>::const_iterator begin = checkedItems.begin();
        std::vector<SearchModelItem*>::const_iterator end   = checkedItems.end();

        if (column == 0)
            return ((std::find(begin, end, item) == checkedItems.end())? Unchecked : Checked);
    }
    else if (role == DecorationRole){
        if (column == 1){
            WString img = Utils::getFileImage(item->file).toUTF8();

            return boost::any(img);
        }
    }

    return boost::any();
}

boost::any SearchModel::headerData(int section, Orientation  orientation, int role) const {
    if (orientation == Horizontal && role == DisplayRole){
        if (section == 1)
            return boost::any(WString("File"));
        else if (section == 2)
            return boost::any(WString("Size"));
        else if (section == 3)
            return boost::any(WString("Path"));
        else if (section == 4)
            return boost::any(WString("TTH"));
        else if (section == 5)
            return boost::any(WString("Host"));
        else
            return boost::any(WString(""));
    }

    return boost::any();
}

WModelIndex SearchModel::index(int row, int column, const WModelIndex  &parent) const{
    if (!hasIndex(row, column, parent))
        return WModelIndex();

    SearchModelItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SearchModelItem*>(parent.internalPointer());

    SearchModelItem *childItem = parentItem->at(row);
    
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return WModelIndex();
}

namespace {

    template <SortOrder order>
    struct Compare {

        void static sort(int col, std::vector<SearchModelItem*>& items) {
            std::sort(items.begin(), items.end(), getAttrComp(col));
        }

    private:
        typedef bool (*AttrComp)(const SearchModelItem * l, const SearchModelItem * r);

        AttrComp static getAttrComp(const int column) {
            static AttrComp attrs[6] = { AttrCmp<WString, &SearchModelItem::file>,
                                         AttrCmp<WString, &SearchModelItem::file>,
                                         AttrCmp<unsigned long, &SearchModelItem::size>,
                                         AttrCmp<WString, &SearchModelItem::path>,
                                         AttrCmp<WString, &SearchModelItem::tth>,
                                         AttrCmp<WString, &SearchModelItem::host>};

            return attrs[column]; //column number checked in SearchModel::sort
        }

        template <typename T, T(SearchModelItem::*attr)>
        bool static AttrCmp(const SearchModelItem * l, const SearchModelItem * r) {
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

} //namespace

void SearchModel::sort(int column, SortOrder order){
    layoutChanged().emit();
    
    if (order == DescendingOrder)
        Compare<DescendingOrder>().sort(column, rootItem->childs);
    else
        Compare<AscendingOrder>().sort(column, rootItem->childs);

    layoutChanged().emit();
}

bool SearchModel::setData(const WModelIndex &index, const boost::any &value, int role){
    if (role != CheckStateRole || index.column() != 0)
        return WAbstractItemModel::setData(index, value, role);

    SearchModelItem *item = reinterpret_cast<SearchModelItem*>(index.internalPointer());
    std::vector<SearchModelItem*>::iterator begin = checkedItems.begin();
    std::vector<SearchModelItem*>::iterator end   = checkedItems.end();
    std::vector<SearchModelItem*>::iterator it = std::find(begin, end, item);

    bool checked = (it != checkedItems.end());

    if (checked){
        std::remove(it, it, *it);
    }
    else {
        checkedItems.push_back(item);
    }

    return true;
}

WFlags<ItemFlag> SearchModel::flags(const WModelIndex &index) const{
    WFlags<ItemFlag> _flags = WAbstractItemModel::flags(index);

    if (index.column() == 0)
        _flags |= ItemIsUserCheckable;

    return _flags;
}

void SearchModel::addResult(SearchModelItem *item){
    if (item)
        rootItem->append(item);
}

void SearchModel::tick() {
    layoutChanged().emit();
}
