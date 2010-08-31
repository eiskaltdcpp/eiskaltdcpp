#ifndef DOWNLOADQUEUEMODEL_H
#define DOWNLOADQUEUEMODEL_H

#include <Wt/WObject>
#include <Wt/WAbstractItemModel>
#include <Wt/WModelIndex>
#include <Wt/WString>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"
#include "dcpp/DirectoryListing.h"

#define COLUMN_DOWNLOADQUEUE_NAME       0
#define COLUMN_DOWNLOADQUEUE_STATUS     1
#define COLUMN_DOWNLOADQUEUE_SIZE       2
#define COLUMN_DOWNLOADQUEUE_DOWN       3
#define COLUMN_DOWNLOADQUEUE_PRIO       4
#define COLUMN_DOWNLOADQUEUE_USER       5
#define COLUMN_DOWNLOADQUEUE_PATH       6
#define COLUMN_DOWNLOADQUEUE_ESIZE      7
#define COLUMN_DOWNLOADQUEUE_ERR        8
#define COLUMN_DOWNLOADQUEUE_ADDED      9
#define COLUN_DOWNLOADQUEUE_TTH        10

class DownloadQueueItem
{

public:
    DownloadQueueItem(const std::vector<boost::any> &data, DownloadQueueItem *parent = 0);
    DownloadQueueItem(const DownloadQueueItem&);
    void operator=(const DownloadQueueItem&);
    virtual ~DownloadQueueItem();

    void appendChild(DownloadQueueItem *child);

    DownloadQueueItem *child(int row);
    int childCount() const;
    int columnCount() const;
    boost::any data(int column) const;
    int row() const;
    DownloadQueueItem *parent();
    void updateColumn(int, boost::any);

    std::vector<DownloadQueueItem*> childItems;

    DownloadQueueItem *nextSibling();

    bool dir;
private:

    std::vector<boost::any> itemData;
    DownloadQueueItem *parentItem;
};

class DownloadQueueModel: public Wt::WAbstractItemModel
{
public:
    DownloadQueueModel(Wt::WObject *parent = NULL);
    virtual ~DownloadQueueModel();

    virtual int columnCount(const Wt::WModelIndex &parent = Wt::WModelIndex()) const;
    virtual int rowCount (const Wt::WModelIndex  &parent = Wt::WModelIndex()) const;
    virtual Wt::WModelIndex parent(const Wt::WModelIndex  &index) const;
    virtual boost::any data(const Wt::WModelIndex  &index, int role = Wt::DisplayRole) const;
    virtual boost::any headerData(int section, Wt::Orientation  orientation = Wt::Horizontal, int role = Wt::DisplayRole) const;
    virtual Wt::WModelIndex index(int row, int column, const Wt::WModelIndex  &parent = Wt::WModelIndex()) const;
    virtual void sort(int column, Wt::SortOrder order = Wt::AscendingOrder);
    virtual bool setData(const Wt::WModelIndex &index, const boost::any &value, int role);

private:

    DownloadQueueItem *rootItem;
};

#endif // DOWNLOADQUEUEMODEL_H
