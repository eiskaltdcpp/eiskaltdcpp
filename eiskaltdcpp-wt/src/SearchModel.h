/* 
 * File:   SearchModel.h
 * Author: negativ
 *
 * Created on 17 Июнь 2010 г., 17:22
 */

#ifndef _WMYMODEL_H
#define	_WMYMODEL_H

#include <Wt/WObject>
#include <Wt/WAbstractItemModel>
#include <Wt/WModelIndex>
#include <Wt/WString>
#include <Wt/WTimer>

#include <boost/any.hpp>

class SearchModelItem{
public:
    SearchModelItem() : parentItem(NULL), size(0) {

    }

    virtual ~SearchModelItem(){
        for (long i = 0; i < childs.size(); i++)
            delete const_cast<SearchModelItem*>(childs.at(i));
    }

    void append(SearchModelItem *item){
        if (item){
            item->parentItem = this;
            
            childs.push_back(item);
        }
    }

    SearchModelItem *at(const long index) {
        if (index >= childs.size())
            return NULL;

        return childs.at(index);
    }

    SearchModelItem *parent() {
        return parentItem;
    }

    int row() const {
        if (parentItem){
            std::vector<SearchModelItem*>::iterator begin = parentItem->childs.begin();
            std::vector<SearchModelItem*>::iterator end   = parentItem->childs.end();


            int index = std::distance(begin, std::find(end, begin, this));

            return index;
        }
    }

    long count() const {
        return childs.size();
    }

    std::vector<SearchModelItem*> childs;

    Wt::WString file;
    Wt::WString path;
    Wt::WString tth;
    Wt::WString cid;
    Wt::WString host;
    unsigned long size;

private:

    SearchModelItem *parentItem;
};

class SearchModel : public Wt::WAbstractItemModel {
public:
    SearchModel(Wt::WObject *parent = NULL);
    virtual ~SearchModel();

    virtual int columnCount(const Wt::WModelIndex &parent = Wt::WModelIndex()) const;
    virtual int rowCount (const Wt::WModelIndex  &parent = Wt::WModelIndex()) const;
    virtual Wt::WModelIndex parent(const Wt::WModelIndex  &index) const;
    virtual boost::any data(const Wt::WModelIndex  &index, int role = Wt::DisplayRole) const;
    virtual boost::any headerData(int section, Wt::Orientation  orientation = Wt::Horizontal, int role = Wt::DisplayRole) const;
    virtual Wt::WModelIndex index(int row, int column, const Wt::WModelIndex  &parent = Wt::WModelIndex()) const;
    virtual void sort(int column, Wt::SortOrder order = Wt::AscendingOrder);
    virtual bool setData(const Wt::WModelIndex &index, const boost::any &value, int role);
    Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const;

    virtual void addResult(SearchModelItem *item);
    std::vector<SearchModelItem*> getCheckedItems(){ return checkedItems; }
    
private:
    SearchModel(const SearchModel& orig){}
    SearchModel& operator =(const SearchModel&){}

    void tick();

    SearchModelItem *rootItem;

    std::vector<SearchModelItem*> checkedItems;

    Wt::WTimer *timer;
};

#endif	/* _WMYMODEL_H */

