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
#include <QKeyEvent>
#include <QTimer>
#include <QShortcut>
#include <QStringListModel>
#include <QCompleter>
#include <QListWidget>
#include <QListWidgetItem>

#include "SearchFrame.h"
#include "MainWindow.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "SearchModel.h"
#include "SearchBlacklist.h"
#include "SearchBlacklistDialog.h"
#include "WulforUtil.h"
#include "Magnet.h"
#include "ArenaWidgetManager.h"
#include "ArenaWidgetFactory.h"
#include "DownloadToHistory.h"
#include "GlobalTimer.h"

#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/StringTokenizer.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Encoder.h"
#include "dcpp/UserCommand.h"

#include <QtDebug>

using namespace dcpp;

class SearchFramePrivate{
public:
    QString arena_title;
    QString token;

    TStringList currentSearch;

    qulonglong dropped;
    qulonglong results;
    SearchFrame::AlreadySharedAction filterShared;
    bool withFreeSlots;

    QStringList hubs;
    QStringList searchHistory;

    QList<dcpp::Client*> client_list;

    QTimer *timer;

    QCompleter *completer;

    QMenu *arena_menu;

    QShortcut *focusShortcut;

    bool saveFileType;

    SearchModel *model;
    SearchStringListModel *str_model;
    SearchProxyModel *proxy;

    bool isHash;
    bool stop;
    int left_pane_old_size;

    QString target;
    uint64_t searchStartTime;
    uint64_t searchEndTime;
    bool waitingResults;
};

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

    return true;
}

SearchFrame::Menu::Menu(){
    WulforUtil *WU = WulforUtil::getInstance();

    menu = new QMenu();

    magnet_menu = new QMenu(tr("Magnet"));

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

    QAction *magnet_web     = new QAction(tr("Copy web-magnet"), NULL);
    magnet_web->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));

    QAction *magnet_info    = new QAction(tr("Properties of magnet"), NULL);
    magnet_info->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));

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

    QAction *sep2       = new QAction(menu);
    sep2->setSeparator(true);

    QAction *sep3       = new QAction(menu);
    sep3->setSeparator(true);

    QAction *rem_queue  = new QAction(tr("Remove from Queue"), NULL);
    rem_queue->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    QAction *rem        = new QAction(tr("Remove"), NULL);
    rem->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    black_list_menu     = new QMenu(tr("Blacklist..."));
    black_list_menu->setIcon(WU->getPixmap(WulforUtil::eiFILTER));

    QAction *blacklist = new QAction(tr("Blacklist"), NULL);
    blacklist->setIcon(WU->getPixmap(WulforUtil::eiFILTER));

    QAction *add_to_blacklist = new QAction(tr("Add to Blacklist"), NULL);
    add_to_blacklist->setIcon(WU->getPixmap(WulforUtil::eiEDITADD));

    black_list_menu->addActions(QList<QAction*>()
                    << add_to_blacklist << blacklist);

    magnet_menu->addActions(QList<QAction*>()
                    << magnet << magnet_web << sep3 << magnet_info);

    actions.insert(down, Download);
    actions.insert(down_wh, DownloadWholeDir);
    actions.insert(find_tth, SearchTTH);
    actions.insert(magnet, Magnet);
    actions.insert(magnet_web, MagnetWeb);
    actions.insert(magnet_info, MagnetInfo);
    actions.insert(browse, Browse);
    actions.insert(match, MatchQueue);
    actions.insert(send_pm, SendPM);
    actions.insert(add_to_fav, AddToFav);
    actions.insert(grant, GrantExtraSlot);
    actions.insert(rem_queue, RemoveFromQueue);
    actions.insert(rem, Remove);
    actions.insert(blacklist, Blacklist);
    actions.insert(add_to_blacklist, AddToBlacklist);

    action_list   << down
                  << down_wh
                  << sep
                  << find_tth
                  << browse
                  << match
                  << send_pm
                  << add_to_fav
                  << grant
                  << sep1
                  << rem_queue
                  << rem
                  << sep2;
}

SearchFrame::Menu::~Menu(){
    qDeleteAll(action_list);

    magnet_menu->deleteLater();
    menu->deleteLater();
    down_to->deleteLater();
    down_wh_to->deleteLater();
    black_list_menu->deleteLater();
}

