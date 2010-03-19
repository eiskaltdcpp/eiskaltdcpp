#include "MainWindow.h"

#include <stdlib.h>
#include <string>
#include <iostream>
#ifdef FREE_SPACE_BAR
#include <boost/filesystem.hpp>
#endif
#include <QPushButton>
#include <QSize>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QtDebug>
#include <QTextCodec>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFileDialog>
#include <QProgressBar>
#include <QFileDialog>
#include <QRegExp>

#include "HubFrame.h"
#include "HubManager.h"
#include "HashProgress.h"
#include "TransferView.h"
#include "ShareBrowser.h"
#include "QuickConnect.h"
#include "SearchFrame.h"
#include "Settings.h"
#include "FavoriteHubs.h"
#include "FavoriteUsers.h"
#include "DownloadQueue.h"
#include "FinishedTransfers.h"
#include "AntiSpamFrame.h"
#include "IPFilterFrame.h"
#include "ToolBar.h"
#include "Magnet.h"
#include "SpyFrame.h"

#include "UPnPMapper.h"
#include "WulforSettings.h"
#include "WulforUtil.h"

#include "Version.h"

using namespace std;

MainWindow::MainWindow (QWidget *parent):
        QMainWindow(parent),
        statusLabel(NULL)
{
    arenaMap.clear();
    arenaWidgets.clear();

    init();

    retranslateUi();

    LogManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);

    startSocket();

    setStatusMessage(tr("Ready"));

    TransferView::newInstance();

    transfer_dock->setWidget(TransferView::getInstance());

    blockSignals(true);
    fileTransfers->setChecked(transfer_dock->isVisible());
    blockSignals(false);

    if (WBGET(WB_ANTISPAM_ENABLED)){
        AntiSpam::newInstance();

        AntiSpam::getInstance()->loadLists();
        AntiSpam::getInstance()->loadSettings();
    }

    if (WBGET(WB_IPFILTER_ENABLED)){
        IPFilter::newInstance();

        IPFilter::getInstance()->loadList();
    }

    QFont f;

    if (!WSGET(WS_APP_FONT).isEmpty() && f.fromString(WSGET(WS_APP_FONT)))
        qApp->setFont(f);

    if (!WSGET(WS_APP_THEME).isEmpty())
        qApp->setStyle(WSGET(WS_APP_THEME));
}

MainWindow::~MainWindow(){
    LogManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);

    if (AntiSpam::getInstance()){
        AntiSpam::getInstance()->saveLists();
        AntiSpam::getInstance()->saveSettings();
        AntiSpam::deleteInstance();
    }

    if (IPFilter::getInstance()){
        IPFilter::getInstance()->saveList();
        IPFilter::deleteInstance();
    }

    delete arena;

    delete fBar;
    delete tBar;

    qDeleteAll(fileMenuActions);
}

void MainWindow::closeEvent(QCloseEvent *c_e){
    if (!isUnload && WBGET(WB_NOTIFY_ENABLED)){
        hide();
        c_e->ignore();

        return;
    }

    saveSettings();

    blockSignals(true);

    if (TransferView::getInstance()){
        TransferView::getInstance()->close();
        TransferView::deleteInstance();
    }

    if (FavoriteHubs::getInstance()){
        FavoriteHubs::getInstance()->setUnload(true);
        FavoriteHubs::getInstance()->close();

        FavoriteHubs::deleteInstance();
    }

    if (FinishedDownloads::getInstance()){
        FinishedDownloads::getInstance()->setUnload(true);
        FinishedDownloads::getInstance()->close();

        FinishedDownloads::deleteInstance();
    }

    if (FinishedUploads::getInstance()){
        FinishedUploads::getInstance()->setUnload(true);
        FinishedUploads::getInstance()->close();

        FinishedUploads::deleteInstance();
    }

    if (FavoriteUsers::getInstance()){
        FavoriteUsers::getInstance()->setUnload(true);
        FavoriteUsers::getInstance()->close();

        FavoriteUsers::deleteInstance();
    }

    if (DownloadQueue::getInstance()){
        DownloadQueue::getInstance()->setUnload(true);
        DownloadQueue::getInstance()->close();

        DownloadQueue::deleteInstance();
    }

    if (SpyFrame::getInstance()){
        SpyFrame::getInstance()->setUnload(true);
        SpyFrame::getInstance()->close();

        SpyFrame::deleteInstance();
    }

    QMap< ArenaWidget*, QWidget* > map = arenaMap;
    QMap< ArenaWidget*, QWidget* >::iterator it = map.begin();

    for(; it != map.end(); ++it){
        if (arenaMap.contains(it.key()))//some widgets can autodelete itself from arena widgets
            it.value()->close();
    }

    c_e->accept();
}

void MainWindow::showEvent(QShowEvent *e){
    if (e->spontaneous())
        redrawToolPanel();

    HubFrame *fr = HubManager::getInstance()->activeHub();

    chatClear->setEnabled(fr == arena->widget());
    findInChat->setEnabled(fr == arena->widget());
    chatDisable->setEnabled(fr == arena->widget());

    e->accept();
}

