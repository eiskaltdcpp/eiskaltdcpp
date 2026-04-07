/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "MainWindow.h"
#include "Notification.h"

#include <stdlib.h>
#include <string>
#include <iostream>

#include <QPainter>
#include <QPushButton>
#include <QSize>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QtDebug>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include <QProgressBar>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>
#include <QDockWidget>
#include <QLabel>
#include <QShortcut>
#include <QKeySequence>
#include <QToolButton>
#include <QTreeView>
#include <QMetaType>
#include <QTimer>
#include <QAction>
#include <QActionGroup>
#include <QStatusBar>
#include <QDate>

#include "ArenaWidgetManager.h"
#include "ArenaWidgetFactory.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "HashProgress.h"
#include "PMWindow.h"
#include "TransferView.h"
#include "ShareBrowser.h"
#include "QuickConnect.h"
#include "SearchFrame.h"
#include "ADLS.h"
#include "CmdDebug.h"
#include "Secretary.h"
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
#include "FileHasher.h"
#include "SpyFrame.h"
#include "SideBar.h"
#include "ActionCustomizer.h"
#include "MultiLineToolBar.h"
#include "SearchBlacklist.h"
#include "QueuedUsers.h"
#ifdef FREE_SPACE_BAR_C
#include "extra/freespace.h"
#endif
#ifdef USE_JS
#include "ScriptManagerDialog.h"
#include "scriptengine/ScriptConsole.h"
#include "scriptengine/ScriptEngine.h"
#endif

#include "dcpp/ShareManager.h"
#include "dcpp/ConnectivityManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/DCPlusPlus.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "QtContext.h"
#include "QtContextAware.h"

using namespace std;

class MainWindowPrivate {
public:
        typedef QList<QAction*> ActionList;

        bool isUnload = false;
        bool exitBegin = false;

        // position and geometry
        bool showMax = false;
        int w = 800;
        int h = 600;
        int xPos = 0;
        int yPos = 0;

        // Widgets
        QDockWidget *arena = nullptr;
        QDockWidget *transfer_dock = nullptr;
        QDockWidget *sideDock = nullptr;

        ToolBar *fBar = nullptr; //for actions
        ToolBar *sBar = nullptr; //for fast search

        QStringList core_msg_history;

        LineEdit *searchLineEdit = nullptr;
        QLabel *statusLabel = nullptr;
        QLabel *statusSPLabel = nullptr;
        QLabel *statusDLabel = nullptr;
        QLabel *statusTRLabel = nullptr;
        QLabel *msgLabel = nullptr;

#if defined(USE_PROGRESS_BARS)
        QProgressBar *progressFreeSpace = nullptr;
#else
        QLabel *progressFreeSpace = nullptr;
#endif
        QProgressBar *progressHashing = nullptr;
        HashProgress *_progress_dialog = nullptr; // Hashing progress dialog

        QMenu   *menuFile = nullptr;
        QAction *fileOpenMagnet = nullptr;
        QAction *fileFileListBrowser = nullptr;
        QAction *fileFileHasher = nullptr;
        QAction *fileFileListBrowserLocal = nullptr;
        QAction *fileFileListMatchAll = nullptr;
        QAction *fileRefreshShareHashProgress = nullptr;
        QAction *fileOpenLogFile = nullptr;
        QAction *fileOpenDownloadDirectory = nullptr;
        QAction *fileHideWindow = nullptr;
        QAction *fileQuit = nullptr;

        QMenu   *menuHubs = nullptr;
        QAction *hubsHubReconnect = nullptr;
        QAction *hubsQuickConnect = nullptr;
        QAction *hubsFavoriteHubs = nullptr;
        QAction *hubsPublicHubs = nullptr;
        QAction *hubsFavoriteUsers = nullptr;

        QMenu   *menuTools = nullptr;
        QAction *toolsSearch = nullptr;
        QAction *toolsADLS = nullptr;
        QAction *toolsCmdDebug = nullptr;
        QAction *toolsTransfers = nullptr;
        QAction *toolsDownloadQueue = nullptr;
        QAction *toolsQueuedUsers = nullptr;
        QAction *toolsFinishedDownloads = nullptr;
        QAction *toolsFinishedUploads = nullptr;
        QAction *toolsSecretary = nullptr;
        QAction *toolsSearchSpy = nullptr;
        QAction *toolsAntiSpam = nullptr;
        QAction *toolsIPFilter = nullptr;
        QAction *menuAwayAction = nullptr;
        QAction *toolsHubManager = nullptr;
        // submenu
        QMenu   *menuAway = nullptr;
        QActionGroup *awayGroup = nullptr;
        QAction *toolsAwayOn = nullptr;
        QAction *toolsAwayOff = nullptr;
        QAction *toolsAutoAway = nullptr;
        // end
        QAction *toolsHideProgressSpace = nullptr;
        QAction *toolsHideLastStatus = nullptr;
        QAction *toolsHideUsersStatisctics = nullptr;
        QAction *toolsCopyWindowTitle = nullptr;
        QAction *toolsOptions = nullptr;
#ifdef USE_JS
        QAction *toolsJS = nullptr;
        QAction *toolsJSConsole = nullptr;
        ScriptConsole *scriptConsole = nullptr;
#endif
        QAction *toolsSwitchSpeedLimit = nullptr;

        QMenu   *menuPanels = nullptr;
        // submenu
        QMenu   *sh_menu = nullptr;
        // end
        QAction *panelsWidgets = nullptr;
        QAction *panelsTools = nullptr;
        QAction *panelsSearch = nullptr;

        // Standalone shortcuts
        QAction *prevTabShortCut = nullptr;
        QAction *nextTabShortCut = nullptr;
        QAction *prevMsgShortCut = nullptr;
        QAction *nextMsgShortCut = nullptr;
        QAction *closeWidgetShortCut = nullptr;
        QAction *toggleMainMenuShortCut = nullptr;

        QAction *chatDisable = nullptr;
        QAction *findInWidget = nullptr;
        QAction *chatClear = nullptr;

        QMenu *menuWidgets = nullptr;
        QHash<QAction*, ArenaWidget*> menuWidgetsHash;

        QMenu   *menuAbout = nullptr;
        QAction *aboutHomepage = nullptr;
        QAction *aboutBuilds = nullptr;
        QAction *aboutSource = nullptr;
        QAction *aboutIssues = nullptr;
        QAction *aboutWiki = nullptr;
        QAction *aboutChangelog = nullptr;
        QAction *aboutClient = nullptr;
        QAction *aboutQt = nullptr;

        ActionList toolBarActions;
        ActionList fileMenuActions;
        ActionList hubsMenuActions;
        ActionList toolsMenuActions;

        QMenu *favHubMenu = nullptr;
};

static const QString &TOOLBUTTON_STYLE = "mainwindow/toolbar-toolbutton-style";
static const QString &EMPTY_SETTINGS = "mainwindow/empty-settings";
static const QString &SIDEBAR_SHOW_CLOSEBUTTONS = "mainwindow/sidebar-with-close-buttons";

MainWindow::MainWindow (dcpp::DCContext& ctx, QWidget *parent):
        QtContextAware(ctx),
        QMainWindow(parent),
        d_ptr(new MainWindowPrivate())
{
    Q_D(MainWindow);

    d->statusLabel = nullptr;
    d->fBar = nullptr;
    d->sBar = nullptr;
    d->_progress_dialog = nullptr;
    d->sideDock = nullptr;
    d->menuPanels = nullptr;
#ifdef USE_JS
    d->scriptConsole = nullptr;
#endif
    d->favHubMenu = nullptr;

    d->exitBegin = false;

    if (qtCtx()->settings()->getBool(WB_ANTISPAM_ENABLED)){
        qtCtx()->createAntiSpam();

        qtCtx()->antiSpam()->loadLists();
        qtCtx()->antiSpam()->loadSettings();
    }

    qtCtx()->createShortcutManager();

    init();

    retranslateUi();

    dcCtx().getLogManager()->addListener(this);
    dcCtx().getTimerManager()->addListener(this);
    dcCtx().getQueueManager()->addListener(this);

    startSocket(false);

    setStatusMessage(tr("Ready"));

    qtCtx()->createTransferView();

    d->transfer_dock->setWidget(qtCtx()->transferView());
    d->toolsTransfers->setChecked(d->transfer_dock->isVisible());

    if (!qtCtx()->settings()->getStr(WS_APP_THEME).isEmpty())
        qApp->setStyle(qtCtx()->settings()->getStr(WS_APP_THEME));

    if (qtCtx()->settings()->getBool(WB_APP_REMOVE_NOT_EX_DIRS)){
        StringPairList directories = dcCtx().getShareManager()->getDirectories();
        for (const auto &it : directories){
            QDir dir(_q(it.second));

            if (!dir.exists()){
                try {
                    dcCtx().getShareManager()->removeDirectory(it.second);
                }
                catch (const std::exception&){}
            }
        }
    }

}

HashProgress* MainWindow::progress_dialog() {
    Q_D(MainWindow);

    if (!d->_progress_dialog)
        d->_progress_dialog = new HashProgress(this);

    return d->_progress_dialog;
}

MainWindow::~MainWindow(){
    dcCtx().getLogManager()->removeListener(this);
    dcCtx().getTimerManager()->removeListener(this);
    dcCtx().getQueueManager()->removeListener(this);

    if (qtCtx()->antiSpam()){
        qtCtx()->antiSpam()->saveLists();
        qtCtx()->antiSpam()->saveSettings();
        qtCtx()->destroyAntiSpam();
    }

    Q_D(MainWindow);

    delete d->arena;
    delete d->fBar;
    delete d->sBar;

    // ShortcutManager is destroyed by QtContext

    delete d_ptr;
}

void MainWindow::setUnload ( bool b ) {
    Q_D(MainWindow);

    d->isUnload = b;
}

void MainWindow::closeEvent(QCloseEvent *c_e){
    Q_D(MainWindow);

#if defined(Q_OS_MAC)
    if (!d->isUnload){
#else // defined(Q_OS_MAC)
    if (!d->isUnload && qtCtx()->settings()->getBool(WB_TRAY_ENABLED)){
#endif // defined(Q_OS_MAC)
        hide();
        c_e->ignore();

        return;
    }

    if (d->isUnload && qtCtx()->settings()->getBool(WB_EXIT_CONFIRM) && !d->exitBegin){
        QMessageBox::StandardButton ret;

        QString dlg_message = tr("Exit program?");
        if (QDate::currentDate().day() == 1 && QDate::currentDate().month() == 4) {
            dlg_message = tr("Kill all humans?");
        }
        ret = QMessageBox::question(this,  tr("Action confirm"),
                                    dlg_message,
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);

        if (ret == QMessageBox::Yes){
            d->exitBegin = true;
        }
        else{
            setUnload(false);

            c_e->ignore();

            return;
        }
    }

    saveSettings();

    if (qtCtx()->settings()->getBool("app/clear-search-history-on-exit", false))
        qtCtx()->settings()->setStr(WS_SEARCH_HISTORY, "");
    if (qtCtx()->settings()->getBool("app/clear-download-directories-history-on-exit", false))
        qtCtx()->settings()->setStr(WS_DOWNLOAD_DIR_HISTORY, "");

    if (d->sideDock)
        d->sideDock->hide();

    d->transfer_dock->hide();

    blockSignals(true);

    if (qtCtx()->transferView()){
        qtCtx()->transferView()->close();
        // TransferView is destroyed by QtContext
    }

    if (dcCtx().getSearchManager())
        dcCtx().getSearchManager()->disconnect();

    if (dcCtx().getConnectionManager())
        dcCtx().getConnectionManager()->disconnect();

    if (qtCtx()->notification()){
        qtCtx()->destroyNotification();
    }

    // Close all hub connections before quitting.  This triggers each
    // HubFrame::closeEvent() which properly disconnects and releases
    // the dcpp Client, preventing use-after-free crashes during
    // dcpp::shutdown().
    if (qtCtx()->hubManager()) {
        QList<QObject*> hubList = qtCtx()->hubManager()->getHubs();
        for (QObject *obj : hubList) {
            HubFrame *hub = qobject_cast<HubFrame*>(obj);
            if (hub)
                hub->close();
        }
    }

    d->arena->hide();
    d->arena->setWidget(nullptr);

    c_e->accept();

    // In Qt6 the quit-on-last-window-closed mechanism may not fire when
    // blockSignals(true) has been called on this widget.  Explicitly ask
    // the application to quit so that QCoreApplication::exec() returns.
    qApp->quit();
}

void MainWindow::beginExit(){
    Q_D(MainWindow);

    d->exitBegin = true;

    setUnload(true);
}

void MainWindow::show(){
    Q_D(MainWindow);

    if (d->showMax)
        showMaximized();
    else
        showNormal();
}

void MainWindow::showEvent(QShowEvent *e){
    Q_D(MainWindow);

    if (!d->showMax && d->w > 0 && d->h > 0 && d->w != width() && d->h != height())
        this->resize(QSize(d->w, d->h));

    if (qtCtx()->settings()->getBool(WB_APP_AUTO_AWAY) && !Util::getManualAway()){
        Util::setAway(dcCtx(), false);

        d->toolsAwayOff->setChecked(true);
    }

    if (d->transfer_dock->isVisible())
        d->toolsTransfers->setChecked(true);

    if (d->sideDock)
        d->sideDock->setVisible(d->panelsWidgets->isChecked());

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(d->arena->widget());

    if (!awgt)
        return;

    ArenaWidget::Role role = awgt->role();

    bool widgetWithFilter = role == ArenaWidget::Hub ||
                            role == ArenaWidget::PrivateMessage ||
                            role == ArenaWidget::ShareBrowser ||
                            role == ArenaWidget::PublicHubs ||
                            role == ArenaWidget::Search;

    d->chatClear->setEnabled(role == ArenaWidget::Hub || role == ArenaWidget::PrivateMessage);
    d->findInWidget->setEnabled(widgetWithFilter);
    d->chatDisable->setEnabled(role == ArenaWidget::Hub);

    if (_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::NICK, true)).isEmpty()){
        activateWindow();
        raise();

        bool ok = false;
        QString new_nick = QInputDialog::getText(this, tr("Enter user nick"), tr("Nick"), QLineEdit::Normal, tr("User"), &ok);

        if (ok && !new_nick.isEmpty()){
            dcCtx().getSettingsManager()->set(SettingsManager::NICK, _tq(new_nick));

            ok = (QMessageBox::question(this, "EiskaltDC++", tr("Would you like to change other settings?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No);
        }

        if (!ok)
            slotToolsSettings();
    }

    e->accept();
}

void MainWindow::getWindowGeometry(){
    Q_D(MainWindow);

    d->showMax = isMaximized();

    if (!d->showMax){
        d->h = height();
        d->w = width();
        d->xPos = x();
        d->yPos = y();
    }
}

void MainWindow::hideEvent(QHideEvent *e){
    Q_D(MainWindow);

    getWindowGeometry();

    e->accept();

    if (d->sideDock && d->sideDock->isFloating())
        d->sideDock->hide();

    if (qtCtx()->settings()->getBool(WB_APP_AUTO_AWAY)){
        Util::setAway(dcCtx(), true);

        d->toolsAwayOn->setChecked(true);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e){
    Q_D(MainWindow);

    if (e->type() == QEvent::WindowActivate) {
        redrawToolPanel();
    }
    else if( obj == d->progressHashing && e->type() == QEvent::MouseButtonDblClick ) {
        slotFileHashProgress();

        return true;
    }
    else if (obj == d->progressFreeSpace && e->type() == QEvent::MouseButtonDblClick ){
        slotFileOpenDownloadDirectory();

        return true;
    }

    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::init(){
    Q_D(MainWindow);

    setObjectName("MainWindow");

    connect(this, &MainWindow::coreLogMessage, this, &MainWindow::setStatusMessage, Qt::QueuedConnection);
    connect(this, &MainWindow::coreOpenShare, this, &MainWindow::showShareBrowser, Qt::QueuedConnection);
    connect(this, &MainWindow::coreUpdateStats, this, &MainWindow::updateStatus, Qt::QueuedConnection);

    d->arena = new QDockWidget();
    d->arena->setWidget(nullptr);
    d->arena->setFloating(false);
    d->arena->setContentsMargins(0, 0, 0, 0);
    d->arena->setAllowedAreas(Qt::RightDockWidgetArea);
    d->arena->setFeatures(QDockWidget::NoDockWidgetFeatures);
    d->arena->setContextMenuPolicy(Qt::CustomContextMenu);
    d->arena->setTitleBarWidget(new QWidget(d->arena));
    d->arena->setMinimumSize( 10, 10 );

    d->transfer_dock = new QDockWidget(this);
    d->transfer_dock->setWidget(nullptr);
    d->transfer_dock->setFloating(false);
    d->transfer_dock->setObjectName("transfer_dock");
    d->transfer_dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    d->transfer_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    d->transfer_dock->setContextMenuPolicy(Qt::CustomContextMenu);
    d->transfer_dock->setTitleBarWidget(new QWidget(d->transfer_dock));
    d->transfer_dock->setMinimumSize(QSize(8, 8));

    setCentralWidget(d->arena);
    //addDockWidget(Qt::RightDockWidgetArea, arena);
    addDockWidget(Qt::BottomDockWidgetArea, d->transfer_dock);

    d->transfer_dock->hide();

    this->setWindowIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiICON_APPL));

    setWindowTitle(QString::fromStdString(eiskaltdcppAppNameString));

    initActions();

    initMenuBar();
#if defined(Q_OS_MAC)
    initDockMenuBar();
#endif

    initStatusBar();

    initSearchBar();

    initToolbar();

    initSideBar();

    loadSettings();

    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::slotExit);

    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::activated, this, &MainWindow::mapWidgetOnArena);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::added,     this, &MainWindow::insertWidget);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::removed,   this, &MainWindow::removeWidget);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::updated,   this, &MainWindow::updated);