SearchFrame::Menu::Action SearchFrame::Menu::exec(QStringList list = QStringList()){
    for (const auto &a : action_list)
        a->setParent(NULL);

    qDeleteAll(down_to->actions());
    qDeleteAll(down_wh_to->actions());
    down_to->clear();
    down_wh_to->clear();

    QString aliases, paths;

    aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toUtf8());
    paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toUtf8());

    QStringList a = aliases.split("\n", QString::SkipEmptyParts);
    QStringList p = paths.split("\n", QString::SkipEmptyParts);

    QStringList temp_pathes = DownloadToDirHistory::get();

    if (!temp_pathes.isEmpty()){
        for (const auto &t : temp_pathes){
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
    menu->insertMenu(action_list.at(5), magnet_menu);
    menu->insertMenu(action_list.at(12),black_list_menu);

    QScopedPointer<QMenu> userm(buildUserCmdMenu(list));

    if (!userm.isNull() && !userm->actions().isEmpty())
        menu->addMenu(userm.data());

    QAction *ret = menu->exec(QCursor::pos());

    if (actions.contains(ret)) {
        return actions.value(ret);
    } else if (down_to->actions().contains(ret)) {
        downToPath = ret->data().toString();
        return DownloadTo;
    } else if (down_wh_to->actions().contains(ret)) {
        downToPath = ret->data().toString();
        return DownloadWholeDirTo;
    } else if (ret) {
        uc_cmd_id = ret->data().toInt();
        return UserCommands;
    } else {
        return None;
    }
}

QMenu *SearchFrame::Menu::buildUserCmdMenu(QList<QString> hub_list){
    if (hub_list.empty())
        return NULL;

    return WulforUtil::getInstance()->buildUserCmdMenu(hub_list, UserCommand::CONTEXT_SEARCH);
}

void SearchFrame::Menu::addTempPath(const QString &path){
    QStringList temp_pathes = DownloadToDirHistory::get();
    temp_pathes.push_front(path);

    DownloadToDirHistory::put(temp_pathes);
}

SearchFrame::SearchFrame(QWidget *parent): QWidget(parent), d_ptr(new SearchFramePrivate())
{
    if (!SearchBlacklist::getInstance())
        SearchBlacklist::newInstance();

    Q_D(SearchFrame);

    d->isHash = false;
    d->arena_menu = NULL;
    d->timer = NULL;
    d->dropped = 0L;
    d->results = 0L;
    d->filterShared = SearchFrame::None;
    d->withFreeSlots = false;
    d->saveFileType = true;
    d->proxy = NULL;
    d->completer = NULL;
    d->stop = false;
    d->arena_title = tr("Search");
    d->searchStartTime = 0;
    d->searchEndTime = 1;
    d->waitingResults = false;

    setupUi(this);

    init();

    ClientManager* clientMgr = ClientManager::getInstance();

#ifdef DO_NOT_USE_MUTEX
    clientMgr->lock();
#else // DO_NOT_USE_MUTEX
    auto lock = clientMgr->lock();
#endif // DO_NOT_USE_MUTEX
    clientMgr->addListener(this);
    Client::List& clients = clientMgr->getClients();

    for (const auto &client : clients) {
        if(!client->isConnected())
            continue;

        d->hubs.push_back(_q(client->getHubUrl()));
        d->client_list.push_back(client);
    }

#ifdef DO_NOT_USE_MUTEX
    clientMgr->unlock();
#endif // DO_NOT_USE_MUTEX

    d->str_model->setStringList(d->hubs);


    for (int i = 0; i < d->str_model->rowCount(); i++)
       d-> str_model->setData(d->str_model->index(i, 0), Qt::Checked, Qt::CheckStateRole);

    SearchManager::getInstance()->addListener(this);
}

SearchFrame::~SearchFrame(){
    Menu::deleteInstance();

    Q_D(SearchFrame);

    treeView_RESULTS->setModel(NULL);

    if (d->completer)
        d->completer->deleteLater();

    if (d->proxy)
        d->proxy->deleteLater();

    d->arena_menu->deleteLater();

    delete d->model;
    delete d->timer;

    delete d_ptr;
}

void SearchFrame::closeEvent(QCloseEvent *e){
    SearchManager::getInstance()->removeListener(this);
    ClientManager::getInstance()->removeListener(this);

    Q_D(SearchFrame);

    if (d->timer)
        d->timer->stop();

    save();

    setAttribute(Qt::WA_DeleteOnClose);

    QWidget::disconnect(this, NULL, this, NULL);

    e->accept();
}

bool SearchFrame::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (static_cast<LineEdit*>(obj) == lineEdit_FILTER && k_e->key() == Qt::Key_Escape){
            lineEdit_FILTER->clear();

            requestFilter();

            return true;
        }
    }

    return QWidget::eventFilter(obj, e);
}