void MainWindow::customEvent(QEvent *e){
    if (e->type() == MainWindowCustomEvent::Event){
        MainWindowCustomEvent *c_e = reinterpret_cast<MainWindowCustomEvent*>(e);

        c_e->func()->call();
    }

    e->accept();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (k_e->modifiers() == Qt::ControlModifier){
            if (k_e->key() == Qt::Key_PageUp)
                tBar->prevTab();
            else if (k_e->key() == Qt::Key_PageDown)
                tBar->nextTab();
            else if (k_e->key() == Qt::Key_W){
                if (arena->widget())
                    arena->widget()->close();
            }
            else
               return QMainWindow::eventFilter(obj, e);

            return true;
        }
    }

    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::init(){
    installEventFilter(this);

    arena = new QDockWidget();
    arena->setFloating(false);
    arena->setAllowedAreas(Qt::RightDockWidgetArea);
    arena->setFeatures(QDockWidget::NoDockWidgetFeatures);
    arena->setContextMenuPolicy(Qt::CustomContextMenu);
    arena->setTitleBarWidget(new QWidget(arena));

    transfer_dock = new QDockWidget(this);
    transfer_dock->setObjectName("transfer_dock");
    transfer_dock->setFloating(false);
    transfer_dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    transfer_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    transfer_dock->setContextMenuPolicy(Qt::CustomContextMenu);
    transfer_dock->setTitleBarWidget(new QWidget(transfer_dock));

    setCentralWidget(arena);
    //addDockWidget(Qt::RightDockWidgetArea, arena);
    addDockWidget(Qt::BottomDockWidgetArea, transfer_dock);

    transfer_dock->hide();

    history.setSize(30);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    setWindowIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));

    setWindowTitle(QString("%1").arg(EISKALTDCPP_WND_TITLE));

    initActions();

    initMenuBar();

    initStatusBar();

    initToolbar();

    loadSettings();

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotExit()));
}

void MainWindow::loadSettings(){
    WulforSettings *WS = WulforSettings::getInstance();

    bool showMax = WS->getBool(WB_MAINWINDOW_MAXIMIZED);
    int w = WS->getInt(WI_MAINWINDOW_WIDTH);
    int h = WS->getInt(WI_MAINWINDOW_HEIGHT);
    int xPos = WS->getInt(WI_MAINWINDOW_X);
    int yPos = WS->getInt(WI_MAINWINDOW_Y);

    QPoint p(xPos, yPos);
    QSize  sz(w, h);

    if (p.x() >= 0 || p.y() >= 0)
        this->move(p);

    if (sz.width() > 0 || sz.height() > 0)
        this->resize(sz);

    if (showMax)
        this->showMaximized();

    QString wstate = WSGET(WS_MAINWINDOW_STATE);

    if (!wstate.isEmpty())
        this->restoreState(QByteArray::fromBase64(wstate.toAscii()));
}

void MainWindow::saveSettings(){
    static bool stateIsSaved = false;

    if (stateIsSaved)
        return;

    WISET(WI_MAINWINDOW_HEIGHT, height());
    WISET(WI_MAINWINDOW_WIDTH, width());
    WISET(WI_MAINWINDOW_X, x());
    WISET(WI_MAINWINDOW_Y, y());

    WBSET(WB_MAINWINDOW_MAXIMIZED, isMaximized());
    WBSET(WB_MAINWINDOW_HIDE, !isVisible());

    WSSET(WS_MAINWINDOW_STATE, saveState().toBase64());

    stateIsSaved = true;
}

