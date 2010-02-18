#include <QComboBox>
#include <QTreeView>
#include <QAction>
#include <QTextCodec>
#include <QDir>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>

#include "SearchFrame.h"
#include "MainWindow.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "SearchModel.h"
#include "WulforUtil.h"
#include "Func.h"

#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/StringTokenizer.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Encoder.h"

#include <QtDebug>

using namespace dcpp;

SearchFrame::Menu::Menu(){
    WulforUtil *WU = WulforUtil::getInstance();

    menu = new QMenu();

    QAction *down       = new QAction(tr("Download"), menu);
    down->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));

    QAction *down_to    = new QAction(tr("Download to"), menu);
    down_to->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD_AS));

    QAction *down_wh    = new QAction(tr("Download Whole Directory"), menu);
    down_wh->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));

    QAction *down_wh_to = new QAction(tr("Download Whole Directory to"), menu);
    down_wh_to->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD_AS));

    QAction *sep        = new QAction(menu);
    sep->setSeparator(true);

    QAction *find_tth   = new QAction(tr("Search TTH"), menu);
    find_tth->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));

    QAction *magnet     = new QAction(tr("Copy magnet"), menu);
    magnet->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));

    QAction *browse     = new QAction(tr("Browse files"), menu);
    browse->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE_OPEN));

    QAction *match      = new QAction(tr("Match Queue"), menu);

    QAction *send_pm    = new QAction(tr("Send Private Message"), menu);
    send_pm->setIcon(WU->getPixmap(WulforUtil::eiMESSAGE));

    QAction *add_to_fav = new QAction(tr("Add to favorites"), menu);
    add_to_fav->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));

    QAction *grant      = new QAction(tr("Grant extra slot"), menu);

    QAction *sep1       = new QAction(menu);
    sep1->setSeparator(true);

    QAction *rem_queue  = new QAction(tr("Remove from Queue"), menu);
    rem_queue->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    QAction *rem        = new QAction(tr("Remove"), menu);
    rem->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    actions.insert(down, Download);
    actions.insert(down_to, DownloadTo);
    actions.insert(down_wh, DownloadWholeDir);
    actions.insert(down_wh_to, DownloadWholeDirTo);
    actions.insert(find_tth, SearchTTH);
    actions.insert(magnet, Magnet);
    actions.insert(browse, Browse);
    actions.insert(match, MatchQueue);
    actions.insert(send_pm, SendPM);
    actions.insert(add_to_fav, AddToFav);
    actions.insert(grant, GrantExtraSlot);
    actions.insert(rem_queue, RemoveFromQueue);
    actions.insert(rem, Remove);

    menu->addActions(QList<QAction*>() << down
                                       << down_to
                                       << down_wh
                                       << down_wh_to
                                       << sep
                                       << find_tth
                                       << magnet
                                       << browse
                                       << match
                                       << send_pm
                                       << add_to_fav
                                       << grant
                                       << sep1
                                       << rem_queue
                                       << rem
                                       );
}

SearchFrame::Menu::~Menu(){
    delete menu;
}

SearchFrame::Menu::Action SearchFrame::Menu::exec(){
    QAction *ret = menu->exec(QCursor::pos());

    if (actions.contains(ret))
        return actions.value(ret);
    else
        return None;
}

SearchFrame::SearchFrame(QWidget *parent):
        arena_title(tr("Search window")),
        QWidget(parent),
        isHash(false),
        arena_menu(NULL),
        timer(NULL),
        dropped(0L),
        results(0L),
        filterShared(SearchFrame::None),
        withFreeSlots(false),
        timer1(NULL)
{
    setupUi(this);

    ClientManager* clientMgr = ClientManager::getInstance();

    clientMgr->lock();
    clientMgr->addListener(this);
    Client::List& clients = clientMgr->getClients();

    for(Client::List::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client* client = *it;

        if(!client->isConnected())
            continue;

        onHubAdded(new HubInfo(client, listWidget_HUBS));
    }

    clientMgr->unlock();

    SearchManager::getInstance()->addListener(this);

    init();
}

SearchFrame::~SearchFrame(){
    Menu::deleteInstance();

    SearchManager::getInstance()->removeListener(this);
    ClientManager::getInstance()->removeListener(this);

    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);

    delete model;
    delete arena_menu;
    delete timer;
}

void SearchFrame::closeEvent(QCloseEvent *e){
    save();

    setAttribute(Qt::WA_DeleteOnClose);

    e->accept();
}

