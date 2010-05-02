#include "MainWindow.h"
#include "Notification.h"

#include <stdlib.h>
#include <string>
#include <iostream>

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
#include <QDir>
#ifdef FREE_SPACE_BAR
    #include <boost/filesystem.hpp>
#endif //FREE_SPACE_BAR
#ifdef FREE_SPACE_BAR_C
    #ifdef WIN32
        #include <io.h>
    #else //WIN32
        extern "C" {
        #include "gnulib/fsusage.h"
        }
    #endif //WIN32
#endif
#include "HubFrame.h"
#include "HubManager.h"
#include "HashProgress.h"
#include "PMWindow.h"
#include "TransferView.h"
#include "ShareBrowser.h"
#include "QuickConnect.h"
#include "SearchFrame.h"
#include "Settings.h"
#include "FavoriteHubs.h"
#include "PublicHubs.h"
#include "FavoriteUsers.h"
#include "DownloadQueue.h"
#include "FinishedTransfers.h"
#include "AntiSpamFrame.h"
#include "IPFilterFrame.h"
#include "ToolBar.h"
#include "Magnet.h"
#include "SpyFrame.h"
#include "SideBar.h"

#include "dcpp/ShareManager.h"

#include "UPnPMapper.h"
#include "WulforSettings.h"
#include "WulforUtil.h"

#include "Version.h"

using namespace std;

MainWindow::MainWindow (QWidget *parent):
        QMainWindow(parent),
        statusLabel(NULL),
        tBar(NULL),
        fBar(NULL),
        sBar(NULL),
        sideDock(NULL),
        sideTree(NULL),
        menuPanels(NULL),
        wcontainer(NULL)

{
    exitBegin = false;

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
    toolsTransfers->setChecked(transfer_dock->isVisible());
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

    if (WBGET(WB_APP_REMOVE_NOT_EX_DIRS)){
        StringPairList directories = ShareManager::getInstance()->getDirectories();
        for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it){
            QDir dir(_q(it->second));

            if (!dir.exists()){
                try {
                    ShareManager::getInstance()->removeDirectory(it->second);
                }
                catch (const std::exception&){}
            }
        }
    }
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
    delete sBar;

    if (tBar)
        delete tBar;
}

void MainWindow::closeEvent(QCloseEvent *c_e){
    if (!isUnload && WBGET(WB_TRAY_ENABLED)){
        hide();
        c_e->ignore();

        return;
    }

    if (isUnload && WBGET(WB_EXIT_CONFIRM) && !exitBegin){
        QMessageBox::StandardButton ret;

        ret = QMessageBox::question(this, tr("Exit confirm"),
                                    tr("Exit program?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);

        if (ret == QMessageBox::Yes){
            exitBegin = true;
        }
        else{
            setUnload(false);

            c_e->ignore();

            return;
        }
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

    if (PublicHubs::getInstance()){
        PublicHubs::getInstance()->setUnload(true);
        PublicHubs::getInstance()->close();

        PublicHubs::deleteInstance();
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

    if (SearchManager::getInstance())
        SearchManager::getInstance()->disconnect();

    if (ConnectionManager::getInstance())
        ConnectionManager::getInstance()->disconnect();

    QMap< ArenaWidget*, QWidget* > map = arenaMap;
    QMap< ArenaWidget*, QWidget* >::iterator it = map.begin();

    for(; it != map.end(); ++it){
        if (arenaMap.contains(it.key()))//some widgets can autodelete itself from arena widgets
            it.value()->close();
    }

    c_e->accept();
}

void MainWindow::beginExit(){
    exitBegin = true;
    setUnload(true);
}

void MainWindow::showEvent(QShowEvent *e){
    if (e->spontaneous())
        redrawToolPanel();

    if (!showMax && w > 0 && h > 0 && w != width() && h != height())
        this->resize(QSize(w, h));

    if (showMax)
        showMaximized();

    if (WBGET(WB_APP_AUTO_AWAY)){
        Util::setAway(false);
        Util::setManualAway(false);
    }

    if (transfer_dock->isVisible())
        toolsTransfers->setChecked(true);

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(arena->widget());

    if (!awgt)
        return;

    ArenaWidget::Role role = awgt->role();

    bool widgetWithFilter = role == ArenaWidget::Hub || role == ArenaWidget::ShareBrowser || role == ArenaWidget::PublicHubs || role == ArenaWidget::Search;

    chatClear->setEnabled(role == ArenaWidget::Hub || role == ArenaWidget::PrivateMessage);
    findInWidget->setEnabled(widgetWithFilter);
    chatDisable->setEnabled(role == ArenaWidget::Hub);

    e->accept();
}

void MainWindow::hideEvent(QHideEvent *e){
    showMax = isMaximized();

    if (!showMax){
        h = height();
        w = width();
        xPos = x();
        yPos = y();
    }

    e->accept();

    if (WBGET(WB_APP_AUTO_AWAY)){
        Util::setAway(true);
        Util::setManualAway(true);
    }
}

void MainWindow::customEvent(QEvent *e){
    if (e->type() == MainWindowCustomEvent::Event){
        MainWindowCustomEvent *c_e = reinterpret_cast<MainWindowCustomEvent*>(e);

        c_e->func()->call();
    }

    e->accept();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e){
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::init(){
    arena = new QDockWidget();
#if QT_VERSION >= 0x040500
    arena->setWidget(NULL);
    arena->setFloating(false);
#endif
    arena->setAllowedAreas(Qt::RightDockWidgetArea);
    arena->setFeatures(QDockWidget::NoDockWidgetFeatures);
    arena->setContextMenuPolicy(Qt::CustomContextMenu);
    arena->setTitleBarWidget(new QWidget(arena));

    transfer_dock = new QDockWidget(this);
#if QT_VERSION >= 0x040500
    transfer_dock->setWidget(NULL);
    transfer_dock->setFloating(false);
#endif
    transfer_dock->setObjectName("transfer_dock");
    transfer_dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    transfer_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    transfer_dock->setContextMenuPolicy(Qt::CustomContextMenu);
    transfer_dock->setTitleBarWidget(new QWidget(transfer_dock));

    setCentralWidget(arena);
    //addDockWidget(Qt::RightDockWidgetArea, arena);
    addDockWidget(Qt::BottomDockWidgetArea, transfer_dock);

    transfer_dock->hide();

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    setWindowIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));

    setWindowTitle(QString("%1").arg(EISKALTDCPP_WND_TITLE));

    initActions();

    initMenuBar();

    initStatusBar();

    initSearchBar();

    initToolbar();

    initSideBar();

    initHotkeys();

    loadSettings();

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotExit()));
}

void MainWindow::loadSettings(){
    WulforSettings *WS = WulforSettings::getInstance();

    showMax = WS->getBool(WB_MAINWINDOW_MAXIMIZED);
    w = WS->getInt(WI_MAINWINDOW_WIDTH);
    h = WS->getInt(WI_MAINWINDOW_HEIGHT);
    xPos = WS->getInt(WI_MAINWINDOW_X);
    yPos = WS->getInt(WI_MAINWINDOW_Y);

    QPoint p(xPos, yPos);
    QSize  sz(w, h);

    if (p.x() >= 0 && p.y() >= 0)
        this->move(p);

    if (sz.width() > 0 && sz.height() > 0)
        this->resize(sz);

    QString wstate = WSGET(WS_MAINWINDOW_STATE);

    if (!wstate.isEmpty())
        this->restoreState(QByteArray::fromBase64(wstate.toAscii()));

    fBar->setVisible(WBGET(WB_TOOLS_PANEL_VISIBLE));
    panelsTools->setChecked(WBGET(WB_TOOLS_PANEL_VISIBLE));

    sBar->setVisible(WBGET(WB_SEARCH_PANEL_VISIBLE));
    panelsSearch->setChecked(WBGET(WB_SEARCH_PANEL_VISIBLE));

    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
        tBar->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));
    else
        sideDock->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));
    panelsWidgets->setChecked(WBGET(WB_WIDGETS_PANEL_VISIBLE));

    menuBar()->setVisible(WBGET(WB_MAIN_MENU_VISIBLE));
}

