/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "TransferView.h"
#include "TransferViewModel.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "Notification.h"
#include "SearchFrame.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
#include "ArenaWidgetFactory.h"

#include "dcpp/Util.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/Transfer.h"
#include "dcpp/Download.h"
#include "dcpp/QueueItem.h"
#include "dcpp/Upload.h"
#include "dcpp/ClientManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/HashManager.h"

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QClipboard>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

TransferView::Menu::Menu(bool showTransferredFilesOnly):
        menu(NULL),
        selectedColumn(0)
{
    WulforUtil *WU = WulforUtil::getInstance();
    menu = new QMenu();

    QAction *browse     = new QAction(tr("Browse files"), menu);
    browse->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));

    QAction *search     = new QAction(tr("Search Alternates"), menu);
    search->setIcon(WU->getPixmap(WulforUtil::eiFIND));

    QAction *match      = new QAction(tr("Match Queue"), menu);
    match->setIcon(WU->getPixmap(WulforUtil::eiDOWN));

    QAction *send_pm    = new QAction(tr("Send Private Message"), menu);
    send_pm->setIcon(WU->getPixmap(WulforUtil::eiMESSAGE));

    QAction *add_to_fav = new QAction(tr("Add to favorites"), menu);
    add_to_fav->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));

    QAction *grant      = new QAction(tr("Grant extra slot"), menu);
    grant->setIcon(WU->getPixmap(WulforUtil::eiEDITADD));

    copy_column = new QMenu(tr("Copy"), menu);
    copy_column->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));

    copy_column->addAction(tr("Users"));
    copy_column->addAction(tr("Speed"));
    copy_column->addAction(tr("Status"));
    copy_column->addAction(tr("Flags"));
    copy_column->addAction(tr("Size"));
    copy_column->addAction(tr("Time left"));
    copy_column->addAction(tr("Filename"));
    copy_column->addAction(tr("Hub"));
    copy_column->addAction(tr("IP"));
    copy_column->addAction(tr("Encryption"));
    copy_column->addAction(tr("Magnet"));

    QAction *sep1        = new QAction(menu);
    sep1->setSeparator(true);

    QAction *rem_queue  = new QAction(tr("Remove Source"), menu);
    rem_queue->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));

    QAction *sep3       = new QAction(menu);
    sep3->setSeparator(true);

    QAction *force    = new QAction(tr("Force attempt"), menu);
    force->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));

    QAction *close = new QAction(tr("Close connection(s)"), menu);
    close->setIcon(WU->getPixmap(WulforUtil::eiCONNECT_NO));

    QAction *show_only_transferred_files = new QAction(tr("Show only transferred files"), menu);
    show_only_transferred_files->setCheckable(true);
    show_only_transferred_files->setChecked(showTransferredFilesOnly);

    actions.insert(browse, Browse);
    actions.insert(match, MatchQueue);
    actions.insert(send_pm, SendPM);
    actions.insert(add_to_fav, AddToFav);
    actions.insert(grant, GrantExtraSlot);
    actions.insert(rem_queue, RemoveFromQueue);
    actions.insert(force, Force);
    actions.insert(close, Close);
    actions.insert(search, SearchAlternates);
    actions.insert(show_only_transferred_files, showTransferredFieldsOnly);

    menu->addActions(QList<QAction*>() << browse
                                       << search
                                       << match
                                       << send_pm
                                       << add_to_fav
                                       << grant);
    menu->addMenu(copy_column);
    menu->addActions(QList<QAction*>() << sep1
                                       << rem_queue
                                       << sep3
                                       << force
                                       << close
                                       << show_only_transferred_files
                                       );
}

TransferView::Menu::~Menu(){
    menu->deleteLater();
    copy_column->deleteLater();
}

TransferView::Menu::Action TransferView::Menu::exec(){
    QAction *ret = menu->exec(QCursor::pos());

    if (actions.contains(ret))
        return actions.value(ret);
    else if (ret){
        selectedColumn = copy_column->actions().indexOf(ret);

        return Copy;
    }

    return None;
}