void SearchFrame::customEvent(QEvent *e){
    if (e->type() == SearchCustomEvent::Event){
        SearchCustomEvent *u_e = reinterpret_cast<SearchCustomEvent*>(e);

        u_e->func()->call();
    }

    e->accept();
}

bool SearchFrame::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);
        int key = k_e->key();

        if (static_cast<QComboBox*>(obj) == comboBox_SEARCHSTR && (key == Qt::Key_Enter || key == Qt::Key_Return))
            slotStartSearch();

    }

    return QWidget::eventFilter(obj, e);
}

void SearchFrame::init(){
    left_pane_old_size = 0;

    timer1 = new QTimer(this);
    timer1->setInterval(1000);

    model = new SearchModel(NULL);

    treeView_RESULTS->setModel(model);
    treeView_RESULTS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RESULTS->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    arena_menu = new QMenu(this->windowTitle());
    QAction *close_wnd = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));
    connect(pushButton_SEARCH, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
    connect(pushButton_CLEAR, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(treeView_RESULTS, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotResultDoubleClicked(QModelIndex)));
    connect(treeView_RESULTS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(treeView_RESULTS->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(timer1, SIGNAL(timeout()), this, SLOT(slotTimer()));
    connect(pushButton_SIDEPANEL, SIGNAL(clicked()), this, SLOT(slotToggleSidePanel()));

    MainWindow *mwnd = MainWindow::getInstance();

    comboBox_SEARCHSTR->installEventFilter(this);
    comboBox_SEARCHSTR->setFocus();

    load();

    mwnd->addArenaWidget(this);
    mwnd->addArenaWidgetOnToolbar(this);
    mwnd->mapWidgetOnArena(this);

    timer1->start();
}

void SearchFrame::load(){
    treeView_RESULTS->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SEARCH_STATE).toAscii()));

    filterShared = static_cast<SearchFrame::AlreadySharedAction>(WIGET(WI_SEARCH_SHARED_ACTION));

    if (filterShared == SearchFrame::Filter)
        radioButton_SHAREDHIDE->setChecked(true);
    else if (filterShared == SearchFrame::Highlight)
        radioButton_SHAREDHIGHLIGHT->setChecked(true);

    checkBox_FILTERSLOTS->setChecked(WBGET(WB_SEARCHFILTER_NOFREE));

    treeView_RESULTS->sortByColumn(WIGET(WI_SEARCH_SORT_COLUMN), WulforUtil::getInstance()->intToSortOrder(WIGET(WI_SEARCH_SORT_ORDER)));
}

void SearchFrame::save(){
    WSSET(WS_SEARCH_STATE, treeView_RESULTS->header()->saveState().toBase64());
    WISET(WI_SEARCH_SORT_COLUMN, model->getSortColumn());
    WISET(WI_SEARCH_SORT_ORDER, WulforUtil::getInstance()->sortOrderToInt(model->getSortOrder()));
    WISET(WI_SEARCH_SHARED_ACTION, static_cast<int>(filterShared));
    WBSET(WB_SEARCHFILTER_NOFREE, checkBox_FILTERSLOTS->isChecked());
}

void SearchFrame::initSecond(){
    if (!timer){
        timer = new QTimer(this);
        timer->setInterval(1000);
        timer->setSingleShot(true);

        connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    }

    timer->start();
}

QWidget *SearchFrame::getWidget(){
    return this;
}

QString SearchFrame::getArenaTitle(){
    return arena_title;
}

QMenu *SearchFrame::getMenu(){
    return arena_menu;
}

void SearchFrame::download(const SearchFrame::VarMap &params){
    string target, cid, filename, hubUrl;
    int64_t size;

    target      = params["TARGET"].toString().toStdString();
    cid         = params["CID"].toString().toStdString();
    filename    = params["FNAME"].toString().toStdString();
    hubUrl      = params["HOST"].toString().toStdString();
    size        = (int64_t)params["ESIZE"].toLongLong();

    try{
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

        if (!user)
            return;
        // Only files have a TTH
        if (!params["TTH"].toString().isEmpty()){
            string subdir = params["FNAME"].toString().split("\\", QString::SkipEmptyParts).last().toStdString();
            QueueManager::getInstance()->add(target + subdir, size, TTHValue(params["TTH"].toString().toStdString()), user, hubUrl);
        }
        else{
            printf("%s %s %s\n", filename.c_str(), hubUrl.c_str(), target.c_str());
            QueueManager::getInstance()->addDirectory(filename, user, hubUrl, target);
        }
    }
    catch (const Exception&){}
}