#ifdef LUA_SCRIPT
    dcCtx().getScriptManager()->load();
    if (qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::USE_LUA, true)){
        // Start as late as possible, as we might (formatting.lua) need to examine settings
        string defaultluascript="startup.lua";
        dcCtx().getScriptManager()->EvaluateFile(defaultluascript);
    }
#endif
}

void MainWindow::loadSettings(){
    Q_D(MainWindow);
    WulforSettings *WS = qtCtx()->settings();

    d->showMax = WS->getBool(WB_MAINWINDOW_MAXIMIZED);
    d->w = WS->getInt(WI_MAINWINDOW_WIDTH);
    d->h = WS->getInt(WI_MAINWINDOW_HEIGHT);
    d->xPos = WS->getInt(WI_MAINWINDOW_X);
    d->yPos = WS->getInt(WI_MAINWINDOW_Y);

    QPoint p(d->xPos, d->yPos);
    QSize  sz(d->w, d->h);

    if (p.x() >= 0 && p.y() >= 0)
        this->move(p);

    if (sz.width() > 0 && sz.height() > 0)
        this->resize(sz);

    QString dockwidgetsState = qtCtx()->settings()->getStr(WS_MAINWINDOW_STATE);

    if (!dockwidgetsState.isEmpty())
        this->restoreState(QByteArray::fromBase64(dockwidgetsState.toUtf8()));

    d->fBar->setVisible(qtCtx()->settings()->getBool(WB_TOOLS_PANEL_VISIBLE));
    d->panelsTools->setChecked(qtCtx()->settings()->getBool(WB_TOOLS_PANEL_VISIBLE));

    d->sBar->setVisible(qtCtx()->settings()->getBool(WB_SEARCH_PANEL_VISIBLE));
    d->panelsSearch->setChecked(qtCtx()->settings()->getBool(WB_SEARCH_PANEL_VISIBLE));

    if (d->sideDock){
        if (d->sideDock->isFloating() && qtCtx()->settings()->getBool(WB_MAINWINDOW_HIDE) && qtCtx()->settings()->getBool(WB_TRAY_ENABLED))
            d->sideDock->hide();
        else
            d->sideDock->setVisible(qtCtx()->settings()->getBool(WB_WIDGETS_PANEL_VISIBLE));
    } else if ( findChild<MultiLineToolBar*> ( "multiLineTabbar" ) ) {
        findChild<MultiLineToolBar*> ( "multiLineTabbar" )->setVisible ( qtCtx()->settings()->getBool( WB_WIDGETS_PANEL_VISIBLE ) );
    } else if ( findChild<ToolBar*> ( "tBar" ) ) {
        findChild<ToolBar*> ( "tBar" )->setVisible ( qtCtx()->settings()->getBool( WB_WIDGETS_PANEL_VISIBLE ) );
    }

    d->panelsWidgets->setChecked(qtCtx()->settings()->getBool(WB_WIDGETS_PANEL_VISIBLE));

    if (!qtCtx()->settings()->getBool(WB_MAIN_MENU_VISIBLE))
        toggleMainMenu(false);

    if (qtCtx()->settings()->getBool("mainwindow/dont-show-icons-in-menus", false))
        qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
}

void MainWindow::saveSettings(){
    Q_D(MainWindow);
    static bool stateIsSaved = false;

    if (stateIsSaved)
        return;

    if (isVisible())
        d->showMax = isMaximized();

    qtCtx()->settings()->setBool(WB_MAINWINDOW_MAXIMIZED, d->showMax);

    if (!d->showMax && height() > 0 && width() > 0){
        qtCtx()->settings()->setInt(WI_MAINWINDOW_HEIGHT, height());
        qtCtx()->settings()->setInt(WI_MAINWINDOW_WIDTH, width());
    }
    else{
        qtCtx()->settings()->setInt(WI_MAINWINDOW_HEIGHT, d->h);
        qtCtx()->settings()->setInt(WI_MAINWINDOW_WIDTH, d->w);
    }

    if (!d->showMax && x() >= 0 && y() >= 0){
        qtCtx()->settings()->setInt(WI_MAINWINDOW_X, x());
        qtCtx()->settings()->setInt(WI_MAINWINDOW_Y, y());
    }
    else{
        qtCtx()->settings()->setInt(WI_MAINWINDOW_X, d->xPos);
        qtCtx()->settings()->setInt(WI_MAINWINDOW_Y, d->yPos);
    }

    if (qtCtx()->settings()->getBool(WB_MAINWINDOW_REMEMBER))
        qtCtx()->settings()->setBool(WB_MAINWINDOW_HIDE, !isVisible());

    QString dockwidgetsState = QString::fromUtf8(saveState().toBase64());
    qtCtx()->settings()->setStr(WS_MAINWINDOW_STATE, dockwidgetsState);

    stateIsSaved = true;
}

void MainWindow::initActions(){
    Q_D(MainWindow);

    WulforUtil *WU = qtCtx()->wulforUtil();
    ShortcutManager *SM = qtCtx()->shortcutManager();

    {
        d->fileOpenMagnet = new QAction("", this);
        d->fileOpenMagnet->setObjectName("fileOpenMagnet");
        SM->registerShortcut(d->fileOpenMagnet, QString("Ctrl+I"));
        d->fileOpenMagnet->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(d->fileOpenMagnet, &QAction::triggered, this, &MainWindow::slotOpenMagnet);

        d->fileFileListBrowserLocal = new QAction("", this);
        d->fileFileListBrowserLocal->setObjectName("fileFileListBrowserLocal");
        SM->registerShortcut(d->fileFileListBrowserLocal, QString("Ctrl+L"));
        d->fileFileListBrowserLocal->setIcon(WU->getPixmap(WulforUtil::eiOWN_FILELIST));
        connect(d->fileFileListBrowserLocal, &QAction::triggered, this, &MainWindow::slotFileBrowseOwnFilelist);

        d->fileFileListBrowser = new QAction("", this);
        d->fileFileListBrowser->setObjectName("fileFileListBrowser");
        d->fileFileListBrowser->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(d->fileFileListBrowser, &QAction::triggered, this, &MainWindow::slotFileBrowseFilelist);

        d->fileFileListMatchAll = new QAction("", this);
        d->fileFileListMatchAll->setObjectName("fileFileListMatchAll");
        //d->fileFileListMatchAll->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(d->fileFileListMatchAll, &QAction::triggered, this, &MainWindow::slotFileMatchAllList);

        d->fileFileHasher = new QAction("", this);
        d->fileFileHasher->setObjectName("fileFileHasher");
        d->fileFileHasher->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(d->fileFileHasher, &QAction::triggered, this, &MainWindow::slotFileHasher);

        d->fileOpenLogFile = new QAction("", this);
        d->fileOpenLogFile->setObjectName("fileOpenLogFile");
        d->fileOpenLogFile->setIcon(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE));
        connect(d->fileOpenLogFile, &QAction::triggered, this, &MainWindow::slotFileOpenLogFile);

        d->fileOpenDownloadDirectory = new QAction("", this);
        d->fileOpenDownloadDirectory->setObjectName("fileOpenDownloadDirectory");
        d->fileOpenDownloadDirectory->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
        connect(d->fileOpenDownloadDirectory, &QAction::triggered, this, &MainWindow::slotFileOpenDownloadDirectory);

        d->fileRefreshShareHashProgress = new QAction("", this);
        d->fileRefreshShareHashProgress->setObjectName("fileRefreshShareHashProgress");
        SM->registerShortcut(d->fileRefreshShareHashProgress, QString("Ctrl+E"));
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        connect(d->fileRefreshShareHashProgress, &QAction::triggered, this, &MainWindow::slotFileRefreshShareHashProgress);

        d->fileHideWindow = new QAction("", this);
        d->fileHideWindow->setObjectName("fileHideWindow");
        SM->registerShortcut(d->fileHideWindow, QString("Ctrl+Alt+H"));
        d->fileHideWindow->setIcon(WU->getPixmap(WulforUtil::eiHIDEWINDOW));
        connect(d->fileHideWindow, &QAction::triggered, this, &MainWindow::slotHideWindow);

        d->fileQuit = new QAction("", this);
        d->fileQuit->setObjectName("fileQuit");
        SM->registerShortcut(d->fileQuit, QString("Ctrl+Q"));
        d->fileQuit->setMenuRole(QAction::QuitRole);
        d->fileQuit->setIcon(WU->getPixmap(WulforUtil::eiEXIT));
        connect(d->fileQuit, &QAction::triggered, this, &MainWindow::slotExit);

        d->hubsHubReconnect = new QAction("", this);
        d->hubsHubReconnect->setObjectName("hubsHubReconnect");
        SM->registerShortcut(d->hubsHubReconnect, QString("Ctrl+R"));
        d->hubsHubReconnect->setIcon(WU->getPixmap(WulforUtil::eiRECONNECT));
        connect(d->hubsHubReconnect, &QAction::triggered, this, &MainWindow::slotHubsReconnect);

        d->hubsQuickConnect = new QAction("", this);
        d->hubsQuickConnect->setObjectName("hubsQuickConnect");
        SM->registerShortcut(d->hubsQuickConnect, QString("Ctrl+N"));
        d->hubsQuickConnect->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
        connect(d->hubsQuickConnect, &QAction::triggered, this, &MainWindow::slotQC);

        d->hubsFavoriteHubs = new QAction("", this);
        d->hubsFavoriteHubs->setObjectName("hubsFavoriteHubs");
        SM->registerShortcut(d->hubsFavoriteHubs, QString("Ctrl+H"));
        d->hubsFavoriteHubs->setIcon(WU->getPixmap(WulforUtil::eiFAVSERVER));
        connect(d->hubsFavoriteHubs, &QAction::triggered, this, &MainWindow::slotHubsFavoriteHubs);

        d->hubsPublicHubs = new QAction("", this);
        d->hubsPublicHubs->setObjectName("hubsPublicHubs");
        SM->registerShortcut(d->hubsPublicHubs, QString("Ctrl+P"));
        d->hubsPublicHubs->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(d->hubsPublicHubs, &QAction::triggered, this, &MainWindow::slotHubsPublicHubs);

        d->hubsFavoriteUsers = new QAction("", this);
        d->hubsFavoriteUsers->setObjectName("hubsFavoriteUsers");
        SM->registerShortcut(d->hubsFavoriteUsers, QString("Ctrl+U"));
        d->hubsFavoriteUsers->setIcon(WU->getPixmap(WulforUtil::eiFAVUSERS));
        connect(d->hubsFavoriteUsers, &QAction::triggered, this, &MainWindow::slotHubsFavoriteUsers);

        d->toolsHubManager = new QAction("", this);
        d->toolsHubManager->setObjectName("toolsHubManager");
        d->toolsHubManager->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(d->toolsHubManager, &QAction::triggered, this, &MainWindow::slotToolsHubManager);

        d->toolsCopyWindowTitle = new QAction("", this);
        d->toolsCopyWindowTitle->setObjectName("toolsCopyWindowTitle");
        d->toolsCopyWindowTitle->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));
        connect(d->toolsCopyWindowTitle, &QAction::triggered, this, &MainWindow::slotToolsCopyWindowTitle);

        d->toolsOptions = new QAction("", this);
        d->toolsOptions->setObjectName("toolsOptions");
        SM->registerShortcut(d->toolsOptions, QString("Ctrl+O"));
        d->toolsOptions->setMenuRole(QAction::PreferencesRole);
        d->toolsOptions->setIcon(WU->getPixmap(WulforUtil::eiCONFIGURE));
        connect(d->toolsOptions, &QAction::triggered, this, &MainWindow::slotToolsSettings);

        d->toolsADLS = new QAction("", this);
        d->toolsADLS->setObjectName("toolsADLS");
        d->toolsADLS->setIcon(WU->getPixmap(WulforUtil::eiADLS));
        connect(d->toolsADLS, &QAction::triggered, this, &MainWindow::slotToolsADLS);

        d->toolsCmdDebug = new QAction("", this);
        d->toolsCmdDebug->setObjectName("toolsCmdDebug");
        d->toolsCmdDebug->setIcon(WU->getPixmap(WulforUtil::eiCONSOLE));
        connect(d->toolsCmdDebug, &QAction::triggered, this, &MainWindow::slotToolsCmdDebug);

        d->toolsSecretary = new QAction("", this);
        d->toolsSecretary->setObjectName("toolsSecretary");
        d->toolsSecretary->setIcon(WU->getPixmap(WulforUtil::eiMAGNET));
        connect(d->toolsSecretary, &QAction::triggered, this, &MainWindow::slotToolsSecretary);

        d->toolsTransfers = new QAction("", this);
        d->toolsTransfers->setObjectName("toolsTransfers");
        SM->registerShortcut(d->toolsTransfers, QString("Ctrl+T"));
        d->toolsTransfers->setIcon(WU->getPixmap(WulforUtil::eiTRANSFER));
        d->toolsTransfers->setCheckable(true);
        connect(d->toolsTransfers, &QAction::toggled, this, &MainWindow::slotToolsTransfer);
        //transfer_dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        d->toolsDownloadQueue = new QAction("", this);
        d->toolsDownloadQueue->setObjectName("toolsDownloadQueue");
        SM->registerShortcut(d->toolsDownloadQueue, QString("Ctrl+D"));
        d->toolsDownloadQueue->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(d->toolsDownloadQueue, &QAction::triggered, this, &MainWindow::slotToolsDownloadQueue);

        d->toolsQueuedUsers = new QAction("", this);
        d->toolsQueuedUsers->setObjectName("toolsQueuedUsers");
        SM->registerShortcut(d->toolsQueuedUsers, QString("Ctrl+Shift+U"));
        d->toolsQueuedUsers->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(d->toolsQueuedUsers, &QAction::triggered, this, &MainWindow::slotToolsQueuedUsers);

        d->toolsFinishedDownloads = new QAction("", this);
        d->toolsFinishedDownloads->setObjectName("toolsFinishedDownloads");
        SM->registerShortcut(d->toolsFinishedDownloads, QString("Ctrl+["));
        d->toolsFinishedDownloads->setIcon(WU->getPixmap(WulforUtil::eiDOWNLIST));
        connect(d->toolsFinishedDownloads, &QAction::triggered, this, &MainWindow::slotToolsFinishedDownloads);

        d->toolsFinishedUploads = new QAction("", this);
        d->toolsFinishedUploads->setObjectName("toolsFinishedUploads");
        SM->registerShortcut(d->toolsFinishedUploads, QString("Ctrl+]"));
        d->toolsFinishedUploads->setIcon(WU->getPixmap(WulforUtil::eiUPLIST));
        connect(d->toolsFinishedUploads, &QAction::triggered, this, &MainWindow::slotToolsFinishedUploads);

        d->toolsSearchSpy = new QAction("", this);
        d->toolsSearchSpy->setObjectName("toolsSpy");
        d->toolsSearchSpy->setIcon(WU->getPixmap(WulforUtil::eiSPY));
        connect(d->toolsSearchSpy, &QAction::triggered, this, &MainWindow::slotToolsSpy);

        d->toolsAntiSpam = new QAction("", this);
        d->toolsAntiSpam->setObjectName("toolsAntiSpam");
        d->toolsAntiSpam->setIcon(WU->getPixmap(WulforUtil::eiSPAM));
        d->toolsAntiSpam->setCheckable(true);
        d->toolsAntiSpam->setChecked(qtCtx()->antiSpam() != nullptr);
        connect(d->toolsAntiSpam, &QAction::triggered, this, &MainWindow::slotToolsAntiSpam);

        d->toolsIPFilter = new QAction("", this);
        d->toolsIPFilter->setObjectName("toolsIPFilter");
        d->toolsIPFilter->setIcon(WU->getPixmap(WulforUtil::eiFILTER));
        d->toolsIPFilter->setCheckable(true);
        d->toolsIPFilter->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::IPFILTER));
        connect(d->toolsIPFilter, &QAction::triggered, this, &MainWindow::slotToolsIPFilter);

        d->toolsAwayOn = new QAction("", this);
        d->toolsAwayOn->setObjectName("toolsAwayOn");
        d->toolsAwayOn->setCheckable(true);
        connect(d->toolsAwayOn, &QAction::triggered, this, &MainWindow::slotToolsSwitchAway);

        d->toolsAwayOff = new QAction("", this);
        d->toolsAwayOff->setObjectName("toolsAwayOff");
        d->toolsAwayOff->setCheckable(true);
        connect(d->toolsAwayOff, &QAction::triggered, this, &MainWindow::slotToolsSwitchAway);

        d->toolsAutoAway = new QAction("", this);
        d->toolsAutoAway->setCheckable(true);
        d->toolsAutoAway->setChecked(qtCtx()->settings()->getBool(WB_APP_AUTO_AWAY));
        connect(d->toolsAutoAway, &QAction::triggered, this, &MainWindow::slotToolsAutoAway);