void MainWindow::saveSettings(){
    static bool stateIsSaved = false;

    if (stateIsSaved)
        return;

    if (isVisible())
        showMax = isMaximized();

    WBSET(WB_MAINWINDOW_MAXIMIZED, showMax);

    if (!showMax && height() > 0 && width() > 0){
        WISET(WI_MAINWINDOW_HEIGHT, height());
        WISET(WI_MAINWINDOW_WIDTH, width());
    }
    else{
        WISET(WI_MAINWINDOW_HEIGHT, h);
        WISET(WI_MAINWINDOW_WIDTH, w);
    }

    if (!showMax && x() >= 0 && y() >= 0){
        WISET(WI_MAINWINDOW_X, x());
        WISET(WI_MAINWINDOW_Y, y());
    }
    else{
        WISET(WI_MAINWINDOW_X, xPos);
        WISET(WI_MAINWINDOW_Y, yPos);
    }

    if (WBGET(WB_MAINWINDOW_REMEMBER))
        WBSET(WB_MAINWINDOW_HIDE, !isVisible());

    WSSET(WS_MAINWINDOW_STATE, saveState().toBase64());

    stateIsSaved = true;
}

void MainWindow::initActions(){

    WulforUtil *WU = WulforUtil::getInstance();

    {
        fileFileListBrowserLocal = new QAction("", this);
        fileFileListBrowserLocal->setShortcut(tr("Ctrl+L"));
        fileFileListBrowserLocal->setObjectName("fileFileListBrowserLocal");
        fileFileListBrowserLocal->setIcon(WU->getPixmap(WulforUtil::eiOWN_FILELIST));
        connect(fileFileListBrowserLocal, SIGNAL(triggered()), this, SLOT(slotFileBrowseOwnFilelist()));

        fileFileListBrowser = new QAction("", this);
        fileFileListBrowser->setObjectName("fileFileListBrowser");
        fileFileListBrowser->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(fileFileListBrowser, SIGNAL(triggered()), this, SLOT(slotFileBrowseFilelist()));

        fileOpenLogFile = new QAction("", this);
        fileOpenLogFile->setObjectName("fileOpenLogFile");
        fileOpenLogFile->setIcon(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE));
        connect(fileOpenLogFile, SIGNAL(triggered()), this, SLOT(slotFileOpenLogFile()));

        fileOpenDownloadDirectory = new QAction("", this);
        fileOpenDownloadDirectory->setObjectName("fileOpenDownloadDirectory");
        fileOpenDownloadDirectory->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
        connect(fileOpenDownloadDirectory, SIGNAL(triggered()), this, SLOT(slotFileOpenDownloadDirectory()));

        fileFileListRefresh = new QAction("", this);
        fileFileListRefresh->setObjectName("fileFileListRefresh");
        fileFileListRefresh->setShortcut(tr("Ctrl+E"));
        fileFileListRefresh->setIcon(WU->getPixmap(WulforUtil::eiREFRLIST));
        connect(fileFileListRefresh, SIGNAL(triggered()), this, SLOT(slotFileRefreshShare()));

        fileHashProgress = new QAction("", this);
        fileHashProgress->setObjectName("fileHashProgress");
        fileHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        connect(fileHashProgress, SIGNAL(triggered()), this, SLOT(slotFileHashProgress()));

        fileHideWindow = new QAction(tr("Hide window"), this);
        fileHideWindow->setObjectName("fileHideWindow");
        fileHideWindow->setShortcut(tr("Esc"));
        fileHideWindow->setIcon(WU->getPixmap(WulforUtil::eiHIDEWINDOW));
        connect(fileHideWindow, SIGNAL(triggered()), this, SLOT(slotHideWindow()));

        if (!WBGET(WB_TRAY_ENABLED))
            fileHideWindow->setText(tr("Show/hide find frame"));

        fileQuit = new QAction("", this);
        fileQuit->setObjectName("fileQuit");
        fileQuit->setShortcut(tr("Ctrl+Q"));
        fileQuit->setMenuRole(QAction::QuitRole);
        fileQuit->setIcon(WU->getPixmap(WulforUtil::eiEXIT));
        connect(fileQuit, SIGNAL(triggered()), this, SLOT(slotExit()));

        hubsHubReconnect = new QAction("", this);
        hubsHubReconnect->setObjectName("hubsHubReconnect");
        hubsHubReconnect->setShortcut(tr("Ctrl+R"));
        hubsHubReconnect->setIcon(WU->getPixmap(WulforUtil::eiRECONNECT));
        connect(hubsHubReconnect, SIGNAL(triggered()), this, SLOT(slotHubsReconnect()));

        hubsQuickConnect = new QAction("", this);
        hubsQuickConnect->setObjectName("hubsQuickConnect");
        hubsQuickConnect->setShortcut(tr("Ctrl+N"));
        hubsQuickConnect->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
        connect(hubsQuickConnect, SIGNAL(triggered()), this, SLOT(slotQC()));

        hubsFavoriteHubs = new QAction("", this);
        hubsFavoriteHubs->setObjectName("hubsFavoriteHubs");
        hubsFavoriteHubs->setShortcut(tr("Ctrl+H"));
        hubsFavoriteHubs->setIcon(WU->getPixmap(WulforUtil::eiFAVSERVER));
        connect(hubsFavoriteHubs, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteHubs()));

        hubsPublicHubs = new QAction("", this);
        hubsPublicHubs->setObjectName("hubsPublicHubs");
        hubsPublicHubs->setShortcut(tr("Ctrl+P"));
        hubsPublicHubs->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(hubsPublicHubs, SIGNAL(triggered()), this, SLOT(slotHubsPublicHubs()));

        hubsFavoriteUsers = new QAction("", this);
        hubsFavoriteUsers->setObjectName("hubsFavoriteUsers");
        hubsFavoriteUsers->setShortcut(tr("Ctrl+U"));
        hubsFavoriteUsers->setIcon(WU->getPixmap(WulforUtil::eiFAVUSERS));
        connect(hubsFavoriteUsers, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteUsers()));

        toolsOptions = new QAction("", this);
        toolsOptions->setObjectName("toolsOptions");
        toolsOptions->setShortcut(tr("Ctrl+O"));
        toolsOptions->setMenuRole(QAction::PreferencesRole);
        toolsOptions->setIcon(WU->getPixmap(WulforUtil::eiCONFIGURE));
        connect(toolsOptions, SIGNAL(triggered()), this, SLOT(slotToolsSettings()));

        toolsTransfers = new QAction("", this);
        toolsTransfers->setObjectName("toolsTransfers");
        toolsTransfers->setShortcut(tr("Ctrl+T"));
        toolsTransfers->setIcon(WU->getPixmap(WulforUtil::eiTRANSFER));
        toolsTransfers->setCheckable(true);
        connect(toolsTransfers, SIGNAL(toggled(bool)), this, SLOT(slotToolsTransfer(bool)));
        //transfer_dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        toolsDownloadQueue = new QAction("", this);
        toolsDownloadQueue->setObjectName("toolsDownloadQueue");
        toolsDownloadQueue->setShortcut(tr("Ctrl+D"));
        toolsDownloadQueue->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(toolsDownloadQueue, SIGNAL(triggered()), this, SLOT(slotToolsDownloadQueue()));

        toolsFinishedDownloads = new QAction("", this);
        toolsFinishedDownloads->setObjectName("toolsFinishedDownloads");
        toolsFinishedDownloads->setIcon(WU->getPixmap(WulforUtil::eiDOWNLIST));
        connect(toolsFinishedDownloads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedDownloads()));

        toolsFinishedUploads = new QAction("", this);
        toolsFinishedUploads->setObjectName("toolsFinishedUploads");
        toolsFinishedUploads->setIcon(WU->getPixmap(WulforUtil::eiUPLIST));
        connect(toolsFinishedUploads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedUploads()));

        toolsSpy = new QAction("", this);
        toolsSpy->setObjectName("toolsSpy");
        toolsSpy->setIcon(WU->getPixmap(WulforUtil::eiSPY));
        connect(toolsSpy, SIGNAL(triggered()), this, SLOT(slotToolsSpy()));

        toolsAntiSpam = new QAction("", this);
        toolsAntiSpam->setObjectName("toolsAntiSpam");
        toolsAntiSpam->setIcon(WU->getPixmap(WulforUtil::eiSPAM));
        connect(toolsAntiSpam, SIGNAL(triggered()), this, SLOT(slotToolsAntiSpam()));

        toolsIPFilter = new QAction("", this);
        toolsIPFilter->setObjectName("toolsIPFilter");
        toolsIPFilter->setIcon(WU->getPixmap(WulforUtil::eiFILTER));
        connect(toolsIPFilter, SIGNAL(triggered()), this, SLOT(slotToolsIPFilter()));

        toolsAwayOn = new QAction("", this);
        toolsAwayOn->setObjectName("toolsAwayOn");
        toolsAwayOn->setCheckable(true);
        connect(toolsAwayOn, SIGNAL(triggered()), this, SLOT(slotToolsSwitchAway()));

        toolsAwayOff = new QAction("", this);
        toolsAwayOff->setObjectName("toolsAwayOff");
        toolsAwayOff->setCheckable(true);
        connect(toolsAwayOff, SIGNAL(triggered()), this, SLOT(slotToolsSwitchAway()));

        toolsAutoAway = new QAction("", this);
        toolsAutoAway->setCheckable(true);
        toolsAutoAway->setChecked(WBGET(WB_APP_AUTO_AWAY));
        connect(toolsAutoAway, SIGNAL(triggered()), this, SLOT(slotToolsAutoAway()));

        menuAwayAction = new QAction("", this);
        // submenu
        QAction *away_sep = new QAction("", this);
        away_sep->setSeparator(true);

        awayGroup = new QActionGroup(this);
        awayGroup->addAction(toolsAwayOn);
        awayGroup->addAction(toolsAwayOff);

        menuAway = new QMenu(this);
        menuAway->addActions(QList<QAction*>() << toolsAwayOn << toolsAwayOff << away_sep << toolsAutoAway);
        {
            QAction *act = Util::getAway()? toolsAwayOn : toolsAwayOff;
            act->setChecked(true);
        }
        // end
        menuAwayAction->setMenu(menuAway);
        menuAwayAction->setIcon(QIcon(WU->getPixmap(WulforUtil::eiAWAY)));

        toolsSearch = new QAction("", this);
        toolsSearch->setObjectName("toolsSearch");
        toolsSearch->setShortcut(tr("Ctrl+S"));
        toolsSearch->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
        connect(toolsSearch, SIGNAL(triggered()), this, SLOT(slotToolsSearch()));

        toolsHideProgressSpace = new QAction(tr("Hide free space bar"), this);
        toolsHideProgressSpace->setObjectName("toolsHideProgressSpace");
        if (!WBGET(WB_SHOW_FREE_SPACE))
            toolsHideProgressSpace->setText(tr("Show free space bar"));