void SearchFrame::getFileList(const VarMap &params, bool match){
    string cid  = params["CID"].toString().toStdString();
    string dir  = params["FNAME"].toString().toStdString();
    string host = params["HOST"].toString().toStdString();

    if (cid.empty())
        return;

    try {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

        if (user){
            QueueItem::FileFlags flag = match? QueueItem::FLAG_MATCH_QUEUE : QueueItem::FLAG_CLIENT_VIEW;

            QueueManager::getInstance()->addList(user, host, flag, dir);
        }
    }
    catch (const Exception&){}

}

void SearchFrame::addToFav(const QString &cid){
    if (!cid.isEmpty()){
        try {
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

            if (user)
                FavoriteManager::getInstance()->addFavoriteUser(user);
        }
        catch (const Exception&){}
    }
}

void SearchFrame::grant(const VarMap &params){
    string cid  = params["CID"].toString().toStdString();
    string host = params["HOST"].toString().toStdString();

    if (cid.empty())
        return;

    try {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

        if (user)
            UploadManager::getInstance()->reserveSlot(user, host);
    }
    catch (const Exception&){}
}

void SearchFrame::removeSource(const VarMap &params){
    string cid  = params["CID"].toString().toStdString();

    if (cid.empty())
        return;

    try {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

        if (user)
            QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
    }
    catch (const Exception&){}

}

void SearchFrame::timerTick(){
    int32_t waitFor = SearchManager::getInstance()->timeToSearch();

    if (waitFor > 0){
        QString msg = tr("Searching too soon, next search in %1 second").arg(waitFor);

        status->setText(msg);

        arena_title = tr("Search - %1").arg(msg);

        timer->start();
    }
    else {
        status->setText(tr("Ready to search..."));
    }
}

void SearchFrame::onHubAdded(SearchFrame::HubInfo* info){
    if (!info)
        return;

    QMap<QListWidgetItem*,HubInfo*>::const_iterator it = hub_items.find(info->item);

    if (it == hub_items.constEnd()){
        hub_items.insert(info->item, info);
        hub_list.insert(info->client, info);
    }
}

void SearchFrame::onHubChanged(SearchFrame::HubInfo* info){
    if (!info)
        return;

    QMap<Client*,HubInfo*>::const_iterator it = hub_list.find(info->client);

    if (it != hub_list.constEnd()){
        Client *client = info->client;
        info->item->setText(QString::fromStdString(client->getHubUrl()) + ": " +QString::fromStdString(client->getHubName()));
    }
}

void SearchFrame::onHubRemoved(SearchFrame::HubInfo* info){
    if (!info)
        return;

    QMap<Client*,HubInfo*>::const_iterator it = hub_list.find(info->client);

    if (it != hub_list.constEnd()){
        listWidget_HUBS->removeItemWidget(info->item);

        hub_items.remove(info->item);
        hub_list.remove(info->client);

        delete info;

        listWidget_HUBS->repaint();
    }
}

void SearchFrame::getParams(SearchFrame::VarMap &map, const dcpp::SearchResultPtr &ptr){
    map.clear();

    map["SIZE"]    = qulonglong(ptr->getSize());

    if (ptr->getType() == SearchResult::TYPE_FILE){
        QString fname = _q(ptr->getFileName());

        map["TTH"]  = _q(ptr->getTTH().toBase32());
        map["FILE"] = fname.split("\\", QString::SkipEmptyParts).last();

        QString path = fname.left(fname.lastIndexOf("\\"));

        if (!path.endsWith("\\"))
            path += "\\";

        map["PATH"] = path;
    }
    else{
        map["PATH"] = "";
        map["TTH"]  = "";
    }

    map["NICK"]    = WulforUtil::getInstance()->getNicks(ptr->getUser()->getCID());
    map["FSLS"]    = ptr->getFreeSlots();
    map["ASLS"]    = ptr->getSlots();
    map["IP"]      = _q(ptr->getIP());
    map["ISDIR"]   = map["FILE"].toString().endsWith("\\");
    map["HUB"]     = QString::fromStdString(ptr->getHubName());
    map["HOST"]    = QString::fromStdString(ptr->getHubURL());
    map["CID"]     = QString::fromStdString(ptr->getUser()->getCID().toBase32());
}

