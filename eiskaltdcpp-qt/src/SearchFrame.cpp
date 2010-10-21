/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
#include <QStringListModel>

#include "SearchFrame.h"
#include "MainWindow.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "SearchModel.h"
#include "WulforUtil.h"

#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/StringTokenizer.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Encoder.h"
#include "dcpp/UserCommand.h"

#include <QtDebug>

using namespace dcpp;

QVariant SearchStringListModel::data(const QModelIndex &index, int role) const{
    if (role != Qt::CheckStateRole || !index.isValid())
        return QStringListModel::data(index, role);

    return (checked.contains(index.data().toString())? Qt::Checked : Qt::Unchecked);
}

bool SearchStringListModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if (role != Qt::CheckStateRole || !index.isValid())
        return QStringListModel::setData(index, value, role);

    if (value.toInt() == Qt::Checked)
        checked.push_back(index.data().toString());
    else if (checked.contains(index.data().toString()))
        checked.removeAt(checked.indexOf(index.data().toString()));
}

SearchFrame::Menu::Menu(){
    WulforUtil *WU = WulforUtil::getInstance();

    menu = new QMenu();

    QAction *down       = new QAction(tr("Download"), NULL);
    down->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));

    down_to             = new QMenu(tr("Download to..."));
    down_to->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD_AS));

    QAction *down_wh    = new QAction(tr("Download Whole Directory"), NULL);
    down_wh->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));

    down_wh_to          = new QMenu(tr("Download Whole Directory to..."));
    down_wh_to->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD_AS));

    QAction *sep        = new QAction(menu);
    sep->setSeparator(true);

    QAction *find_tth   = new QAction(tr("Search TTH"), NULL);
    find_tth->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));

    QAction *magnet     = new QAction(tr("Copy magnet"), NULL);
    magnet->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));

    QAction *browse     = new QAction(tr("Browse files"), NULL);
    browse->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));

    QAction *match      = new QAction(tr("Match Queue"), NULL);
    match->setIcon(WU->getPixmap(WulforUtil::eiDOWN));

    QAction *send_pm    = new QAction(tr("Send Private Message"), NULL);
    send_pm->setIcon(WU->getPixmap(WulforUtil::eiMESSAGE));

    QAction *add_to_fav = new QAction(tr("Add to favorites"), NULL);
    add_to_fav->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));

    QAction *grant      = new QAction(tr("Grant extra slot"), NULL);
    grant->setIcon(WU->getPixmap(WulforUtil::eiEDITADD));

    QAction *sep1       = new QAction(menu);
    sep1->setSeparator(true);

    QAction *rem_queue  = new QAction(tr("Remove from Queue"), NULL);
    rem_queue->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    QAction *rem        = new QAction(tr("Remove"), NULL);
    rem->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    actions.insert(down, Download);
    actions.insert(down_wh, DownloadWholeDir);
    actions.insert(find_tth, SearchTTH);
    actions.insert(magnet, Magnet);
    actions.insert(browse, Browse);
    actions.insert(match, MatchQueue);
    actions.insert(send_pm, SendPM);
    actions.insert(add_to_fav, AddToFav);
    actions.insert(grant, GrantExtraSlot);
    actions.insert(rem_queue, RemoveFromQueue);
    actions.insert(rem, Remove);

    action_list   << down
                  << down_wh
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
                  << rem;
}

SearchFrame::Menu::~Menu(){
    qDeleteAll(action_list);

    delete menu;
    delete down_to;
    delete down_wh_to;
}