#if (!defined FREE_SPACE_BAR && !defined FREE_SPACE_BAR_C)
        toolsHideProgressSpace->setVisible(false);
#endif
        toolsHideProgressSpace->setIcon(WU->getPixmap(WulforUtil::eiFREESPACE));
        connect(toolsHideProgressSpace, SIGNAL(triggered()), this, SLOT(slotHideProgressSpace()));

        toolsHideLastStatus = new QAction(tr("Hide last status message"), this);
        toolsHideLastStatus->setObjectName("toolsHideLastStatus");
        toolsHideLastStatus->setIcon(WU->getPixmap(WulforUtil::eiSTATUS));
        connect(toolsHideLastStatus, SIGNAL(triggered()), this, SLOT(slotHideLastStatus()));
        if (!WBGET(WB_LAST_STATUS))
            toolsHideLastStatus->setText(tr("Show last status message"));

        toolsHideUsersStatisctics = new QAction(tr("Hide users statistics"), this);
        toolsHideUsersStatisctics->setObjectName("toolsHideUsersStatisctics");
        toolsHideUsersStatisctics->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(toolsHideUsersStatisctics, SIGNAL(triggered()), this, SLOT(slotHideUsersStatistics()));
        if (!WBGET(WB_USERS_STATISTICS))
            toolsHideUsersStatisctics->setText(tr("Show users statistics"));

        chatClear = new QAction("", this);
        chatClear->setObjectName("chatClear");
        chatClear->setIcon(WU->getPixmap(WulforUtil::eiCLEAR));
        connect(chatClear, SIGNAL(triggered()), this, SLOT(slotChatClear()));

        findInWidget = new QAction("", this);
        findInWidget->setObjectName("findInWidget");
        findInWidget->setShortcut(tr("Ctrl+F"));
        findInWidget->setIcon(WU->getPixmap(WulforUtil::eiFIND));
        connect(findInWidget, SIGNAL(triggered()), this, SLOT(slotFind()));

        chatDisable = new QAction("", this);
        hubsFavoriteHubs->setObjectName("hubsFavoriteHubs");
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

        fileMenuActions << fileFileListBrowser
                << fileFileListBrowserLocal
                << fileFileListRefresh
                << fileHashProgress
                << separator0
                << fileOpenLogFile
                << fileOpenDownloadDirectory
                << separator1
                << fileHideWindow
                << separator2
                << fileQuit;

        hubsMenuActions << hubsHubReconnect
                << hubsQuickConnect
                << hubsFavoriteHubs
                << hubsPublicHubs
                << separator0
                << hubsFavoriteUsers;

        toolsMenuActions << toolsSearch
                << separator0
                << toolsTransfers
                << toolsDownloadQueue
                << toolsFinishedDownloads
                << toolsFinishedUploads
                << separator1
                << toolsSpy
                << toolsAntiSpam
                << toolsIPFilter
                << separator2
                << menuAwayAction
                << separator3
                << toolsHideProgressSpace
                << toolsHideLastStatus
                << toolsHideUsersStatisctics
                << separator4
                << toolsOptions;

        toolBarActions << toolsOptions
                << separator0
                << fileFileListBrowserLocal
                << fileFileListRefresh
                << fileHashProgress
                << separator1
                << hubsHubReconnect
                << hubsQuickConnect
                << separator2
                << hubsFavoriteHubs
                << hubsFavoriteUsers
                << toolsSearch
                << hubsPublicHubs
                << separator3
                << toolsTransfers
                << toolsDownloadQueue
                << toolsFinishedDownloads
                << toolsFinishedUploads
                << separator4
                << chatClear
                << findInWidget
                << chatDisable
                << separator5
                << toolsSpy
                << toolsAntiSpam
                << toolsIPFilter
                << separator6
                << fileQuit;
    }
    {
        menuWidgets = new QMenu("", this);
    }
    {
        panelsWidgets = new QAction("", this);
        panelsWidgets->setCheckable(true);
        connect(panelsWidgets, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));

        panelsTools = new QAction("", this);
        panelsTools->setCheckable(true);
        connect(panelsTools, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));

        panelsSearch = new QAction("", this);
        panelsSearch->setCheckable(true);
        connect(panelsSearch, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));
    }
    {
        aboutClient = new QAction("", this);
        aboutClient->setMenuRole(QAction::AboutRole);
        aboutClient->setIcon(WU->getPixmap(WulforUtil::eiICON_APPL));
        connect(aboutClient, SIGNAL(triggered()), this, SLOT(slotAboutClient()));

        aboutQt = new QAction("", this);
        aboutQt->setMenuRole(QAction::AboutQtRole);
        aboutQt->setIcon(WU->getPixmap(WulforUtil::eiQT_LOGO));
        connect(aboutQt, SIGNAL(triggered()), this, SLOT(slotAboutQt()));
    }
}