bool SearchFrame::getDownloadParams(SearchFrame::VarMap &params, SearchItem *item){
    if (!item)
        return false;

    params.clear();

    params["CID"]   = item->cid;
    params["FNAME"] = item->data(COLUMN_SF_PATH).toString() + item->data(COLUMN_SF_FILENAME).toString();
    params["ESIZE"] = item->data(COLUMN_SF_ESIZE);
    params["TTH"]   = item->data(COLUMN_SF_TTH);
    params["HOST"]  = item->data(COLUMN_SF_HOST);
    params["TARGET"]= _q(SETTING(DOWNLOAD_DIRECTORY));

    return true;
}

bool SearchFrame::getWholeDirParams(SearchFrame::VarMap &params, SearchItem *item){
    if (!item)
        return false;

    params.clear();

    params["CID"]   = item->cid;

    if (item->isDir)
        params["FNAME"] = item->data(COLUMN_SF_PATH).toString() + item->data(COLUMN_SF_FILENAME).toString();
    else
        params["FNAME"] = item->data(COLUMN_SF_PATH).toString();//Download directory that containing a file

    params["ESIZE"] = 0;
    params["TTH"]   = "";
    params["HOST"]  = item->data(COLUMN_SF_HOST);
    params["TARGET"]= _q(SETTING(DOWNLOAD_DIRECTORY));

    return true;
}

void SearchFrame::addResult(QMap<QString, QVariant> map){
    try {
        model->addResultPtr(map);

        results++;
    }
    catch (const SearchListException&){}
}

void SearchFrame::searchAlternates(const QString &tth){
    if (tth.isEmpty())
        return;

    comboBox_SEARCHSTR->setEditText(tth);
    comboBox_FILETYPES->setCurrentIndex(SearchManager::TYPE_TTH);
    lineEdit_SIZE->setText("");

    slotStartSearch();
}

void SearchFrame::slotStartSearch(){
    MainWindow *MW = MainWindow::getInstance();
    QString s = comboBox_SEARCHSTR->currentText();
    StringList clients;

    if (s.isEmpty())
        return;

    QMap<Client*,HubInfo*>::iterator it = hub_list.begin();

    for (; it != hub_list.end(); ++it){
        Client  *cl   = it.key();
        HubInfo *info = it.value();

        if (info->item && info->item->checkState() == Qt::Checked)
            clients.push_back(cl->getHubUrl());
    }

    if (clients.empty())
        return;

    QString str_size = lineEdit_SIZE->text();
    double lsize = Util::toDouble(Text::fromT(str_size.toStdString()));

    switch (comboBox_SIZE->currentIndex()){
        case 1:
            lsize *= 1024.0;

            break;
        case 2:
            lsize *= (1024.0*1024.0);

            break;
        case 3:
            lsize *= (1024.0*1024.0*1024.0);

            break;
    }

    quint64 llsize = (quint64)lsize;

    if (!searchHistory.contains(s)){
        searchHistory.push_front(s);

        comboBox_SEARCHSTR->clear();
        comboBox_SEARCHSTR->addItems(searchHistory);
    }

    {
        currentSearch = StringTokenizer<tstring>(s.toStdString(), ' ').getTokens();
        s = "";

        //strip out terms beginning with -
        for(TStringList::iterator si = currentSearch.begin(); si != currentSearch.end(); ) {
            if(si->empty()) {
                si = currentSearch.erase(si);
                continue;
            }

            if ((*si)[0] != '-')
                s += QString::fromStdString(*si) + ' ';

            ++si;
        }

        token = _q(Util::toString(Util::rand()));
    }

    SearchManager::SizeModes searchMode((SearchManager::SizeModes)comboBox_SIZETYPE->currentIndex());

    if(llsize == 0 || lineEdit_SIZE->text() == "")
        searchMode = SearchManager::SIZE_DONTCARE;

    int ftype = comboBox_FILETYPES->currentIndex();
    isHash = (ftype == SearchManager::TYPE_TTH);

    filterShared = SearchFrame::None;

    if (radioButton_SHAREDHIDE->isChecked())
        filterShared = SearchFrame::Filter;
    else if (radioButton_SHAREDHIGHLIGHT->isChecked())
        filterShared = SearchFrame::Highlight;

    withFreeSlots = checkBox_FILTERSLOTS->isChecked();

    model->setFilterRole(static_cast<int>(filterShared));
    model->clearModel();

    dropped = results = 0;

    if(SearchManager::getInstance()->okToSearch()) {
        SearchManager::getInstance()->search(clients, s.toStdString(), llsize, (SearchManager::TypeModes)ftype,
                                             searchMode, token.toStdString());

        QList<int> panes = splitter->sizes();

        panes[1] = panes[0] + panes[1];
        left_pane_old_size = panes[0];

        panes[0] = 0;

        splitter->setSizes(panes);

        arena_title = tr("Search - %1").arg(s);

    } else {
        int32_t waitFor = SearchManager::getInstance()->timeToSearch();
        QString msg = tr("Searching too soon, next search in %1 second").arg(waitFor);

        status->setText(msg);

        arena_title = tr("Search - %1").arg(msg);
        // Start the countdown timer
        initSecond();
    }

    MW->redrawToolPanel();
}