TransferView::TransferView(QWidget *parent):
        QWidget(parent),
        model(NULL)
{
    setupUi(this);

    init();

    QueueManager::getInstance()->addListener(this);
    DownloadManager::getInstance()->addListener(this);
    UploadManager::getInstance()->addListener(this);
    ConnectionManager::getInstance()->addListener(this);
}

TransferView::~TransferView(){
    QueueManager::getInstance()->removeListener(this);
    DownloadManager::getInstance()->removeListener(this);
    UploadManager::getInstance()->removeListener(this);
    ConnectionManager::getInstance()->removeListener(this);

    delete model;
}

void TransferView::closeEvent(QCloseEvent *e){
    save();

    e->accept();
}

void TransferView::resizeEvent(QResizeEvent *e){
    e->accept();

    if (isVisible())
        WISET(WI_TRANSFER_HEIGHT, height());
}

void TransferView::hideEvent(QHideEvent *e){
    save();

    WISET(WI_TRANSFER_HEIGHT, height());

    e->accept();
}

void TransferView::save(){
    WSSET(WS_TRANSFERS_STATE, treeView_TRANSFERS->header()->saveState().toBase64());
}

void TransferView::load(){
    int h = WIGET(WI_TRANSFER_HEIGHT);

    if (h >= 0)
        resize(this->width(), h);

    treeView_TRANSFERS->header()->restoreState(QByteArray::fromBase64(WSGET(WS_TRANSFERS_STATE).toUtf8()));
}

QSize TransferView::sizeHint() const{
    int h = WIGET(WI_TRANSFER_HEIGHT);

    if (h > 0)
        return QSize(300, h);

    return QSize(300, 250);
}

void TransferView::init(){
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    model = new TransferViewModel();

    treeView_TRANSFERS->setModel(model);
    treeView_TRANSFERS->setItemDelegate(new TransferViewDelegate(this));
    treeView_TRANSFERS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_TRANSFERS->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(treeView_TRANSFERS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(treeView_TRANSFERS->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));

    connect(this, SIGNAL(coreDMRequesting(VarMap)),     model, SLOT(initTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreDMStarting(VarMap)),       model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreDMTick(VarMap)),           model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUpdateParents()),          model, SLOT(updateParents()), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUpdateParents()),          model, SLOT(sort()), Qt::QueuedConnection);
    connect(this, SIGNAL(coreDMComplete(VarMap)),       model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUpdateTransferPosition(VarMap,qint64)), model, SLOT(updateTransferPos(VarMap,qint64)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreDMFailed(VarMap)),         model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreCMAdded(VarMap)),          model, SLOT(addConnection(VarMap)));
    connect(this, SIGNAL(coreCMConnected(VarMap)),      model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreCMRemoved(VarMap)),        model, SLOT(removeTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreCMFailed(VarMap)),         model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreCMStatusChanged(VarMap)),  model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreQMFinished(VarMap)),       model, SLOT(finishParent(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreQMRemoved(VarMap)),        model, SLOT(finishParent(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreDownloadComplete(QString)), this, SLOT(downloadComplete(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUMStarting(VarMap)),       model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUMTick(VarMap)),           model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUMComplete(VarMap)),       model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUMFailed(VarMap)),         model, SLOT(updateTransfer(VarMap)), Qt::QueuedConnection);

    load();
}

void TransferView::getFileList(const QString &cid, const QString &host){
    if (cid.isEmpty() || host.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            QueueManager::getInstance()->addList(HintedUser(user, _tq(host)), QueueItem::FLAG_CLIENT_VIEW, "");
    }
    catch (const Exception&){}
}

void TransferView::matchQueue(const QString &cid, const QString &host){
    if (cid.isEmpty() || host.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            QueueManager::getInstance()->addList(HintedUser(user, _tq(host)), QueueItem::FLAG_MATCH_QUEUE, "");
    }
    catch (const Exception&){}
}

void TransferView::addFavorite(const QString &cid){
    if (cid.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            FavoriteManager::getInstance()->addFavoriteUser(user);
    }
    catch (const Exception&){}
}

