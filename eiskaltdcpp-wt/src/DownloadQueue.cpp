#include "DownloadQueue.h"
#include "DownloadQueueModel.h"

using namespace Wt;

DownloadQueue::DownloadQueue(WContainerWidget *parent): WContainerWidget(parent)
{
    resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));

    vlayout = new WVBoxLayout();

    setLayout(vlayout);

    {//Head
        WHBoxLayout *hlayout = new WHBoxLayout();

        headcontainer = new WContainerWidget();
        headcontainer->setStyleClass("search-control-container");

        pushButton_DELETE = new WPushButton("Remove selected", headcontainer);

        hlayout->addWidget(headcontainer, 1, AlignRight);
        vlayout->addLayout(hlayout);
    }
    {//Body
        view = new WTreeView(this);
        view->setAlternatingRowColors(true);
        view->setSortingEnabled(true);
        view->setModel(new DownloadQueueModel());

        vlayout->addWidget(view, 5);

        view->resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));
        view->setColumnWidth(0, WLength(30, WLength::Pixel));
    }
}

DownloadQueue::~DownloadQueue(){

}