void MainWindow::initHotkeys(){
    // Standalone shortcuts
    ctrl_pgdown = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown), this);
    ctrl_pgup   = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp), this);
    ctrl_down   = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this);
    ctrl_up     = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this);
    ctrl_w      = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
    ctrl_m      = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this);
    del         = new QShortcut(QKeySequence(Qt::Key_Delete), this);

    if (tBar){
        connect(ctrl_pgdown, SIGNAL(activated()), tBar, SLOT(nextTab()));
        connect(ctrl_pgup,   SIGNAL(activated()), tBar, SLOT(prevTab()));
    }
    connect(ctrl_down,   SIGNAL(activated()), this, SLOT(nextMsg()));
    connect(ctrl_up,     SIGNAL(activated()), this, SLOT(prevMsg()));
    connect(ctrl_w,      SIGNAL(activated()), this, SLOT(slotCloseCurrentWidget()));
    connect(ctrl_m,      SIGNAL(activated()), this, SLOT(slotHideMainMenu()));
    connect(del,         SIGNAL(activated()), this, SLOT(slotDel()));
}

void MainWindow::initMenuBar(){
#ifdef Q_WS_MAC
    setMenuBar(new QMenuBar());
    menuBar()->setParent(NULL);

    setUnifiedTitleAndToolBarOnMac(true);

    connect(this, SIGNAL(destroyed()), menuBar(), SLOT(deleteLater()));
#endif

    {
        menuFile = new QMenu("", this);

        menuFile->addActions(fileMenuActions);
    }
    {
        menuHubs = new QMenu("", this);

        menuHubs->addActions(hubsMenuActions);
    }
    {
        menuTools = new QMenu("", this);

        menuTools->addActions(toolsMenuActions);
    }
    {
        menuPanels = new QMenu("", this);

        menuPanels->addAction(panelsWidgets);
        menuPanels->addAction(panelsTools);
        menuPanels->addAction(panelsSearch);
    }
    {
        menuAbout = new QMenu("", this);

        menuAbout->addAction(aboutClient);
        menuAbout->addAction(aboutQt);
    }

    menuBar()->addMenu(menuFile);
    menuBar()->addMenu(menuHubs);
    menuBar()->addMenu(menuTools);
    menuBar()->addMenu(menuWidgets);
    menuBar()->addMenu(menuPanels);
    menuBar()->addMenu(menuAbout);
    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);
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
    statusDSPLabel->setToolTip(tr("Download speed"));

    statusUSPLabel = new QLabel(statusBar());
    statusUSPLabel->setFrameShadow(QFrame::Plain);
    statusUSPLabel->setFrameShape(QFrame::NoFrame);
    statusUSPLabel->setAlignment(Qt::AlignRight);
    statusUSPLabel->setToolTip(tr("Upload speed"));

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
    msgLabel->setWordWrap(true);
    msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

#if (defined FREE_SPACE_BAR || defined FREE_SPACE_BAR_C)
    progressSpace = new QProgressBar(this);
    progressSpace->setMaximum(100);
    progressSpace->setMinimum(0);
    progressSpace->setMinimumWidth(100);
    progressSpace->setMaximumWidth(250);
    progressSpace->setFixedHeight(18);
    progressSpace->setToolTip(tr("Space free"));

    if (!WBGET(WB_SHOW_FREE_SPACE))
        progressSpace->hide();
#else //FREE_SPACE_BAR || FREE_SPACE_BAR_C
    WBSET(WB_SHOW_FREE_SPACE, false);
#endif //FREE_SPACE_BAR || FREE_SPACE_BAR_C

    statusBar()->addWidget(msgLabel);
    statusBar()->addPermanentWidget(statusDLabel);
    statusBar()->addPermanentWidget(statusULabel);
    statusBar()->addPermanentWidget(statusDSPLabel);
    statusBar()->addPermanentWidget(statusUSPLabel);
    statusBar()->addPermanentWidget(statusLabel);