#ifdef USE_JS
        d->toolsJS = new QAction("", this);
        d->toolsJS->setObjectName("toolsJS");
        d->toolsJS->setIcon(WU->getPixmap(WulforUtil::eiPLUGIN));
        connect(d->toolsJS, &QAction::triggered, this, &MainWindow::slotToolsJS);

        d->toolsJSConsole = new QAction("", this);
        d->toolsJSConsole->setObjectName("toolsJSConsole");
        SM->registerShortcut(d->toolsJSConsole, QString("Ctrl+Alt+J"));
        d->toolsJSConsole->setIcon(WU->getPixmap(WulforUtil::eiCONSOLE));
        connect(d->toolsJSConsole, &QAction::triggered, this, &MainWindow::slotToolsJSConsole);
#endif

        d->menuAwayAction = new QAction("", this);
        // submenu
        QAction *away_sep = new QAction("", this);
        away_sep->setSeparator(true);

        d->awayGroup = new QActionGroup(this);
        d->awayGroup->addAction(d->toolsAwayOn);
        d->awayGroup->addAction(d->toolsAwayOff);

        d->menuAway = new QMenu(this);
        d->menuAway->addActions(QList<QAction*>() << d->toolsAwayOn << d->toolsAwayOff << away_sep << d->toolsAutoAway);
        {
            QAction *act = Util::getAway()? d->toolsAwayOn : d->toolsAwayOff;
            act->setChecked(true);
        }
        // end
        d->menuAwayAction->setMenu(d->menuAway);
        d->menuAwayAction->setIcon(QIcon(WU->getPixmap(WulforUtil::eiAWAY)));

        d->toolsSearch = new QAction("", this);
        d->toolsSearch->setObjectName("toolsSearch");
        SM->registerShortcut(d->toolsSearch, QString("Ctrl+S"));
        d->toolsSearch->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
        connect(d->toolsSearch, &QAction::triggered, this, &MainWindow::slotToolsSearch);

        d->toolsHideProgressSpace = new QAction("", this);
        d->toolsHideProgressSpace->setObjectName("toolsHideProgressSpace");

#if (!defined FREE_SPACE_BAR_C)
        d->toolsHideProgressSpace->setVisible(false);
#endif
        d->toolsHideProgressSpace->setIcon(WU->getPixmap(WulforUtil::eiFREESPACE));
        connect(d->toolsHideProgressSpace, &QAction::triggered, this, &MainWindow::slotHideProgressSpace);

        d->toolsHideLastStatus = new QAction("", this);
        d->toolsHideLastStatus->setObjectName("toolsHideLastStatus");
        d->toolsHideLastStatus->setIcon(WU->getPixmap(WulforUtil::eiSTATUS));
        connect(d->toolsHideLastStatus, &QAction::triggered, this, &MainWindow::slotHideLastStatus);

        d->toolsHideUsersStatisctics = new QAction("", this);
        d->toolsHideUsersStatisctics->setObjectName("toolsHideUsersStatisctics");
        d->toolsHideUsersStatisctics->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(d->toolsHideUsersStatisctics, &QAction::triggered, this, &MainWindow::slotHideUsersStatistics);

        d->toolsSwitchSpeedLimit = new QAction("", this);
        d->toolsSwitchSpeedLimit->setObjectName("toolsSwitchSpeedLimit");
        SM->registerShortcut(d->toolsSwitchSpeedLimit, QString("Ctrl+K"));
        d->toolsSwitchSpeedLimit->setIcon(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::THROTTLE_ENABLE, true)? WU->getPixmap(WulforUtil::eiSPEED_LIMIT_ON) : WU->getPixmap(WulforUtil::eiSPEED_LIMIT_OFF));
        d->toolsSwitchSpeedLimit->setCheckable(true);
        d->toolsSwitchSpeedLimit->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::THROTTLE_ENABLE, true));
        connect(d->toolsSwitchSpeedLimit, &QAction::triggered, this, &MainWindow::slotToolsSwitchSpeedLimit);

        d->chatClear = new QAction("", this);
        d->chatClear->setObjectName("chatClear");
        d->chatClear->setIcon(WU->getPixmap(WulforUtil::eiCLEAR));
        connect(d->chatClear, &QAction::triggered, this, &MainWindow::slotChatClear);

        d->findInWidget = new QAction("", this);
        d->findInWidget->setObjectName("findInWidget");
        SM->registerShortcut(d->findInWidget, QString("Ctrl+F"));
        d->findInWidget->setIcon(WU->getPixmap(WulforUtil::eiFIND));
        connect(d->findInWidget, &QAction::triggered, this, &MainWindow::slotFind);

        d->chatDisable = new QAction("", this);
        d->chatDisable->setObjectName("chatDisable");
        d->chatDisable->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
        connect(d->chatDisable, &QAction::triggered, this, &MainWindow::slotChatDisable);

        QAction *separator0 = new QAction("", this);
        separator0->setObjectName("separator0");
        separator0->setSeparator(true);
        QAction *separator1 = new QAction("", this);
        separator1->setObjectName("separator1");
        separator1->setSeparator(true);
        QAction *separator2 = new QAction("", this);
        separator2->setObjectName("separator2");
        separator2->setSeparator(true);
        QAction *separator3 = new QAction("", this);
        separator3->setObjectName("separator3");
        separator3->setSeparator(true);
        QAction *separator4 = new QAction("", this);
        separator4->setObjectName("separator4");
        separator4->setSeparator(true);
        QAction *separator5 = new QAction("", this);
        separator5->setObjectName("separator5");
        separator5->setSeparator(true);
        QAction *separator6 = new QAction("", this);
        separator6->setObjectName("separator6");
        separator6->setSeparator(true);

        d->fileMenuActions << d->fileOpenMagnet
                << separator3
                << d->fileFileListBrowser
                << d->fileFileListBrowserLocal
                << d->fileFileListMatchAll
                << d->fileRefreshShareHashProgress
                << separator0
                << d->fileOpenLogFile
                << d->fileOpenDownloadDirectory
                << d->fileFileHasher
                << separator1
                << d->fileHideWindow
                << separator2
                << d->fileQuit;

        d->hubsMenuActions << d->hubsHubReconnect
                << d->hubsQuickConnect
                << d->hubsFavoriteHubs
                << d->hubsPublicHubs
                << separator0
                << d->hubsFavoriteUsers;

        d->toolsMenuActions << d->toolsSearch
                << d->toolsADLS
                << d->toolsSecretary
                << separator0
                << d->toolsTransfers
                << d->toolsDownloadQueue
                << d->toolsQueuedUsers
                << d->toolsFinishedDownloads
                << d->toolsFinishedUploads
                << d->toolsSwitchSpeedLimit
                //<< toolsHubManager
                << separator1
                << d->toolsSearchSpy
                << d->toolsAntiSpam
                << d->toolsIPFilter
                << d->toolsCmdDebug
                << separator2
                << d->menuAwayAction
                << separator3
                << d->toolsHideProgressSpace
                << d->toolsHideLastStatus
                << d->toolsHideUsersStatisctics
#ifdef USE_JS
                << separator6
                << d->toolsJS
                << d->toolsJSConsole
