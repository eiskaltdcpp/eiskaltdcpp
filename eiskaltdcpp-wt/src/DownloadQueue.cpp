#include "DownloadQueue.h"
#include "DownloadQueueModel.h"
#include "Utils.h"

using namespace Wt;
using namespace dcpp;

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
        model = new DownloadQueueModel();

        view = new WTreeView(this);
        view->setAlternatingRowColors(true);
        view->setSortingEnabled(true);
        view->setModel(model);

        vlayout->addWidget(view, 5);

        view->resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));
        view->setColumnWidth(0, WLength(30, WLength::Pixel));
    }

    std::map<Wt::WString, boost::any> params;

    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

    for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it){
        getParams(params, it->second);

        model->addFile(params);
    }

    QueueManager::getInstance()->unlockQueue();

    QueueManager::getInstance()->addListener(this);
}

DownloadQueue::~DownloadQueue(){
    QueueManager::getInstance()->removeListener(this);
}

void DownloadQueue::getParams(std::map<Wt::WString, boost::any> &params, const dcpp::QueueItem *item){
    WString nick = "";
    /*QMap<QString, QString> source;*/
    int online = 0;

    if (!item)
        return;

    params["FNAME"]     = _q(item->getTargetFileName());
    params["PATH"]      = _q(Util::getFilePath(item->getTarget()));
    params["TARGET"]    = _q(item->getTarget());

    params["USERS"] = boost::any(WString(""));

    QueueItem::SourceConstIter it = item->getSources().begin();

    for (; it != item->getSources().end(); ++it){
        UserPtr usr = it->getUser();

        if (usr->isOnline())
            ++online;

        if (!boost::any_cast<WString>(params["USERS"]).empty())
            params["USERS"] = boost::any_cast<WString>(params["USERS"]) +", ";

        nick = Utils::getNicks(usr->getCID());

        params["USERS"] = boost::any_cast<WString>(params["USERS"]) + nick;
    }

    if (boost::any_cast<WString>(params["USERS"]).empty())
        params["USERS"] = WString("No users...");


    if (item->isWaiting())
        params["STATUS"] = WString("{1} of {2} user(s) online").arg(online).arg((int)item->getSources().size());
    else
        params["STATUS"] = WString("Running...");

    params["ESIZE"] = (long long)item->getSize();
    params["DOWN"]  = (long long)item->getDownloadedBytes();
    params["PRIO"]  = static_cast<int>(item->getPriority());
    params["ERRORS"] = WString("");
    params["ADDED"] = _q(Util::formatTime("%Y-%m-%d %H:%M", item->getAdded()));
    params["TTH"] = _q(item->getTTH().toBase32());
}

void DownloadQueue::on(QueueManagerListener::Added, QueueItem *item) throw(){
    std::map<Wt::WString, boost::any> params;
    getParams(params, item);

    model->addFile(params);
}

void DownloadQueue::on(QueueManagerListener::Moved, QueueItem *item, const std::string &oldTarget) throw(){
    std::map<Wt::WString, boost::any> params;
    getParams(params, item);

    model->remFile(params);
    model->addFile(params);
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem *item) throw(){
    std::map<Wt::WString, boost::any> params;
    getParams(params, item);

    model->remFile(params);
}

void DownloadQueue::on(QueueManagerListener::SourcesUpdated, QueueItem *item) throw(){
    std::map<Wt::WString, boost::any> params;
    getParams(params, item);

    model->updFile(params);
}

void DownloadQueue::on(QueueManagerListener::StatusUpdated, QueueItem *item) throw(){
    std::map<Wt::WString, boost::any> params;
    getParams(params, item);

    model->updFile(params);
}