#if (defined FREE_SPACE_BAR || defined FREE_SPACE_BAR_C)
    statusBar()->addPermanentWidget(progressSpace);
#endif //FREE_SPACE_BAR
}

void MainWindow::initSearchBar(){
    searchLineEdit = new LineEdit(this);

    connect(searchLineEdit,   SIGNAL(returnPressed()), this, SLOT(slotToolsSearch()));
}

void MainWindow::retranslateUi(){
    //Retranslate menu actions
    {
        menuFile->setTitle(tr("&File"));

        fileOpenLogFile->setText(tr("Open log file"));

        fileOpenDownloadDirectory->setText(tr("Open download directory"));

        fileFileListBrowser->setText(tr("Open filelist..."));

        fileFileListBrowserLocal->setText(tr("Open own filelist"));

        fileFileListRefresh->setText(tr("Refresh share"));

        fileHashProgress->setText(tr("Hash progress"));

        fileQuit->setText(tr("Quit"));

        menuHubs->setTitle(tr("&Hubs"));

        hubsHubReconnect->setText(tr("Reconnect to hub"));

        hubsFavoriteHubs->setText(tr("Favourite hubs"));

        hubsPublicHubs->setText(tr("Public hubs"));

        hubsFavoriteUsers->setText(tr("Favourite users"));

        hubsQuickConnect->setText(tr("Quick connect"));

        menuTools->setTitle(tr("&Tools"));

        toolsTransfers->setText(tr("Transfers"));

        toolsDownloadQueue->setText(tr("Download queue"));

        toolsFinishedDownloads->setText(tr("Finished downloads"));

        toolsFinishedUploads->setText(tr("Finished uploads"));

        toolsSpy->setText(tr("Search Spy"));

        toolsAntiSpam->setText(tr("AntiSpam module"));

        toolsIPFilter->setText(tr("IPFilter module"));

        menuAway->setTitle(tr("Away message"));

        toolsAwayOn->setText(tr("On"));

        toolsAwayOff->setText(tr("Off"));

        toolsAutoAway->setText(tr("Away when not visible"));

        toolsOptions->setText(tr("Options"));

        toolsSearch->setText(tr("Search"));

        chatClear->setText(tr("Clear chat"));

        findInWidget->setText(tr("Find/Filter"));

        chatDisable->setText(tr("Disable/enable chat"));

        menuWidgets->setTitle(tr("&Widgets"));

        menuPanels->setTitle(tr("&Panels"));

        if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
            panelsWidgets->setText(tr("Widgets panel"));
        else
            panelsWidgets->setText(tr("Widgets side dock"));

        panelsTools->setText(tr("Tools panel"));

        panelsSearch->setText(tr("Fast search panel"));

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
    fBar->setWindowTitle(tr("Actions"));
    addToolBar(fBar);

    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR)){
        tBar = new ToolBar(this);
        tBar->setObjectName("tBar");
        tBar->initTabs();
        tBar->setContextMenuPolicy(Qt::CustomContextMenu);
        tBar->setMovable(true);
        tBar->setFloatable(true);
        tBar->setAllowedAreas(Qt::AllToolBarAreas);

        wcontainer = static_cast<ArenaWidgetContainer*>(tBar);
        addToolBar(tBar);
    }

    sBar = new ToolBar(this);
    sBar->setObjectName("sBar");
    sBar->addWidget(searchLineEdit);
    sBar->setContextMenuPolicy(Qt::CustomContextMenu);
    sBar->setMovable(true);
    sBar->setFloatable(true);
    sBar->setAllowedAreas(Qt::AllToolBarAreas);
    addToolBar(sBar);
}

void MainWindow::initSideBar(){
    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
        return;

    SideBarModel *model = new SideBarModel(this);
    sideTree = new QTreeView(this);
    sideDock = new QDockWidget("", this);

    sideDock->setWidget(sideTree);
    sideDock->setFeatures(sideDock->features() & (~QDockWidget::DockWidgetClosable));
    sideDock->setObjectName("sideDock");
    sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    sideTree->setModel(model);
    sideTree->setItemsExpandable(true);
    sideTree->setSortingEnabled(false);
    sideTree->setContextMenuPolicy(Qt::CustomContextMenu);
    sideTree->expandAll();

    wcontainer = static_cast<ArenaWidgetContainer*>(model);

    addDockWidget(Qt::LeftDockWidgetArea, sideDock);

    connect(sideTree, SIGNAL(clicked(QModelIndex)),    model, SLOT(slotIndexClicked(QModelIndex)));
    connect(sideTree, SIGNAL(activated(QModelIndex)),  model, SLOT(slotIndexClicked(QModelIndex)));
    connect(sideTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSidebarContextMenu()));
    connect(model,    SIGNAL(mapWidget(ArenaWidget*)), this,  SLOT(mapWidgetOnArena(ArenaWidget*)));
    connect(model,    SIGNAL(selectIndex(QModelIndex)),this,  SLOT(slotSelectSidebarIndex(QModelIndex)));
}