void MainWindow::initActions(){

    WulforUtil *WU = WulforUtil::getInstance();

    {
        fileOptions = new QAction("", this);
        fileOptions->setShortcut(tr("Ctrl+O"));
        fileOptions->setIcon(WU->getPixmap(WulforUtil::eiCONFIGURE));
        connect(fileOptions, SIGNAL(triggered()), this, SLOT(slotFileSettings()));

        fileFileListBrowserLocal = new QAction("", this);
        fileFileListBrowserLocal->setShortcut(tr("Ctrl+L"));
        fileFileListBrowserLocal->setIcon(WU->getPixmap(WulforUtil::eiOWN_FILELIST));
        connect(fileFileListBrowserLocal, SIGNAL(triggered()), this, SLOT(slotFileBrowseOwnFilelist()));

        fileFileListBrowser = new QAction("", this);
        fileFileListBrowser->setShortcut(tr("Shift+L"));
        fileFileListBrowser->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
        connect(fileFileListBrowser, SIGNAL(triggered()), this, SLOT(slotFileBrowseFilelist()));

        fileOpenLogFile = new QAction("", this);
        fileOpenLogFile->setIcon(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE));
        connect(fileOpenLogFile, SIGNAL(triggered()), this, SLOT(slotFileOpenLogFile()));

        fileFileListRefresh = new QAction("", this);
        fileFileListRefresh->setShortcut(tr("Ctrl+R"));
        fileFileListRefresh->setIcon(WU->getPixmap(WulforUtil::eiRELOAD));
        connect(fileFileListRefresh, SIGNAL(triggered()), this, SLOT(slotFileRefreshShare()));

        fileHubReconnect = new QAction("", this);
        fileHubReconnect->setIcon(WU->getPixmap(WulforUtil::eiRECONNECT));
        connect(fileHubReconnect, SIGNAL(triggered()), this, SLOT(slotFileReconnect()));

        fileHashProgress = new QAction("", this);
        fileHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        connect(fileHashProgress, SIGNAL(triggered()), this, SLOT(slotFileHashProgress()));

        fileQuickConnect = new QAction("", this);
        fileQuickConnect->setShortcut(tr("Ctrl+H"));
        fileQuickConnect->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
        connect(fileQuickConnect, SIGNAL(triggered()), this, SLOT(slotQC()));

        fileTransfers = new QAction("", this);
        fileTransfers->setShortcut(tr("Ctrl+T"));
        fileTransfers->setIcon(WU->getPixmap(WulforUtil::eiTRANSFER));
        fileTransfers->setCheckable(true);
        connect(fileTransfers, SIGNAL(toggled(bool)), this, SLOT(slotFileTransfer(bool)));
        //transfer_dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        fileDownloadQueue = new QAction("", this);
        fileDownloadQueue->setShortcut(tr("Ctrl+D"));
        fileDownloadQueue->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(fileDownloadQueue, SIGNAL(triggered()), this, SLOT(slotFileDownloadQueue()));

        fileFinishedDownloads = new QAction("", this);
        fileFinishedDownloads->setIcon(WU->getPixmap(WulforUtil::eiDOWNLIST));
        connect(fileFinishedDownloads, SIGNAL(triggered()), this, SLOT(slotFileFinishedDownloads()));

        fileFinishedUploads = new QAction("", this);
        fileFinishedUploads->setIcon(WU->getPixmap(WulforUtil::eiUPLIST));
        connect(fileFinishedUploads, SIGNAL(triggered()), this, SLOT(slotFileFinishedUploads()));

        fileFavoriteHubs = new QAction("", this);
        fileFavoriteHubs->setIcon(WU->getPixmap(WulforUtil::eiFAVSERVER));
        connect(fileFavoriteHubs, SIGNAL(triggered()), this, SLOT(slotFileFavoriteHubs()));

        fileFavoriteUsers = new QAction("", this);
        fileFavoriteUsers->setIcon(WU->getPixmap(WulforUtil::eiFAVUSERS));
        connect(fileFavoriteUsers, SIGNAL(triggered()), this, SLOT(slotFileFavoriteUsers()));

        fileSpy = new QAction("", this);
        fileSpy->setIcon(WU->getPixmap(WulforUtil::eiSPY));
        connect(fileSpy, SIGNAL(triggered()), this, SLOT(slotFileSpy()));

        fileAntiSpam = new QAction("", this);
        fileAntiSpam->setIcon(WU->getPixmap(WulforUtil::eiSPAM));
        connect(fileAntiSpam, SIGNAL(triggered()), this, SLOT(slotFileAntiSpam()));

        fileIPFilter = new QAction("", this);
        fileIPFilter->setIcon(WU->getPixmap(WulforUtil::eiFILTER));
        connect(fileIPFilter, SIGNAL(triggered()), this, SLOT(slotFileIPFilter()));

        fileSearch = new QAction("", this);
        fileSearch->setShortcut(tr("Ctrl+S"));
        fileSearch->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
        connect(fileSearch, SIGNAL(triggered()), this, SLOT(slotFileSearch()));

        fileHideWindow = new QAction(tr("Hide window"), this);
        fileHideWindow->setShortcut(tr("Esc"));
        fileHideWindow->setIcon(WU->getPixmap(WulforUtil::eiHIDEWINDOW));
        fileHideWindow->setEnabled(WBGET(WB_TRAY_ENABLED));
        connect(fileHideWindow, SIGNAL(triggered()), this, SLOT(slotHideWindow()));

        fileHideProgressSpace = new QAction(tr("Hide free space bar"), this);
        if (!WBGET(WB_SHOW_FREE_SPACE))
            fileHideProgressSpace->setText(tr("Show free space bar"));
#ifndef FREE_SPACE_BAR
        fileHideProgressSpace->setEnabled(false);
#endif
        fileHideProgressSpace->setIcon(WU->getPixmap(WulforUtil::eiFREESPACE));
        connect(fileHideProgressSpace, SIGNAL(triggered()), this, SLOT(slotHideProgressSpace()));

        fileQuit = new QAction("", this);
        fileQuit->setShortcut(tr("Ctrl+Q"));
        fileQuit->setIcon(WU->getPixmap(WulforUtil::eiEXIT));
        connect(fileQuit, SIGNAL(triggered()), this, SLOT(slotExit()));

        chatClear = new QAction("", this);
        chatClear->setIcon(WU->getPixmap(WulforUtil::eiCLEAR));
        connect(chatClear, SIGNAL(triggered()), this, SLOT(slotChatClear()));

        findInChat = new QAction("", this);
        findInChat->setShortcut(tr("Ctrl+F"));
        findInChat->setIcon(WU->getPixmap(WulforUtil::eiFIND));
        connect(findInChat, SIGNAL(triggered()), this, SLOT(slotFindInChat()));

        chatDisable = new QAction("", this);
        chatDisable->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
        connect(chatDisable, SIGNAL(triggered()), this, SLOT(slotChatDisable()));

        QAction *separator0 = new QAction("", this);
        separator0->setSeparator(true);
        QAction *separator1 = new QAction("", this);
        separator1->setSeparator(true);
        QAction *separator2 = new QAction("", this);
        separator2->setSeparator(true);
        QAction *separator3 = new QAction("", this);
        separator3->setSeparator(true);
        QAction *separator4 = new QAction("", this);
        separator4->setSeparator(true);
        QAction *separator5 = new QAction("", this);
        separator5->setSeparator(true);
        QAction *separator6 = new QAction("", this);
        separator6->setSeparator(true);

        fileMenuActions << fileOptions
                << separator1
                << fileOpenLogFile
                << fileFileListBrowser
                << fileFileListBrowserLocal
                << fileFileListRefresh
                << fileHashProgress
                << separator2
                << fileHubReconnect
                << fileQuickConnect
                << separator0
                << fileTransfers
                << fileDownloadQueue
                << fileFinishedDownloads
                << fileFinishedUploads
                << separator4
                << fileFavoriteHubs
                << fileFavoriteUsers
                << fileSearch
                << separator3
                << fileSpy
                << fileAntiSpam
                << fileIPFilter
                << separator5
                << fileHideProgressSpace
                << fileHideWindow
                << fileQuit;

        toolBarActions << fileOptions
                << separator1
                << fileFileListBrowserLocal
                << fileFileListRefresh
                << fileHashProgress
                << separator6
                << chatClear
                << findInChat
                << chatDisable
                << separator2
                << fileHubReconnect
                << fileQuickConnect
                << separator0
                << fileTransfers
                << fileDownloadQueue
                << fileFinishedDownloads
                << fileFinishedUploads
                << separator4
                << fileFavoriteHubs
                << fileFavoriteUsers
                << fileSearch
                << separator3
                << fileSpy
                << fileAntiSpam
                << fileIPFilter
                << separator5
                << fileQuit;
    }
    {
        menuWidgets = new QMenu("", this);
    }
    {
        aboutClient = new QAction("", this);
        aboutClient->setIcon(WU->getPixmap(WulforUtil::eiICON_APPL));
        connect(aboutClient, SIGNAL(triggered()), this, SLOT(slotAboutClient()));

        aboutQt = new QAction("", this);
        aboutQt->setIcon(WU->getPixmap(WulforUtil::eiQT_LOGO));
        connect(aboutQt, SIGNAL(triggered()), this, SLOT(slotAboutQt()));
    }
}