#endif
                << separator4
                << d->toolsCopyWindowTitle
                << separator5
                << d->toolsOptions;

        d->toolBarActions << d->toolsOptions
                << separator0
                << d->fileFileListBrowserLocal
                << d->fileRefreshShareHashProgress
                << separator1
                << d->hubsHubReconnect
                << d->hubsQuickConnect
                << separator2
                << d->hubsFavoriteHubs
                << d->hubsFavoriteUsers
                << d->toolsQueuedUsers
                << d->toolsSearch
                << d->hubsPublicHubs
                << separator3
                << d->toolsTransfers
                << d->toolsDownloadQueue
                << d->toolsFinishedDownloads
                << d->toolsFinishedUploads
                << d->toolsSwitchSpeedLimit
                << separator4
                << d->chatClear
                << d->findInWidget
                << d->chatDisable
                << separator5
                << d->toolsADLS
                << d->toolsSecretary
                << d->toolsSearchSpy
                << d->toolsAntiSpam
                << d->toolsIPFilter
                << separator6
                << d->fileQuit;
    }
    {
        d->menuWidgets = new QMenu("", this);
    }
    {
        // submenu
        d->nextTabShortCut     = new  QAction(this);
        d->prevTabShortCut     = new  QAction(this);
        d->nextMsgShortCut     = new  QAction(this);
        d->prevMsgShortCut     = new  QAction(this);
        d->closeWidgetShortCut      = new  QAction(this);
        d->toggleMainMenuShortCut   = new  QAction(this);

        d->nextTabShortCut->setObjectName("nextTabShortCut");
        d->prevTabShortCut->setObjectName("prevTabShortCut");
        d->nextMsgShortCut->setObjectName("nextMsgShortCut");
        d->prevMsgShortCut->setObjectName("prevMsgShortCut");
        d->closeWidgetShortCut->setObjectName("closeWidgetShortCut");
        d->toggleMainMenuShortCut->setObjectName("toggleMainMenuShortCut");

        d->nextTabShortCut->setText(tr("Next widget"));
        d->prevTabShortCut->setText(tr("Previous widget"));
        d->nextMsgShortCut->setText(tr("Next message"));
        d->prevMsgShortCut->setText(tr("Previous message"));
        d->closeWidgetShortCut->setText(tr("Close current widget"));
        d->toggleMainMenuShortCut->setText(tr("Toggle main menu"));

        d->nextTabShortCut->setShortcutContext(Qt::ApplicationShortcut);
        d->prevTabShortCut->setShortcutContext(Qt::ApplicationShortcut);
        d->nextMsgShortCut->setShortcutContext(Qt::ApplicationShortcut);
        d->prevMsgShortCut->setShortcutContext(Qt::ApplicationShortcut);
        d->closeWidgetShortCut->setShortcutContext(Qt::ApplicationShortcut);
        d->toggleMainMenuShortCut->setShortcutContext(Qt::ApplicationShortcut);

        SM->registerShortcut(d->nextTabShortCut, QString("Ctrl+PgDown"));
        SM->registerShortcut(d->prevTabShortCut, QString("Ctrl+PgUp"));
        SM->registerShortcut(d->nextMsgShortCut, QString("Ctrl+Down"));
        SM->registerShortcut(d->prevMsgShortCut, QString("Ctrl+Up"));
        SM->registerShortcut(d->closeWidgetShortCut, QString("Ctrl+W"));
        SM->registerShortcut(d->toggleMainMenuShortCut, QString("Ctrl+M"));

        connect(d->nextMsgShortCut,        &QAction::triggered, this, &MainWindow::nextMsg);
        connect(d->prevMsgShortCut,        &QAction::triggered, this, &MainWindow::prevMsg);
        connect(d->closeWidgetShortCut,    &QAction::triggered, this, &MainWindow::slotCloseCurrentWidget);
        connect(d->toggleMainMenuShortCut, &QAction::triggered, this, &MainWindow::slotHideMainMenu);
        // end

        d->sh_menu = new QMenu(this);
        d->sh_menu->addActions(QList<QAction*>()
                            << d->nextTabShortCut
                            << d->prevTabShortCut
                            << d->nextMsgShortCut
                            << d->prevMsgShortCut
                            << d->closeWidgetShortCut
                            << d->toggleMainMenuShortCut);

        d->panelsWidgets = new QAction("", this);
        d->panelsWidgets->setCheckable(true);
        connect(d->panelsWidgets, &QAction::triggered, this, &MainWindow::slotPanelMenuActionClicked);

        d->panelsTools = new QAction("", this);
        d->panelsTools->setCheckable(true);
        connect(d->panelsTools, &QAction::triggered, this, &MainWindow::slotPanelMenuActionClicked);

        d->panelsSearch = new QAction("", this);
        d->panelsSearch->setCheckable(true);
        connect(d->panelsSearch, &QAction::triggered, this, &MainWindow::slotPanelMenuActionClicked);
    }
    {
        d->aboutHomepage = new QAction("", this);
        connect(d->aboutHomepage, &QAction::triggered, this, &MainWindow::slotAboutOpenUrl);

        d->aboutBuilds = new QAction("", this);
        connect(d->aboutBuilds, &QAction::triggered, this, &MainWindow::slotAboutOpenUrl);

        d->aboutSource = new QAction("", this);
        connect(d->aboutSource, &QAction::triggered, this, &MainWindow::slotAboutOpenUrl);

        d->aboutIssues = new QAction("", this);
        connect(d->aboutIssues, &QAction::triggered, this, &MainWindow::slotAboutOpenUrl);

        d->aboutWiki = new QAction("", this);
        connect(d->aboutWiki, &QAction::triggered, this, &MainWindow::slotAboutOpenUrl);

        d->aboutChangelog = new QAction("", this);
        connect(d->aboutChangelog, &QAction::triggered, this, &MainWindow::slotAboutOpenUrl);

        d->aboutClient = new QAction("", this);
        d->aboutClient->setMenuRole(QAction::AboutRole);
        d->aboutClient->setIcon(WU->getPixmap(WulforUtil::eiICON_APPL)
                    .scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        connect(d->aboutClient, &QAction::triggered, this, &MainWindow::slotAboutClient);

        d->aboutQt = new QAction("", this);
        d->aboutQt->setMenuRole(QAction::AboutQtRole);
        d->aboutQt->setIcon(WU->getPixmap(WulforUtil::eiQT_LOGO));
        connect(d->aboutQt, &QAction::triggered, this, &MainWindow::slotAboutQt);
    }
}

void MainWindow::initMenuBar(){
#if defined(Q_OS_MAC)
    setMenuBar(new QMenuBar());
    menuBar()->setParent(nullptr);
    connect(this, &QObject::destroyed, menuBar(), &QObject::deleteLater);
#endif

    Q_D(MainWindow);

    {
        d->menuFile = new QMenu("", this);

        d->menuFile->addActions(d->fileMenuActions);
    }
    {
        d->menuHubs = new QMenu("", this);

        d->menuHubs->addActions(d->hubsMenuActions);
    }
    {
        d->menuTools = new QMenu("", this);

        d->menuTools->addActions(d->toolsMenuActions);
    }
    {
        QAction *sep0 = new QAction("", this);
        sep0->setSeparator(true);

        d->menuPanels = new QMenu("", this);

        d->menuPanels->addMenu(d->sh_menu);
        d->menuPanels->addAction(sep0);
        d->menuPanels->addAction(d->panelsWidgets);
        d->menuPanels->addAction(d->panelsTools);
        d->menuPanels->addAction(d->panelsSearch);
    }
    {
        QAction *sep0 = new QAction("", this);
        sep0->setSeparator(true);
        QAction *sep1 = new QAction("", this);
        sep1->setSeparator(true);

        d->menuAbout = new QMenu("", this);

        d->menuAbout->addAction(d->aboutHomepage);
        d->menuAbout->addAction(d->aboutBuilds);
        d->menuAbout->addAction(d->aboutIssues);
        d->menuAbout->addAction(d->aboutWiki);
        d->menuAbout->addAction(sep0);
        d->menuAbout->addAction(d->aboutChangelog);
        d->menuAbout->addAction(d->aboutSource);
        d->menuAbout->addAction(sep1);
        d->menuAbout->addAction(d->aboutClient);
        d->menuAbout->addAction(d->aboutQt);
    }

    menuBar()->addMenu(d->menuFile);
    menuBar()->addMenu(d->menuHubs);
    menuBar()->addMenu(d->menuTools);
    menuBar()->addMenu(d->menuWidgets);
    menuBar()->addMenu(d->menuPanels);
    menuBar()->addMenu(d->menuAbout);
    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::initStatusBar(){
    Q_D(MainWindow);

    d->statusLabel = new QLabel(statusBar());
    d->statusLabel->setFrameShadow(QFrame::Sunken);
    d->statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->statusLabel->setToolTip(tr("Counts"));
    d->statusLabel->setContentsMargins(0, 0, 0, 0);

    d->statusSPLabel = new QLabel(statusBar());
    d->statusSPLabel->setFrameShadow(QFrame::Sunken);
    d->statusSPLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->statusSPLabel->setToolTip(tr("Download/Upload speed"));
    d->statusSPLabel->setContentsMargins(0, 0, 0, 0);

    d->statusDLabel = new QLabel(statusBar());
    d->statusDLabel->setFrameShadow(QFrame::Sunken);
    d->statusDLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->statusDLabel->setToolTip(tr("Downloaded/Uploaded"));
    d->statusDLabel->setContentsMargins(0, 0, 0, 0);

    d->msgLabel = new QLabel(statusBar());
    d->msgLabel->setFrameShadow(QFrame::Plain);
    d->msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->msgLabel->setWordWrap(true);
    d->msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    d->msgLabel->setContentsMargins(0, 0, 0, 0);

#if (defined FREE_SPACE_BAR_C)
#if defined(USE_PROGRESS_BARS)
    d->progressFreeSpace = new QProgressBar(this);
    d->progressFreeSpace->setMaximum(100);
    d->progressFreeSpace->setMinimum(0);
    d->progressFreeSpace->setAlignment(Qt::AlignHCenter);
#else
    d->progressFreeSpace = new QLabel(this);
    d->progressFreeSpace->setTextFormat(Qt::PlainText);
    d->progressFreeSpace->setText(tr("Space free"));
    d->progressFreeSpace->setAlignment(Qt::AlignRight);
#endif

    d->progressFreeSpace->setMinimumWidth(100);
    d->progressFreeSpace->setMaximumWidth(250);
    d->progressFreeSpace->setFixedHeight(18);
    d->progressFreeSpace->setToolTip(tr("Space free"));
    d->progressFreeSpace->installEventFilter(this);

    if (!qtCtx()->settings()->getBool(WB_SHOW_FREE_SPACE))
        d->progressFreeSpace->hide();
#else //FREE_SPACE_BAR_C
    qtCtx()->settings()->setBool(WB_SHOW_FREE_SPACE, false);
#endif //FREE_SPACE_BAR_C

    d->progressHashing = new QProgressBar(this);
    d->progressHashing->setMaximum(100);
    d->progressHashing->setMinimum(0);
    d->progressHashing->setAlignment( Qt::AlignHCenter );
    d->progressHashing->setFixedHeight(18);
    d->progressHashing->setToolTip(tr("Hashing progress"));
    d->progressHashing->hide();
    d->progressHashing->installEventFilter( this );

    statusBar()->addWidget(d->progressHashing);
    statusBar()->addWidget(d->msgLabel, 1);
    statusBar()->addPermanentWidget(d->statusDLabel);
    statusBar()->addPermanentWidget(d->statusSPLabel);
    statusBar()->addPermanentWidget(d->statusLabel);
#if (defined FREE_SPACE_BAR_C)
    statusBar()->addPermanentWidget(d->progressFreeSpace);
#endif //FREE_SPACE_BAR_C
}

void MainWindow::initSearchBar(){
    Q_D(MainWindow);

    d->searchLineEdit = new LineEdit(this);

    connect(d->searchLineEdit,   &QLineEdit::returnPressed, this, &MainWindow::slotToolsSearch);
}

void MainWindow::retranslateUi(){
    Q_D(MainWindow);

    //Retranslate menu actions
    {
        d->menuFile->setTitle(tr("&File"));

        d->fileOpenMagnet->setText(tr("Open magnet link"));

        d->fileOpenLogFile->setText(tr("Open log file"));

        d->fileOpenDownloadDirectory->setText(tr("Open download directory"));

        d->fileFileListBrowser->setText(tr("Open filelist..."));

        d->fileFileHasher->setText(tr("Calculate file TTH"));

        d->fileFileListBrowserLocal->setText(tr("Open own filelist"));

        d->fileFileListMatchAll->setText(tr("Match all listings"));

        d->fileRefreshShareHashProgress->setText(tr("Refresh share"));

        d->fileHideWindow->setText(tr("Hide window"));

        if (!qtCtx()->settings()->getBool(WB_TRAY_ENABLED))
            d->fileHideWindow->setText(tr("Show/hide find frame"));

        d->fileQuit->setText(tr("Quit"));

        d->menuHubs->setTitle(tr("&Hubs"));

        d->hubsHubReconnect->setText(tr("Reconnect to hub"));

        d->hubsFavoriteHubs->setText(tr("Favourite hubs"));

        d->hubsPublicHubs->setText(tr("Public hubs"));

        d->hubsFavoriteUsers->setText(tr("Favourite users"));

        d->hubsQuickConnect->setText(tr("Quick connect"));

        d->menuTools->setTitle(tr("&Tools"));

        d->toolsTransfers->setText(tr("Transfers"));

        d->toolsDownloadQueue->setText(tr("Download queue"));

        d->toolsQueuedUsers->setText(tr("Queued Users"));

        d->toolsHubManager->setText(tr("Hub Manager"));

        d->toolsFinishedDownloads->setText(tr("Finished downloads"));

        d->toolsFinishedUploads->setText(tr("Finished uploads"));

        d->toolsSearchSpy->setText(tr("Search Spy"));

        d->toolsAntiSpam->setText(tr("AntiSpam module"));

        d->toolsIPFilter->setText(tr("IPFilter module"));

        d->toolsHideProgressSpace->setText(tr("Hide free space bar"));

        if (!qtCtx()->settings()->getBool(WB_SHOW_FREE_SPACE))
            d->toolsHideProgressSpace->setText(tr("Show free space bar"));

        d->toolsHideLastStatus->setText(tr("Hide last status message"));

        if (!qtCtx()->settings()->getBool(WB_LAST_STATUS))
            d->toolsHideLastStatus->setText(tr("Show last status message"));

        d->toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

        if (!qtCtx()->settings()->getBool(WB_USERS_STATISTICS))
            d->toolsHideUsersStatisctics->setText(tr("Show users statistics"));

        d->menuAway->setTitle(tr("Away message"));

        d->toolsAwayOn->setText(tr("On"));

        d->toolsAwayOff->setText(tr("Off"));

        d->toolsAutoAway->setText(tr("Away when not visible"));

        d->toolsCopyWindowTitle->setText(tr("Copy window title"));

        d->toolsOptions->setText(tr("Preferences"));

        d->toolsSearch->setText(tr("Search"));

        d->toolsADLS->setText(tr("ADLSearch"));

        d->toolsCmdDebug->setText(tr("Debug Console"));

        d->toolsSecretary->setText(tr("Secretary"));

        d->toolsSwitchSpeedLimit->setText(tr("Speed limit On/Off"));

#ifdef USE_JS
        d->toolsJS->setText(tr("Scripts Manager"));

        d->toolsJSConsole->setText(tr("Script Console"));
#endif

        d->chatClear->setText(tr("Clear chat"));

        d->findInWidget->setText(tr("Find/Filter"));

        d->chatDisable->setText(tr("Disable/enable chat"));

        d->menuWidgets->setTitle(tr("&Widgets"));

        d->menuPanels->setTitle(tr("&Panels"));

        if (!qtCtx()->settings()->getBool(WB_MAINWINDOW_USE_SIDEBAR))
            d->panelsWidgets->setText(tr("Widgets panel"));
        else
            d->panelsWidgets->setText(tr("Widgets side dock"));

        d->panelsTools->setText(tr("Tools panel"));

        d->panelsSearch->setText(tr("Fast search panel"));

        d->menuAbout->setTitle(tr("&Help"));

        d->aboutHomepage->setText(tr("Homepage"));

        d->aboutBuilds->setText(tr("Download program"));

        d->aboutIssues->setText(tr("Report a Bug"));

        d->aboutWiki->setText(tr("Wiki of project"));

        d->aboutChangelog->setText(tr("Changelog (git)"));

        d->aboutSource->setText(tr("Source code (git)"));

        d->aboutClient->setText(tr("About EiskaltDC++"));

        d->aboutQt->setText(tr("About Qt"));
    }
    {
        d->sh_menu->setTitle(tr("Actions"));
    }
    {
        d->arena->setWindowTitle(tr("Main layout"));
    }
}

void MainWindow::initToolbar(){
    Q_D(MainWindow);

    d->fBar = new ToolBar(this);
    d->fBar->setObjectName("fBar");

    QStringList enabled_actions = QString(QByteArray::fromBase64(qtCtx()->settings()->getStr(WS_MAINWINDOW_TOOLBAR_ACTS).toUtf8())).split(";", Qt::SkipEmptyParts);

    if (enabled_actions.isEmpty())
        d->fBar->addActions(d->toolBarActions);
    else {
        for (const auto &objName : enabled_actions){
            QAction *act = findChild<QAction*>(objName);

            if (act)
                d->fBar->addAction(act);
        }
    }

    initFavHubMenu();

    d->fBar->setContextMenuPolicy(Qt::CustomContextMenu);
    d->fBar->setMovable(true);
    d->fBar->setFloatable(true);
    d->fBar->setAllowedAreas(Qt::AllToolBarAreas);
    d->fBar->setWindowTitle(tr("Actions"));
    d->fBar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(qtCtx()->settings()->getInt(TOOLBUTTON_STYLE, Qt::ToolButtonIconOnly)));

    connect(d->fBar, &QWidget::customContextMenuRequested, this, &MainWindow::slotToolbarCustomization);

    addToolBar(d->fBar);

    if (!qtCtx()->settings()->getBool(WB_MAINWINDOW_USE_SIDEBAR) && qtCtx()->settings()->getBool(WB_MAINWINDOW_USE_M_TABBAR)){

        MultiLineToolBar *mBar = new MultiLineToolBar(this);
        mBar->setContextMenuPolicy(Qt::CustomContextMenu);
        mBar->setVisible(qtCtx()->settings()->getBool(WB_WIDGETS_PANEL_VISIBLE));

        connect(d->nextTabShortCut, &QAction::triggered, mBar, &MultiLineToolBar::nextTab);
        connect(d->prevTabShortCut, &QAction::triggered, mBar, &MultiLineToolBar::prevTab);

        addToolBar(mBar);
    }
    else if (!qtCtx()->settings()->getBool(WB_MAINWINDOW_USE_SIDEBAR) && !qtCtx()->settings()->getBool(WB_MAINWINDOW_USE_M_TABBAR)){

        ToolBar *tBar = new ToolBar(this);
        tBar->setObjectName("tBar");
        tBar->initTabs();
        tBar->setMovable(true);
        tBar->setFloatable(true);
        tBar->setAllowedAreas(Qt::AllToolBarAreas);
        tBar->setContextMenuPolicy(Qt::CustomContextMenu);

        addToolBar(tBar);

        connect(d->nextTabShortCut, &QAction::triggered, tBar, &ToolBar::nextTab);
        connect(d->prevTabShortCut, &QAction::triggered, tBar, &ToolBar::prevTab);
    }

    d->sBar = new ToolBar(this);
    d->sBar->setObjectName("sBar");
    d->sBar->addWidget(d->searchLineEdit);
    d->sBar->setContextMenuPolicy(Qt::CustomContextMenu);
    d->sBar->setMovable(true);
    d->sBar->setFloatable(true);
    d->sBar->setAllowedAreas(Qt::AllToolBarAreas);

    addToolBar(d->sBar);
}

