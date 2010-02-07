#include "TransferView.h"
#include "TransferViewModel.h"
#include "WulforUtil.h"
#include "WulforManager.h"
#include "WulforSettings.h"
#include "Func.h"
#include "IPFilter.h"
#include "HubFrame.h"
#include "HubManager.h"

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

#include <QItemSelectionModel>
#include <QModelIndex>

TransferView::Menu::Menu():
        menu(NULL)
{
    WulforUtil *WU = WulforUtil::getInstance();
    menu = new QMenu();

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

    QAction *sep2       = new QAction(menu);
    sep2->setSeparator(true);

    QAction *u_c        = new QAction(tr("User command"), menu);
    u_c->setEnabled(false);

    QAction *sep3       = new QAction(menu);
    sep3->setSeparator(true);

    QAction *force    = new QAction(tr("Force attempt"), menu);
    force->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));

    QAction *close = new QAction(tr("Close connection(s)"), menu);
    close->setIcon(WU->getPixmap(WulforUtil::eiCONNECT_NO));

    actions.insert(browse, Browse);
    actions.insert(match, MatchQueue);
    actions.insert(send_pm, SendPM);
    actions.insert(add_to_fav, AddToFav);
    actions.insert(grant, GrantExtraSlot);
    actions.insert(rem_queue, RemoveFromQueue);
    actions.insert(u_c, UserCommand);
    actions.insert(force, Force);
    actions.insert(close, Close);

    menu->addActions(QList<QAction*>() << browse
                                       << match
                                       << send_pm
                                       << add_to_fav
                                       << grant
                                       << sep1
                                       << rem_queue
                                       << sep2
                                       << u_c
                                       << sep3
                                       << force
                                       << close
                                       );
}

TransferView::Menu::~Menu(){
    delete menu;
}