void MainWindow::initMenuBar(){
    {
        menuFile = new QMenu("", this);

        menuFile->addActions(fileMenuActions);
    }
    {
        menuAbout = new QMenu("", this);

        menuAbout->addAction(aboutClient);
        menuAbout->addAction(aboutQt);
    }

    menuBar()->addMenu(menuFile);
    menuBar()->addMenu(menuWidgets);
    menuBar()->addMenu(menuAbout);
}

void MainWindow::initStatusBar(){
    statusLabel = new QLabel(statusBar());
    statusLabel->setFrameShadow(QFrame::Plain);
    statusLabel->setFrameShape(QFrame::NoFrame);
    statusLabel->setAlignment(Qt::AlignRight);
    statusLabel->setToolTip(tr("Counts"));

    statusDSPLabel = new QLabel(statusBar());
    statusDSPLabel->setFrameShadow(QFrame::Plain);
    statusDSPLabel->setFrameShape(QFrame::NoFrame);
    statusDSPLabel->setAlignment(Qt::AlignRight);
    statusDSPLabel->setToolTip(tr("Download speed (per sec.)"));

    statusUSPLabel = new QLabel(statusBar());
    statusUSPLabel->setFrameShadow(QFrame::Plain);
    statusUSPLabel->setFrameShape(QFrame::NoFrame);
    statusUSPLabel->setAlignment(Qt::AlignRight);
    statusUSPLabel->setToolTip(tr("Upload speed (per sec.)"));

    statusDLabel = new QLabel(statusBar());
    statusDLabel->setFrameShadow(QFrame::Plain);
    statusDLabel->setFrameShape(QFrame::NoFrame);
    statusDLabel->setAlignment(Qt::AlignRight);
    statusDLabel->setToolTip(tr("Downloaded"));

    statusULabel = new QLabel(statusBar());
    statusULabel->setFrameShadow(QFrame::Plain);
    statusULabel->setFrameShape(QFrame::NoFrame);
    statusULabel->setAlignment(Qt::AlignRight);
    statusULabel->setToolTip(tr("Uploaded"));

    msgLabel = new QLabel(statusBar());
    msgLabel->setFrameShadow(QFrame::Plain);
    msgLabel->setFrameShape(QFrame::NoFrame);
    msgLabel->setAlignment(Qt::AlignLeft);
#ifdef FREE_SPACE_BAR
    progressSpace = new QProgressBar(this);
    progressSpace->setMaximum(100);
    progressSpace->setMinimum(0);
    progressSpace->setMinimumWidth(100);
    progressSpace->setMaximumWidth(250);
    progressSpace->setFixedHeight(18);
    progressSpace->setToolTip(tr("Space free"));

    if (!WBGET(WB_SHOW_FREE_SPACE))
        progressSpace->hide();
#else //FREE_SPACE_BAR
WBSET(WB_SHOW_FREE_SPACE, false);
#endif //FREE_SPACE_BAR

    statusBar()->addWidget(msgLabel);
    statusBar()->addPermanentWidget(statusDLabel);
    statusBar()->addPermanentWidget(statusULabel);
    statusBar()->addPermanentWidget(statusDSPLabel);
    statusBar()->addPermanentWidget(statusUSPLabel);
    statusBar()->addPermanentWidget(statusLabel);
#ifdef FREE_SPACE_BAR
    statusBar()->addPermanentWidget(progressSpace);
#endif //FREE_SPACE_BAR
}