void TransferView::grantSlot(const QString &cid, const QString &host){
    if (cid.isEmpty() || host.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            UploadManager::getInstance()->reserveSlot(HintedUser(user, _tq(host)));
    }
    catch (const Exception&){}
}

void TransferView::removeFromQueue(const QString &cid){
    if (cid.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
    }
    catch (const Exception&){}
}

void TransferView::forceAttempt(const QString &cid){
    if (cid.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            ConnectionManager::getInstance()->force(user);
    }
    catch (const Exception&){}
}

void TransferView::closeConection(const QString &cid, bool download){
    if (cid.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            ConnectionManager::getInstance()->disconnect(user, download);
    }
    catch (const Exception&){}
}

void TransferView::searchAlternates(const QString &tth){
    if (tth.isEmpty())
        return;

    SearchFrame *sfr = ArenaWidgetFactory().create<SearchFrame>();
    sfr->searchAlternates(tth);
}

void TransferView::downloadComplete(QString target){
    Notification::getInstance()->showMessage(Notification::TRANSFER, tr("Download complete"), target);
}

QString TransferView::getTTHFromItem(const TransferViewItem *item){
    QString tth_str = "";

    if (item->download)
        tth_str = item->tth;
    else {
        const TTHValue *tth = dcpp::HashManager::getInstance()->getFileTTHif(_tq(item->target));

        if (tth)
            tth_str = _q(tth->toBase32());
    }

    return tth_str;
}

void TransferView::getParams(TransferView::VarMap &params, const dcpp::ConnectionQueueItem *item){
    const dcpp::UserPtr &user = item->getUser();
    WulforUtil *WU = WulforUtil::getInstance();

    params["CID"]   = _q(user->getCID().toBase32());
    params["USER"]  = WU->getNicks(user->getCID());
    params["HUB"]   = WU->getHubNames(user);
    params["FAIL"]  = false;
    params["HOST"]  = _q(item->getUser().hint);
    params["DOWN"]  = item->getDownload();
}

void TransferView::getParams(TransferView::VarMap &params, const dcpp::Transfer *trf){
    const UserPtr& user = trf->getUser();
    WulforUtil *WU = WulforUtil::getInstance();
    double percent = 0.0;

    params["CID"]   = _q(user->getCID().toBase32());

    if (trf->getType() == Transfer::TYPE_PARTIAL_LIST || trf->getType() == Transfer::TYPE_FULL_LIST)
        params["FNAME"] = tr("File list");
    else if (trf->getType() == Transfer::TYPE_TREE)
        params["FNAME"] = tr("TTH: ") + _q(Util::getFileName(trf->getPath()));
    else
        params["FNAME"] = _q(Util::getFileName(trf->getPath()));

    QString nick = WU->getNicks(user->getCID());

    if (!nick.isEmpty())//Do not update user nick if user is offline
        params["USER"]  = nick;

    params["HUB"]   = WU->getHubNames(user);
    params["PATH"]  = _q(Util::getFilePath(trf->getPath()));
    params["ESIZE"] = (qlonglong)trf->getSize();
    params["DPOS"]  = (qlonglong)trf->getPos();
    params["SPEED"] = trf->getAverageSpeed();

    if (trf->getSize() > 0)
        percent = static_cast<double>(trf->getPos() * 100.0)/ trf->getSize();

    params["IP"]    = _q(trf->getUserConnection().getRemoteIp());
    params["TLEFT"] = qlonglong(trf->getSecondsLeft() > 0 ? trf->getSecondsLeft() : -1);
    params["TARGET"]= _q(trf->getPath());
    params["HOST"]  = _q(trf->getUserConnection().getHubUrl());
    params["PERC"]  = percent;
    params["DOWN"]  = true;
    params["TTH"] = _q(trf->getTTH().toBase32());
    if (trf->getUserConnection().isSecure())
    {
        params["ENCRYPTION"] = _q(trf->getUserConnection().getCipherName());
    }
    else
    {
        params["ENCRYPTION"] = _q("Plain");
    }
}