TransferView::Menu::Action TransferView::Menu::exec(){
    QAction *ret = menu->exec(QCursor::pos());

    if (actions.contains(ret))
        return actions.value(ret);

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

void TransferView::customEvent(QEvent *e){
    if (e->type() == TransferViewCustomEvent::Event){
        TransferViewCustomEvent *u_e = reinterpret_cast<TransferViewCustomEvent*>(e);

        u_e->func()->call();
    }

    e->accept();
}

void TransferView::save(){
    unsigned columnMap = 0;
    QString columnWidths;

    for (unsigned i = 0; i < model->columnCount(); i++){
        if (!treeView_TRANSFERS->isColumnHidden(i))
            columnMap |= (1 << i);

        columnWidths += QString().setNum(treeView_TRANSFERS->columnWidth(i)) + ",";
    }

    WSSET(WS_TRANSFERS_COLUMN_WIDTHS, columnWidths);
    WISET(WI_TRANSFER_COL_BITMAP, columnMap);
}

void TransferView::load(){
    unsigned columnMap = static_cast<unsigned>(WIGET(WI_TRANSFER_COL_BITMAP));
    int h = WIGET(WI_TRANSFER_HEIGHT);

    QStringList columnWidths = WSGET(WS_TRANSFERS_COLUMN_WIDTHS).split(",", QString::SkipEmptyParts);
    bool hasWidth = (columnWidths.size() == model->columnCount());

    for (unsigned i =0; i < model->columnCount(); i++){
        treeView_TRANSFERS->setColumnHidden(i, (columnMap & (1 << i)) == 0);

        if (hasWidth)
            treeView_TRANSFERS->setColumnWidth(i, columnWidths.at(i).toInt());
    }

    if (h >= 0)
        resize(this->width(), h);

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

    load();
}

void TransferView::getFileList(const QString &cid, const QString &host){
    if (cid.isEmpty() || host.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            QueueManager::getInstance()->addList(user, _tq(host), QueueItem::FLAG_CLIENT_VIEW);
    }
    catch (const Exception&){}
}

void TransferView::matchQueue(const QString &cid, const QString &host){
    if (cid.isEmpty() || host.isEmpty())
        return;

    try{
        dcpp::UserPtr user = ClientManager::getInstance()->getUser(CID(_tq(cid)));

        if (user)
            QueueManager::getInstance()->addList(user, _tq(host), QueueItem::FLAG_MATCH_QUEUE);
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
            UploadManager::getInstance()->reserveSlot(user, _tq(host));
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

void TransferView::getParams(TransferView::VarMap &params, const dcpp::ConnectionQueueItem *item){
    const dcpp::UserPtr &user = item->getUser();
    WulforUtil *WU = WulforUtil::getInstance();

    params.clear();

    params["CID"]   = _q(user->getCID().toBase32());
    params["USER"]  = WU->getNicks(user->getCID());
    params["HUB"]   = WU->getHubNames(user);
    params["FAIL"]  = false;
    params["HOST"]  = _q(item->getHubHint());
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

    params["USER"]  = WU->getNicks(user->getCID());
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
}

void TransferView::slotContextMenu(const QPoint &){
    QItemSelectionModel *selection_model = treeView_TRANSFERS->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);

    if (list.size() < 1)
        return;

    QList<TransferViewItem*> items;

    foreach (QModelIndex index, list){
        TransferViewItem *i = reinterpret_cast<TransferViewItem*>(index.internalPointer());

        if (i->childCount() > 0){
            foreach(TransferViewItem *child, i->childItems)
                items.append(child);
        }
        else if (!items.contains(i))
            items.append(i);
    }

    if (items.size() < 1)
        return;

    Menu::Action act;

    {
        Menu m;
        act = m.exec();
    }

    switch (act){

    case Menu::None:
    {
        break;
    }
    case Menu::Browse:
    {
        foreach(TransferViewItem *i, items)
            getFileList(i->cid, vstr(i->data(COLUMN_TRANSFER_HOST)));

        break;
    }
    case Menu::MatchQueue:
    {
        foreach(TransferViewItem *i, items)
            matchQueue(i->cid, vstr(i->data(COLUMN_TRANSFER_HOST)));

        break;
    }
    case Menu::AddToFav:
    {
        foreach(TransferViewItem *i, items)
            addFavorite(i->cid);

        break;
    }
    case Menu::GrantExtraSlot:
    {
        foreach(TransferViewItem *i, items)
            grantSlot(i->cid, vstr(i->data(COLUMN_TRANSFER_HOST)));

        break;
    }
    case Menu::RemoveFromQueue:
    {
        foreach(TransferViewItem *i, items)
            removeFromQueue(i->cid);

        break;
    }
    case Menu::Force:
    {
        foreach(TransferViewItem *i, items)
            forceAttempt(i->cid);

        break;
    }
    case Menu::Close:
    {
        foreach(TransferViewItem *i, items)
            closeConection(i->cid, i->download);

        break;
    }
    case Menu::UserCommand:
    {
#warning "Implement user commands"
        break;
    }
    case Menu::SendPM:
    {
        HubFrame *fr = NULL;

        foreach(TransferViewItem *i, items){
            dcpp::CID cid(_tq(i->cid));
            QString hubUrl = i->data(COLUMN_TRANSFER_HOST).toString();

            fr = HubManager::getInstance()->getHub(hubUrl);

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
    QMenu * mcols = new QMenu(this);
    QAction * column;
    int index;

    for (int i = 0; i < model->columnCount(); ++i) {
        index = treeView_TRANSFERS->header()->logicalIndex(i);
        column = mcols->addAction(model->headerData(index, Qt::Horizontal).toString());
        column->setCheckable(true);

        column->setChecked(!treeView_TRANSFERS->header()->isSectionHidden(index));
        column->setData(index);
    }

    QAction * chosen = mcols->exec(QCursor::pos());

    if (chosen) {
        index = chosen->data().toInt();

        if (treeView_TRANSFERS->header()->isSectionHidden(index)) {
            treeView_TRANSFERS->header()->showSection(index);
        } else {
            treeView_TRANSFERS->header()->hideSection(index);
        }
    }

    delete mcols;
}

void TransferView::on(dcpp::DownloadManagerListener::Requesting, dcpp::Download* dl) throw(){
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

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::initTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::DownloadManagerListener::Starting, dcpp::Download* dl) throw(){
    VarMap params;

    getParams(params, dl);

    params["STAT"] = tr("Download starting...");
    params["FPOS"]  = (qlonglong)QueueManager::getInstance()->getPos(dl->getPath());

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::DownloadManagerListener::Tick, const dcpp::DownloadList& dls) throw(){
    for (DownloadList::const_iterator it = dls.begin(); it != dls.end(); ++it){
        Download* dl = *it;
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

        str += " " + QString(tr("Downloaded %1")).arg(_q(Util::formatBytes(dl->getPos())))
            + QString(tr(" (%1%) in ")).arg(vdbl(params["PERC"]), 0, 'f', 1)
            + _q(Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000));

        params["STAT"] = str;

        typedef Func1<TransferViewModel, VarMap> FUNC;
        FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

        QApplication::postEvent(this, new TransferViewCustomEvent(f));
    }
}

void TransferView::on(dcpp::DownloadManagerListener::Complete, dcpp::Download* dl) throw(){
    VarMap params;

    getParams(params, dl);

    params["STAT"]  = tr("Download complete...");
    params["SPEED"] = 0;

    qint64 pos = QueueManager::getInstance()->getPos(dl->getPath()) + dl->getPos();

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC  *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    typedef Func2<TransferViewModel, VarMap, qint64> FUNC1;
    FUNC1 *f1= new FUNC1(model, &TransferViewModel::updateTransferPos, params, pos);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
    QApplication::postEvent(this, new TransferViewCustomEvent(f1));
}

void TransferView::on(dcpp::DownloadManagerListener::Failed, dcpp::Download* dl, const std::string& reason) throw(){
    VarMap params;

    getParams(params, dl);

    params["STAT"]  = _q(reason);
    params["SPEED"] = 0;
    params["FAIL"]  = true;
    params["TLEFT"] = -1;

    qint64 pos = QueueManager::getInstance()->getPos(dl->getPath()) + dl->getPos();

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC  *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    typedef Func2<TransferViewModel, VarMap, qint64> FUNC1;
    FUNC1 *f1= new FUNC1(model, &TransferViewModel::updateTransferPos, params, pos);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
    QApplication::postEvent(this, new TransferViewCustomEvent(f1));
}

void TransferView::on(dcpp::ConnectionManagerListener::Added, dcpp::ConnectionQueueItem* cqi) throw(){
    VarMap params;

    getParams(params, cqi);

    params["STAT"] = tr("Connecting...");

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::addConnection, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::ConnectionManagerListener::Connected, dcpp::ConnectionQueueItem* cqi) throw(){
    VarMap params;

    getParams(params, cqi);

    params["STAT"] = tr("Connected");

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::ConnectionManagerListener::Removed, dcpp::ConnectionQueueItem* cqi) throw(){
    VarMap params;

    getParams(params, cqi);

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::removeTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::ConnectionManagerListener::Failed, dcpp::ConnectionQueueItem* cqi, const std::string &reason) throw(){
    VarMap params;

    getParams(params, cqi);

    params["STAT"] = _q(reason);
    params["FAIL"] = true;
    params["SPEED"] = (qlonglong)0;
    params["TLEFT"] = -1;

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::ConnectionManagerListener::StatusChanged, dcpp::ConnectionQueueItem* cqi) throw(){
    VarMap params;
    getParams(params, cqi);

    if (cqi->getState() == ConnectionQueueItem::CONNECTING)
        params["STAT"] = tr("Connecting");
    else if (cqi->getState() == ConnectionQueueItem::NO_DOWNLOAD_SLOTS)
        params["STAT"] = tr("No download slots");
    else
        params["STAT"] = tr("Waiting to retry");

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem* qi, const std::string&, int64_t) throw(){
    VarMap params;
    params["TARGET"] = _q(qi->getTarget());

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::finishParent, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem* qi) throw(){
    VarMap params;
    params["TARGET"] = _q(qi->getTarget());

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::finishParent, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::UploadManagerListener::Starting, dcpp::Upload* ul) throw(){
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

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::UploadManagerListener::Tick, const dcpp::UploadList& uls) throw(){
    for (UploadList::const_iterator it = uls.begin(); it != uls.end(); ++it){
        Upload* ul = *it;
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

        stat += QString(tr(" Uploaded %1 (%2%) ")).arg(_q(Util::formatBytes(ul->getPos()))).arg(vdbl(params["PERC"]), 0, 'f', 1)
             += QString(tr("in %1")).arg(_q(Util::formatSeconds((GET_TICK() - ul->getStart()) / 1000)));

        params["STAT"] = stat;
        params["DOWN"] = false;
        params["FAIL"] = false;

        typedef Func1<TransferViewModel, VarMap> FUNC;
        FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

        QApplication::postEvent(this, new TransferViewCustomEvent(f));
    }
}

void TransferView::on(dcpp::UploadManagerListener::Complete, dcpp::Upload* ul) throw(){
    VarMap params;

    getParams(params, ul);

    params["STAT"] = tr("Upload complete");
    params["DOWN"] = false;
    params["FAIL"] = false;

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}

void TransferView::on(dcpp::UploadManagerListener::Failed, dcpp::Upload* ul, const std::string& reason) throw(){
    VarMap params;

    getParams(params, ul);

    params["STAT"] = tr("Upload failed");
    params["DOWN"] = false;
    params["FAIL"] = false;

    typedef Func1<TransferViewModel, VarMap> FUNC;
    FUNC *f = new FUNC(model, &TransferViewModel::updateTransfer, params);

    QApplication::postEvent(this, new TransferViewCustomEvent(f));
}