void MainWindow::retranslateUi(){
    //Retranslate menu actions
    {
        menuFile->setTitle(tr("&File"));

        fileOptions->setText(tr("Options"));

        fileOpenLogFile->setText(tr("Open log file"));

        fileFileListBrowser->setText(tr("Open filelist..."));

        fileFileListBrowserLocal->setText(tr("Open own filelist"));

        fileFileListRefresh->setText(tr("Refresh share"));

        fileHashProgress->setText(tr("Hash progress"));

        fileHubReconnect->setText(tr("Reconnect to hub"));

        fileTransfers->setText(tr("Transfers"));

        fileDownloadQueue->setText(tr("Download queue"));

        fileFinishedDownloads->setText(tr("Finished downloads"));

        fileFinishedUploads->setText(tr("Finished uploads"));

        fileSpy->setText(tr("Search Spy"));

        fileAntiSpam->setText(tr("AntiSpam module"));

        fileIPFilter->setText(tr("IPFilter module"));

        fileFavoriteHubs->setText(tr("Favourite hubs"));

        fileFavoriteUsers->setText(tr("Favourite users"));

        fileSearch->setText(tr("Search"));

        fileQuickConnect->setText(tr("Quick connect"));

        fileQuit->setText(tr("Quit"));

        chatClear->setText(tr("Clear chat"));

        findInChat->setText(tr("Find in chat"));

        chatDisable->setText("Disable/Enable chat");

        menuWidgets->setTitle(tr("&Widgets"));

        menuAbout->setTitle(tr("&Help"));

        aboutClient->setText(tr("About EiskaltDC++"));

        aboutQt->setText(tr("About Qt"));
    }
    {
        arena->setWindowTitle(tr("Main layout"));
    }
}

void MainWindow::initToolbar(){
    fBar = new ToolBar(this);
    fBar->setObjectName("fBar");
    fBar->addActions(toolBarActions);
    fBar->setContextMenuPolicy(Qt::CustomContextMenu);
    fBar->setMovable(true);
    fBar->setFloatable(true);
    fBar->setAllowedAreas(Qt::AllToolBarAreas);
    fBar->installEventFilter(this);

    tBar = new ToolBar(this);
    tBar->setObjectName("tBar");
    tBar->initTabs();
    tBar->setContextMenuPolicy(Qt::CustomContextMenu);
    tBar->setMovable(true);
    tBar->setFloatable(true);
    tBar->setAllowedAreas(Qt::AllToolBarAreas);
    tBar->installEventFilter(this);

    addToolBar(fBar);
    addToolBar(tBar);
}

void MainWindow::newHubFrame(QString address, QString enc){
    if (address.isEmpty())
        return;

    HubFrame *fr = NULL;

    if (fr = HubManager::getInstance()->getHub(address)){
        mapWidgetOnArena(fr);

        return;
    }

    fr = new HubFrame(NULL, address, enc);
    fr->setAttribute(Qt::WA_DeleteOnClose);

    addArenaWidget(fr);
    mapWidgetOnArena(fr);

    addArenaWidgetOnToolbar(fr);
}

void MainWindow::updateStatus(QMap<QString, QString> map){
    if (!statusLabel)
        return;

    statusLabel->setText(map["STATS"]);
    statusUSPLabel->setText(map["USPEED"]);
    statusDSPLabel->setText(map["DSPEED"]);
    statusDLabel->setText(map["DOWN"]);
    statusULabel->setText(map["UP"]);

    QFontMetrics metrics(font());

    statusUSPLabel->setFixedWidth(metrics.width(statusUSPLabel->text()) > statusUSPLabel->width()? metrics.width(statusUSPLabel->text()) + 10 : statusUSPLabel->width());
    statusDSPLabel->setFixedWidth(metrics.width(statusDSPLabel->text()) > statusDSPLabel->width()? metrics.width(statusDSPLabel->text()) + 10 : statusDSPLabel->width());
    statusDLabel->setFixedWidth(metrics.width(statusDLabel->text()) > statusDLabel->width()? metrics.width(statusDLabel->text()) + 10 : statusDLabel->width());
    statusULabel->setFixedWidth(metrics.width(statusULabel->text()) > statusULabel->width()? metrics.width(statusULabel->text()) + 10 : statusULabel->width());
#ifdef FREE_SPACE_BAR
    if (WBGET(WB_SHOW_FREE_SPACE)) {
        boost::filesystem::space_info info;
        if (boost::filesystem::exists(SETTING(DOWNLOAD_DIRECTORY)))
            info = boost::filesystem::space(boost::filesystem::path(SETTING(DOWNLOAD_DIRECTORY)));
        else if (boost::filesystem::exists(Util::getPath(Util::PATH_USER_CONFIG)))
            info = boost::filesystem::space(boost::filesystem::path(Util::getPath(Util::PATH_USER_CONFIG)));

        if (info.capacity) {
            float total = info.capacity;
            float percent = 100.0f*(total-info.available)/total;

            QString format = tr("Free %1 of %2")
                         .arg(_q(dcpp::Util::formatBytes(info.available)))
                         .arg(_q(dcpp::Util::formatBytes(total)));

            progressSpace->setFormat(format);
            progressSpace->setValue(static_cast<unsigned>(percent));
        }
    }
#endif //FREE_SPACE_BAR

}

void MainWindow::setStatusMessage(QString msg){
    msgLabel->setText(msg);
}

void MainWindow::autoconnect(){
    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;

        if (entry->getConnect()) {
            if (entry->getNick().empty() && SETTING(NICK).empty())
                continue;

            QString encoding = WulforUtil::getInstance()->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));

            newHubFrame(QString::fromStdString(entry->getServer()), encoding);
        }
    }
}