void SearchFrame::slotClear(){
    model->clearModel();
    comboBox_SEARCHSTR->setEditText("");
    lineEdit_SIZE->setText("");

    dropped = results = 0;
}

void SearchFrame::slotResultDoubleClicked(const QModelIndex &index){
    if (!index.isValid() || !index.internalPointer())
        return;

    SearchItem *item = reinterpret_cast<SearchItem*>(index.internalPointer());
    VarMap params;

    if (getDownloadParams(params, item))
        download(params);
}

void SearchFrame::slotContextMenu(const QPoint &){
    QItemSelectionModel *selection_model = treeView_RESULTS->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);

    if (list.size() < 1)
        return;

    if (!Menu::getInstance())
        Menu::newInstance();

    Menu::Action act = Menu::getInstance()->exec();

    switch (act){
        case Menu::None:
        {
            break;
        }
        case Menu::Download:
        {
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    download(params);
            }

            break;
        }
        case Menu::DownloadTo:
        {
            QString target = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

            if (target.isEmpty())
                break;

            if (!target.endsWith(QDir::separator()))
                target += QDir::separator();

            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item)){
                    params["TARGET"] = target;
                    download(params);
                }
            }

            break;
        }
        case Menu::DownloadWholeDir:
        {
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getWholeDirParams(params, item))
                    download(params);
            }

            break;
        }
        case Menu::DownloadWholeDirTo:
        {
            QString target = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

            if (target.isEmpty())
                break;

            if (!target.endsWith(QDir::separator()))
                target += QDir::separator();

            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getWholeDirParams(params, item)){
                    params["TARGET"] = target;
                    download(params);
                }
            }

            break;
        }
        case Menu::SearchTTH:
        {
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                if (!item->isDir){//only one file
                    SearchFrame *sf = new SearchFrame();

                    sf->searchAlternates(item->data(COLUMN_SF_TTH).toString());

                    break;
                }
            }

            break;
        }
        case Menu::Magnet:
        {
            QString magnets = "";
            WulforUtil *WU = WulforUtil::getInstance();

            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                if (!item->isDir){//only files
                    qlonglong size = item->data(COLUMN_SF_ESIZE).toLongLong();
                    QString tth = item->data(COLUMN_SF_TTH).toString();
                    QString name = item->data(COLUMN_SF_FILENAME).toString();

                    QString magnet = WU->makeMagnet(name, size, tth);

                    if (!magnet.isEmpty())
                        magnets += magnet + "\n";
                }
            }

            magnets = magnets.trimmed();

            if (!magnets.isEmpty())
                qApp->clipboard()->setText(magnets, QClipboard::Clipboard);

            break;
        }
        case Menu::Browse:
        {
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getWholeDirParams(params, item))
                    getFileList(params, false);
            }
            
            break;
        }
        case Menu::MatchQueue:
        {
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getWholeDirParams(params, item)){
                    params["FNAME"] = "";
                    getFileList(params, true);
                }
            }
            
            break;
        }
        case Menu::SendPM:
        {
            HubFrame *fr = NULL;
            HubManager *hm = HubManager::getInstance();
            
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                QString hubUrl = item->data(COLUMN_SF_HOST).toString();
                dcpp::CID cid(_tq(item->cid));

                fr = hm->getHub(hubUrl);

                if (fr)
                    fr->createPMWindow(cid);
            }            

            break;
        }
        case Menu::AddToFav:
        {
            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    addToFav(params["CID"].toString());

            }
            
            break;          
        }
        case Menu::GrantExtraSlot:
        {
             foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    grant(params);

            }
             
            break;
        }
        case Menu::RemoveFromQueue:
        {
             foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    removeSource(params);

             }
             
             break;
        }
        case Menu::Remove:
        {
             selection_model->clearSelection();

             foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                model->removeItem(item);

                model->repaint();
            }
             
            break;
        }
        default:
        {
            break;
        }
    }
}