ArenaWidget *MainWindow::widgetForRole(ArenaWidget::Role r) const{
    ArenaWidget *awgt = NULL;

    switch (r){
    case ArenaWidget::Downloads:
        {
            if (!DownloadQueue::getInstance()) DownloadQueue::newInstance();
            awgt = DownloadQueue::getInstance();

            break;
        }
    case ArenaWidget::FinishedUploads:
        {
            if (!FinishedUploads::getInstance()) FinishedUploads::newInstance();
            awgt = FinishedUploads::getInstance();

            break;
        }
    case ArenaWidget::FinishedDownloads:
        {
            if (!FinishedDownloads::getInstance()) FinishedDownloads::newInstance();
            awgt = FinishedDownloads::getInstance();

            break;
        }
    case ArenaWidget::FavoriteHubs:
        {
            if (!FavoriteHubs::getInstance()) FavoriteHubs::newInstance();
            awgt = FavoriteHubs::getInstance();

            break;
        }
    case ArenaWidget::FavoriteUsers:
        {
            if (!FavoriteUsers::getInstance()) FavoriteUsers::newInstance();
            awgt = FavoriteUsers::getInstance();

            break;
        }
    case ArenaWidget::PublicHubs:
        {
            if (!PublicHubs::getInstance()) PublicHubs::newInstance();
            awgt = PublicHubs::getInstance();

            break;
        }
    case ArenaWidget::Spy:
        {
            if (!SpyFrame::getInstance()) SpyFrame::newInstance();
            awgt = SpyFrame::getInstance();

            break;
        }
    default:
        break;
    }

    return awgt;
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
    statusUSPLabel->setText(map["USPEED"]+tr("/s"));
    statusDSPLabel->setText(map["DSPEED"]+tr("/s"));
    statusDLabel->setText(map["DOWN"]);
    statusULabel->setText(map["UP"]);

    if (Notification::getInstance())
        Notification::getInstance()->setToolTip(map["DSPEED"]+tr("/s"), map["USPEED"]+tr("/s"), map["DOWN"], map["UP"]);

    QFontMetrics metrics(font());

    statusUSPLabel->setFixedWidth(metrics.width(statusUSPLabel->text()) > statusUSPLabel->width()? metrics.width(statusUSPLabel->text()) + 10 : statusUSPLabel->width());
    statusDSPLabel->setFixedWidth(metrics.width(statusDSPLabel->text()) > statusDSPLabel->width()? metrics.width(statusDSPLabel->text()) + 10 : statusDSPLabel->width());
    statusDLabel->setFixedWidth(metrics.width(statusDLabel->text()) > statusDLabel->width()? metrics.width(statusDLabel->text()) + 10 : statusDLabel->width());
    statusULabel->setFixedWidth(metrics.width(statusULabel->text()) > statusULabel->width()? metrics.width(statusULabel->text()) + 10 : statusULabel->width());

    if (WBGET(WB_SHOW_FREE_SPACE)) {
#ifdef FREE_SPACE_BAR
        boost::filesystem::space_info info;
        if (boost::filesystem::exists(SETTING(DOWNLOAD_DIRECTORY)))
            info = boost::filesystem::space(boost::filesystem::path(SETTING(DOWNLOAD_DIRECTORY)));
        else if (boost::filesystem::exists(Util::getPath(Util::PATH_USER_CONFIG)))
            info = boost::filesystem::space(boost::filesystem::path(Util::getPath(Util::PATH_USER_CONFIG)));

        if (info.capacity) {
            float total = info.capacity;
            float percent = 100.0f*(total-info.available)/total;
            QString format = tr("Free %1")
                             .arg(WulforUtil::formatBytes(info.available));

            QString tooltip = tr("Free %1 of %2")
                              .arg(WulforUtil::formatBytes(info.available))
                              .arg(WulforUtil::formatBytes(total));

            progressSpace->setFormat(format);
            progressSpace->setToolTip(tooltip);
            progressSpace->setValue(static_cast<unsigned>(percent));
        }
#elif defined FREE_SPACE_BAR_C
    std::string s = SETTING(DOWNLOAD_DIRECTORY);
    unsigned long long available = 0;
    unsigned long long total = 0;
    if (!s.empty()) {
        if (MainWindow::FreeDiscSpace(s.c_str() , &available, &total) == false) {
            s = Util::getPath(Util::PATH_USER_CONFIG);
            if (MainWindow::FreeDiscSpace(s.c_str() , &available, &total) == false) {
            available = 0;
            total = 0;
            }
        }
    }
    float percent = 100.0f*(total-available)/total;
    QString format = tr("Free %1")
                         .arg(WulforUtil::formatBytes(available));

    QString tooltip = tr("Free %1 of %2")
                         .arg(WulforUtil::formatBytes(available))
                         .arg(WulforUtil::formatBytes(total));

            progressSpace->setFormat(format);
            progressSpace->setToolTip(tooltip);
            progressSpace->setValue(static_cast<unsigned>(percent));
#endif //FREE_SPACE_BAR
    }

    if ((Util::getAway() && !toolsAwayOn->isChecked()) || (!Util::getAway() && toolsAwayOff->isChecked())){
        QAction *act = Util::getAway()? toolsAwayOn : toolsAwayOff;

        act->setChecked(true);
    }
}

void MainWindow::setStatusMessage(QString msg){
    QString pre = tr("<b>Last kernel message:</b><br/>%1").replace(" ","&nbsp;");

    msgLabel->setText(msg);
    msgLabel->setToolTip(pre.arg(msg));

    msgLabel->setMaximumHeight(statusLabel->height());
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
    if (tBar)
        tBar->redraw();
    else if (sideTree)
        sideTree->repaint();

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

        if (arena->widget() == awgt->getWidget()){
            arena->setWidget(NULL);

            chatClear->setEnabled(false);
            findInWidget->setEnabled(false);
            chatDisable->setEnabled(false);
        }
    }
}

void MainWindow::mapWidgetOnArena(ArenaWidget *awgt){
    if (!arenaWidgets.contains(awgt))
        return;

    if (arena->widget())
        arena->widget()->hide();

    arena->setWidget(arenaMap[awgt]);

    setWindowTitle(awgt->getArenaTitle() + " :: " + QString("%1").arg(EISKALTDCPP_WND_TITLE));

    wcontainer->mapped(awgt);

    QWidget *wg = arenaMap[awgt];

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    ArenaWidget::Role role = awgt->role();

    HubFrame     *fr = qobject_cast<HubFrame *>(wg);
    PMWindow     *pm = qobject_cast<PMWindow *>(wg);

    bool widgetWithFilter = role == ArenaWidget::Hub || role == ArenaWidget::ShareBrowser || role == ArenaWidget::PublicHubs || role == ArenaWidget::Search;

    chatClear->setEnabled(role == ArenaWidget::Hub || role == ArenaWidget::PrivateMessage);
    findInWidget->setEnabled(widgetWithFilter);
    chatDisable->setEnabled(role == ArenaWidget::Hub);

    if (fr)
        fr->slotActivate();
    else if(pm)
        pm->slotActivate();
    else
        wg->setFocus();
}

void MainWindow::remWidgetFromArena(ArenaWidget *awgt){
    if (!arenaWidgets.contains(awgt))
        return;

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(false);

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

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    wcontainer->insertWidget(awgt);
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

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(false);

    wcontainer->removeWidget(awgt);
}