void SearchFrame::init(){
    Q_D(SearchFrame);

    d->model = new SearchModel(NULL);
    d->str_model = new SearchStringListModel(this);

    for (int i = 0; i < d->model->columnCount(); i++)
        comboBox_FILTERCOLUMNS->addItem(d->model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());

    comboBox_FILTERCOLUMNS->setCurrentIndex(COLUMN_SF_FILENAME);

    frame_FILTER->setVisible(false);

    pushButton_STOP->hide();

    toolButton_CLOSEFILTER->setIcon(WICON(WulforUtil::eiEDITDELETE));

    treeView_RESULTS->setModel(d->model);
    treeView_RESULTS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RESULTS->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    treeView_HUBS->setModel(d->str_model);

    d->arena_menu = new QMenu(this->windowTitle());
    QAction *close_wnd = new QAction(WICON(WulforUtil::eiFILECLOSE), tr("Close"), d->arena_menu);
    d->arena_menu->addAction(close_wnd);
    const SettingsManager::SearchTypes &searchTypes = SettingsManager::getInstance()->getSearchTypes();
    QStringList filetypes;
    // Predefined
    for (int i = SearchManager::TYPE_ANY; i < SearchManager::TYPE_LAST; i++)
    {
            filetypes << _q(SearchManager::getTypeStr(i));
    }

    // Customs
    for (const auto &i : searchTypes)
    {
        string type = i.first;
        if (!(type.size() == 1 && type[0] >= '1' && type[0] <= '7'))
        {
                filetypes << _q(type);
        }
    }
    comboBox_FILETYPES->addItems(filetypes);
    comboBox_FILETYPES->setCurrentIndex(0);

    QList<WulforUtil::Icons> icons;
    icons   << WulforUtil::eiFILETYPE_UNKNOWN  << WulforUtil::eiFILETYPE_MP3         << WulforUtil::eiFILETYPE_ARCHIVE
            << WulforUtil::eiFILETYPE_DOCUMENT << WulforUtil::eiFILETYPE_APPLICATION << WulforUtil::eiFILETYPE_PICTURE
            << WulforUtil::eiFILETYPE_VIDEO    << WulforUtil::eiFOLDER_BLUE          << WulforUtil::eiFIND
            << WulforUtil::eiFILETYPE_ARCHIVE;

    for (int i = 0; i < icons.size(); i++)
        comboBox_FILETYPES->setItemIcon(i, WICON(icons.at(i)));

    QString     raw  = QByteArray::fromBase64(WSGET(WS_SEARCH_HISTORY).toUtf8());
    d->searchHistory = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    QMenu *m = new QMenu();

    for (const auto &s : d->searchHistory)
        m->addAction(s);

    d->focusShortcut = new QShortcut(QKeySequence(Qt::Key_F6), this);
    d->focusShortcut->setContext(Qt::WidgetWithChildrenShortcut);

    lineEdit_SEARCHSTR->setMenu(m);
    lineEdit_SEARCHSTR->setPixmap(WICON(WulforUtil::eiEDITADD).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    lineEdit_FILTER->installEventFilter(this);

    connect(this, SIGNAL(coreClientConnected(QString)),    this, SLOT(onHubAdded(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreClientDisconnected(QString)), this, SLOT(onHubRemoved(QString)),Qt::QueuedConnection);
    connect(this, SIGNAL(coreClientUpdated(QString)),      this, SLOT(onHubChanged(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreSR(VarMap)),                  this, SLOT(addResult(VarMap)), Qt::QueuedConnection);

    connect(d->focusShortcut, SIGNAL(activated()), lineEdit_SEARCHSTR, SLOT(setFocus()));
    connect(d->focusShortcut, SIGNAL(activated()), lineEdit_SEARCHSTR, SLOT(selectAll()));
    connect(close_wnd, SIGNAL(triggered()), this, SLOT(slotClose()));
    connect(pushButton_SEARCH, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
    connect(pushButton_STOP, SIGNAL(clicked()), this, SLOT(slotStopSearch()));
    connect(pushButton_CLEAR, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(treeView_RESULTS, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotResultDoubleClicked(QModelIndex)));
    connect(treeView_RESULTS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(treeView_RESULTS->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(GlobalTimer::getInstance(), SIGNAL(second()), this, SLOT(slotTimer()));
    connect(pushButton_SIDEPANEL, SIGNAL(clicked()), this, SLOT(slotToggleSidePanel()));
    connect(lineEdit_SEARCHSTR, SIGNAL(returnPressed()), this, SLOT(slotStartSearch()));
    connect(lineEdit_SIZE,      SIGNAL(returnPressed()), this, SLOT(slotStartSearch()));
    connect(comboBox_FILETYPES, SIGNAL(currentIndexChanged(int)), lineEdit_SEARCHSTR, SLOT(setFocus()));
    connect(comboBox_FILETYPES, SIGNAL(currentIndexChanged(int)), lineEdit_SEARCHSTR, SLOT(selectAll()));
    connect(toolButton_CLOSEFILTER, SIGNAL(clicked()), this, SLOT(slotFilter()));
    connect(comboBox_FILTERCOLUMNS, SIGNAL(currentIndexChanged(int)), lineEdit_FILTER, SLOT(selectAll()));
    connect(comboBox_FILTERCOLUMNS, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeProxyColumn(int)));

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));

    load();

    setAttribute(Qt::WA_DeleteOnClose);

    QList<int> panes = splitter->sizes();
    d->left_pane_old_size = panes[0];

    lineEdit_SEARCHSTR->setFocus();
}

void SearchFrame::load(){
    Q_D(SearchFrame);

    treeView_RESULTS->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SEARCH_STATE).toUtf8()));
    treeView_RESULTS->setSortingEnabled(true);

    d->filterShared = static_cast<SearchFrame::AlreadySharedAction>(WIGET(WI_SEARCH_SHARED_ACTION));

    comboBox_SHARED->setCurrentIndex(static_cast<int>(d->filterShared));

    checkBox_FILTERSLOTS->setChecked(WBGET(WB_SEARCHFILTER_NOFREE));
    checkBox_HIDEPANEL->setChecked(WBGET(WB_SEARCH_DONTHIDEPANEL));

    comboBox_FILETYPES->setCurrentIndex(WIGET(WI_SEARCH_LAST_TYPE));

    treeView_RESULTS->sortByColumn(WIGET(WI_SEARCH_SORT_COLUMN), WulforUtil::getInstance()->intToSortOrder(WIGET(WI_SEARCH_SORT_ORDER)));

    QString raw = QByteArray::fromBase64(WSGET(WS_SEARCH_HISTORY).toUtf8());
    QStringList list = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    d->completer = new QCompleter(list, lineEdit_SEARCHSTR);
    d->completer->setCaseSensitivity(Qt::CaseInsensitive);
    d->completer->setWrapAround(false);

    lineEdit_SEARCHSTR->setCompleter(d->completer);
}