void SearchFrame::slotHeaderMenu(const QPoint&){
    QMenu * mcols = new QMenu(this);
    QAction * column;
    int index;

    for (int i = 0; i < model->columnCount(); ++i) {
        index = treeView_RESULTS->header()->logicalIndex(i);
        column = mcols->addAction(model->headerData(index, Qt::Horizontal).toString());
        column->setCheckable(true);

        column->setChecked(!treeView_RESULTS->header()->isSectionHidden(index));
        column->setData(index);
    }

    QAction * chosen = mcols->exec(QCursor::pos());

    if (chosen) {
        index = chosen->data().toInt();

        if (treeView_RESULTS->header()->isSectionHidden(index)){
            treeView_RESULTS->header()->showSection(index);
            treeView_RESULTS->setColumnWidth(index, 50);
        }
        else
            treeView_RESULTS->header()->hideSection(index);
    }

    delete mcols;
}

void SearchFrame::slotTimer(){
    if (dropped == results && dropped == 0){

        if (currentSearch.empty())
            status->hide();
        else {
            status->show();

            QString text = QString(tr("<b>No results</b>"));

            status->setText(text);
        }
    }
    else {
        if (!status->isVisible())
            status->show();

        QString text = QString(tr("Found: <b>%1</b>  Dropped: <b>%2</b>")).arg(results).arg(dropped);

        status->setText(text);
    }
}

void SearchFrame::slotToggleSidePanel(){
    QList<int> panes = splitter->sizes();

    if (panes[0] < 15){//left pane can't have width less than 15px
        panes[0] = left_pane_old_size;
        panes[1] = panes[1]-left_pane_old_size;
    }
    else {
        panes[1] = panes[0] + panes[1];
        left_pane_old_size = panes[0];
        panes[0] = 0;
    }

    splitter->setSizes(panes);
}

void SearchFrame::on(SearchManagerListener::SR, const dcpp::SearchResultPtr& aResult) throw() {
    if (currentSearch.empty() || aResult == NULL)
        return;

    if (!aResult->getToken().empty() && token != _q(aResult->getToken())){
        dropped++;

        return;
    }

    if(isHash) {
        if(aResult->getType() != SearchResult::TYPE_FILE || TTHValue(Text::fromT(currentSearch[0])) != aResult->getTTH()) {
            dropped++;

            return;
        }
    }
    else {
        for(TStringIter j = currentSearch.begin(); j != currentSearch.end(); ++j) {
            if((*j->begin() != ('-') && Util::findSubString(aResult->getFile(), Text::fromT(*j)) == tstring::npos) ||
               (*j->begin() == ('-') && j->size() != 1 && Util::findSubString(aResult->getFile(), Text::fromT(j->substr(1))) != tstring::npos)
              )
           {
                    dropped++;

                    return;
           }
        }
    }

    if (filterShared == Filter){
        const TTHValue& t = aResult->getTTH();

        if (ShareManager::getInstance()->isTTHShared(t)) {
            dropped++;

            return;
        }
    }

    if (withFreeSlots && aResult->getFreeSlots() == 0){
        dropped++;

        return;
    }

    typedef Func1<SearchFrame, QMap<QString, QVariant> > FUNC;

    QMap<QString, QVariant> map;
    getParams(map, aResult);

    FUNC *func = new FUNC(this, &SearchFrame::addResult, map);

    QApplication::postEvent(this, new SearchCustomEvent(func));
    //WulforManager::getInstance()->dispatchClientFunc(func);
}

void SearchFrame::on(ClientConnected, Client* c) throw(){
    if (!hub_list.contains(c))
        onHubAdded(new HubInfo(c, listWidget_HUBS));
}

void SearchFrame::on(ClientUpdated, Client* c) throw(){
    if (hub_list.contains(c))
        onHubChanged(hub_list[c]);
}

void SearchFrame::on(ClientDisconnected, Client* c) throw(){
    if (hub_list.contains(c))
        onHubRemoved(hub_list[c]);
}