void MainWindow::initSideBar(){
    if (!qtCtx()->settings()->getBool(WB_MAINWINDOW_USE_SIDEBAR))
        return;

    Q_D(MainWindow);

    d->sideDock = new QDockWidget("", this);

    d->sideDock->setWidget(new SideBarView(this));
    d->sideDock->setFeatures(d->sideDock->features() & (~QDockWidget::DockWidgetClosable));
    d->sideDock->setObjectName("sideDock");
    d->sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    d->sideDock->setContextMenuPolicy(Qt::CustomContextMenu);

    addDockWidget(Qt::LeftDockWidgetArea, d->sideDock);

    connect(d->sideDock, &QWidget::customContextMenuRequested, this, &MainWindow::slotSideBarDockMenu);
}

void MainWindow::initFavHubMenu() {
    Q_D(MainWindow);

    if (!d->fBar)
        return;

    if (!d->favHubMenu) {
        d->favHubMenu = new QMenu(this);

        connect(d->favHubMenu, &QMenu::aboutToShow, this, &MainWindow::slotUpdateFavHubMenu);
        connect(d->favHubMenu, &QMenu::triggered, this, &MainWindow::slotConnectFavHub);
    }

    QToolButton * btn = qobject_cast<QToolButton *>(d->fBar->widgetForAction(d->hubsFavoriteHubs));
    if (btn) {
        btn->setMenu(d->favHubMenu);
        btn->setPopupMode(QToolButton::MenuButtonPopup);
    }
}

QObject *MainWindow::getToolBar(){
    Q_D(MainWindow);

    if (!d->fBar)
        return nullptr;

    return qobject_cast<QObject*>(reinterpret_cast<QToolBar*>(d->fBar->qt_metacast("QToolBar")));
}

ArenaWidget *MainWindow::widgetForRole(ArenaWidget::Role r) const{
    ArenaWidget *awgt = nullptr;
    Q_D(const MainWindow);
    auto *ctx = qtCtx();

    switch (r){
    case ArenaWidget::Downloads:
        {
            if (!qtCtx()->downloadQueue()) {
                ctx->createDownloadQueue();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->downloadQueue());
            }
            awgt = qtCtx()->downloadQueue();
            awgt->setToolButton(d->toolsDownloadQueue);

            break;
        }
    case ArenaWidget::FinishedUploads:
        {
            if (!ctx->finishedUploads()) {
                ctx->createFinishedUploads();
                qtCtx()->arenaWidgetManager()->add(ctx->finishedUploads());
            }
            awgt = ctx->finishedUploads();
            awgt->setToolButton(d->toolsFinishedUploads);

            break;
        }
    case ArenaWidget::FinishedDownloads:
        {
            if (!ctx->finishedDownloads()) {
                ctx->createFinishedDownloads();
                qtCtx()->arenaWidgetManager()->add(ctx->finishedDownloads());
            }
            awgt = ctx->finishedDownloads();
            awgt->setToolButton(d->toolsFinishedDownloads);

            break;
        }
    case ArenaWidget::FavoriteHubs:
        {
            if (!qtCtx()->favoriteHubs()) {
                ctx->createFavoriteHubs();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->favoriteHubs());
            }
            awgt = qtCtx()->favoriteHubs();
            awgt->setToolButton(d->hubsFavoriteHubs);

            break;
        }
    case ArenaWidget::FavoriteUsers:
        {
            if (!qtCtx()->favoriteUsers()) {
                ctx->createFavoriteUsers();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->favoriteUsers());
            }
            awgt = qtCtx()->favoriteUsers();
            awgt->setToolButton(d->hubsFavoriteUsers);

            break;
        }
    case ArenaWidget::PublicHubs:
        {
            if (!qtCtx()->publicHubs()) {
                ctx->createPublicHubs();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->publicHubs());
            }
            awgt = qtCtx()->publicHubs();
            awgt->setToolButton(d->hubsPublicHubs);

            break;
        }
    case ArenaWidget::SearchSpy:
        {
            if (!qtCtx()->spyFrame()) {
                ctx->createSpyFrame();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->spyFrame());
            }
            awgt = qtCtx()->spyFrame();
            awgt->setToolButton(d->toolsSearchSpy);

            break;
        }
    case ArenaWidget::ADLS:
        {
            if (!qtCtx()->adls()) {
                ctx->createADLS();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->adls());
            }
            awgt = qtCtx()->adls();
            awgt->setToolButton(d->toolsADLS);

            break;
        }
    case ArenaWidget::CmdDebug:
    {
        if (!qtCtx()->cmdDebug()) {
            ctx->createCmdDebug();
            qtCtx()->arenaWidgetManager()->add(qtCtx()->cmdDebug());
        }
        awgt = qtCtx()->cmdDebug();
        awgt->setToolButton(d->toolsCmdDebug);

        break;
    }
    case ArenaWidget::Secretary:
    {
        if (!qtCtx()->secretary()) {
            ctx->createSecretary();
            qtCtx()->arenaWidgetManager()->add(qtCtx()->secretary());
        }
        awgt = qtCtx()->secretary();
        awgt->setToolButton(d->toolsSecretary);

        break;
    }
    case ArenaWidget::QueuedUsers:
        {
            if (!qtCtx()->queuedUsers()) {
                ctx->createQueuedUsers();
                qtCtx()->arenaWidgetManager()->add(qtCtx()->queuedUsers());
            }
            awgt = qtCtx()->queuedUsers();
            awgt->setToolButton(d->toolsQueuedUsers);

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

    address = QUrl::fromPercentEncoding(address.toUtf8());

    HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->getHub(address));

    if (fr){
        qtCtx()->arenaWidgetManager()->activate(fr);

        return;
    }

    fr = ArenaWidgetFactory().create<HubFrame, QWidget*, QString, QString>(this, address, enc);

    qtCtx()->arenaWidgetManager()->activate(fr);
}

void MainWindow::updateStatus(const QMap<QString, QString> &map){
    Q_D(MainWindow);

    if (!d->statusLabel)
        return;

    QString statsText = map["STATS"];
    QFontMetrics metrics(d->statusLabel->font());

    d->statusLabel->setText(statsText);

    QString speedText = tr("%1/s / %2/s").arg(map["DSPEED"]).arg(map["USPEED"]);
    QString downText = tr("%1 / %2").arg(map["DOWN"]).arg(map["UP"]);

    d->statusSPLabel->setText(speedText);
    d->statusDLabel->setText(downText);

    Notification *N = qtCtx()->notification();
    if (N)
        N->setToolTip(map["DSPEED"]+tr("/s"), map["USPEED"]+tr("/s"), map["DOWN"], map["UP"]);

    int leftM=0, topM=0, rightM=0, bottomM=0;
    const QMargins cm = d->statusSPLabel->contentsMargins();
    leftM = cm.left(); topM = cm.top(); rightM = cm.right(); bottomM = cm.bottom();
    int boundWidth = leftM + rightM;

    d->statusSPLabel->setFixedWidth(metrics.horizontalAdvance(speedText) > d->statusSPLabel->width()? metrics.horizontalAdvance(speedText) + boundWidth : d->statusSPLabel->width());
    d->statusDLabel->setFixedWidth(metrics.horizontalAdvance(downText) > d->statusDLabel->width()? metrics.horizontalAdvance(downText) + boundWidth : d->statusDLabel->width());

    if (qtCtx()->settings()->getBool(WB_SHOW_FREE_SPACE)) {
#ifdef FREE_SPACE_BAR_C
        std::string s = qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::DOWNLOAD_DIRECTORY, true);
        unsigned long long available = 0;
        unsigned long long total = 0;
        if (!s.empty()) {
            if (FreeSpace::FreeDiscSpace(s, &available, &total) == false) {
                available = 0;
                total = 0;
            }
        }
        const QString text = tr("Free %1")
                .arg(WulforUtil::formatBytes(available));

        const QString tooltip = tr("Free %1 of %2")
                .arg(WulforUtil::formatBytes(available))
                .arg(WulforUtil::formatBytes(total));

#if defined(USE_PROGRESS_BARS)
        const float percent = 100.0f*(total-available)/total;
        d->progressFreeSpace->setFormat(text);
        d->progressFreeSpace->setValue(static_cast<unsigned>(percent));
#else
        d->progressFreeSpace->setText(text);
#endif
        d->progressFreeSpace->setToolTip(tooltip);

        if (metrics.horizontalAdvance(text) > d->progressFreeSpace->width()) {
            d->progressFreeSpace->setFixedWidth(metrics.horizontalAdvance(d->progressFreeSpace->text()) + 40);
        }
        else {
            d->progressFreeSpace->setFixedWidth(d->progressFreeSpace->width());
        }
#endif //FREE_SPACE_BAR_C
    }

    if ((Util::getAway() && !d->toolsAwayOn->isChecked()) || (!Util::getAway() && d->toolsAwayOff->isChecked())){
        QAction *act = Util::getAway()? d->toolsAwayOn : d->toolsAwayOff;

        act->setChecked(true);
    }

    updateHashProgressStatus();
}

void MainWindow::updateHashProgressStatus() {
    WulforUtil *WU = qtCtx()->wulforUtil();
    Q_D(MainWindow);

    switch( HashProgress::getHashStatus() ) {
    case HashProgress::IDLE:
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiREFRLIST));
        d->fileRefreshShareHashProgress->setText(tr("Refresh share"));
        {
            progress_dialog()->resetProgress(); // Here dialog will be actually created
            d->progressHashing->hide();
        }
        //qDebug("idle");
        break;
    case HashProgress::LISTUPDATE:
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        d->fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            d->progressHashing->setValue( 100 );
            d->progressHashing->setFormat(tr("List update"));
            d->progressHashing->show();
        }
        //qDebug("listupdate");
        break;
    case HashProgress::DELAYED:
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        d->fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            if (qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::HASHING_START_DELAY, true) >= 0){
                int left = qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::HASHING_START_DELAY, true) - Util::getUpTime();
                d->progressHashing->setValue( 100 * left / qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::HASHING_START_DELAY, true) );
                d->progressHashing->setFormat(tr("Delayed"));
                d->progressHashing->show();
            }
            else {
                d->progressHashing->hide();
            }
        }
        //qDebug("delayed");
        break;
    case HashProgress::PAUSED:
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        d->fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            if (qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::HASHING_START_DELAY, true) >= 0){
                d->progressHashing->setValue( 100 );
                d->progressHashing->setFormat(tr("Paused"));
                d->progressHashing->show();
            }
            else {
                d->progressHashing->hide();
            }
        }
        //qDebug("paused");
        break;
    case HashProgress::RUNNING:
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        d->fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            int progress = static_cast<int>( progress_dialog()->getProgress()*100 );
            d->progressHashing->setFormat(tr("%p%"));
            d->progressHashing->setValue( progress );
            d->progressHashing->show();
        }
        //qDebug("running");
        break;
    default:
        //qDebug("unknown");
        break;
    }
}