SearchFrame::Menu::Action SearchFrame::Menu::exec(QStringList list = QStringList()){
    foreach(QAction *a, action_list)
        a->setParent(NULL);

    qDeleteAll(down_to->actions());
    qDeleteAll(down_wh_to->actions());
    down_to->clear();
    down_wh_to->clear();

    QString aliases, paths;

    aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toAscii());
    paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toAscii());

    QStringList a = aliases.split("\n", QString::SkipEmptyParts);
    QStringList p = paths.split("\n", QString::SkipEmptyParts);

    QString raw = QByteArray::fromBase64(WSGET(WS_DOWNLOAD_DIR_HISTORY).toAscii());
    QStringList temp_pathes = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    if (!temp_pathes.isEmpty()){
        foreach (QString t, temp_pathes){
            QAction *act = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), QDir(t).dirName(), down_to);
            act->setToolTip(t);
            act->setData(t);

            down_to->addAction(act);

            QAction *act1 = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), QDir(t).dirName(), down_to);
            act1->setToolTip(t);
            act1->setData(t);

            down_wh_to->addAction(act1);
        }

        down_to->addSeparator();
        down_wh_to->addSeparator();
    }

    if (a.size() == p.size() && !a.isEmpty()){
        for (int i = 0; i < a.size(); i++){
            QAction *act = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), a.at(i), down_to);
            act->setData(p.at(i));

            down_to->addAction(act);

            QAction *act1 = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), a.at(i), down_to);
            act1->setData(p.at(i));

            down_wh_to->addAction(act1);
        }

        down_to->addSeparator();
        down_wh_to->addSeparator();
    }

    QAction *browse = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), tr("Browse"), down_to);
    browse->setData("");

    QAction *browse1 = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), tr("Browse"), down_to);
    browse->setData("");

    down_to->addAction(browse);
    down_wh_to->addAction(browse1);

    menu->clear();
    menu->addActions(action_list);
    menu->insertMenu(action_list.at(1), down_to);
    menu->insertMenu(action_list.at(2), down_wh_to);

    QMenu *userm = buildUserCmdMenu(list);

    if (userm)
        menu->addMenu(userm);

    QAction *ret = menu->exec(QCursor::pos());

    if (actions.contains(ret)){
        delete userm;

        return actions.value(ret);
    }
    else if (down_to->actions().contains(ret)){
        downToPath = ret->data().toString();

        return DownloadTo;
    }
    else if (down_wh_to->actions().contains(ret)){
        downToPath = ret->data().toString();

        return DownloadWholeDirTo;
    }
    else if (ret){
        ucParams["LAST"] = ret->toolTip();
        ucParams["NAME"] = ret->statusTip();
        ucParams["HOST"] =  ret->data().toString();

        delete userm;

        return UserCommands;
    }
    else{
        delete userm;

        return None;
    }
}

QMenu *SearchFrame::Menu::buildUserCmdMenu(QList<QString> hub_list){
    if (hub_list.empty())
        return NULL;

    return WulforUtil::getInstance()->buildUserCmdMenu(hub_list, UserCommand::CONTEXT_SEARCH);
}

void SearchFrame::Menu::addTempPath(const QString &path){
    QString raw = QByteArray::fromBase64(WSGET(WS_DOWNLOAD_DIR_HISTORY).toAscii());
    QStringList temp_pathes = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    if (!temp_pathes.contains(path) && !path.isEmpty() && QDir(path).exists()){
        temp_pathes.push_front(path);

        if (temp_pathes.count() > 5)
            temp_pathes.removeLast();

        QString raw = temp_pathes.join("\n");
        WSSET(WS_DOWNLOAD_DIR_HISTORY, raw.toAscii().toBase64());
    }
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
        timer1(NULL),
        saveFileType(true),
        proxy(NULL),
        completer(NULL)
{
    setupUi(this);

    init();

    ClientManager* clientMgr = ClientManager::getInstance();

    clientMgr->lock();
    clientMgr->addListener(this);
    Client::List& clients = clientMgr->getClients();

    for(Client::List::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client* client = *it;

        if(!client->isConnected())
            continue;

        hubs.push_back(_q(client->getHubUrl()));
        client_list.push_back(client);
    }

    clientMgr->unlock();

    str_model->setStringList(hubs);

    for (int i = 0; i < str_model->rowCount(); i++)
        str_model->setData(str_model->index(i, 0), Qt::Checked, Qt::CheckStateRole);

    SearchManager::getInstance()->addListener(this);
}

SearchFrame::~SearchFrame(){
    Menu::deleteInstance();

    SearchManager::getInstance()->removeListener(this);
    ClientManager::getInstance()->removeListener(this);

    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);

    if (completer)
        completer->deleteLater();

    delete model;
    delete arena_menu;
    delete timer;
    proxy->deleteLater();
}

