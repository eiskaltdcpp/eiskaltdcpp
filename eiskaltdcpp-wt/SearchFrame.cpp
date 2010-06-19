#include "SearchFrame.h"

#include <stdio.h>

using namespace Wt;
using namespace dcpp;

SearchFrame::SearchFrame(Wt::WContainerWidget *parent): WContainerWidget(parent) {
    resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));
    
    vlayout = new WVBoxLayout();
    setLayout(vlayout);

    hlayout = new WHBoxLayout();

    vlayout->addLayout(hlayout);

    {//Header
        container = new WContainerWidget();
        container->setStyleClass("search-control-container");

        label1 = new WLabel("File type");
        label1->setStyleClass("search-label");

        comboBox_TYPE = new WComboBox();
        comboBox_TYPE->setStyleClass("combobox-types");

        comboBox_TYPE->addItem("Any");
        comboBox_TYPE->addItem("Audio");
        comboBox_TYPE->addItem("Archive");
        comboBox_TYPE->addItem("Document");
        comboBox_TYPE->addItem("Executable");
        comboBox_TYPE->addItem("Image");
        comboBox_TYPE->addItem("Video");
        comboBox_TYPE->addItem("Directory");
        comboBox_TYPE->addItem("TTH");

        comboBox_TYPE->setReadOnly(true);
        comboBox_TYPE->resize(WLength::Auto, label1->lineHeight());

        lineEdit_SEARCH = new WLineEdit();
        lineEdit_SEARCH->setStyleClass("search-edit");
        lineEdit_SEARCH->setEmptyText("Search for...");
        lineEdit_SEARCH->enterPressed().connect(this, &SearchFrame::startSearch);
        lineEdit_SEARCH->enterPressed().preventDefaultAction(true);

        pushButton_SEARCH = new WPushButton("Search");
        pushButton_SEARCH->setStyleClass("search-button");
        pushButton_SEARCH->clicked().connect(this, &SearchFrame::startSearch);

        pushButton_DOWNLOAD = new WPushButton("Download selected");

        container->addWidget(label1);
        container->addWidget(comboBox_TYPE);
        container->addWidget(lineEdit_SEARCH);
        container->addWidget(pushButton_SEARCH);
        container->addWidget(pushButton_DOWNLOAD);

        hlayout->addWidget(container, 1, AlignLeft);
    }
    {//Body
        view = new WTreeView(this);
        view->setModel(model = new SearchModel(view));
        view->setAlternatingRowColors(true);
        view->resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));

        vlayout->addWidget(view, 5);

        view->setColumnWidth(0, WLength(30, WLength::Pixel));
    }

    SearchManager::getInstance()->addListener(this);
}

SearchFrame::SearchFrame(const SearchFrame& orig) {
}

SearchFrame::~SearchFrame() {
    /*delete container;
    delete vlayout;
    delete hlayout;
    delete label1;
    delete comboBox_TYPE;
    delete lineEdit_SEARCH;
    delete pushButton_SEARCH;
    delete view;

    delete model;*/
}

void SearchFrame::startSearch() {
    if (lineEdit_SEARCH->text().empty())
        return;

    StringList clients;

    ClientManager* clientMgr = ClientManager::getInstance();

    clientMgr->lock();
    Client::List& _clients = clientMgr->getClients();

    for(Client::List::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        Client* client = *it;

        if(!client->isConnected())
            continue;

       printf("%s\n", client->getHubUrl().c_str());
       clients.push_back(client->getHubUrl());
    }

    clientMgr->unlock();

    if (clients.empty())
        return;

    WString s = lineEdit_SEARCH->text();

    {
        currentSearch = StringTokenizer<tstring>(s.toUTF8(), ' ').getTokens();
        s = "";

        //strip out terms beginning with -
        for(TStringList::iterator si = currentSearch.begin(); si != currentSearch.end(); ) {
            if(si->empty()) {
                si = currentSearch.erase(si);
                continue;
            }

            if ((*si)[0] != '-')
                s += (WString::fromUTF8(*si) + " ");

            ++si;
        }

        token = (Util::toString(Util::rand()));
    }

    SearchManager::SizeModes searchMode = SearchManager::SIZE_DONTCARE;
    int ftype = comboBox_TYPE->currentIndex();

    if(SearchManager::getInstance()->okToSearch()) {
        printf("Now Search: %s!\n", s.toUTF8().c_str());
        SearchManager::getInstance()->search(clients, s.toUTF8(), 0, (SearchManager::TypeModes)ftype,
                                             searchMode, token);

    }
}

void SearchFrame::on(SearchManagerListener::SR, const SearchResultPtr& aResult) throw(){
    SearchModelItem *item = new SearchModelItem();

    WString s =  WString::fromUTF8(aResult->getFileName(), false);
    WString fname = "";
    WString path = "";
    TStringList list;

    {
        list = StringTokenizer<tstring>(s.toUTF8(), '\\').getTokens();
        fname = WString::fromUTF8(list.at(list.size()-1));

        //strip out terms beginning with -
        for(TStringList::iterator si = list.begin(); si != (--list.end()); ) {
            path += (WString::fromUTF8(*si) + "\\");

            ++si;
        }

        token = (Util::toString(Util::rand()));
    }

    item->file = fname;
    item->path = path;
    item->size = aResult->getSize();
    item->tth  = WString::fromUTF8(aResult->getTTH().toBase32(), false);
    item->cid  = WString::fromUTF8(aResult->getUser()->getCID().toBase32(), false);
    item->host = WString::fromUTF8(aResult->getHubURL(), false);

    model->addResult(item);
}