void MainWindow::toggleSingletonWidget(ArenaWidget *a){
    if (!a)
        return;

    if (sender() && typeid(*sender()) == typeid(QAction) && a->getWidget()){
        QAction *act = reinterpret_cast<QAction*>(sender());;

        act->setCheckable(typeid(*wcontainer) == typeid(ToolBar));

        a->setToolButton(act);
    }

    if (wcontainer->hasWidget(a)){
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

        wcontainer->removeWidget(a);
    }
    else {
        QAction *act = new QAction(a->getArenaShortTitle(), this);
        act->setIcon(a->getPixmap());

        connect(act, SIGNAL(triggered()), this, SLOT(slotWidgetsToggle()));

        menuWidgetsActions.push_back(act);
        menuWidgetsHash.insert(act, a);

        menuWidgets->clear();
        menuWidgets->addActions(menuWidgetsActions);

        mapWidgetOnArena(a);

        wcontainer->insertWidget(a);
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

void MainWindow::reloadSomeSettings(){
    for (int k = 0; k < arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(arenaMap[arenaWidgets.at(k)]);
        PMWindow *pm = qobject_cast<PMWindow *>(arenaMap[arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
        else if (pm)
            pm->reloadSomeSettings();
    }
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

void MainWindow::slotFileOpenDownloadDirectory(){
    QString dir = QString::fromStdString(SETTING(DOWNLOAD_DIRECTORY));

    if (!dir.isEmpty())
        QDesktopServices::openUrl(dir);
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

void MainWindow::slotHubsReconnect(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->reconnect();
}

void MainWindow::slotToolsSearch(){
    SearchFrame *sf = new SearchFrame();

    sf->setAttribute(Qt::WA_DeleteOnClose);

    QLineEdit *le = qobject_cast<QLineEdit *>(sender());

    if (le == searchLineEdit){
        QString text = searchLineEdit->text();
        bool isTTH = false;

        if (!text.isEmpty()){
            if (text.startsWith("magnet:")){
                QString link = text;
                QString tth = "", name = "";
                int64_t size = 0;

                WulforUtil::splitMagnet(link, size, tth, name);

                text  = tth;
                isTTH = true;
            }
            else if (text.length() == 39){
                if (text.contains(QRegExp("[A-Z0-9]",Qt::CaseSensitive)))
                    isTTH = true;
            }
        }

        sf->fastSearch(text, isTTH);
    }
}

void MainWindow::slotToolsDownloadQueue(){
    if (!DownloadQueue::getInstance())
        DownloadQueue::newInstance();

    toggleSingletonWidget(DownloadQueue::getInstance());
}

void MainWindow::slotToolsFinishedDownloads(){
    if (!FinishedDownloads::getInstance())
        FinishedDownloads::newInstance();

    toggleSingletonWidget(FinishedDownloads::getInstance());
}

void MainWindow::slotToolsFinishedUploads(){
    if (!FinishedUploads::getInstance())
        FinishedUploads::newInstance();

    toggleSingletonWidget(FinishedUploads::getInstance());
}

void MainWindow::slotToolsSpy(){
    if (!SpyFrame::getInstance())
        SpyFrame::newInstance();

    toggleSingletonWidget(SpyFrame::getInstance());
}

void MainWindow::slotToolsAntiSpam(){
    AntiSpamFrame fr(this);

    fr.exec();
}

void MainWindow::slotToolsIPFilter(){
    IPFilterFrame fr(this);

    fr.exec();
}

void MainWindow::slotToolsAutoAway(){
    WBSET(WB_APP_AUTO_AWAY, toolsAutoAway->isChecked());
}

void MainWindow::slotToolsSwitchAway(){
    bool away = (sender() == toolsAwayOn)? true : false;

    Util::setAway(away);
    Util::setManualAway(away);
}

void MainWindow::slotHubsFavoriteHubs(){
    if (!FavoriteHubs::getInstance())
        FavoriteHubs::newInstance();

    toggleSingletonWidget(FavoriteHubs::getInstance());
}

void MainWindow::slotHubsPublicHubs(){
    if (!PublicHubs::getInstance())
        PublicHubs::newInstance();

    toggleSingletonWidget(PublicHubs::getInstance());
}

void MainWindow::slotHubsFavoriteUsers(){
    if (!FavoriteUsers::getInstance())
        FavoriteUsers::newInstance();

    toggleSingletonWidget(FavoriteUsers::getInstance());
}

void MainWindow::slotToolsSettings(){
    Settings s;

    s.exec();

    //reload some settings
    if (!WBGET(WB_TRAY_ENABLED))
        fileHideWindow->setText(tr("Show/hide find frame"));
    else
        fileHideWindow->setText(tr("Hide window"));
}

void MainWindow::slotToolsTransfer(bool toggled){
    if (toggled){
        transfer_dock->setVisible(true);
        transfer_dock->setWidget(TransferView::getInstance());
    }
    else {
        transfer_dock->setWidget(NULL);
        transfer_dock->setVisible(false);
    }
}

void MainWindow::slotPanelMenuActionClicked(){
    QAction *act = qobject_cast<QAction *>(sender());

    if (act == 0)
        return;

    if (act == panelsWidgets){
        if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
            tBar->setVisible(panelsWidgets->isChecked());
        else
            sideDock->setVisible(panelsWidgets->isChecked());
        WBSET(WB_WIDGETS_PANEL_VISIBLE, panelsWidgets->isChecked());
    }
    else if (act == panelsTools){
        fBar->setVisible(panelsTools->isChecked());
        WBSET(WB_TOOLS_PANEL_VISIBLE, panelsTools->isChecked());
    }
    else if (act == panelsSearch){
        sBar->setVisible(panelsSearch->isChecked());
        WBSET(WB_SEARCH_PANEL_VISIBLE, panelsSearch->isChecked());
    }
}

void MainWindow::slotChatClear(){
    HubFrame *fr = qobject_cast<HubFrame *>(arena->widget());
    PMWindow *pm = qobject_cast<PMWindow *>(arena->widget());

    if (fr)
        fr->clearChat();
    else if (pm)
        pm->clearChat();
}

void MainWindow::slotFind(){
    if (!arena->widget() || !qobject_cast<ArenaWidget*>(arena->widget()))
        return;

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(arena->widget());
    awgt->CTRL_F_pressed();
}

void MainWindow::slotDel(){
    if (!arena->widget() || !qobject_cast<ArenaWidget*>(arena->widget()))
        return;

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(arena->widget());
    awgt->DEL_pressed();
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

void MainWindow::slotHideMainMenu(){
    bool b = menuBar()->isVisible();
    menuBar()->setVisible(!b);
    WBSET(WB_MAIN_MENU_VISIBLE, !b);
}

void MainWindow::slotHideWindow(){
    QWidget *wg = arena->widget();

    HubFrame     *fr = qobject_cast<HubFrame *>(wg);
    PublicHubs   *ph = qobject_cast<PublicHubs *>(wg);
    ShareBrowser *sb = qobject_cast<ShareBrowser *>(wg);
    SearchFrame  *sf = qobject_cast<SearchFrame *>(wg);

    if (fr){
        if (fr->isFindFrameActivated() && WBGET(WB_TRAY_ENABLED)){
            fr->slotHideFindFrame();
            return;
        }
        else if (!WBGET(WB_TRAY_ENABLED)){
            fr->slotHideFindFrame();
            return;
        }
    }
    else if (ph){
        if (ph->isFindFrameActivated()){
            ph->slotFilter();
            return;
        }
        else if (!WBGET(WB_TRAY_ENABLED)){
            ph->slotFilter();
            return;
        }
    }
    else if (sb){
        if (sb->isFindFrameActivated()){
            sb->slotFilter();
            return;
        }
        else if (!WBGET(WB_TRAY_ENABLED)){
            sb->slotFilter();
            return;
        }
    }
    else if (sf){
        if (sf->isFindFrameActivated()){
            sf->slotFilter();
            return;
        }
        else if (!WBGET(WB_TRAY_ENABLED)){
            sf->slotFilter();
            return;
        }
    }

    if (!isUnload && isActiveWindow() && WBGET(WB_TRAY_ENABLED)) {
        hide();
    }
}

void MainWindow::slotHideProgressSpace() {
    if (WBGET(WB_SHOW_FREE_SPACE)) {
        progressSpace->hide();
        toolsHideProgressSpace->setText(tr("Show free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, false);
    } else {
        progressSpace->show();
        toolsHideProgressSpace->setText(tr("Hide free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, true);
    }
}

void MainWindow::slotHideLastStatus(){
    bool st = WBGET(WB_LAST_STATUS);

    st = !st;

    if (!st)
        toolsHideLastStatus->setText(tr("Show last status message"));
    else
        toolsHideLastStatus->setText(tr("Hide last status message"));

    WBSET(WB_LAST_STATUS, st);

    for (int k = 0; k < arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(arenaMap[arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }
}

void MainWindow::slotHideUsersStatistics(){
    bool st = WBGET(WB_USERS_STATISTICS);

    st = !st;

    if (!st)
        toolsHideUsersStatisctics->setText(tr("Show users statistics"));
    else
        toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

    WBSET(WB_USERS_STATISTICS, st);

    for (int k = 0; k < arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(arenaMap[arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
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
    a.label_ABOUT->setText(QString("%1<br/><br/> %2 %3 %4<br/><br/> %5 %6<br/><br/> %7 <b>%8</b> <br/> %9 <b>%10</b>")
                           .arg(tr("EiskaltDC++ is a graphical client for Direct Connect and ADC protocols."))
                           .arg(tr("DC++ core version:"))
                           .arg(DCVERSIONSTRING)
                           .arg(tr("(modified)"))
                           .arg(tr("Home page:"))
                           .arg("<a href=\"http://code.google.com/p/eiskaltdc/\">"
                                "http://code.google.com/p/eiskaltdc/</a>")
                           .arg(tr("Total up:"))
                           .arg(WulforUtil::formatBytes(app_total_up))
                           .arg(tr("Total down:"))
                           .arg(WulforUtil::formatBytes(app_total_down)));

    a.textBrowser_AUTHORS->setText(
            tr("Please use <a href=\"http://code.google.com/p/eiskaltdc/issues/list\">"
               "http://code.google.com/p/eiskaltdc/issues/list</a> to report bugs.<br/>")+
            tr("<br/>"
               "<b>Developers</b><br/>")+
            tr("<br/>"
               "&nbsp;Andrey Karlov<br/>"
               "&nbsp;&lt;dein.negativ@gmail.com&gt;<br/>"
               "&nbsp;(main developer 0.4.10 and later)<br/>")+
            tr("<br/>"
               "&nbsp;Boris Pek  aka  Tehnick<br/>"
               "&nbsp;&lt;tehnick-8@mail.ru&gt;<br/>"
               "&nbsp;(maintainer and developer 1.89.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;Eugene Petrov<br/>"
               "&nbsp;&lt;dhamp@ya.ru&gt;<br/>"
               "&nbsp;(maintainer and developer 0.4.10 and later)<br/>")+
            tr("<br/>"
               "<b>Logo and Splash Screen Logo</b><br/>")+
            tr("<br/>"
               "&nbsp;Uladzimir Bely<br/>"
               "&nbsp;&lt;wiselord1983@gmail.com&gt;<br/>"
               "&nbsp;(version 0.4.10 and later)<br/>")
            );

    a.textBrowser_TRANSLATION->setText(
            tr("<b>Translators</b><br/>")+
            tr("<br/>"
               "&nbsp;<u>Russian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Uladzimir Bely<br/>"
               "&nbsp;&lt;wiselord1983@gmail.com&gt;<br/>"
               "&nbsp;(for 0.4.10 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Belarusian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Paval Shalamitski  aka  Klyok<br/>"
               "&nbsp;&lt;i.kliok@gmail.com&gt;<br/>"
               "&nbsp;(for 1.0.40 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Hungarian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Akos Berki  aka  sumo<br/>"
               "&nbsp;&lt;husumo@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.1 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>French translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Alexandre Wallimann  aka  Ale<br/>"
               "&nbsp;&lt;alexandre.wallimann@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.2 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Polish translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Arahael<br/>"
               "&nbsp;(for 2.0.2 and later)<br/>")
            );

    a.exec();
}

void MainWindow::slotUnixSignal(int sig){
    printf("%i\n");
}

void MainWindow::slotCloseCurrentWidget(){
    if (arena->widget())
        arena->widget()->close();
}

void MainWindow::slotSidebarContextMenu(){
    QItemSelectionModel *s_m =sideTree->selectionModel();
    QModelIndexList selected = s_m->selectedRows(0);

    if (selected.size() < 1)
        return;

    SideBarItem *item = reinterpret_cast<SideBarItem*>(selected.at(0).internalPointer());

    if (!(item && item->getWidget() && item->getWidget()->getMenu()))
        return;

    item->getWidget()->getMenu()->exec(QCursor::pos());
}

void MainWindow::slotSelectSidebarIndex(const QModelIndex &index){
    QItemSelectionModel *s_m =sideTree->selectionModel();

    s_m->select(index, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
}

void MainWindow::slotAboutQt(){
    QMessageBox::aboutQt(this);
}

void MainWindow::nextMsg(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->nextMsg();
    else{
        QWidget *wg = arena->widget();

        bool pmw = false;

        if (wg != 0)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm)
                pm->nextMsg();
        }
    }
}

void MainWindow::prevMsg(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->prevMsg();
    else{
        QWidget *wg = arena->widget();

        bool pmw = false;

        if (wg != 0)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm)
                pm->prevMsg();
        }
    }
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
    map["DSPEED"]   = WulforUtil::formatBytes(downBytes);
    map["DOWN"]     = WulforUtil::formatBytes(Socket::getTotalDown());
    map["USPEED"]   = WulforUtil::formatBytes(upBytes);
    map["UP"]       = WulforUtil::formatBytes(Socket::getTotalUp());

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
#ifdef FREE_SPACE_BAR_C
bool MainWindow::FreeDiscSpace ( std::string path,  unsigned long long * res, unsigned long long * res2) {
        if ( !res ) {
            return false;
        }

#ifdef WIN32
        ULARGE_INTEGER lpFreeBytesAvailableToCaller; // receives the number of bytes on
                                               // disk available to the caller
        ULARGE_INTEGER lpTotalNumberOfBytes;    // receives the number of bytes on disk
        ULARGE_INTEGER lpTotalNumberOfFreeBytes; // receives the free bytes on disk

        if ( GetDiskFreeSpaceEx( path.c_str(), &lpFreeBytesAvailableToCaller,
                                &lpTotalNumberOfBytes,
                                &lpTotalNumberOfFreeBytes ) == true ) {
                *res = lpTotalNumberOfFreeBytes.QuadPart;
                *res2 = lpTotalNumberOfBytes.QuadPart;
                return true;
        } else {
            return false;
        }
#else //WIN32
        struct fs_usage fsp;
        if ( get_fs_usage(path.c_str(),path.c_str(),&fsp) == 0 ) {
                // printf("ok %d\n",fsp.fsu_bavail_top_bit_set);
                *res = fsp.fsu_bavail*fsp.fsu_blocksize;
                *res2 =fsp.fsu_blocks*fsp.fsu_blocksize;
                return true;
        } else {
                printf("ERROR: %s doesn't exist\n",path.c_str());
                return false;
        }
#endif //WIN32
}
#endif //FREE_SPACE_BAR_C