void MainWindow::parseCmdLine(){
    QStringList args = qApp->arguments();

    foreach (QString arg, args){
        if (arg.startsWith("magnet:?xt=urn:tree:tiger:")){
            Magnet m(this);
            m.setLink(arg);

            m.exec();
        }
        else if (arg.startsWith("dchub://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainWindow::parseInstanceLine(QString data){
    QStringList args = data.split("\n", QString::SkipEmptyParts);

    foreach (QString arg, args){
        if (arg.startsWith("magnet:?xt=urn:tree:tiger:")){
            Magnet m(this);
            m.setLink(arg);

            m.exec();
        }
        else if (arg.startsWith("dchub://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainWindow::browseOwnFiles(){
    slotFileBrowseOwnFilelist();
}

void MainWindow::slotFileBrowseFilelist(){
    static ShareBrowser *local_share = NULL;
    QString file = QFileDialog::getOpenFileName(this, tr("Choose file to open"), QString::fromStdString(Util::getPath(Util::PATH_FILE_LISTS)),
            tr("Modern XML Filelists") + " (*.xml.bz2);;" +
            tr("Modern XML Filelists uncompressed") + " (*.xml);;" +
            tr("All files") + " (*)");
    UserPtr user = DirectoryListing::getUserFromFilename(_tq(file));
    if (user) {
        local_share = new ShareBrowser(user, file, "");
    } else {
        setStatusMessage(tr("Unable to load file list: Invalid file list name"));
    }
}

void MainWindow::redrawToolPanel(){
    tBar->redraw();

    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
    QHash<QAction*, ArenaWidget*>::iterator end = menuWidgetsHash.end();

    for(; it != end; ++it){//also redraw all widget menu items
        it.key()->setText(it.value()->getArenaShortTitle());
        it.key()->setIcon(it.value()->getPixmap());
    }
}

void MainWindow::addArenaWidget(ArenaWidget *wgt){
    if (!arenaWidgets.contains(wgt) && wgt && wgt->getWidget()){
        arenaWidgets.push_back(wgt);
        arenaMap[wgt] = wgt->getWidget();
    }
}

void MainWindow::remArenaWidget(ArenaWidget *awgt){
    if (arenaWidgets.contains(awgt)){
        arenaWidgets.removeAt(arenaWidgets.indexOf(awgt));
        arenaMap.erase(arenaMap.find(awgt));

        if (arena->widget() == awgt->getWidget())
            arena->setWidget(NULL);
    }
}

void MainWindow::mapWidgetOnArena(ArenaWidget *awgt){
    if (!arenaWidgets.contains(awgt))
        return;

    if (arena->widget())
        arena->widget()->hide();

    arena->setWidget(arenaMap[awgt]);

    setWindowTitle(awgt->getArenaTitle() + " :: " + QString("%1").arg(EISKALTDCPP_WND_TITLE));

    tBar->mapped(awgt);

    HubFrame *fr = HubManager::getInstance()->activeHub();

    chatClear->setEnabled(fr == awgt->getWidget());
    findInChat->setEnabled(fr == awgt->getWidget());
    chatDisable->setEnabled(fr == awgt->getWidget());

    //arenaMap[awgt]->setFocus();
}

void MainWindow::remWidgetFromArena(ArenaWidget *awgt){
    if (!arenaWidgets.contains(awgt))
        return;

    if (arena->widget() == awgt->getWidget())
        arena->widget()->hide();
}

void MainWindow::addArenaWidgetOnToolbar(ArenaWidget *awgt, bool keepFocus){
    if (!arenaWidgets.contains(awgt))
        return;

    QAction *act = new QAction(awgt->getArenaShortTitle(), this);
    act->setIcon(awgt->getPixmap());

    connect(act, SIGNAL(triggered()), this, SLOT(slotWidgetsToggle()));

    menuWidgetsActions.push_back(act);
    menuWidgetsHash.insert(act, awgt);

    menuWidgets->clear();
    menuWidgets->addActions(menuWidgetsActions);

    tBar->insertWidget(awgt, keepFocus);
}

void MainWindow::remArenaWidgetFromToolbar(ArenaWidget *awgt){
    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
    for (; it != menuWidgetsHash.end(); ++it){
        if (it.value() == awgt){
            menuWidgetsActions.removeAt(menuWidgetsActions.indexOf(it.key()));
            menuWidgetsHash.erase(it);

            menuWidgets->clear();

            menuWidgets->addActions(menuWidgetsActions);

            break;
        }
    }

    tBar->removeWidget(awgt);
}

void MainWindow::toggleSingletonWidget(ArenaWidget *a){
    if (!a)
        return;

    if (tBar->hasWidget(a)){
        QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
        for (; it != menuWidgetsHash.end(); ++it){
            if (it.value() == a){
                menuWidgetsActions.removeAt(menuWidgetsActions.indexOf(it.key()));
                menuWidgetsHash.erase(it);

                menuWidgets->clear();

                menuWidgets->addActions(menuWidgetsActions);

                break;
            }
        }

        tBar->removeWidget(a);
        remWidgetFromArena(a);
    }
    else {
        tBar->insertWidget(a);

        QAction *act = new QAction(a->getArenaShortTitle(), this);
        act->setIcon(a->getPixmap());

        connect(act, SIGNAL(triggered()), this, SLOT(slotWidgetsToggle()));

        menuWidgetsActions.push_back(act);
        menuWidgetsHash.insert(act, a);

        menuWidgets->clear();
        menuWidgets->addActions(menuWidgetsActions);

        mapWidgetOnArena(a);
    }
}

void MainWindow::startSocket(){
    SearchManager::getInstance()->disconnect();
    ConnectionManager::getInstance()->disconnect();

    if (ClientManager::getInstance()->isActive()) {
        QString msg = "";
        try {
            ConnectionManager::getInstance()->listen();
        } catch(const Exception &e) {
            msg = tr("Cannot listen socket because: \n") + QString::fromStdString(e.getError()) + tr("\n\nPlease check your connection settings");

            QMessageBox::warning(this, tr("Connection Manager: Warning"), msg, QMessageBox::Ok);
        }
        try {
            SearchManager::getInstance()->listen();
        } catch(const Exception &e) {
            msg = tr("Cannot listen socket because: \n") + QString::fromStdString(e.getError()) + tr("\n\nPlease check your connection settings");

            QMessageBox::warning(this, tr("Search Manager: Warning"), msg, QMessageBox::Ok);
        }
    }

    UPnPMapper::getInstance()->forward();
}

void MainWindow::showShareBrowser(dcpp::UserPtr usr, QString file, QString jump_to){
    ShareBrowser *sb = new ShareBrowser(usr, file, jump_to);
}

void MainWindow::slotFileOpenLogFile(){
    QString f = QFileDialog::getOpenFileName(this, tr("Open log file"),_q(SETTING(LOG_DIRECTORY)), tr("Log files (*.log);;All files (*.*)"));

    if (!f.isEmpty()){
        if (f.startsWith("/"))
            f = "file://" + f;
        else
            f = "file:///" + f;

        QDesktopServices::openUrl(f);
    }
}

void MainWindow::slotFileBrowseOwnFilelist(){
    static ShareBrowser *local_share = NULL;

    if (arenaWidgets.contains(local_share)){
        mapWidgetOnArena(local_share);

        return;
    }

    UserPtr user = ClientManager::getInstance()->getMe();
    QString file = QString::fromStdString(ShareManager::getInstance()->getOwnListFile());

    local_share = new ShareBrowser(user, file, "");
}

void MainWindow::slotFileRefreshShare(){
    ShareManager *SM = ShareManager::getInstance();

    SM->setDirty();
    SM->refresh(true);

    HashProgress progress(this);
    progress.slotAutoClose(true);

    progress.exec();
}

void MainWindow::slotFileHashProgress(){
    HashProgress progress(this);

    progress.exec();
}

void MainWindow::slotFileReconnect(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->reconnect();
}

void MainWindow::slotFileSearch(){
    SearchFrame *sf = new SearchFrame();

    sf->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::slotFileDownloadQueue(){
    if (!DownloadQueue::getInstance())
        DownloadQueue::newInstance();

    toggleSingletonWidget(DownloadQueue::getInstance());
}

void MainWindow::slotFileFinishedDownloads(){
    if (!FinishedDownloads::getInstance())
        FinishedDownloads::newInstance();

    toggleSingletonWidget(FinishedDownloads::getInstance());
}

void MainWindow::slotFileFinishedUploads(){
    if (!FinishedUploads::getInstance())
        FinishedUploads::newInstance();

    toggleSingletonWidget(FinishedUploads::getInstance());
}

void MainWindow::slotFileSpy(){
    if (!SpyFrame::getInstance())
        SpyFrame::newInstance();

    toggleSingletonWidget(SpyFrame::getInstance());
}

void MainWindow::slotFileAntiSpam(){
    AntiSpamFrame fr(this);

    fr.exec();
}

void MainWindow::slotFileIPFilter(){
    IPFilterFrame fr(this);

    fr.exec();
}

void MainWindow::slotFileFavoriteHubs(){
    if (!FavoriteHubs::getInstance())
        FavoriteHubs::newInstance();

    toggleSingletonWidget(FavoriteHubs::getInstance());
}

void MainWindow::slotFileFavoriteUsers(){
    if (!FavoriteUsers::getInstance())
        FavoriteUsers::newInstance();

    toggleSingletonWidget(FavoriteUsers::getInstance());
}

void MainWindow::slotFileSettings(){
    Settings s;

    s.exec();

    //reload some settings
    fileHideWindow->setEnabled(WBGET(WB_TRAY_ENABLED));
}

void MainWindow::slotFileTransfer(bool toggled){
    if (toggled){
        transfer_dock->setVisible(true);
        transfer_dock->setWidget(TransferView::getInstance());
    }
    else {
        transfer_dock->setWidget(NULL);
        transfer_dock->setVisible(false);
    }
}

void MainWindow::slotChatClear(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->clearChat();
}

void MainWindow::slotFindInChat(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->slotHideFindFrame();
}

void MainWindow::slotChatDisable(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->disableChat();
}

void MainWindow::slotWidgetsToggle(){
    QAction *act = reinterpret_cast<QAction*>(sender());
    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.find(act);

    if (it == menuWidgetsHash.end())
        return;

    mapWidgetOnArena(it.value());
}

void MainWindow::slotQC(){
    QuickConnect qc;

    qc.exec();
}

void MainWindow::slotHideWindow(){
    if (findInChat->isEnabled()){
        HubFrame *fr = HubManager::getInstance()->activeHub();

        if (fr->frame->isVisible()){
            if (fr->lineEdit_FIND->hasFocus()){
                fr->slotHideFindFrame();

                return;
            }
        }
    }
    if (!isUnload && isActiveWindow() && WBGET(WB_TRAY_ENABLED)) {
        hide();
    }
}

void MainWindow::slotHideProgressSpace() {
    if (WBGET(WB_SHOW_FREE_SPACE)) {
        progressSpace->hide();
        fileHideProgressSpace->setText(tr("Show free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, false);
    } else {
        progressSpace->show();
        fileHideProgressSpace->setText(tr("Hide free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, true);
    }
}

void MainWindow::slotExit(){
    setUnload(true);

    close();
}

void MainWindow::slotAboutClient(){
    About a(this);

    qulonglong app_total_down = WSGET(WS_APP_TOTAL_DOWN).toULongLong();
    qulonglong app_total_up   = WSGET(WS_APP_TOTAL_UP).toULongLong();

#ifndef DCPP_REVISION
    a.label->setText(QString("<b>%1</b> %2 (%3)")
                     .arg(EISKALTDCPP_WND_TITLE)
                     .arg(EISKALTDCPP_VERSION)
                     .arg(EISKALTDCPP_VERSION_SFX));
#else
    a.label->setText(QString("<b>%1</b> %2 - %3 %4")
                     .arg(EISKALTDCPP_WND_TITLE)
                     .arg(EISKALTDCPP_VERSION)
                     .arg(EISKALTDCPP_VERSION_SFX)
                     .arg(DCPP_REVISION));
#endif
    a.label_ABOUT->setTextFormat(Qt::RichText);
    a.label_ABOUT->setText(QString("%1<br><br> %2 %3 %4<br><br> %5 %6<br><br> %7 <b>%8</b> <br> %9 <b>%10</b>")
                           .arg(tr("EiskaltDC++ is a graphical client for Direct Connect and ADC protocols."))
                           .arg(tr("DC++ core version:"))
                           .arg(DCVERSIONSTRING)
                           .arg(tr("(modified)"))
                           .arg(tr("Home page:"))
                           .arg("<a href=\"http://code.google.com/p/eiskaltdc/\">"
                                "http://code.google.com/p/eiskaltdc/</a>")
                           .arg(tr("Total up:"))
                           .arg(_q(Util::formatBytes(app_total_up)))
                           .arg(tr("Total down:"))
                           .arg(_q(Util::formatBytes(app_total_down))));

    a.exec();
}

void MainWindow::slotUnixSignal(int sig){
    printf("%i\n");
}

void MainWindow::slotAboutQt(){
    QMessageBox::aboutQt(this);
}

void MainWindow::on(dcpp::LogManagerListener::Message, time_t t, const std::string& m) throw(){
    QTextCodec *codec = QTextCodec::codecForLocale();

    typedef Func1<MainWindow, QString> FUNC;
    FUNC *func = new FUNC(this, &MainWindow::setStatusMessage, codec->toUnicode(m.c_str()));

    QApplication::postEvent(this, new MainWindowCustomEvent(func));
}

void MainWindow::on(dcpp::QueueManagerListener::Finished, QueueItem *item, const std::string &dir, int64_t) throw(){
    if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)){
        UserPtr user = item->getDownloads()[0]->getUser();
        QString listName = QString::fromStdString(item->getListName());

        typedef Func3<MainWindow, UserPtr, QString, QString> FUNC;
        FUNC *func = new FUNC(this, &MainWindow::showShareBrowser, user, listName, QString::fromStdString(dir));

        QApplication::postEvent(this, new MainWindowCustomEvent(func));
    }
}

void MainWindow::on(dcpp::TimerManagerListener::Second, uint32_t ticks) throw(){
    static quint32 lastUpdate = 0;
    static quint64 lastUp = 0, lastDown = 0;

    quint64 now = GET_TICK();

    quint64 diff = now - lastUpdate;
    quint64 downBytes = 0;
    quint64 upBytes = 0;

    if (diff < 100U)
        diff = 1U;

    quint64 downDiff = Socket::getTotalDown() - lastDown;
    quint64 upDiff = Socket::getTotalUp() - lastUp;

    downBytes = (downDiff * 1000) / diff;
    upBytes = (upDiff * 1000) / diff;

    QMap<QString, QString> map;

    map["STATS"]    = _q(Client::getCounts());
    map["DSPEED"]   = _q(Util::formatBytes(downBytes));
    map["DOWN"]     = _q(Util::formatBytes(Socket::getTotalDown()));
    map["USPEED"]   = _q(Util::formatBytes(upBytes));
    map["UP"]       = _q(Util::formatBytes(Socket::getTotalUp()));

    qulonglong app_total_down = WSGET(WS_APP_TOTAL_DOWN).toULongLong()+downDiff;
    qulonglong app_total_up   = WSGET(WS_APP_TOTAL_UP).toULongLong()+upDiff;

    WSSET(WS_APP_TOTAL_DOWN, QString().setNum(app_total_down));
    WSSET(WS_APP_TOTAL_UP, QString().setNum(app_total_up));

    lastUpdate = ticks;
    lastUp = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();

    typedef Func1<MainWindow, QMap<QString, QString> > FUNC;
    FUNC *func = new FUNC(this, &MainWindow::updateStatus, map);

    QApplication::postEvent(this, new MainWindowCustomEvent(func));
}