void SearchFrame::closeEvent(QCloseEvent *e){
    save();

    setAttribute(Qt::WA_DeleteOnClose);

    e->accept();
}

void SearchFrame::init(){
    timer1 = new QTimer(this);
    timer1->setInterval(1000);

    model = new SearchModel(NULL);
    str_model = new SearchStringListModel(this);

    for (int i = 0; i < model->columnCount(); i++)
        comboBox_FILTERCOLUMNS->addItem(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());

    comboBox_FILTERCOLUMNS->setCurrentIndex(COLUMN_SF_FILENAME);

    frame_FILTER->setVisible(false);

    toolButton_CLOSEFILTER->setIcon(WICON(WulforUtil::eiEDITDELETE));

    treeView_RESULTS->setModel(model);
    treeView_RESULTS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RESULTS->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    treeView_HUBS->setModel(str_model);

    arena_menu = new QMenu(this->windowTitle());
    QAction *close_wnd = new QAction(WICON(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    QList<WulforUtil::Icons> icons;
    icons   << WulforUtil::eiFILETYPE_UNKNOWN  << WulforUtil::eiFILETYPE_MP3         << WulforUtil::eiFILETYPE_ARCHIVE
            << WulforUtil::eiFILETYPE_DOCUMENT << WulforUtil::eiFILETYPE_APPLICATION << WulforUtil::eiFILETYPE_PICTURE
            << WulforUtil::eiFILETYPE_VIDEO    << WulforUtil::eiFOLDER_BLUE          << WulforUtil::eiFIND;

    for (int i = 0; i < icons.size(); i++)
        comboBox_FILETYPES->setItemIcon(i, WICON(icons.at(i)));

    QString     raw  = QByteArray::fromBase64(WSGET(WS_SEARCH_HISTORY).toAscii());
    searchHistory = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    QMenu *m = new QMenu();

    foreach (QString s, searchHistory)
        m->addAction(s);

    focusShortcut = new QShortcut(QKeySequence(Qt::Key_F6), this);
    focusShortcut->setContext(Qt::WidgetWithChildrenShortcut);

    lineEdit_SEARCHSTR->setMenu(m);
    lineEdit_SEARCHSTR->setPixmap(WICON(WulforUtil::eiEDITADD).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    connect(this, SIGNAL(coreClientConnected(QString)),    this, SLOT(onHubAdded(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreClientDisconnected(QString)), this, SLOT(onHubRemoved(QString)),Qt::QueuedConnection);
    connect(this, SIGNAL(coreClientUpdated(QString)),      this, SLOT(onHubChanged(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreSR(VarMap)),                   this, SLOT(addResult(VarMap)), Qt::QueuedConnection);

    connect(focusShortcut, SIGNAL(activated()), lineEdit_SEARCHSTR, SLOT(setFocus()));
    connect(focusShortcut, SIGNAL(activated()), lineEdit_SEARCHSTR, SLOT(selectAll()));
    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));
    connect(pushButton_SEARCH, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
    connect(pushButton_CLEAR, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(treeView_RESULTS, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotResultDoubleClicked(QModelIndex)));
    connect(treeView_RESULTS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(treeView_RESULTS->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(timer1, SIGNAL(timeout()), this, SLOT(slotTimer()));
    connect(pushButton_SIDEPANEL, SIGNAL(clicked()), this, SLOT(slotToggleSidePanel()));
    connect(lineEdit_SEARCHSTR, SIGNAL(returnPressed()), this, SLOT(slotStartSearch()));
    connect(comboBox_FILETYPES, SIGNAL(currentIndexChanged(int)), lineEdit_SEARCHSTR, SLOT(setFocus()));
    connect(comboBox_FILETYPES, SIGNAL(currentIndexChanged(int)), lineEdit_SEARCHSTR, SLOT(selectAll()));
    connect(toolButton_CLOSEFILTER, SIGNAL(clicked()), this, SLOT(slotFilter()));
    connect(comboBox_FILTERCOLUMNS, SIGNAL(currentIndexChanged(int)), lineEdit_FILTER, SLOT(selectAll()));
    connect(comboBox_FILTERCOLUMNS, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeProxyColumn(int)));

    MainWindow *mwnd = MainWindow::getInstance();

    load();

    mwnd->addArenaWidget(this);
    mwnd->addArenaWidgetOnToolbar(this);
    mwnd->mapWidgetOnArena(this);

    setAttribute(Qt::WA_DeleteOnClose);

    QList<int> panes = splitter->sizes();
    left_pane_old_size = panes[0];

    timer1->start();

    lineEdit_SEARCHSTR->setFocus();
}

void SearchFrame::load(){
    treeView_RESULTS->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SEARCH_STATE).toAscii()));
    treeView_RESULTS->setSortingEnabled(true);

    filterShared = static_cast<SearchFrame::AlreadySharedAction>(WIGET(WI_SEARCH_SHARED_ACTION));

    comboBox_SHARED->setCurrentIndex(static_cast<int>(filterShared));

    checkBox_FILTERSLOTS->setChecked(WBGET(WB_SEARCHFILTER_NOFREE));
    checkBox_HIDEPANEL->setChecked(WBGET(WB_SEARCH_DONTHIDEPANEL));

    comboBox_FILETYPES->setCurrentIndex(WIGET(WI_SEARCH_LAST_TYPE));

    treeView_RESULTS->sortByColumn(WIGET(WI_SEARCH_SORT_COLUMN), WulforUtil::getInstance()->intToSortOrder(WIGET(WI_SEARCH_SORT_ORDER)));

    QString raw = QByteArray::fromBase64(WSGET(WS_SEARCH_HISTORY).toAscii());
    QStringList list = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    completer = new QCompleter(list, lineEdit_SEARCHSTR);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);

    lineEdit_SEARCHSTR->setCompleter(completer);
}

void SearchFrame::save(){
    WSSET(WS_SEARCH_STATE, treeView_RESULTS->header()->saveState().toBase64());
    WISET(WI_SEARCH_SORT_COLUMN, model->getSortColumn());
    WISET(WI_SEARCH_SORT_ORDER, WulforUtil::getInstance()->sortOrderToInt(model->getSortOrder()));
    WISET(WI_SEARCH_SHARED_ACTION, static_cast<int>(filterShared));

    if (saveFileType)
        WISET(WI_SEARCH_LAST_TYPE, comboBox_FILETYPES->currentIndex());

    WBSET(WB_SEARCHFILTER_NOFREE, checkBox_FILTERSLOTS->isChecked());
    WBSET(WB_SEARCH_DONTHIDEPANEL, checkBox_HIDEPANEL->isChecked());
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

QString SearchFrame::getArenaShortTitle(){
    return getArenaTitle();
}

QMenu *SearchFrame::getMenu(){
    return arena_menu;
}

const QPixmap &SearchFrame::getPixmap(){
    return WICON(WulforUtil::eiFILEFIND);
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

void SearchFrame::onHubAdded(const QString &info){
    if (hubs.contains(info) || info.isEmpty())
        return;

    hubs.push_back(info);
    client_list.push_back(ClientManager::getInstance()->getClient(_tq(info)));

    str_model->setStringList(hubs);
}

void SearchFrame::onHubChanged(const QString &info){
    if (!hubs.contains(info) || info.isEmpty())
        return;

    Client *cl = ClientManager::getInstance()->getClient(_tq(info));
    if (!cl || client_list.indexOf(cl) < 0)
        return;

    hubs.removeAt(client_list.indexOf(cl));
    client_list.removeAt(client_list.indexOf(cl));

    hubs.push_back(info);
    client_list.push_back(cl);

    str_model->setStringList(hubs);
}

void SearchFrame::onHubRemoved(const QString &info){
    if (!hubs.contains(info) || info.isEmpty())
        return;

    client_list.removeAt(hubs.indexOf(info));
    hubs.removeAt(hubs.indexOf(info));

    str_model->setStringList(hubs);
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
        map["ISDIR"]   = false;
    }
    else{
        map["FILE"] = _q(ptr->getFileName()).split('\\', QString::SkipEmptyParts).last();
        map["PATH"] = _q(ptr->getFile()).left(_q(ptr->getFile()).lastIndexOf(map["FILE"].toString()));
        map["TTH"]  = "";
        map["ISDIR"] = true;
    }

    map["NICK"]    = WulforUtil::getInstance()->getNicks(ptr->getUser()->getCID());
    map["FSLS"]    = ptr->getFreeSlots();
    map["ASLS"]    = ptr->getSlots();
    map["IP"]      = _q(ptr->getIP());
    map["HUB"]     = _q(ptr->getHubName());
    map["HOST"]    = _q(ptr->getHubURL());
    map["CID"]     = _q(ptr->getUser()->getCID().toBase32());
}

bool SearchFrame::getDownloadParams(SearchFrame::VarMap &params, SearchItem *item){
    if (!item)
        return false;

    params.clear();

    QString fname = item->data(COLUMN_SF_PATH).toString() + item->data(COLUMN_SF_FILENAME).toString();

    if (item->isDir && !fname.endsWith('\\'))
        fname += "\\";

    params["CID"]   = item->cid;
    params["FNAME"] = fname;
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

void SearchFrame::addResult(const QMap<QString, QVariant> &map){
    try {
        if (model->addResultPtr(map))
            results++;
    }
    catch (const SearchListException&){}
}

void SearchFrame::searchAlternates(const QString &tth){
    if (tth.isEmpty())
        return;

    lineEdit_SEARCHSTR->setText(tth);
    comboBox_FILETYPES->setCurrentIndex(SearchManager::TYPE_TTH);
    lineEdit_SIZE->setText("");

    slotStartSearch();

    saveFileType = false;
}

void SearchFrame::searchFile(const QString &file){
    if (file.isEmpty())
        return;

    lineEdit_SEARCHSTR->setText(file);
    comboBox_FILETYPES->setCurrentIndex(SearchManager::TYPE_ANY);
    lineEdit_SIZE->setText("");

    saveFileType = false;

    slotStartSearch();
}

void SearchFrame::fastSearch(const QString &text, bool isTTH){
    if (text.isEmpty())
        return;

    if (!isTTH)
        comboBox_FILETYPES->setCurrentIndex(0); // set type "Any"
    else
        comboBox_FILETYPES->setCurrentIndex(8); // set type "TTH"

    lineEdit_SEARCHSTR->setText(text);

    slotStartSearch();
}

void SearchFrame::slotStartSearch(){
    if (lineEdit_SEARCHSTR->text().trimmed().isEmpty())
        return;

    MainWindow *MW = MainWindow::getInstance();
    QString s = lineEdit_SEARCHSTR->text().trimmed();
    StringList clients;

    for (int i = 0; i < str_model->rowCount(); i++){
        QModelIndex index = str_model->index(i, 0);

        if (index.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            clients.push_back(_tq(index.data().toString()));
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

        QMenu *m = new QMenu();

        foreach (QString s, searchHistory)
            m->addAction(s);

        lineEdit_SEARCHSTR->setMenu(m);

        if (searchHistory.count() > 10)
            searchHistory.removeLast();

        QString hist = searchHistory.join("\n");
        WSSET(WS_SEARCH_HISTORY, hist.toAscii().toBase64());
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

    filterShared = static_cast<AlreadySharedAction>(comboBox_SHARED->currentIndex());

    withFreeSlots = checkBox_FILTERSLOTS->isChecked();

    model->setFilterRole(static_cast<int>(filterShared));
    model->clearModel();

    dropped = results = 0;

    if(SearchManager::getInstance()->okToSearch()) {
        SearchManager::getInstance()->search(clients, s.toStdString(), llsize, (SearchManager::TypeModes)ftype,
                                             searchMode, token.toStdString());

        if (!checkBox_HIDEPANEL->isChecked()){
            QList<int> panes = splitter->sizes();

            panes[1] = panes[0] + panes[1];

            left_pane_old_size = panes[0] > 15 ? panes[0] : left_pane_old_size;

            panes[0] = 0;

            splitter->setSizes(panes);
        }

        arena_title = tr("Search - %1").arg(s);

    }
    else {
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
    treeView_RESULTS->clearSelection();
    model->clearModel();
    lineEdit_SEARCHSTR->clear();
    lineEdit_SIZE->setText("");

    dropped = results = 0;
}

void SearchFrame::slotResultDoubleClicked(const QModelIndex &index){
    if (!index.isValid() || !index.internalPointer())
        return;

    QModelIndex i = proxy? proxy->mapToSource(index) : index;

    SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
    VarMap params;

    if (getDownloadParams(params, item)){
        download(params);

        if (item->childCount() > 0 && !SETTING(DONT_DL_ALREADY_QUEUED)){//download all child items
            QString fname = params["FNAME"].toString();

            foreach (SearchItem *i, item->childItems){
                if (getDownloadParams(params, i)){
                    params["FNAME"] = fname;

                    download(params);
                }
            }
        }
    }
}

void SearchFrame::slotContextMenu(const QPoint &){
    QItemSelectionModel *selection_model = treeView_RESULTS->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);

    if (list.size() < 1)
        return;

    if (proxy){
        QModelIndexList _list;
        foreach (QModelIndex i, list)
            _list.push_back(proxy->mapToSource(i));
        list = _list;
    }

    if (!Menu::getInstance())
        Menu::newInstance();

    QStringList hubs;

    foreach (QModelIndex i, list){
        SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
        QString host = item->data(COLUMN_SF_HOST).toString();

        if (!hubs.contains(host))
            hubs.push_back(host);
    }

    Menu::Action act = Menu::getInstance()->exec(hubs);

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

                if (getDownloadParams(params, item)){
                    download(params);

                    if (item->childCount() > 0 && !SETTING(DONT_DL_ALREADY_QUEUED)){//download all child items
                        QString fname = params["FNAME"].toString();

                        foreach (SearchItem *i, item->childItems){
                            if (getDownloadParams(params, i)){
                                params["FNAME"] = fname;

                                download(params);
                            }
                        }
                    }
                }
            }

            break;
        }
        case Menu::DownloadTo:
        {
            static QString old_target = QDir::homePath();
            QString target = Menu::getInstance()->getDownloadToPath();

            if (!QDir(target).exists() || target.isEmpty()){
                target = QFileDialog::getExistingDirectory(this, tr("Select directory"), old_target);

                target = QDir::toNativeSeparators(target);

                Menu::getInstance()->addTempPath(target);
            }

            if (target.isEmpty())
                break;

            if (!target.endsWith(QDir::separator()))
                target += QDir::separator();

            old_target = target;

            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item)){
                    params["TARGET"] = target;
                    download(params);

                    if (item->childCount() > 0 && !SETTING(DONT_DL_ALREADY_QUEUED)){//download all child items
                        QString fname = params["FNAME"].toString();

                        foreach (SearchItem *i, item->childItems){
                            if (getDownloadParams(params, i)){
                                params["FNAME"]  = fname;
                                params["TARGET"] = target;

                                download(params);
                            }
                        }
                    }
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
            static QString old_target = QDir::homePath();
            QString target = Menu::getInstance()->getDownloadToPath();

            if (!QDir(target).exists() || target.isEmpty()){
                target = QFileDialog::getExistingDirectory(this, tr("Select directory"), old_target);

                target = QDir::toNativeSeparators(target);

                Menu::getInstance()->addTempPath(target);
            }

            if (target.isEmpty())
                break;

            if (!target.endsWith(QDir::separator()))
                target += QDir::separator();

            old_target = target;

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

            foreach (QModelIndex i, list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                QString hubUrl = item->data(COLUMN_SF_HOST).toString();
                dcpp::CID cid(_tq(item->cid));

                fr = HubManager::getInstance()->getHub(hubUrl);

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
        case Menu::UserCommands:
        {
            foreach (QModelIndex i, list){
               SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
               QString cmd_name = Menu::getInstance()->ucParams["NAME"];
               QString hub      = Menu::getInstance()->ucParams["HOST"];
               QString last_user_cmd = Menu::getInstance()->ucParams["LAST"];

               int id = FavoriteManager::getInstance()->findUserCommand(cmd_name.toStdString(), hub.toStdString());
               UserCommand uc;

               if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
                   break;

               StringMap params;

               if (WulforUtil::getInstance()->getUserCommandParams(last_user_cmd, params)){
                   UserPtr user = ClientManager::getInstance()->findUser(CID(item->cid.toStdString()));

                   if (user && user->isOnline()){
                       params["fileFN"]     = _tq(item->data(COLUMN_SF_PATH).toString() + item->data(COLUMN_SF_FILENAME).toString());
                       params["fileSI"]     = _tq(item->data(COLUMN_SF_ESIZE).toString());
                       params["fileSIshort"]= _tq(item->data(COLUMN_SF_SIZE).toString());

                       if(!item->isDir)
                           params["fileTR"] = _tq(item->data(COLUMN_SF_TTH).toString());

                       // compatibility with 0.674 and earlier
                       params["file"] = params["fileFN"];
                       params["filesize"] = params["fileSI"];
                       params["filesizeshort"] = params["fileSIshort"];
                       params["tth"] = params["fileTR"];

                       ClientManager::getInstance()->userCommand(user, uc, params, true);
                   }

                }
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
    WulforUtil::headerMenu(treeView_RESULTS);
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

void SearchFrame::slotFilter(){
    if (frame_FILTER->isVisible()){
        treeView_RESULTS->setModel(model);

        disconnect(lineEdit_FILTER, SIGNAL(textChanged(QString)), proxy, SLOT(setFilterFixedString(QString)));
    }
    else {
        proxy = (proxy? proxy : (new SearchProxyModel(this)));
        proxy->setDynamicSortFilter(true);
        proxy->setFilterFixedString(lineEdit_FILTER->text());
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(comboBox_FILTERCOLUMNS->currentIndex());
        proxy->setSourceModel(model);

        treeView_RESULTS->setModel(proxy);

        connect(lineEdit_FILTER, SIGNAL(textChanged(QString)), proxy, SLOT(setFilterFixedString(QString)));

        if (!lineEdit_SEARCHSTR->selectedText().isEmpty()){
            lineEdit_FILTER->setText(lineEdit_SEARCHSTR->selectedText());
            lineEdit_FILTER->selectAll();
        }

        lineEdit_FILTER->setFocus();

        if (!lineEdit_FILTER->text().isEmpty())
            lineEdit_FILTER->selectAll();
    }

    frame_FILTER->setVisible(!frame_FILTER->isVisible());
}

void SearchFrame::slotChangeProxyColumn(int col){
    if (proxy)
        proxy->setFilterKeyColumn(col);
}

bool SearchFrame::isFindFrameActivated(){
    return (frame_FILTER->isVisible() && lineEdit_FILTER->hasFocus());
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
            if((*j->begin() != ('-') && Util::findSubString(aResult->getFile(), *j) == tstring::npos) ||
               (*j->begin() == ('-') && j->size() != 1 && Util::findSubString(aResult->getFile(), j->substr(1)) != tstring::npos)
              )
           {
                    dropped++;

                    return;
           }
        }
    }

    if (filterShared == Filter && aResult->getType() == SearchResult::TYPE_FILE){
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

    QMap<QString, QVariant> map;
    getParams(map, aResult);

    emit coreSR(map);
}

void SearchFrame::on(ClientConnected, Client* c) throw(){
    emit coreClientConnected(_q(c->getHubUrl()));
}

void SearchFrame::on(ClientUpdated, Client* c) throw(){
    emit coreClientUpdated((_q(c->getHubUrl())));
}

void SearchFrame::on(ClientDisconnected, Client* c) throw(){
    emit coreClientDisconnected((_q(c->getHubUrl())));
}
