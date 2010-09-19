#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WTreeView>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>

#include <map>
#include <boost/any.hpp>

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/QueueManager.h>
#include <dcpp/Singleton.h>
#include "dcpp/ClientManager.h"
#include "dcpp/User.h"

class DownloadQueueModel;

class DownloadQueue:
        public Wt::WContainerWidget,
        public dcpp::QueueManagerListener
{
public:
    DownloadQueue(Wt::WContainerWidget *parent = NULL);
    virtual ~DownloadQueue();

protected:
    void on(dcpp::QueueManagerListener::Added, dcpp::QueueItem *item) throw();
    void on(dcpp::QueueManagerListener::Moved, dcpp::QueueItem *item, const std::string &oldTarget) throw();
    void on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem *item) throw();
    void on(dcpp::QueueManagerListener::SourcesUpdated, dcpp::QueueItem *item) throw();
    void on(dcpp::QueueManagerListener::StatusUpdated, dcpp::QueueItem *item) throw();

private:
    void getParams(std::map<Wt::WString, boost::any> &map, const dcpp::QueueItem *item);

    Wt::WVBoxLayout *vlayout;
    Wt::WContainerWidget *headcontainer;
    Wt::WTreeView *view;
    Wt::WPushButton *pushButton_DELETE;

    DownloadQueueModel *model;
};

#endif // DOWNLOADQUEUE_H