void MainWindow::setStatusMessage(QString msg){
    Q_D(MainWindow);

    QFontMetrics m(d->msgLabel->font());
    QString pure_msg = msg;

    if (m.horizontalAdvance(msg) > d->msgLabel->width())
        pure_msg = m.elidedText(msg, Qt::ElideRight, d->msgLabel->width(), 0);

    qtCtx()->wulforUtil()->textToHtml(pure_msg, true);
    qtCtx()->wulforUtil()->textToHtml(msg, true);

    d->msgLabel->setText(pure_msg);

    d->core_msg_history.push_back(msg);

    if (qtCtx()->settings()->getInt(WI_STATUSBAR_HISTORY_SZ) > 0){
        while (d->core_msg_history.size() > qtCtx()->settings()->getInt(WI_STATUSBAR_HISTORY_SZ))
            d->core_msg_history.removeFirst();
    }
    else
        d->core_msg_history.clear();

    d->msgLabel->setToolTip(d->core_msg_history.join("\n"));
    d->msgLabel->setMaximumHeight(d->statusLabel->height());
}

void MainWindow::autoconnect(){
    const FavoriteHubEntryList& fl = dcCtx().getFavoriteManager()->getFavoriteHubs();

    for (const auto &i : fl) {
        FavoriteHubEntry* entry = i;

        if (entry->getConnect()) {
            if (entry->getNick().empty() && qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::NICK, true).empty())
                continue;

            QString encoding = qtCtx()->wulforUtil()->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));

            newHubFrame(QString::fromStdString(entry->getServer()), encoding);
        }
    }
}

void MainWindow::parseCmdLine(const QStringList &args){
    for (const auto &arg : args){
        if (arg.startsWith("magnet:?")){
            Magnet m(this);
            m.setLink(arg);
            m.exec();
        }
        else if (arg.startsWith("dchub://") || arg.startsWith("nmdcs://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainWindow::parseInstanceLine(const QString &data){
    if (!isVisible()){
        show();
        raise();

        redrawToolPanel();
    }

    const QStringList args = data.split("\n", Qt::SkipEmptyParts);
    parseCmdLine(args);
}

void MainWindow::browseOwnFiles(){
    slotFileBrowseOwnFilelist();
}

void MainWindow::slotFileBrowseFilelist(){
    QString file = QFileDialog::getOpenFileName(this, tr("Choose file to open"),
                QString::fromStdString(Util::getPath(Util::PATH_FILE_LISTS)),
                tr("Modern XML Filelists") + " (*.xml.bz2);;" +
                tr("Modern XML Filelists uncompressed") + " (*.xml);;" +
                tr("All files") + " (*)");

    if (file.isEmpty())
        return;

    file = QDir::toNativeSeparators(file);
    UserPtr user = DirectoryListing::getUserFromFilename(dcCtx(), _tq(file));

    if (user) {
        ArenaWidgetFactory().create<ShareBrowser, UserPtr, QString, QString>(user, file, "");
    } else {
        setStatusMessage(tr("Unable to load file list: Invalid file list name"));
    }
}

void MainWindow::slotFileMatchAllList(){
    dcCtx().getQueueManager()->matchAllListings();
}

void MainWindow::redrawToolPanel(){
    Q_D(MainWindow);

    ArenaWidget *awgt = nullptr;
    PMWindow *pm = nullptr;
    bool has_unread = false;

    // Also redraw all widget menu items and change window title if needed:
    auto end = d->menuWidgetsHash.end();
    for (auto it = d->menuWidgetsHash.begin(); it != end; ++it){
        awgt = it.value();
        if (!awgt)
            continue;

        it.key()->setText(awgt->getArenaShortTitle());
        it.key()->setIcon(awgt->getPixmap());

        pm = qobject_cast<PMWindow *>(awgt->getWidget());
        if (pm && pm->hasNewMessages())
            has_unread = true;

        if (d->arena->widget() && d->arena->widget() == awgt->getWidget())
            setWindowTitle(awgt->getArenaTitle() + " :: " + QString::fromStdString(eiskaltdcppAppNameString));
    }

#if !defined(Q_OS_MAC)
    if (!has_unread)
        qtCtx()->notification()->resetTrayIcon();
#else // !defined(Q_OS_MAC)
    // Change program icon in dock when there are new unread personal messages.
    if (has_unread)
        qApp->setWindowIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiMESSAGE_TRAY_ICON));
    else
        qApp->setWindowIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiICON_APPL));
#endif // !defined(Q_OS_MAC)

    emit redrawWidgetPanels();
}

void MainWindow::mapWidgetOnArena(ArenaWidget *awgt){
    Q_D(MainWindow);

    if (!(awgt && awgt->getWidget())){
        d->arena->setWidget(nullptr);

        return;
    }

    if (d->arena->widget() != awgt->getWidget())
        d->arena->setWidget(awgt->getWidget());

    setWindowTitle(awgt->getArenaTitle() + " :: " + QString::fromStdString(eiskaltdcppAppNameString));

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    ArenaWidget::Role role = awgt->role();

    const bool widgetWithFilter = (
                role == ArenaWidget::CmdDebug ||
                role == ArenaWidget::Hub ||
                role == ArenaWidget::PrivateMessage ||
                role == ArenaWidget::PublicHubs ||
                role == ArenaWidget::Search ||
                role == ArenaWidget::Secretary ||
                role == ArenaWidget::ShareBrowser
                );

    const bool widgetWithCleanup = (
                role == ArenaWidget::CmdDebug ||
                role == ArenaWidget::Hub ||
                role == ArenaWidget::PrivateMessage ||
                role == ArenaWidget::SearchSpy ||
                role == ArenaWidget::Secretary
                );

    d->chatClear->setEnabled(widgetWithCleanup);
    d->findInWidget->setEnabled(widgetWithFilter);
    d->chatDisable->setEnabled(role == ArenaWidget::Hub);

    awgt->requestFocus();
}

void MainWindow::insertWidget ( ArenaWidget* awgt ) {
    if (!awgt || (awgt && (awgt->state() & ArenaWidget::Hidden)))
        return;

    Q_D(MainWindow);

    QAction *act = d->menuWidgets->addAction(awgt->getPixmap(), awgt->getArenaShortTitle());

    d->menuWidgetsHash.insert(act, awgt);

    connect(act, &QAction::triggered, this, &MainWindow::slotWidgetsToggle);
}

void MainWindow::removeWidget ( ArenaWidget* awgt ) {
    Q_D(MainWindow);

    QAction *act = d->menuWidgetsHash.key(awgt);

    if (!act)
        return;

    d->menuWidgetsHash.remove(act);

    act->deleteLater();
}

void MainWindow::updated ( ArenaWidget* awgt ) {
    if (!awgt)
        return;

    if (awgt->state() & ArenaWidget::Hidden)
        removeWidget(awgt);
    else
        insertWidget(awgt);
}

void MainWindow::addActionOnToolBar(QAction *new_act){
    Q_D(MainWindow);

    if (!d->fBar || d->toolBarActions.contains(new_act))
        return;

    d->fBar->insertAction(d->toolBarActions.last(), new_act);
    d->toolBarActions.append(new_act);
}

void MainWindow::remActionFromToolBar(QAction *act){
    Q_D(MainWindow);

    if (!d->fBar || !d->toolBarActions.contains(act))
        return;

    d->fBar->removeAction(act);
    d->toolBarActions.removeAt(d->toolBarActions.indexOf(act));
}

void MainWindow::toggleSingletonWidget(ArenaWidget *a){
    if (!a)
        throw std::runtime_error(_tq(Q_FUNC_INFO) + ": NULL argument");

    if (sender() && qobject_cast<QAction*>(sender()) && a->getWidget()){
        QAction *act = reinterpret_cast<QAction*>(sender());

        act->setCheckable(true);

        a->setToolButton(act);
    }

    if (!a->getWidget()->isVisible())
        qtCtx()->arenaWidgetManager()->activate(a);
    else
        qtCtx()->arenaWidgetManager()->toggle(a);
}

void MainWindow::toggleMainMenu(bool showMenu){
    static QAction *compactMenus = nullptr;

    menuBar()->setVisible(showMenu);

    Q_D(MainWindow);

    if (showMenu){
        if (compactMenus && d->fBar)
            d->fBar->removeAction(compactMenus);
    }
    else {
        if (d->fBar){
            if (!compactMenus){
                compactMenus = new QAction(tr("Menu"), this);
                compactMenus->setObjectName("compactMenus");
                compactMenus->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEDIT));
            }
            else {
                compactMenus->menu()->deleteLater();
                compactMenus->setMenu(nullptr);
            }

            QMenu *m = new QMenu(this);

            for (const auto &a : menuBar()->actions())
                m->addAction(a);

            compactMenus->setMenu(m);

            connect(compactMenus, &QAction::triggered, this, &MainWindow::slotShowMainMenu);
        }

        if (d->fBar)
            d->fBar->insertAction(d->toolBarActions.first(), compactMenus);
    }

    qtCtx()->settings()->setBool(WB_MAIN_MENU_VISIBLE, showMenu);
}

void MainWindow::startSocket(bool changed){
    if (changed)
        dcCtx().getConnectivityManager()->updateLast();
    try {
        dcCtx().getConnectivityManager()->setup(true);
    } catch (const Exception& e) {
        showPortsError(e.getError());
    }
    dcCtx().getClientManager()->infoUpdated();
}
void MainWindow::showPortsError(const string& port) {
    QString msg = tr("Unable to open %1 port. Searching or file transfers will not work correctly until you change settings or turn off any application that might be using that port.").arg(_q(port));
    QMessageBox::warning(this, tr("Connectivity Manager: Warning"), msg, QMessageBox::Ok);
}
void MainWindow::showShareBrowser(dcpp::UserPtr usr, const QString &file, const QString &jump_to){
    ArenaWidgetFactory().create<ShareBrowser, UserPtr, QString, QString>(usr, file, jump_to);
}

void MainWindow::reloadSomeSettings(){
    Q_D(MainWindow);

    for (const auto &awgt : d->menuWidgetsHash.values()){
        HubFrame *fr = qobject_cast<HubFrame *>(awgt->getWidget());

        if (fr)
            fr->reloadSomeSettings();
    }

    d->toolsSwitchSpeedLimit->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::THROTTLE_ENABLE, true));
}

void MainWindow::slotFileOpenLogFile(){
    QString f = QFileDialog::getOpenFileName(this, tr("Open log file"),_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_DIRECTORY, true)), tr("Log files (*.log);;All files (*.*)"));

    if (!f.isEmpty()){
        f = QDir::toNativeSeparators(f);

        if (f.startsWith("/"))
            f = "file://" + f;
        else
            f = "file:///" + f;

        QDesktopServices::openUrl(QUrl(f));
    }
}

void MainWindow::slotFileOpenDownloadDirectory(){
    QString directory = QString::fromStdString(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::DOWNLOAD_DIRECTORY, true));

    directory.prepend( directory.startsWith("/")? ("file://") : ("file:///"));

    QDesktopServices::openUrl(QUrl::fromEncoded(directory.toUtf8()));
}

void MainWindow::slotFileBrowseOwnFilelist(){
    UserPtr user = dcCtx().getClientManager()->getMe();
    QString file = QString::fromStdString(dcCtx().getShareManager()->getOwnListFile());

    ArenaWidgetFactory().create<ShareBrowser, UserPtr, QString, QString>(user, file, "");
}

void MainWindow::slotFileHashProgress(){
    progress_dialog()->slotAutoClose(false);
    progress_dialog()->show();
}

void MainWindow::slotFileHasher(){
    FileHasher *m = new FileHasher(qtCtx()->mainWindow());
    m->setModal(true);
    m->exec();
    delete m;
}

void MainWindow::slotFileRefreshShareHashProgress(){
    switch( HashProgress::getHashStatus() ) {
    case HashProgress::IDLE:
    {
        ShareManager *SM = dcCtx().getShareManager();

        SM->setDirty();
        SM->refresh(true);

        updateHashProgressStatus();
        progress_dialog()->resetProgress();
    }
        break;
    case HashProgress::LISTUPDATE:
    case HashProgress::PAUSED:
    case HashProgress::DELAYED:
    case HashProgress::RUNNING:
        slotFileHashProgress();
        break;
    default:
        break;
    }
}

void MainWindow::slotOpenMagnet(){
    QString text = qApp->clipboard()->text(QClipboard::Clipboard);
    bool ok = false;

    text = (text.startsWith("magnet:?")? text : "");

    QString result = QInputDialog::getText(this, tr("Open magnet link"), tr("Enter magnet link:"), QLineEdit::Normal, text, &ok);

    if (!ok)
        return;

    if (result.startsWith("magnet:?")){
        Magnet m(this);
        m.setLink(result);
        m.exec();
    }
}

void MainWindow::slotHubsReconnect(){
    HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->activeHub());

    if (fr)
        fr->reconnect();
}

void MainWindow::slotToolsADLS(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::ADLS));
}

void MainWindow::slotToolsCmdDebug()
{
    toggleSingletonWidget(widgetForRole(ArenaWidget::CmdDebug));
}

void MainWindow::slotToolsSecretary()
{
    toggleSingletonWidget(widgetForRole(ArenaWidget::Secretary));
}

void MainWindow::slotToolsSearch() {
    SearchFrame *sf = ArenaWidgetFactory().create<SearchFrame>();

    QLineEdit *le = qobject_cast<QLineEdit *> ( sender() );

    Q_D(MainWindow);

    if ( le != d->searchLineEdit )
        return;

    QString text = d->searchLineEdit->text();
    bool isTTH = false;

    if ( text.startsWith ( "magnet:" ) ) {
        QString link = text;
        QString tth = "", name = "";
        int64_t size = 0;

        WulforUtil::splitMagnet ( link, size, tth, name );

        text  = tth;
        isTTH = true;
    }

    sf->fastSearch ( text, isTTH || WulforUtil::isTTH ( text ) );
}

void MainWindow::slotToolsDownloadQueue(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::Downloads));
}

void MainWindow::slotToolsQueuedUsers(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::QueuedUsers));
}