void TransferView::slotContextMenu(const QPoint &){
    QItemSelectionModel *selection_model = treeView_TRANSFERS->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);

    if (list.size() < 1)
        return;

    Menu::Action act;
    Menu m(model->getShowTranferedFilesOnlyState());

    act = m.exec();


    list = selection_model->selectedRows(0);

    if (list.size() < 1)
        return;

    QList<TransferViewItem*> items;

    for (const auto &index : list){
        TransferViewItem *i = reinterpret_cast<TransferViewItem*>(index.internalPointer());

        if (i->childCount() > 0){
            for (const auto &child : i->childItems)
                items.append(child);
        }
        else if (!items.contains(i))
            items.append(i);
    }

    if (items.size() < 1)
        return;

    switch (act){

    case Menu::None:
    {
        break;
    }
    case Menu::Browse:
    {
        for (const auto &i : items)
            getFileList(i->cid, vstr(i->data(COLUMN_TRANSFER_HOST)));

        break;
    }
    case Menu::SearchAlternates:
    {
        QStringList tths;
        QString tth_str = "";
        for (const auto &item : items) {
            tth_str = getTTHFromItem(item);
            if (!tth_str.isEmpty() && !tths.contains(tth_str)){
                tths.push_back(tth_str);
                searchAlternates(tth_str);
            }
        }

        break;
    }
    case Menu::MatchQueue:
    {
        for (const auto &i : items)
            matchQueue(i->cid, vstr(i->data(COLUMN_TRANSFER_HOST)));

        break;
    }
    case Menu::AddToFav:
    {
        for (const auto &i : items)
            addFavorite(i->cid);

        break;
    }
    case Menu::GrantExtraSlot:
    {
        for (const auto &i : items)
            grantSlot(i->cid, vstr(i->data(COLUMN_TRANSFER_HOST)));

        break;
    }
    case Menu::Copy:
    {
        int col = m.copyColumn();
        QString data = "";

        if (col <= (model->columnCount()-1)){
            for (const auto &i : items)
                data += i->data(col).toString() + "\n";
        }
        else {
            QString tth_str = "";
            for (const auto &i : items){
                QFileInfo fi(i->target);
                tth_str = getTTHFromItem(i);

                if (tth_str.isEmpty()) {
                    QString str = QDir::toNativeSeparators(fi.canonicalFilePath() ); // try to follow symlinks
                    const TTHValue *tth = HashManager::getInstance()->getFileTTHif(str.toStdString());

                    if (tth)
                        tth_str = _q(tth->toBase32());
                }

                if (!tth_str.isEmpty())
                    data += WulforUtil::getInstance()->makeMagnet(fi.fileName(), fi.size(), tth_str) + "\n";
            }
        }

        if (!data.isEmpty())
            qApp->clipboard()->setText(data, QClipboard::Clipboard);

        break;
    }
    case Menu::RemoveFromQueue:
    {
        for (const auto &i : items)
            removeFromQueue(i->cid);

        break;
    }
    case Menu::Force:
    {
        for (const auto &i : items)
            forceAttempt(i->cid);

        break;
    }
    case Menu::showTransferredFieldsOnly:
    {
        model->setShowTranferedFilesOnlyState(!model->getShowTranferedFilesOnlyState());
        break;
    }
    case Menu::Close:
    {
        for (const auto &i : items)
            closeConection(i->cid, i->download);

        break;
    }
    case Menu::SendPM:
    {
        HubFrame *fr = NULL;

        for (const auto &i : items){
            dcpp::CID cid(_tq(i->cid));
            QString hubUrl = i->data(COLUMN_TRANSFER_HOST).toString();

            fr = qobject_cast<HubFrame*>(HubManager::getInstance()->getHub(hubUrl));

            if (fr)
                fr->createPMWindow(cid);
        }

        break;
    }
    default:
        break;
    }
}

void TransferView::slotHeaderMenu(const QPoint &){
    WulforUtil::headerMenu(treeView_TRANSFERS);
}