void SearchFrame::save(){
    Q_D(SearchFrame);

    WSSET(WS_SEARCH_STATE, treeView_RESULTS->header()->saveState().toBase64());
    WISET(WI_SEARCH_SORT_COLUMN, d->model->getSortColumn());
    WISET(WI_SEARCH_SORT_ORDER, WulforUtil::getInstance()->sortOrderToInt(d->model->getSortOrder()));
    WISET(WI_SEARCH_SHARED_ACTION, static_cast<int>(d->filterShared));

    if (d->saveFileType)
        WISET(WI_SEARCH_LAST_TYPE, comboBox_FILETYPES->currentIndex());

    WBSET(WB_SEARCHFILTER_NOFREE, checkBox_FILTERSLOTS->isChecked());
    WBSET(WB_SEARCH_DONTHIDEPANEL, checkBox_HIDEPANEL->isChecked());
}

void SearchFrame::initSecond(){
    /*Q_D(SearchFrame);

    if (!d->timer){
        d->timer = new QTimer(this);
        d->timer->setInterval(1000);
        d->timer->setSingleShot(true);

        connect(d->timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    }

    d->timer->start();*/
}

QWidget *SearchFrame::getWidget(){
    return this;
}

QString SearchFrame::getArenaTitle(){
    Q_D(SearchFrame);

    return d->arena_title;
}

QString SearchFrame::getArenaShortTitle(){
    return getArenaTitle();
}