void MainWindow::slotToolsHubManager(){
}

void MainWindow::slotToolsFinishedDownloads(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::FinishedDownloads));
}

void MainWindow::slotToolsFinishedUploads(){
   toggleSingletonWidget(widgetForRole(ArenaWidget::FinishedUploads));
}

void MainWindow::slotToolsSpy(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::SearchSpy));
}

void MainWindow::slotToolsAntiSpam(){
    AntiSpamFrame fr(this);

    fr.exec();

    Q_D(MainWindow);

    d->toolsAntiSpam->setChecked(qtCtx()->antiSpam() != nullptr);
}

void MainWindow::slotToolsIPFilter(){
    IPFilterFrame fr(this);

    fr.exec();

    Q_D(MainWindow);

    d->toolsIPFilter->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::IPFILTER));
}

void MainWindow::slotToolsAutoAway(){
    Q_D(MainWindow);

    qtCtx()->settings()->setBool(WB_APP_AUTO_AWAY, d->toolsAutoAway->isChecked());
}

void MainWindow::slotToolsSwitchAway(){
    Q_D(MainWindow);

    if ((sender() != d->toolsAwayOff) && (sender() != d->toolsAwayOn))
        return;

    bool away = d->toolsAwayOn->isChecked();

    Util::setAway(dcCtx(), away);
    Util::setManualAway(away);
}

void MainWindow::slotToolsJS(){
#ifdef USE_JS
    ScriptManagerDialog(this).exec();
#endif
}

#ifdef USE_JS
namespace {
    enum class ScriptChangedAction: int {
        DoNothing=0,
        AskUser,
        ReloadIt
    };
}
#endif

void MainWindow::slotJSFileChanged(const QString &script){
#ifdef USE_JS
    enum ScriptChangedAction act = (enum ScriptChangedAction)qtCtx()->settings()->getInt("scriptmanager/script-changed-action", 0);
    bool ask = false;

    switch (act){
    case ScriptChangedAction::DoNothing:
        break;
    case ScriptChangedAction::AskUser:
        ask = true;
    case ScriptChangedAction::ReloadIt:
    {
        auto raiseMe = [this]() -> bool {
            if (!this->isVisible()){
                this->show();
                this->raise();
            }

            return true;
        };

        if (ask && raiseMe() && (QMessageBox::warning(this,
                                                      tr("Script Engine"),
                                                      QString("\'%1\' has been changed. Reload it?").arg(script),
                                                      QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes))
            break;


        qtCtx()->scriptEngine()->loadScript(script);

        break;
    }
    }
#else
    Q_UNUSED(script)
#endif
}


void MainWindow::slotToolsJSConsole(){
#ifdef USE_JS
    Q_D(MainWindow);

    if (!d->scriptConsole)
        d->scriptConsole = new ScriptConsole(this);

    d->scriptConsole->setWindowModality(Qt::NonModal);
    d->scriptConsole->show();
    d->scriptConsole->raise();
#endif
}

void MainWindow::slotHubsFavoriteHubs(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::FavoriteHubs));
}

void MainWindow::slotHubsPublicHubs(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::PublicHubs));
}

void MainWindow::slotHubsFavoriteUsers(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::FavoriteUsers));
}

void MainWindow::slotToolsCopyWindowTitle(){
    QString text = windowTitle();

    if (!text.isEmpty())
        qApp->clipboard()->setText(text, QClipboard::Clipboard);
}

void MainWindow::slotToolsSettings(){
    Settings s;

    s.exec();

    reloadSomeSettings();

    Q_D(MainWindow);

    //reload some settings
    if (!qtCtx()->settings()->getBool(WB_TRAY_ENABLED))
        d->fileHideWindow->setText(tr("Show/hide find frame"));
    else
        d->fileHideWindow->setText(tr("Hide window"));
}

void MainWindow::slotToolsTransfer(bool toggled){
    Q_D(MainWindow);

    if (toggled){
        d->transfer_dock->setVisible(true);
        d->transfer_dock->setWidget(qtCtx()->transferView());
    }
    else {
        d->transfer_dock->setWidget(nullptr);
        d->transfer_dock->setVisible(false);
    }
}

void MainWindow::slotToolsSwitchSpeedLimit(){
    auto *WU = qtCtx()->wulforUtil();
    Q_D(MainWindow);

    dcCtx().getSettingsManager()->set(SettingsManager::THROTTLE_ENABLE, d->toolsSwitchSpeedLimit->isChecked());
    d->toolsSwitchSpeedLimit->setIcon(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::THROTTLE_ENABLE, true)? WU->getPixmap(WulforUtil::eiSPEED_LIMIT_ON) : WU->getPixmap(WulforUtil::eiSPEED_LIMIT_OFF));
}

void MainWindow::slotPanelMenuActionClicked(){
    QAction *act = qobject_cast<QAction *>(sender());

    if (!act)
        return;

    Q_D(MainWindow);

    if (act == d->panelsWidgets){
        if (findChild<MultiLineToolBar*>("multiLineTabbar")){
            findChild<MultiLineToolBar*>("multiLineTabbar")->setVisible(d->panelsWidgets->isChecked());
        }
        else if (findChild<ToolBar*>("tBar")){
            findChild<ToolBar*>("tBar")->setVisible(d->panelsWidgets->isChecked());
        }
        else if (d->sideDock)
            d->sideDock->setVisible(d->panelsWidgets->isChecked());

        qtCtx()->settings()->setBool(WB_WIDGETS_PANEL_VISIBLE, d->panelsWidgets->isChecked());
    }
    else if (act == d->panelsTools){
        d->fBar->setVisible(d->panelsTools->isChecked());
        qtCtx()->settings()->setBool(WB_TOOLS_PANEL_VISIBLE, d->panelsTools->isChecked());
    }
    else if (act == d->panelsSearch){
        d->sBar->setVisible(d->panelsSearch->isChecked());
        qtCtx()->settings()->setBool(WB_SEARCH_PANEL_VISIBLE, d->panelsSearch->isChecked());
    }
}

void MainWindow::slotChatClear(){
    Q_D(MainWindow);

    if (!d->arena->widget() || !qobject_cast<ArenaWidget*>(d->arena->widget()))
        return;

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(d->arena->widget());
    awgt->requestClear();
}

void MainWindow::slotFind(){
    Q_D(MainWindow);

    if (!d->arena->widget() || !qobject_cast<ArenaWidget*>(d->arena->widget()))
        return;

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(d->arena->widget());
    awgt->requestFilter();
}

void MainWindow::slotChatDisable(){
    HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->activeHub());

    if (fr)
        fr->disableChat();
}

void MainWindow::slotWidgetsToggle(){
    Q_D(MainWindow);

    QAction *act = reinterpret_cast<QAction*>(sender());
    auto it = d->menuWidgetsHash.find(act);

    if (it == d->menuWidgetsHash.end())
        return;

    qtCtx()->arenaWidgetManager()->activate(it.value());
}

void MainWindow::slotQC(){
    QuickConnect qc;

    qc.exec();
}

void MainWindow::slotHideMainMenu(){
    toggleMainMenu(!menuBar()->isVisible());
}

void MainWindow::slotShowMainMenu() {
    QAction *act = qobject_cast<QAction*>(sender());

    if (!(act && act->menu()))
        return;

    act->menu()->exec(QCursor::pos());
}

void MainWindow::slotHideWindow(){
    Q_D(MainWindow);

    if (!d->isUnload && isActiveWindow() && qtCtx()->settings()->getBool(WB_TRAY_ENABLED)) {
        hide();
    }
}

void MainWindow::slotHideProgressSpace() {
    Q_D(MainWindow);

    if (qtCtx()->settings()->getBool(WB_SHOW_FREE_SPACE)) {
        d->progressFreeSpace->hide();
        d->toolsHideProgressSpace->setText(tr("Show free space bar"));

        qtCtx()->settings()->setBool(WB_SHOW_FREE_SPACE, false);
    } else {
        d->progressFreeSpace->show();
        d->toolsHideProgressSpace->setText(tr("Hide free space bar"));

        qtCtx()->settings()->setBool(WB_SHOW_FREE_SPACE, true);
    }
}

void MainWindow::slotHideLastStatus(){
    Q_D(MainWindow);

    bool st = qtCtx()->settings()->getBool(WB_LAST_STATUS);

    st = !st;

    if (!st)
        d->toolsHideLastStatus->setText(tr("Show last status message"));
    else
        d->toolsHideLastStatus->setText(tr("Hide last status message"));

    qtCtx()->settings()->setBool(WB_LAST_STATUS, st);

    reloadSomeSettings();
}

void MainWindow::slotHideUsersStatistics(){
    Q_D(MainWindow);

    bool st = qtCtx()->settings()->getBool(WB_USERS_STATISTICS);

    st = !st;

    if (!st)
        d->toolsHideUsersStatisctics->setText(tr("Show users statistics"));
    else
        d->toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

    qtCtx()->settings()->setBool(WB_USERS_STATISTICS, st);

    reloadSomeSettings();
}

void MainWindow::slotExit(){
    setUnload(true);

    close();
}

void MainWindow::slotToolbarCustomization() {
    Q_D(MainWindow);

    QMenu *m = new QMenu(this);

    QMenu *toolButtonStyle = new QMenu(tr("Button style"), this);
    toolButtonStyle->addAction(tr("Icons only"))->setData(Qt::ToolButtonIconOnly);
    toolButtonStyle->addAction(tr("Text only"))->setData(Qt::ToolButtonTextOnly);
    toolButtonStyle->addAction(tr("Text beside icons"))->setData(Qt::ToolButtonTextBesideIcon);
    toolButtonStyle->addAction(tr("Text under icons"))->setData(Qt::ToolButtonTextUnderIcon);

    for (const auto &a : toolButtonStyle->actions()){
        a->setCheckable(true);
        a->setChecked(d->fBar->toolButtonStyle() == static_cast<Qt::ToolButtonStyle>(a->data().toInt()));
    }

    m->addMenu(toolButtonStyle);
    m->addSeparator();

    QAction *customize = m->addAction(tr("Customize"));
    QAction *ret = m->exec(QCursor::pos());

    m->deleteLater();
    toolButtonStyle->deleteLater();

    if (ret == customize){
        ActionCustomizer customizer(d->toolBarActions, d->fBar->actions(), this);
        connect(&customizer, &ActionCustomizer::done, this, &MainWindow::slotToolbarCustomizerDone);

        customizer.exec();
    }
    else if (ret){
        d->fBar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(ret->data().toInt()));

        qtCtx()->settings()->setInt(TOOLBUTTON_STYLE, static_cast<int>(d->fBar->toolButtonStyle()));
    }
}

void MainWindow::slotToolbarCustomizerDone(const QList<QAction*> &enabled){
    Q_D(MainWindow);

    d->fBar->clear();

    QStringList enabled_list;

    for (const auto &act : enabled){
        if (!act)
            continue;

        d->fBar->addAction(act);
        enabled_list.push_back(act->objectName());
    }

    initFavHubMenu();

    qtCtx()->settings()->setStr(WS_MAINWINDOW_TOOLBAR_ACTS, enabled_list.join(";").toUtf8().toBase64());
}

void MainWindow::slotAboutOpenUrl(){
    Q_D(MainWindow);

    QAction *act = qobject_cast<QAction *>(sender());

    const QHash<QAction*, QUrl> urlsTable = {
        { d->aboutHomepage,     QUrl("https://github.com/eiskaltdcpp/eiskaltdcpp/#description") },
        { d->aboutBuilds,       QUrl("https://github.com/eiskaltdcpp/eiskaltdcpp/#packages-and-installers") },
        { d->aboutSource,       QUrl("https://github.com/eiskaltdcpp/eiskaltdcpp/") },
        { d->aboutIssues,       QUrl("https://github.com/eiskaltdcpp/eiskaltdcpp/issues") },
        { d->aboutWiki,         QUrl("https://github.com/eiskaltdcpp/eiskaltdcpp/wiki") },
        { d->aboutChangelog,    QUrl("https://github.com/eiskaltdcpp/eiskaltdcpp/blob/master/ChangeLog.txt") },
    };

    if (urlsTable.contains(act)) {
        QDesktopServices::openUrl(urlsTable[act]);
    }
}