void TransferView::on(dcpp::DownloadManagerListener::Requesting, dcpp::Download* dl) noexcept{
    VarMap params;

    getParams(params, dl);

    if (IPFilter::getInstance()){
        if (!IPFilter::getInstance()->OK(vstr(params["IP"]), eDIRECTION_IN)){
            closeConection(vstr(params["CID"]), true);
            return;
        }
    }

    params["ESIZE"] = (qlonglong)QueueManager::getInstance()->getSize(dl->getPath());
    params["FPOS"]  = (qlonglong)QueueManager::getInstance()->getPos(dl->getPath());
    params["STAT"]  = tr("Requesting");
    params["FAIL"]  = false;

    emit coreDMRequesting(params);
}

void TransferView::on(dcpp::DownloadManagerListener::Starting, dcpp::Download* dl) noexcept{
    VarMap params;

    getParams(params, dl);

    params["STAT"] = tr("Download starting...");
    params["FPOS"]  = (qlonglong)QueueManager::getInstance()->getPos(dl->getPath());

    emit coreDMStarting(params);
}

void TransferView::on(dcpp::DownloadManagerListener::Tick, const dcpp::DownloadList& dls) noexcept{
    for (const auto &it : dls){
        Download* dl = it;
        VarMap params;
        QString str;

        getParams(params, dl);
        params["FPOS"]  = (qlonglong)QueueManager::getInstance()->getPos(dl->getPath());

        if (dl->getUserConnection().isSecure())
        {
            if (dl->getUserConnection().isTrusted())
               str += tr("[S]");
            else
               str += tr("[U]");
        }

        if (dl->isSet(Download::FLAG_TTH_CHECK))
            str += tr("[T]");
        if (dl->isSet(Download::FLAG_ZDOWNLOAD))
            str += tr("[Z]");
        
        params["FLAGS"] = str;

        str = QString(tr("Downloaded %1")).arg(WulforUtil::formatBytes(dl->getPos()))
            + QString(tr(" (%1%)")).arg(vdbl(params["PERC"]), 0, 'f', 1);

        params["STAT"] = str;

        emit coreDMTick(params);
    }

    emit coreUpdateParents();
}

void TransferView::on(dcpp::DownloadManagerListener::Complete, dcpp::Download* dl) noexcept{
    VarMap params;

    getParams(params, dl);

    params["STAT"]  = tr("Download complete");
    params["SPEED"] = 0;

    qint64 pos = QueueManager::getInstance()->getPos(dl->getPath()) + dl->getPos();

    emit coreDMComplete(params);
    emit coreUpdateTransferPosition(params, pos);
}

void TransferView::on(dcpp::DownloadManagerListener::Failed, dcpp::Download* dl, const std::string& reason) noexcept {
    onFailed(dl, reason);
}

void TransferView::on(dcpp::QueueManagerListener::CRCFailed, dcpp::Download* dl, const std::string& reason) noexcept {
    onFailed(dl, reason);
}

void TransferView::onFailed(dcpp::Download* dl, const std::string& reason) {
    VarMap params;

    getParams(params, dl);

    params["STAT"]  = _q(reason);
    params["SPEED"] = 0;
    params["FAIL"]  = true;
    params["TLEFT"] = -1;

    qint64 pos = QueueManager::getInstance()->getPos(dl->getPath()) + dl->getPos();

    emit coreDMFailed(params);
    emit coreUpdateTransferPosition(params, pos);
}

void TransferView::on(dcpp::ConnectionManagerListener::Added, dcpp::ConnectionQueueItem* cqi) noexcept{
    VarMap params;

    getParams(params, cqi);

    params["FNAME"] = "";
    params["STAT"] = tr("Connecting...");

    if(cqi->getDownload()) {
        string aTarget; int64_t aSize; int aFlags = 0;
        if(QueueManager::getInstance()->getQueueInfo(cqi->getUser(), aTarget, aSize, aFlags)) {
            params["TARGET"] = _q(aTarget);
            params["ESIZE"] = (qlonglong)aSize;

            if (!aFlags)
                params["FNAME"] = _q(Util::getFileName(aTarget));

            params["BGROUP"] = !aFlags;
        }
    }

    emit coreCMAdded(params);
}