QMenu *SearchFrame::getMenu(){
    Q_D(SearchFrame);

    return d->arena_menu;
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
            QueueManager::getInstance()->add(target + subdir, size, TTHValue(params["TTH"].toString().toStdString()), HintedUser(user, hubUrl));
        }
        else{
            QueueManager::getInstance()->addDirectory(filename, HintedUser(user, hubUrl), target);
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

            QueueManager::getInstance()->addList(HintedUser(user, host), flag, dir);
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
            UploadManager::getInstance()->reserveSlot(HintedUser(user, host));
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

void SearchFrame::timerTick() {

}

void SearchFrame::onHubAdded(const QString &info){
    Q_D(SearchFrame);

    if (d->hubs.contains(info) || info.isEmpty())
        return;

    d->hubs.push_back(info);
    d->client_list.push_back(ClientManager::getInstance()->getClient(_tq(info)));

    d->str_model->setStringList(d->hubs);
}

void SearchFrame::onHubChanged(const QString &info){
    Q_D(SearchFrame);

    if (!d->hubs.contains(info) || info.isEmpty())
        return;

    Client *cl = ClientManager::getInstance()->getClient(_tq(info));
    if (!cl || d->client_list.indexOf(cl) < 0)
        return;

    d->hubs.removeAt(d->client_list.indexOf(cl));
    d->client_list.removeAt(d->client_list.indexOf(cl));

    d->hubs.push_back(info);
    d->client_list.push_back(cl);

    d->str_model->setStringList(d->hubs);
}

void SearchFrame::onHubRemoved(const QString &info){
    Q_D(SearchFrame);

    if (!d->hubs.contains(info) || info.isEmpty())
        return;

    d->client_list.removeAt(d->hubs.indexOf(info));
    d->hubs.removeAt(d->hubs.indexOf(info));

    d->str_model->setStringList(d->hubs);
}

void SearchFrame::getParams(SearchFrame::VarMap &map, const dcpp::SearchResultPtr &ptr){
    map.clear();

    map["SIZE"]    = qulonglong(ptr->getSize());

    QString fname = _q(ptr->getFile());
    const QStringList &fname_parts = fname.split('\\', QString::SkipEmptyParts);

    map["FILE"] = fname_parts.isEmpty()? fname : fname_parts.last();

    if (ptr->getType() == SearchResult::TYPE_FILE){
        map["TTH"]  = _q(ptr->getTTH().toBase32());

        QString path = fname.left(fname.lastIndexOf("\\"));

        if (!path.endsWith("\\"))
            path += "\\";

        map["PATH"] = path;
        map["ISDIR"]   = false;
    }
    else{
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
    Q_D(SearchFrame);
    static SearchBlacklist *SB = SearchBlacklist::getInstance();

    try {
        if (SB->ok(map["FILE"].toString(), SearchBlacklist::NAME) && SB->ok(map["TTH"].toString(), SearchBlacklist::TTH)){
            if (d->model->addResult(map["FILE"].toString(),
                                    map["SIZE"].toULongLong(),
                                    map["TTH"].toString(),
                                    map["PATH"].toString(),
                                    map["NICK"].toString(),
                                    map["FSLS"].toULongLong(),
                                    map["ASLS"].toULongLong(),
                                    map["IP"].toString(),
                                    map["HUB"].toString(),
                                    map["HOST"].toString(),
                                    map["CID"].toString(),
                                    map["ISDIR"].toBool()))
                d->results++;
        }
    }
    catch (const SearchListException&){}
}

void SearchFrame::searchAlternates(const QString &tth){
    if (tth.isEmpty())
        return;

    Q_D(SearchFrame);

    lineEdit_SEARCHSTR->setText(tth);
    comboBox_FILETYPES->setCurrentIndex(SearchManager::TYPE_TTH);
    lineEdit_SIZE->setText("");

    slotStartSearch();

    d->saveFileType = false;
}

void SearchFrame::searchFile(const QString &file){
    if (file.isEmpty())
        return;

    Q_D(SearchFrame);

    lineEdit_SEARCHSTR->setText(file);
    comboBox_FILETYPES->setCurrentIndex(SearchManager::TYPE_ANY);
    lineEdit_SIZE->setText("");

    d->saveFileType = false;

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
    if (qobject_cast<QPushButton*>(sender()) != pushButton_SEARCH){
        pushButton_SEARCH->click(); //Generating clicked() signal that shows pushButton_STOP button.
                                    //Anybody can suggest something better?
        return;
    }

    Q_D(SearchFrame);

    d->stop = false;

    if (lineEdit_SEARCHSTR->text().trimmed().isEmpty())
        return;

    MainWindow *MW = MainWindow::getInstance();
    QString s = lineEdit_SEARCHSTR->text().trimmed();
    StringList clients;

    for (int i = 0; i < d->str_model->rowCount(); i++){
        QModelIndex index = d->str_model->index(i, 0);

        if (index.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            clients.push_back(_tq(index.data().toString()));
    }

#ifndef WITH_DHT
    if (clients.empty())
        return;
#endif

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

    if (!d->searchHistory.contains(s)){
        bool isTTH = WulforUtil::isTTH(s);

        if ((WBGET("memorize-tth-search-phrases", false) && isTTH) || !isTTH)
            d->searchHistory.push_front(s);

        QMenu *m = new QMenu();

        for (const auto &s : d->searchHistory)
            m->addAction(s);

        lineEdit_SEARCHSTR->setMenu(m);

        uint maxItemsNumber = WIGET("search-history-items-number", 10);
        while (d->searchHistory.count() > maxItemsNumber)
                d->searchHistory.removeLast();

        QString hist = d->searchHistory.join("\n");
        WSSET(WS_SEARCH_HISTORY, hist.toUtf8().toBase64());
    }

    {
        d->currentSearch = StringTokenizer<tstring>(s.toStdString(), ' ').getTokens();
        s = "";

        //strip out terms beginning with -
        for (auto si = d->currentSearch.begin(); si != d->currentSearch.end(); ) {
            if(si->empty()) {
                si = d->currentSearch.erase(si);
                continue;
            }

            if ((*si)[0] != '-')
                s += QString::fromStdString(*si) + ' ';

            ++si;
        }

        d->token = _q(Util::toString(Util::rand()));
    }

    SearchManager::SizeModes searchMode((SearchManager::SizeModes)comboBox_SIZETYPE->currentIndex());

    if(!llsize || lineEdit_SIZE->text() == "")
        searchMode = SearchManager::SIZE_DONTCARE;

    int ftype = comboBox_FILETYPES->currentIndex();

    d->isHash = (ftype == SearchManager::TYPE_TTH);
    d->filterShared = static_cast<AlreadySharedAction>(comboBox_SHARED->currentIndex());
    d->withFreeSlots = checkBox_FILTERSLOTS->isChecked();

    d->model->setFilterRole(static_cast<int>(d->filterShared));
    d->model->clearModel();

    d->dropped = d->results = 0;

    string ftypeStr;
    if (ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_LAST)
        ftypeStr = SearchManager::getInstance()->getTypeStr(ftype);
    else
    {
        ftypeStr = _tq(lineEdit_SEARCHSTR->text());
        ftype = SearchManager::TYPE_ANY;
    }

    StringList exts;
    try{
        if (ftype == SearchManager::TYPE_ANY){
            // Custom searchtype
            exts = SettingsManager::getInstance()->getExtensions(ftypeStr);
        }
        else if ((ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_DIRECTORY) || ftype == SearchManager::TYPE_CD_IMAGE){
            // Predefined searchtype
            exts = SettingsManager::getInstance()->getExtensions(string(1, '0' + ftype));
        }
    }
    catch (const SearchTypeException&){
        ftype = SearchManager::TYPE_ANY;
    }

    d->target = s;
    d->searchStartTime = GlobalTimer::getInstance()->getTicks()*1000;

    uint64_t maxDelayBeforeSearch = SearchManager::getInstance()->search(clients, s.toStdString(), llsize, (SearchManager::TypeModes)ftype, searchMode, d->token.toStdString(), exts, (void*)this);
    uint64_t waitingResultsTime = 20000; // just assumption that user receives most of results in 20 seconds

    d->searchEndTime = d->searchStartTime + maxDelayBeforeSearch + waitingResultsTime;
    d->waitingResults = true;

    if (!checkBox_HIDEPANEL->isChecked()){
        QList<int> panes = splitter->sizes();

        panes[1] = panes[0] + panes[1];

        d->left_pane_old_size = panes[0] > 15 ? panes[0] : d->left_pane_old_size;

        panes[0] = 0;

        splitter->setSizes(panes);
    }

    d->arena_title = tr("Search - %1").arg(s);

    lineEdit_SEARCHSTR->setEnabled(false);

    MW->redrawToolPanel();
}

void SearchFrame::slotClear(){
    Q_D(SearchFrame);

    treeView_RESULTS->clearSelection();
    d->model->clearModel();
    lineEdit_SEARCHSTR->clear();
    lineEdit_SIZE->setText("");

    d->dropped = d->results = 0;
}

void SearchFrame::slotResultDoubleClicked(const QModelIndex &index){
    if (!index.isValid() || !index.internalPointer())
        return;

    Q_D(SearchFrame);

    QModelIndex i = d->proxy? d->proxy->mapToSource(index) : index;

    SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
    VarMap params;

    if (getDownloadParams(params, item)){
        download(params);

        if (item->childCount() > 0 && !SETTING(DONT_DL_ALREADY_QUEUED)){//download all child items
            QString fname = params["FNAME"].toString();

            for (const auto &i : item->childItems){
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
    Q_D(SearchFrame);

    if (list.size() < 1)
        return;

    if (d->proxy)
        std::transform(list.begin(), list.end(), list.begin(), [&d](QModelIndex i) { return d->proxy->mapToSource(i); } );

    if (!Menu::getInstance())
        Menu::newInstance();

    QStringList hubs;

    for (const auto &i : list){
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
            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item)){
                    download(params);

                    if (item->childCount() > 0 && !SETTING(DONT_DL_ALREADY_QUEUED)){//download all child items
                        QString fname = params["FNAME"].toString();

                        for (const auto &i : item->childItems){
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

            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item)){
                    params["TARGET"] = target;
                    download(params);

                    if (item->childCount() > 0 && !SETTING(DONT_DL_ALREADY_QUEUED)){//download all child items
                        QString fname = params["FNAME"].toString();

                        for (const auto  &i : item->childItems){
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
            for (const auto &i : list){
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

            for (const auto &i : list){
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
            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                if (!item->isDir){//only one file
                    SearchFrame *sf = ArenaWidgetFactory().create<SearchFrame>();

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

            for (const auto &i : list){
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
        case Menu::MagnetWeb:
        {
            QString magnets = "";
            WulforUtil *WU = WulforUtil::getInstance();

            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                if (!item->isDir){//only files
                    qlonglong size = item->data(COLUMN_SF_ESIZE).toLongLong();
                    QString tth = item->data(COLUMN_SF_TTH).toString();
                    QString name = item->data(COLUMN_SF_FILENAME).toString();

                    QString magnet = "[magnet=\"" + WU->makeMagnet(name, size, tth) + "\"]"+name+"[/magnet]";

                    if (!magnet.isEmpty())
                        magnets += magnet + "\n";
                }
            }

            magnets = magnets.trimmed();

            if (!magnets.isEmpty())
                qApp->clipboard()->setText(magnets, QClipboard::Clipboard);

            break;
        }
        case Menu::MagnetInfo:
        {
            WulforUtil *WU = WulforUtil::getInstance();

            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                if (!item->isDir){//only files
                    qlonglong size = item->data(COLUMN_SF_ESIZE).toLongLong();
                    QString tth = item->data(COLUMN_SF_TTH).toString();
                    QString name = item->data(COLUMN_SF_FILENAME).toString();

                    QString magnet = WU->makeMagnet(name, size, tth);

                    if (!magnet.isEmpty()){
                        Magnet m(this);
                        m.setLink(magnet + "\n");
                        m.exec();
                    }
                }
            }

            break;
        }
        case Menu::Browse:
        {
            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getWholeDirParams(params, item))
                    getFileList(params, false);
            }

            break;
        }
        case Menu::MatchQueue:
        {
            for (const auto &i : list){
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

            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                QString hubUrl = item->data(COLUMN_SF_HOST).toString();
                dcpp::CID cid(_tq(item->cid));

                fr = qobject_cast<HubFrame*>(HubManager::getInstance()->getHub(hubUrl));

                if (fr)
                    fr->createPMWindow(cid);
            }

            break;
        }
        case Menu::AddToFav:
        {
            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    addToFav(params["CID"].toString());

            }

            break;
        }
        case Menu::GrantExtraSlot:
        {
             for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    grant(params);

            }

            break;
        }
        case Menu::RemoveFromQueue:
        {
             for (const auto &i : list){
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

             for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                d->model->removeItem(item);

                d->model->repaint();
            }

            break;
        }
        case Menu::UserCommands:
        {
            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());

                int id = Menu::getInstance()->getCommandId();

                UserCommand uc;
                if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
                    break;

                StringMap params;

                if (WulforUtil::getInstance()->getUserCommandParams(uc, params)){
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

                        string hubUrl = _tq(i.data(COLUMN_SF_HOST).toString());

                        ClientManager::getInstance()->userCommand(HintedUser(user, hubUrl), uc, params, true);
                    }

                }
            }

            break;
        }
        case Menu::Blacklist:
        {
            SearchBlackListDialog dlg(this);

            dlg.exec();

            break;
        }
        case Menu::AddToBlacklist:
        {
            QList <QString> new_inst;

            for (const auto &i : list){
                SearchItem *item = reinterpret_cast<SearchItem*>(i.internalPointer());
                VarMap params;

                if (getDownloadParams(params, item))
                    new_inst << item->data(COLUMN_SF_FILENAME).toString();
            }
            if (!new_inst.isEmpty()){
                static SearchBlacklist *SB = SearchBlacklist::getInstance();
                QList <QString> list = SB->getList(SearchBlacklist::NAME);
                list.append(new_inst);
                SB->setList(SearchBlacklist::NAME, list);
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
    Q_D(SearchFrame);

    if (d->waitingResults) {
        uint64_t now = GlobalTimer::getInstance()->getTicks()*1000;
        float fraction  = 100.0f*(now - d->searchStartTime)/(d->searchEndTime - d->searchStartTime);
        if (fraction >= 100.0) {
            fraction = 100.0;
            d->waitingResults = false;
        }
        QString msg = tr("Searching for %1 ...").arg(d->target);
        progressBar->setFormat(msg);
        progressBar->setValue(static_cast<unsigned>(fraction));
    } else {
        QString msg = "";
        progressBar->setFormat(msg);
        progressBar->setValue(0);
        lineEdit_SEARCHSTR->setEnabled(true);
    }

    if (d->dropped == d->results && !d->dropped){

        if (d->currentSearch.empty())
            frame_PROGRESS->hide();
        else {
            frame_PROGRESS->show();

            QString text = QString(tr("<b>No results</b>"));

            status->setText(text);
        }
    }
    else {
        if (!frame_PROGRESS->isVisible())
            frame_PROGRESS->show();

        QString text = QString(tr("Found: <b>%1</b>  Dropped: <b>%2</b>")).arg(d->results).arg(d->dropped);

        status->setText(text);
    }
}

void SearchFrame::slotToggleSidePanel(){
    QList<int> panes = splitter->sizes();
    Q_D(SearchFrame);

    if (panes[0] < 15){//left pane can't have width less than 15px
        panes[0] = d->left_pane_old_size;
        panes[1] = panes[1] - d->left_pane_old_size;
    }
    else {
        panes[1] = panes[0] + panes[1];
        d->left_pane_old_size = panes[0];
        panes[0] = 0;
    }

    splitter->setSizes(panes);
}

void SearchFrame::slotFilter(){
    Q_D(SearchFrame);

    if (frame_FILTER->isVisible()){
        treeView_RESULTS->setModel(d->model);

        disconnect(lineEdit_FILTER, SIGNAL(textChanged(QString)), d->proxy, SLOT(setFilterFixedString(QString)));
    }
    else {
        d->proxy = (d->proxy? d->proxy : (new SearchProxyModel(this)));
        d->proxy->setDynamicSortFilter(true);
        d->proxy->setFilterFixedString(lineEdit_FILTER->text());
        d->proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        d->proxy->setFilterKeyColumn(comboBox_FILTERCOLUMNS->currentIndex());
        d->proxy->setSourceModel(d->model);

        treeView_RESULTS->setModel(d->proxy);

        connect(lineEdit_FILTER, SIGNAL(textChanged(QString)), d->proxy, SLOT(setFilterFixedString(QString)));

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
    Q_D(SearchFrame);

    if (d->proxy)
        d->proxy->setFilterKeyColumn(col);
}

void SearchFrame::slotSettingsChanged(const QString &key, const QString &value){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void SearchFrame::on(SearchManagerListener::SR, const dcpp::SearchResultPtr& aResult) noexcept {
    Q_D(SearchFrame);

    if (d->currentSearch.empty() || !aResult || d->stop == true)
        return;

    if (!aResult->getToken().empty() && d->token != _q(aResult->getToken())){
        d->dropped++;

        return;
    }

    if(d->isHash) {
        if(aResult->getType() != SearchResult::TYPE_FILE || TTHValue(Text::fromT(d->currentSearch[0])) != aResult->getTTH()) {
            d->dropped++;

            return;
        }
    }
    else {
        for (const auto &j : d->currentSearch) {
            if((*j.begin() != ('-') && Util::findSubString(aResult->getFile(), j) == tstring::npos) ||
               (*j.begin() == ('-') && j.size() != 1 && Util::findSubString(aResult->getFile(), j.substr(1)) != tstring::npos)
              )
           {
                    d->dropped++;

                    return;
           }
        }
    }

    if (d->filterShared == Filter && aResult->getType() == SearchResult::TYPE_FILE){
        const TTHValue& t = aResult->getTTH();

        if (ShareManager::getInstance()->isTTHShared(t)) {
            d->dropped++;

            return;
        }
    }

    if (d->withFreeSlots && !aResult->getFreeSlots()){
        d->dropped++;

        return;
    }

    QMap<QString, QVariant> map;
    getParams(map, aResult);

    emit coreSR(map);
}

void SearchFrame::slotClose() {
    ArenaWidgetManager::getInstance()->rem(this);
}

void SearchFrame::on(ClientConnected, Client* c) noexcept{
    emit coreClientConnected(_q(c->getHubUrl()));
}

void SearchFrame::on(ClientUpdated, Client* c) noexcept{
    emit coreClientUpdated((_q(c->getHubUrl())));
}

void SearchFrame::on(ClientDisconnected, Client* c) noexcept{
    emit coreClientDisconnected((_q(c->getHubUrl())));
}

void SearchFrame::slotStopSearch(){
    Q_D(SearchFrame);

    d->stop = true;
    d->waitingResults = false;
}