void MainWindow::slotAboutClient() {
    About a(this);

    double ratio;
    double down = static_cast<double>(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::TOTAL_DOWNLOAD, true));
    double up   = static_cast<double>(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::TOTAL_UPLOAD, true));

    if (down > 0)
        ratio = up / down;
    else
        ratio = 0;

    a.label->setText(QString("<b>%1</b> %2")
                     .arg(QString::fromStdString(eiskaltdcppAppNameString))
                     .arg(QString::fromStdString(eiskaltdcppVersionString)));

    QString html_format = "a { text-decoration:none; }\n"
                          "a:hover { text-decoration: underline; }\n";

    QString about_text = tr("EiskaltDC++ is a graphical client for Direct Connect and ADC protocols.")+
                         QString("<br/>")+
                         QString("<br/>")+
                         tr("DC++ core version: %1 (modified)").arg(DCPP_VERSION)+
                         QString("<br/>")+
                         QString("<br/>")+
                         tr("Home page: ")+
                         QString("<a href=\"https://github.com/eiskaltdcpp/eiskaltdcpp/\">"
                                 "https://github.com/eiskaltdcpp/eiskaltdcpp/</a>")+
                         QString("<br/>")+
                         QString("<br/>")+
                         tr("Total up: <b>%1</b>").arg(WulforUtil::formatBytes(up))+
                         QString("<br/>")+
                         tr("Total down: <b>%1</b>").arg(WulforUtil::formatBytes(down))+
                         QString("<br/>")+
                         tr("Ratio: <b>%1</b>").arg(ratio, 0, 'f', 3);

    a.label_ABOUT->setText(about_text);

    a.textBrowser_AUTHORS->document()->setDefaultStyleSheet(html_format);

    a.textBrowser_AUTHORS->setText(
        tr("Please use <a href=\"https://github.com/eiskaltdcpp/eiskaltdcpp/issues\">"
        "https://github.com/eiskaltdcpp/eiskaltdcpp/issues</a> to report bugs.<br/>")+
        QString("<br/>")+
        tr("<b>Developers</b><br/>")+
        QString("<br/>")+
        QString("&nbsp; 2009-2012 <a href=\"mailto:dein.negativ@gmail.com\">Andrey Karlov</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("lead developer")+QString(", 2009-2012")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("release manager")+QString(", 2009-2010")+
        QString("<br/>")+
        QString("<br/>")+
        QString("&nbsp; 2009-2015 <a href=\"mailto:dhamp@ya.ru\">Eugene Petrov</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("Arch Linux maintainer")+QString(", 2009-2015")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("developer")+QString(", 2009-2015")+
        QString("<br/>")+
        QString("<br/>")+
        QString("&nbsp; 2010-2023 <a href=\"mailto:tehnick-8@yandex.ru\">Boris Pek</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("Debian/Ubuntu maintainer")+QString(", 2010-2019")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("developer")+QString(", 2010-2019")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("translations coordinator")+QString(", 2010-2019")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("release manager")+QString(", 2010-2019")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("lead developer")+QString(", 2012-2019")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("macOS maintainer")+QString(", 2018-2019")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("MS Windows maintainer")+QString(", 2019")+
        QString("<br/>")+
        QString("<br/>")+
        QString("&nbsp; 2010-2015 <a href=\"mailto:pavelvat@gmail.com\">Pavel Vatagin</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("MS Windows maintainer")+QString(", 2010-2017")+
        QString("<br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("developer")+QString(", 2010-2017")+
        QString("<br/>")+
        QString("<br/>")+
        QString("&nbsp; 2010-2014 <a href=\"mailto:tka4ev@gmail.com\">Alexandr Tkachev</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("developer")+QString(", 2010-2014")+
        QString("<br/>")+
        QString("<br/>")+
        tr("<b>Graphic files</b><br/>")+
        QString("<br/>")+
        QString("&nbsp; 2009-2010 <a href=\"mailto:wiselord1983@gmail.com\">Uladzimir Bely</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("creator of the logo of the project")+
        QString("<br/>")+
        QString("<br/>")+
        QString("&nbsp; 2010-2015 <a href=\"mailto:tehnick-8@yandex.ru\">Boris Pek</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; * ")+
        tr("tiny updates of the logo")+
        QString("<br/>")+
        QString("<br/>")
        );

    a.textBrowser_TRANSLATION->document()->setDefaultStyleSheet(html_format);

    a.textBrowser_TRANSLATION->setText(
        tr("Participate in the translation. It is easy:")+
        QString("<br/>")+
        QString("<a href=\"https://www.transifex.com/tehnick/eiskaltdcpp/\">https://www.transifex.com/tehnick/eiskaltdcpp/</a><br/>")+
        QString("<br/>")+
        tr("Russian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2023 <a href=\"mailto:tehnick-8@yandex.ru\">Boris Pek</a> aka Tehnick<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2009-2010 <a href=\"mailto:wiselord1983@gmail.com\">Uladzimir Bely</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2012 <a href=\"mailto:tret2003@gmail.com\">Vyacheslav Tretyakov</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2018 <a href=\"https://app.transifex.com/user/profile/adem4ik/\">Andrei Stepanov</a><br/>")+
        QString("<br/>")+
        tr("Belarusian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2009-2013 <a href=\"mailto:i.kliok@gmail.com\">Paval Shalamitski</a> aka Klyok<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2015 <a href=\"mailto:m_d@tut.by\">Zmicer Melayok</a><br/>")+
        QString("<br/>")+
        tr("Hungarian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2012 <a href=\"mailto:husumo@gmail.com\">Akos Berki</a> aka sumo<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2011-2014 <a href=\"mailto:marcus@elitemail.hu\">Márk Lutring</a><br/>")+
        QString("<br/>")+
        tr("French translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2023 <a href=\"mailto:alexandre.wallimann@gmail.com\">Alexandre Wallimann</a> aka Jellyffs<br/>")+
        QString("<br/>")+
        tr("Polish translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2012 <a href=\"mailto:arahael@gmail.com\">Arahael</a><br/>")+
        QString("<br/>")+
        tr("Ukrainian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010 <a href=\"mailto:dmytro.demenko@gmail.com\">Dmytro Demenko</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2013-2014 <a href=\"mailto:grayich@ukr.net\">gray</a> aka grayich<br/>")+
        QString("<br/>")+
        tr("Serbian (Cyrillic) translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2014 <a href=\"mailto:trifunovic@openmailbox.org\">Marko Trifunović</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2015 <a href=\"mailto:miroslav031@gmail.com\">Miroslav Petrovic</a><br/>")+
        QString("<br/>")+
        tr("Serbian (Latin) translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2015 <a href=\"mailto:miroslav031@gmail.com\">Miroslav Petrovic</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2014 <a href=\"mailto:trifunovic@openmailbox.org\">Marko Trifunović</a><br/>")+
        QString("<br/>")+
        tr("Spanish translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2019 <a href=\"mailto:sl1pkn07@gmail.com\">Gustavo Alvarez</a> aka sL1pKn07<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2012-2021 <a href=\"mailto:klondike@klondike.es\">Francisco Blas Izquierdo Riera</a> aka klondike<br/>")+
        QString("<br/>")+
        tr("Basque translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2014-2015 <a href=\"mailto:egoitzro2@hotmail.com\">Egoitz Rodriguez</a><br/>")+
        QString("<br/>")+
        tr("Bulgarian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2012 <a href=\"mailto:dimitrov.rusi@gmail.com\">Rusi Dimitrov</a> aka PsyTrip<br/>")+
        QString("<br/>")+
        tr("Slovak translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2010-2012 <a href=\"mailto:martin.durisin@gmail.com\">Martin Durisin</a><br/>")+
        QString("<br/>")+
        tr("Czech translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2011-2012 <a href=\"mailto:uhlikx@seznam.cz\">Uhlik</a><br/>")+
        QString("<br/>")+
        tr("German translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2011-2012 <a href=\"mailto:kgeorgokitsos@yahoo.de\">Konstantinos Georgokitsos</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2011-2012 <a href=\"mailto:tilkax@gmail.com\">Tillmann Karras</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2012-2016 <a href=\"mailto:be.w@mail.ru\">Benjamin Weber</a><br/>")+
        QString("<br/>")+
        tr("Greek translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2011-2012 <a href=\"mailto:kgeorgokitsos@yahoo.de\">Konstantinos Georgokitsos</a><br/>")+
        QString("<br/>")+
        tr("Italian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2012 <a href=\"mailto:netcelli@gmail.com\">Stefano Simoncelli</a><br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2012 <a href=\"mailto:lorenzo.keller@gmail.com\">Lorenzo Keller</a><br/>")+
        QString("<br/>")+
        tr("Portuguese (Brazil) translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2013-2015 <a href=\"mailto:heldercro@gmail.com\">Helder Cesar</a> aka redrum<br/>")+
        QString("<br/>")+
        tr("Vietnamese translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2013 <a href=\"mailto:ppanhh@gmail.com\">Anh Phan</a><br/>")+
        QString("<br/>")+
        tr("Chinese (China) translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2013 <a href=\"mailto:syaomingl@gmail.com\">Syaoming Lai</a><br/>")+
        QString("<br/>")+
        tr("Swedish (Sweden) translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2014-2023 <a href=\"mailto:sopor@hotmail.com\">Sopor</a><br/>")+
        QString("<br/>")+
        tr("Turkish translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2015-2023 <a href=\"https://app.transifex.com/user/profile/mauron/\">mauron</a><br/>")+
        QString("<br/>")+
        tr("Danish translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2021 <a href=\"mailto:What2Write@protonmail.com\">What2Write</a><br/>")+
        QString("<br/>")+
        tr("Georgian translation<br/>")+
        QString("&nbsp;&nbsp;&nbsp; 2023 <a href=\"mailto:temuri.doghonadze@gmail.com\">Temuri Doghonadze</a><br/>")+
        QString("<br/>")
        );

    a.textBrowser_LICENSE->document()->setDefaultStyleSheet(html_format);

    a.textBrowser_LICENSE->setText(
        QString("This program is free software: you can redistribute it and/or modify it under the terms "
                "of the GNU General Public License as published by the Free Software Foundation, either "
                "version 3 of the License, or (at your option) any later version.<br/>"
                "<br/>"
                "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
                "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
                "See the GNU General Public License for more details.<br/>"
                "<br/>"
                "You should have received a copy of the GNU General Public License along with this program. "
                "If not, see &lt;<a href=\"https://www.gnu.org/licenses/\">https://www.gnu.org/licenses/</a>&gt;.<br/>")
        );

    a.exec();
}

void MainWindow::slotUnixSignal(int sig){
    printf("Received unix signal %i\n", sig);
}

void MainWindow::slotCloseCurrentWidget(){
    Q_D(MainWindow);

    ArenaWidget *awgt = dynamic_cast<ArenaWidget*>(d->arena->widget());

    if (awgt)
        qtCtx()->arenaWidgetManager()->rem(awgt);
}

void MainWindow::slotSideBarDockMenu(){
    Q_D(MainWindow);

    QMenu *m = new QMenu(this);
    QAction *act = new QAction(tr("Show close buttons"), m);

    act->setCheckable(true);
    act->setChecked(qtCtx()->settings()->getBool(SIDEBAR_SHOW_CLOSEBUTTONS, true));

    m->addAction(act);

    if (m->exec(QCursor::pos())){
        qtCtx()->settings()->setBool(SIDEBAR_SHOW_CLOSEBUTTONS, act->isChecked());

        // Tiny hack for repainting rows!
        d->sideDock->resize(d->sideDock->size()+QSize(0, -2));
    }

    m->deleteLater();
}

void MainWindow::slotAboutQt(){
    QMessageBox::aboutQt(this);
}

void MainWindow::slotUpdateFavHubMenu() {
    Q_D(MainWindow);

    d->favHubMenu->clear();

    const FavoriteHubEntryList& fl = dcCtx().getFavoriteManager()->getFavoriteHubs();

    for (auto &i : fl) {
        const FavoriteHubEntry &entry = *i;

        QString url = _q(entry.getServer());
        QString name = entry.getName().empty() ? tr("[No name]") : _q(entry.getName());
        QString encoding = qtCtx()->wulforUtil()->dcEnc2QtEnc(QString::fromStdString(entry.getEncoding()));
        QString menuItem = QString("%1 - %2").arg(name).arg(url);

        QAction *action = new QAction(menuItem, d->favHubMenu);
        action->setStatusTip(encoding);
        action->setToolTip(url);

        if (qobject_cast<HubFrame*>(qtCtx()->hubManager()->getHub(url))) {
            action->setCheckable(true);
            action->setChecked(true);
        }

        d->favHubMenu->addAction(action);
    }
}

void MainWindow::slotConnectFavHub(QAction *action) {

    QString url = action->toolTip();
    QString encoding = action->statusTip();

    newHubFrame(url, encoding);
}

void MainWindow::nextMsg(){
    Q_D(MainWindow);

    HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->activeHub());

    if (fr)
        fr->nextMsg();
    else{
        QWidget *wg = d->arena->widget();

        bool pmw = false;

        if (wg)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm)
                pm->nextMsg();
        }
    }
}

void MainWindow::prevMsg(){
    Q_D(MainWindow);
    HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->activeHub());

    if (fr)
        fr->prevMsg();
    else{
        QWidget *wg = d->arena->widget();

        bool pmw = false;

        if (wg)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm)
                pm->prevMsg();
        }
    }
}

void MainWindow::on(dcpp::LogManagerListener::Message, time_t t, const std::string& m) noexcept{
    Q_UNUSED(t)
    emit coreLogMessage(_q(m.c_str()));
}

void MainWindow::on(dcpp::QueueManagerListener::Finished, QueueItem *item, const std::string &dir, int64_t) noexcept{
    if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)){
        UserPtr user = item->getDownloads()[0]->getUser();
        QString listName = _q(item->getListName());

        emit coreOpenShare(user, listName, _q(dir));
    }

    const int qsize = dcCtx().getQueueManager()->lockQueue().size();
    dcCtx().getQueueManager()->unlockQueue();

    if (qsize == 1)
        emit notifyMessage(Notification::TRANSFER, tr("Download Queue"), tr("All downloads complete"));
}

void MainWindow::on(dcpp::TimerManagerListener::Second, uint64_t ticks) noexcept{
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

    lastUpdate = ticks;
    lastUp     = Socket::getTotalUp();
    lastDown   = Socket::getTotalDown();

    SettingsManager *SM = dcCtx().getSettingsManager();
    SM->set(SettingsManager::TOTAL_UPLOAD,   qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::TOTAL_UPLOAD, true)   + upDiff);
    SM->set(SettingsManager::TOTAL_DOWNLOAD, qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::TOTAL_DOWNLOAD, true) + downDiff);

    emit coreUpdateStats(map);
}

void MainWindow::slotShowSpeedLimits(){
    Notification *N = qtCtx()->notification();
    if (N)
        N->slotShowSpeedLimits();
}

void MainWindow::slotSuppressTxt(){
    Notification *N = qtCtx()->notification();
    QAction *act = qobject_cast<QAction*>(sender());
    if (N && act)
        N->setSuppressTxt(act->isChecked());
}

void MainWindow::slotSuppressSnd(){
    Notification *N = qtCtx()->notification();
    QAction *act = qobject_cast<QAction*>(sender());
    if (N && act)
        N->setSuppressSnd(act->isChecked());
}

#if defined(Q_OS_MAC)
void MainWindow::initDockMenuBar(){
    QMenu *menu = new QMenu(this);
    QAction *setup_speed_lim = new QAction(tr("Setup speed limits"), menu);

    setup_speed_lim->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiSPEED_LIMIT_ON));

    QMenu *menuAdditional = new QMenu(tr("Additional"), this);
    QAction *actSuppressSnd = new QAction(tr("Suppress sound notifications"), menuAdditional);
    QAction *actSuppressTxt = new QAction(tr("Suppress text notifications"), menuAdditional);

    actSuppressSnd->setCheckable(true);
    actSuppressSnd->setChecked(false);

    actSuppressTxt->setCheckable(true);
    actSuppressTxt->setChecked(false);

    connect(setup_speed_lim, &QAction::triggered, this, &MainWindow::slotShowSpeedLimits);
    connect(actSuppressTxt, &QAction::triggered, this, &MainWindow::slotSuppressTxt);
    connect(actSuppressSnd, &QAction::triggered, this, &MainWindow::slotSuppressSnd);

    menuAdditional->addActions(QList<QAction*>() << actSuppressTxt << actSuppressSnd);
    menu->addAction(setup_speed_lim);
    menu->addMenu(menuAdditional);

    menu->setAsDockMenu();
}
#endif // defined(Q_OS_MAC)

