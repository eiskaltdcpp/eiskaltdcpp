#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WTreeView>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>

class DownloadQueue:
        public Wt::WContainerWidget
{
public:
    DownloadQueue(Wt::WContainerWidget *parent = NULL);
    virtual ~DownloadQueue();

private:
    Wt::WVBoxLayout *vlayout;
    Wt::WContainerWidget *headcontainer;
    Wt::WTreeView *view;
    Wt::WPushButton *pushButton_DELETE;
};

#endif // DOWNLOADQUEUE_H