void TransferView::on(dcpp::ConnectionManagerListener::Connected, dcpp::ConnectionQueueItem* cqi) noexcept{

    VarMap params;

    getParams(params, cqi);

    params["STAT"] = tr("Connected");

    emit coreCMConnected(params);
}

void TransferView::on(dcpp::ConnectionManagerListener::Removed, dcpp::ConnectionQueueItem* cqi) noexcept{
    VarMap params;

    getParams(params, cqi);

    emit coreCMRemoved(params);
}

void TransferView::on(dcpp::ConnectionManagerListener::Failed, dcpp::ConnectionQueueItem* cqi, const std::string &reason) noexcept{
    VarMap params;

    getParams(params, cqi);

    params["STAT"] = _q(reason);
    params["FAIL"] = true;
    params["SPEED"] = (qlonglong)0;
    params["TLEFT"] = -1;

    emit coreCMFailed(params);
}

void TransferView::on(dcpp::ConnectionManagerListener::StatusChanged, dcpp::ConnectionQueueItem* cqi) noexcept{
    VarMap params;
    getParams(params, cqi);

    if (cqi->getState() == ConnectionQueueItem::CONNECTING)
        params["STAT"] = tr("Connecting");
    else if (cqi->getState() == ConnectionQueueItem::NO_DOWNLOAD_SLOTS)
        params["STAT"] = tr("No download slots");
    else
        params["STAT"] = tr("Waiting to retry");

    emit coreCMStatusChanged(params);
}

void TransferView::on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem* qi, const std::string&, int64_t) noexcept{
    VarMap params;
    params["TARGET"] = _q(qi->getTarget());

    emit coreQMFinished(params);

    if (!qi->isSet(QueueItem::FLAG_USER_LIST))//Do not show notify for filelists
        emit coreDownloadComplete(_q(qi->getTarget()).split(QDir::separator()).last());
}

void TransferView::on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem* qi) noexcept{
    VarMap params;
    params["TARGET"] = _q(qi->getTarget());

    emit coreQMRemoved(params);
}

void TransferView::on(dcpp::UploadManagerListener::Starting, dcpp::Upload* ul) noexcept{
    VarMap params;

    getParams(params, ul);

    if (IPFilter::getInstance()){
        if (!IPFilter::getInstance()->OK(vstr(params["IP"]), eDIRECTION_OUT)){
            closeConection(vstr(params["CID"]), false);
            return;
        }
    }

    params["STAT"] = tr("Upload starting...");
    params["DOWN"] = false;
    params["FAIL"] = false;

    emit coreUMStarting(params);
}

void TransferView::on(dcpp::UploadManagerListener::Tick, const dcpp::UploadList& uls) noexcept{
    for (const auto &it : uls){
        Upload* ul = it;
        VarMap params;
        QString stat = "";

        getParams(params, ul);

        if (ul->getUserConnection().isSecure())
        {
            if (ul->getUserConnection().isTrusted())
                stat += tr("[S]");
            else
                stat += tr("[U]");
        }
        if (ul->isSet(Upload::FLAG_ZUPLOAD))
            stat += tr("[Z]");
        
        params["FLAGS"] = stat;

        stat = QString(tr("Uploaded %1 (%2%) ")).arg(WulforUtil::formatBytes(ul->getPos())).arg(vdbl(params["PERC"]), 0, 'f', 1);

        params["STAT"] = stat;
        params["DOWN"] = false;
        params["FAIL"] = false;

        emit coreUMTick(params);
    }

    emit coreUpdateParents();
}

void TransferView::on(dcpp::UploadManagerListener::Complete, dcpp::Upload* ul) noexcept{
    VarMap params;

    getParams(params, ul);

    params["STAT"] = tr("Upload complete");
    params["DOWN"] = false;
    params["FAIL"] = false;

    emit coreUMComplete(params);
}

void TransferView::on(dcpp::UploadManagerListener::Failed, dcpp::Upload* ul, const std::string& reason) noexcept{
    VarMap params;

    getParams(params, ul);

    params["STAT"] = tr("Upload failed");
    params["DOWN"] = false;
    params["FAIL"] = false;

    emit coreUMFailed(params);
}
