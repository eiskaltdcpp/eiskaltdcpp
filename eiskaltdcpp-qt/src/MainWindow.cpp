/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
#include <QTextCodec>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include <QProgressBar>
#include <QFileDialog>
#include <QRegExp>
#include <QDir>
#include <QInputDialog>
#include <QDockWidget>
#include <QLabel>
#include <QShortcut>
#include <QKeySequence>
#include <QToolButton>
#include <QRegExp>
#include <QTreeView>
#include <QMetaType>
#include <QTimer>
#include <QAction>
#include <QStatusBar>

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
#include "IPFilter.h"
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
#include "dcpp/Singleton.h"
#include "dcpp/SettingsManager.h"
#include "WulforSettings.h"
#include "WulforUtil.h"

using namespace std;

class MainWindowPrivate {
public:
        typedef QList<QAction*> ActionList;
        typedef QList<ArenaWidget*> ArenaWidgetList;
        typedef QMap<ArenaWidget*, QWidget*> ArenaWidgetMap;
        
        bool isUnload;
        bool exitBegin;

        // position and geometry
        bool showMax;
        int w;
        int h;
        int xPos;
        int yPos;

        // Widgets
        QDockWidget *arena;
        QDockWidget *transfer_dock;
        QDockWidget *sideDock;

        ToolBar *fBar; //for actions
        ToolBar *sBar; //for fast search

        LineEdit   *searchLineEdit;
        QStringList core_msg_history;
        QLabel *statusLabel;
        QLabel *statusSPLabel;
        QLabel *statusDLabel;
        QLabel *statusTRLabel;
        QLabel *msgLabel;
        QProgressBar *progressSpace;
        QProgressBar *progressHashing;
        HashProgress *_progress_dialog; // Hashing progress dialog

        QMenu   *menuFile;
        QAction *fileOpenMagnet;
        QAction *fileFileListBrowser;
        QAction *fileFileHasher;
        QAction *fileFileListBrowserLocal;
        QAction *fileRefreshShareHashProgress;
        QAction *fileOpenLogFile;
        QAction *fileOpenDownloadDirectory;
        QAction *fileHideWindow;
        QAction *fileQuit;

        QMenu   *menuHubs;
        QAction *hubsHubReconnect;
        QAction *hubsQuickConnect;
        QAction *hubsFavoriteHubs;
        QAction *hubsPublicHubs;
        QAction *hubsFavoriteUsers;

        QMenu   *menuTools;
        QAction *toolsSearch;
        QAction *toolsADLS;
        QAction *toolsTransfers;
        QAction *toolsDownloadQueue;
        QAction *toolsQueuedUsers;
        QAction *toolsFinishedDownloads;
        QAction *toolsFinishedUploads;
        QAction *toolsSpy;
        QAction *toolsAntiSpam;
        QAction *toolsIPFilter;
        QAction *menuAwayAction;
        QAction *toolsHubManager;
        // submenu
        QMenu   *menuAway;
        QActionGroup *awayGroup;
        QAction *toolsAwayOn;
        QAction *toolsAwayOff;
        QAction *toolsAutoAway;
        // end
        QAction *toolsHideProgressSpace;
        QAction *toolsHideLastStatus;
        QAction *toolsHideUsersStatisctics;
        QAction *toolsCopyWindowTitle;
        QAction *toolsOptions;
#ifdef USE_JS
        QAction *toolsJS;
        QAction *toolsJSConsole;
        ScriptConsole *scriptConsole;
#endif
        QAction *toolsSwitchSpeedLimit;

        QMenu   *menuPanels;
        // submenu
        QMenu   *sh_menu;
        // end
        QAction *panelsWidgets;
        QAction *panelsTools;
        QAction *panelsSearch;

        // Standalone shortcuts
        QAction *prevTabShortCut;
        QAction *nextTabShortCut;
        QAction *prevMsgShortCut;
        QAction *nextMsgShortCut;
        QAction *closeWidgetShortCut;
        QAction *toggleMainMenuShortCut;

        QAction *chatDisable;
        QAction *findInWidget;
        QAction *chatClear;

        QMenu *menuWidgets;
        QHash<QAction*, ArenaWidget*> menuWidgetsHash;

        QMenu   *menuAbout;
        QAction *aboutHomepage;
        QAction *aboutSource;
        QAction *aboutIssues;
        QAction *aboutWiki;
        QAction *aboutChangelog;
        QAction *aboutClient;
        QAction *aboutQt;

        ActionList toolBarActions;
        ActionList fileMenuActions;
        ActionList hubsMenuActions;
        ActionList toolsMenuActions;
        ArenaWidgetList arenaWidgets;
        ArenaWidgetMap arenaMap;
};

static const QString &TOOLBUTTON_STYLE = "mainwindow/toolbar-toolbutton-style";
static const QString &EMPTY_SETTINGS = "mainwindow/empty-settings";
static const QString &SIDEBAR_SHOW_CLOSEBUTTONS = "mainwindow/sidebar-with-close-buttons";

MainWindow::MainWindow (QWidget *parent):
        QMainWindow(parent),
        d_ptr(new MainWindowPrivate())
{
    Q_D(MainWindow);
    
    d->statusLabel = NULL;
    d->fBar = NULL;
    d->sBar = NULL;
    d->_progress_dialog = NULL;
    d->sideDock = NULL;
    d->menuPanels = NULL;
#ifdef USE_JS
    d->scriptConsole = NULL;
#endif
    
    d->exitBegin = false;

    if (WBGET(WB_ANTISPAM_ENABLED)){
        AntiSpam::newInstance();

        AntiSpam::getInstance()->loadLists();
        AntiSpam::getInstance()->loadSettings();
    }

    if (WBGET(WB_IPFILTER_ENABLED)){
        IPFilter::newInstance();

        IPFilter::getInstance()->loadList();
    }

    ShortcutManager::newInstance();

    init();

    retranslateUi();

    LogManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);

    startSocket(false);

    setStatusMessage(tr("Ready"));

    TransferView::newInstance();

    d->transfer_dock->setWidget(TransferView::getInstance());
    d->toolsTransfers->setChecked(d->transfer_dock->isVisible());

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

HashProgress* MainWindow::progress_dialog() {
    Q_D(MainWindow);
    
    if( d->_progress_dialog == NULL )
        d->_progress_dialog = new HashProgress(this);
    
    return d->_progress_dialog;
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
    
    Q_D(MainWindow);
    
    delete d->arena;
    delete d->fBar;
    delete d->sBar;

    ShortcutManager::deleteInstance();
    SearchBlacklist::deleteInstance();
    
    delete d_ptr;
}

void MainWindow::setUnload ( bool b ) {
    Q_D(MainWindow); 
    
    d->isUnload = b;
}

void MainWindow::closeEvent(QCloseEvent *c_e){
    Q_D(MainWindow);
    
    if (!d->isUnload && WBGET(WB_TRAY_ENABLED)){
        hide();
        c_e->ignore();

        return;
    }

    if (d->isUnload && WBGET(WB_EXIT_CONFIRM) && !d->exitBegin){
        QMessageBox::StandardButton ret;

        ret = QMessageBox::question(this, tr("Exit confirm"),
                                    tr("Exit program?"),
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

    if (WBGET("app/clear-search-history-on-exit", false))
        WSSET(WS_SEARCH_HISTORY, "");
    if (WBGET("app/clear-download-directories-history-on-exit", false))
        WSSET(WS_DOWNLOAD_DIR_HISTORY, "");

    if (d->sideDock)
        d->sideDock->hide();

    d->transfer_dock->hide();

    blockSignals(true);

    if (TransferView::getInstance()){
        TransferView::getInstance()->close();
        TransferView::deleteInstance();
    }

    if (SearchManager::getInstance())
        SearchManager::getInstance()->disconnect();

    if (ConnectionManager::getInstance())
        ConnectionManager::getInstance()->disconnect();

    c_e->accept();
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

    if (WBGET(WB_APP_AUTO_AWAY) && !Util::getManualAway()){
        Util::setAway(false);

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

    if (_q(SETTING(NICK)).isEmpty()){
        activateWindow();
        raise();

        bool ok = false;
        QString new_nick = QInputDialog::getText(this, tr("Enter user nick"), tr("Nick"), QLineEdit::Normal, tr("User"), &ok);

        if (ok && !new_nick.isEmpty()){
            SettingsManager::getInstance()->set(SettingsManager::NICK, _tq(new_nick));

            ok = (QMessageBox::question(this, "EiskaltDC++", tr("Would you like to change other settings?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No);
        }

        if (!ok)
            slotToolsSettings();
    }

    e->accept();
}

void MainWindow::hideEvent(QHideEvent *e){
    Q_D(MainWindow);
    
    d->showMax = isMaximized();

    if (!d->showMax){
        d->h = height();
        d->w = width();
        d->xPos = x();
        d->yPos = y();
    }

    e->accept();

    if (d->sideDock && d->sideDock->isFloating())
        d->sideDock->hide();

    if (WBGET(WB_APP_AUTO_AWAY)){
        Util::setAway(true);

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
    else if (obj == d->progressSpace && e->type() == QEvent::MouseButtonDblClick ){
        slotFileOpenDownloadDirectory();

        return true;
    }

    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::init(){
    Q_D(MainWindow);
    
    setObjectName("MainWindow");

    connect(this, SIGNAL(coreLogMessage(QString)), this, SLOT(setStatusMessage(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreOpenShare(dcpp::UserPtr,QString,QString)), this, SLOT(showShareBrowser(dcpp::UserPtr,QString,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUpdateStats(QMap<QString,QString>)), this, SLOT(updateStatus(QMap<QString,QString>)), Qt::QueuedConnection);

    d->arena = new QDockWidget();
#if QT_VERSION >= 0x040500
    d->arena->setWidget(NULL);
    d->arena->setFloating(false);
#endif
    d->arena->setContentsMargins(0, 0, 0, 0);
    d->arena->setAllowedAreas(Qt::RightDockWidgetArea);
    d->arena->setFeatures(QDockWidget::NoDockWidgetFeatures);
    d->arena->setContextMenuPolicy(Qt::CustomContextMenu);
    d->arena->setTitleBarWidget(new QWidget(d->arena));
    d->arena->setMinimumSize( 10, 10 );

    d->transfer_dock = new QDockWidget(this);
#if QT_VERSION >= 0x040500
    d->transfer_dock->setWidget(NULL);
    d->transfer_dock->setFloating(false);
#endif
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

    setWindowIcon(WICON(WulforUtil::eiICON_APPL));

    setWindowTitle(QString("%1").arg(EISKALTDCPP_WND_TITLE));

    initActions();

    initMenuBar();

    initStatusBar();

    initSearchBar();

    initToolbar();

    initSideBar();

    loadSettings();

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotExit()));
    
    connect(ArenaWidgetManager::getInstance(), SIGNAL(activated(ArenaWidget*)), this, SLOT(mapWidgetOnArena(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(added(ArenaWidget*)),     this, SLOT(insertWidget(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(removed(ArenaWidget*)),   this, SLOT(removeWidget(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(updated(ArenaWidget*)),   this, SLOT(updated(ArenaWidget*)));

#ifdef LUA_SCRIPT
    ScriptManager::getInstance()->load();
    if (BOOLSETTING(USE_LUA)){
        // Start as late as possible, as we might (formatting.lua) need to examine settings
        string defaultluascript="startup.lua";
        ScriptManager::getInstance()->EvaluateFile(defaultluascript);
    }
#endif
}

void MainWindow::loadSettings(){
    Q_D(MainWindow);
    WulforSettings *WS = WulforSettings::getInstance();

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

    QString dockwidgetsState = WSGET(WS_MAINWINDOW_STATE);

    if (!dockwidgetsState.isEmpty()){
        this->restoreState(QByteArray::fromBase64(dockwidgetsState.toAscii()));
        qDebug() << dockwidgetsState;
    }

    d->fBar->setVisible(WBGET(WB_TOOLS_PANEL_VISIBLE));
    d->panelsTools->setChecked(WBGET(WB_TOOLS_PANEL_VISIBLE));

    d->sBar->setVisible(WBGET(WB_SEARCH_PANEL_VISIBLE));
    d->panelsSearch->setChecked(WBGET(WB_SEARCH_PANEL_VISIBLE));

    if (d->sideDock){
        if (d->sideDock->isFloating() && WBGET(WB_MAINWINDOW_HIDE) && WBGET(WB_TRAY_ENABLED))
            d->sideDock->hide();
        else
            d->sideDock->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));
    } else if ( findChild<MultiLineToolBar*> ( "multiLineTabbar" ) ) {
        findChild<MultiLineToolBar*> ( "multiLineTabbar" )->setVisible ( WBGET ( WB_WIDGETS_PANEL_VISIBLE ) );
    } else if ( findChild<ToolBar*> ( "tBar" ) ) {
        findChild<ToolBar*> ( "tBar" )->setVisible ( WBGET ( WB_WIDGETS_PANEL_VISIBLE ) );
    }

    d->panelsWidgets->setChecked(WBGET(WB_WIDGETS_PANEL_VISIBLE));

    if (!WBGET(WB_MAIN_MENU_VISIBLE))
        toggleMainMenu(false);

    if (WBGET("mainwindow/dont-show-icons-in-menus", false))
        qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
}

void MainWindow::saveSettings(){
    Q_D(MainWindow);
    static bool stateIsSaved = false;

    if (stateIsSaved)
        return;

    if (isVisible())
        d->showMax = isMaximized();

    WBSET(WB_MAINWINDOW_MAXIMIZED, d->showMax);

    if (!d->showMax && height() > 0 && width() > 0){
        WISET(WI_MAINWINDOW_HEIGHT, height());
        WISET(WI_MAINWINDOW_WIDTH, width());
    }
    else{
        WISET(WI_MAINWINDOW_HEIGHT, d->h);
        WISET(WI_MAINWINDOW_WIDTH, d->w);
    }

    if (!d->showMax && x() >= 0 && y() >= 0){
        WISET(WI_MAINWINDOW_X, x());
        WISET(WI_MAINWINDOW_Y, y());
    }
    else{
        WISET(WI_MAINWINDOW_X, d->xPos);
        WISET(WI_MAINWINDOW_Y, d->yPos);
    }

    if (WBGET(WB_MAINWINDOW_REMEMBER))
        WBSET(WB_MAINWINDOW_HIDE, !isVisible());

    QString dockwidgetsState = QString::fromAscii(saveState().toBase64());
    WSSET(WS_MAINWINDOW_STATE, dockwidgetsState);
    qDebug() << dockwidgetsState;

    stateIsSaved = true;
}

void MainWindow::initActions(){
    Q_D(MainWindow);

    WulforUtil *WU = WulforUtil::getInstance();
    ShortcutManager *SM = ShortcutManager::getInstance();

    {
        d->fileOpenMagnet = new QAction("", this);
        d->fileOpenMagnet->setObjectName("fileOpenMagnet");
        SM->registerShortcut(d->fileOpenMagnet, tr("Ctrl+I"));
        d->fileOpenMagnet->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(d->fileOpenMagnet, SIGNAL(triggered()), this, SLOT(slotOpenMagnet()));

        d->fileFileListBrowserLocal = new QAction("", this);
        d->fileFileListBrowserLocal->setObjectName("fileFileListBrowserLocal");
        SM->registerShortcut(d->fileFileListBrowserLocal, tr("Ctrl+L"));
        d->fileFileListBrowserLocal->setIcon(WU->getPixmap(WulforUtil::eiOWN_FILELIST));
        connect(d->fileFileListBrowserLocal, SIGNAL(triggered()), this, SLOT(slotFileBrowseOwnFilelist()));

        d->fileFileListBrowser = new QAction("", this);
        d->fileFileListBrowser->setObjectName("fileFileListBrowser");
        d->fileFileListBrowser->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(d->fileFileListBrowser, SIGNAL(triggered()), this, SLOT(slotFileBrowseFilelist()));

        d->fileFileHasher = new QAction("", this);
        d->fileFileHasher->setObjectName("fileFileHasher");
        d->fileFileHasher->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(d->fileFileHasher, SIGNAL(triggered()), this, SLOT(slotFileHasher()));

        d->fileOpenLogFile = new QAction("", this);
        d->fileOpenLogFile->setObjectName("fileOpenLogFile");
        d->fileOpenLogFile->setIcon(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE));
        connect(d->fileOpenLogFile, SIGNAL(triggered()), this, SLOT(slotFileOpenLogFile()));

        d->fileOpenDownloadDirectory = new QAction("", this);
        d->fileOpenDownloadDirectory->setObjectName("fileOpenDownloadDirectory");
        d->fileOpenDownloadDirectory->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
        connect(d->fileOpenDownloadDirectory, SIGNAL(triggered()), this, SLOT(slotFileOpenDownloadDirectory()));

        d->fileRefreshShareHashProgress = new QAction("", this);
        d->fileRefreshShareHashProgress->setObjectName("fileRefreshShareHashProgress");
        SM->registerShortcut(d->fileRefreshShareHashProgress, tr("Ctrl+E"));
        d->fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        connect(d->fileRefreshShareHashProgress, SIGNAL(triggered()), this, SLOT(slotFileRefreshShareHashProgress()));

        d->fileHideWindow = new QAction("", this);
        d->fileHideWindow->setObjectName("fileHideWindow");
        SM->registerShortcut(d->fileHideWindow, tr("Ctrl+Alt+H"));
        d->fileHideWindow->setIcon(WU->getPixmap(WulforUtil::eiHIDEWINDOW));
        connect(d->fileHideWindow, SIGNAL(triggered()), this, SLOT(slotHideWindow()));

        d->fileQuit = new QAction("", this);
        d->fileQuit->setObjectName("fileQuit");
        SM->registerShortcut(d->fileQuit, tr("Ctrl+Q"));
        d->fileQuit->setMenuRole(QAction::QuitRole);
        d->fileQuit->setIcon(WU->getPixmap(WulforUtil::eiEXIT));
        connect(d->fileQuit, SIGNAL(triggered()), this, SLOT(slotExit()));

        d->hubsHubReconnect = new QAction("", this);
        d->hubsHubReconnect->setObjectName("hubsHubReconnect");
        SM->registerShortcut(d->hubsHubReconnect, tr("Ctrl+R"));
        d->hubsHubReconnect->setIcon(WU->getPixmap(WulforUtil::eiRECONNECT));
        connect(d->hubsHubReconnect, SIGNAL(triggered()), this, SLOT(slotHubsReconnect()));

        d->hubsQuickConnect = new QAction("", this);
        d->hubsQuickConnect->setObjectName("hubsQuickConnect");
        SM->registerShortcut(d->hubsQuickConnect, tr("Ctrl+N"));
        d->hubsQuickConnect->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
        connect(d->hubsQuickConnect, SIGNAL(triggered()), this, SLOT(slotQC()));

        d->hubsFavoriteHubs = new QAction("", this);
        d->hubsFavoriteHubs->setObjectName("hubsFavoriteHubs");
        SM->registerShortcut(d->hubsFavoriteHubs, tr("Ctrl+H"));
        d->hubsFavoriteHubs->setIcon(WU->getPixmap(WulforUtil::eiFAVSERVER));
        connect(d->hubsFavoriteHubs, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteHubs()));

        d->hubsPublicHubs = new QAction("", this);
        d->hubsPublicHubs->setObjectName("hubsPublicHubs");
        SM->registerShortcut(d->hubsPublicHubs, tr("Ctrl+P"));
        d->hubsPublicHubs->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(d->hubsPublicHubs, SIGNAL(triggered()), this, SLOT(slotHubsPublicHubs()));

        d->hubsFavoriteUsers = new QAction("", this);
        d->hubsFavoriteUsers->setObjectName("hubsFavoriteUsers");
        SM->registerShortcut(d->hubsFavoriteUsers, tr("Ctrl+U"));
        d->hubsFavoriteUsers->setIcon(WU->getPixmap(WulforUtil::eiFAVUSERS));
        connect(d->hubsFavoriteUsers, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteUsers()));

        d->toolsHubManager = new QAction("", this);
        d->toolsHubManager->setObjectName("toolsHubManager");
        d->toolsHubManager->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(d->toolsHubManager, SIGNAL(triggered()), this, SLOT(slotToolsHubManager()));

        d->toolsCopyWindowTitle = new QAction("", this);
        d->toolsCopyWindowTitle->setObjectName("toolsCopyWindowTitle");
        d->toolsCopyWindowTitle->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));
        connect(d->toolsCopyWindowTitle, SIGNAL(triggered()), this, SLOT(slotToolsCopyWindowTitle()));

        d->toolsOptions = new QAction("", this);
        d->toolsOptions->setObjectName("toolsOptions");
        SM->registerShortcut(d->toolsOptions, tr("Ctrl+O"));
        d->toolsOptions->setMenuRole(QAction::PreferencesRole);
        d->toolsOptions->setIcon(WU->getPixmap(WulforUtil::eiCONFIGURE));
        connect(d->toolsOptions, SIGNAL(triggered()), this, SLOT(slotToolsSettings()));

        d->toolsADLS = new QAction("", this);
        d->toolsADLS->setObjectName("toolsADLS");
        d->toolsADLS->setIcon(WU->getPixmap(WulforUtil::eiADLS));
        connect(d->toolsADLS, SIGNAL(triggered()), this, SLOT(slotToolsADLS()));

        d->toolsTransfers = new QAction("", this);
        d->toolsTransfers->setObjectName("toolsTransfers");
        SM->registerShortcut(d->toolsTransfers, tr("Ctrl+T"));
        d->toolsTransfers->setIcon(WU->getPixmap(WulforUtil::eiTRANSFER));
        d->toolsTransfers->setCheckable(true);
        connect(d->toolsTransfers, SIGNAL(toggled(bool)), this, SLOT(slotToolsTransfer(bool)));
        //transfer_dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        d->toolsDownloadQueue = new QAction("", this);
        d->toolsDownloadQueue->setObjectName("toolsDownloadQueue");
        SM->registerShortcut(d->toolsDownloadQueue, tr("Ctrl+D"));
        d->toolsDownloadQueue->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(d->toolsDownloadQueue, SIGNAL(triggered()), this, SLOT(slotToolsDownloadQueue()));

        d->toolsQueuedUsers = new QAction("", this);
        d->toolsQueuedUsers->setObjectName("toolsQueuedUsers");
        SM->registerShortcut(d->toolsQueuedUsers, tr("Ctrl+Shift+U"));
        d->toolsQueuedUsers->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(d->toolsQueuedUsers, SIGNAL(triggered()), this, SLOT(slotToolsQueuedUsers()));

        d->toolsFinishedDownloads = new QAction("", this);
        d->toolsFinishedDownloads->setObjectName("toolsFinishedDownloads");
        d->toolsFinishedDownloads->setIcon(WU->getPixmap(WulforUtil::eiDOWNLIST));
        connect(d->toolsFinishedDownloads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedDownloads()));

        d->toolsFinishedUploads = new QAction("", this);
        d->toolsFinishedUploads->setObjectName("toolsFinishedUploads");
        d->toolsFinishedUploads->setIcon(WU->getPixmap(WulforUtil::eiUPLIST));
        connect(d->toolsFinishedUploads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedUploads()));

        d->toolsSpy = new QAction("", this);
        d->toolsSpy->setObjectName("toolsSpy");
        d->toolsSpy->setIcon(WU->getPixmap(WulforUtil::eiSPY));
        connect(d->toolsSpy, SIGNAL(triggered()), this, SLOT(slotToolsSpy()));

        d->toolsAntiSpam = new QAction("", this);
        d->toolsAntiSpam->setObjectName("toolsAntiSpam");
        d->toolsAntiSpam->setIcon(WU->getPixmap(WulforUtil::eiSPAM));
        d->toolsAntiSpam->setCheckable(true);
        d->toolsAntiSpam->setChecked(AntiSpam::getInstance() != NULL);
        connect(d->toolsAntiSpam, SIGNAL(triggered()), this, SLOT(slotToolsAntiSpam()));

        d->toolsIPFilter = new QAction("", this);
        d->toolsIPFilter->setObjectName("toolsIPFilter");
        d->toolsIPFilter->setIcon(WU->getPixmap(WulforUtil::eiFILTER));
        d->toolsIPFilter->setCheckable(true);
        d->toolsIPFilter->setChecked(IPFilter::getInstance() != NULL);
        connect(d->toolsIPFilter, SIGNAL(triggered()), this, SLOT(slotToolsIPFilter()));

        d->toolsAwayOn = new QAction("", this);
        d->toolsAwayOn->setObjectName("toolsAwayOn");
        d->toolsAwayOn->setCheckable(true);
        connect(d->toolsAwayOn, SIGNAL(triggered()), this, SLOT(slotToolsSwitchAway()));

        d->toolsAwayOff = new QAction("", this);
        d->toolsAwayOff->setObjectName("toolsAwayOff");
        d->toolsAwayOff->setCheckable(true);
        connect(d->toolsAwayOff, SIGNAL(triggered()), this, SLOT(slotToolsSwitchAway()));

        d->toolsAutoAway = new QAction("", this);
        d->toolsAutoAway->setCheckable(true);
        d->toolsAutoAway->setChecked(WBGET(WB_APP_AUTO_AWAY));
        connect(d->toolsAutoAway, SIGNAL(triggered()), this, SLOT(slotToolsAutoAway()));

#ifdef USE_JS
        d->toolsJS = new QAction("", this);
        d->toolsJS->setObjectName("toolsJS");
        d->toolsJS->setIcon(WU->getPixmap(WulforUtil::eiPLUGIN));
        connect(d->toolsJS, SIGNAL(triggered()), this, SLOT(slotToolsJS()));

        d->toolsJSConsole = new QAction("", this);
        d->toolsJSConsole->setObjectName("toolsJSConsole");
        SM->registerShortcut(d->toolsJSConsole, tr("Ctrl+Alt+J"));
        d->toolsJSConsole->setIcon(WU->getPixmap(WulforUtil::eiCONSOLE));
        connect(d->toolsJSConsole, SIGNAL(triggered()), this, SLOT(slotToolsJSConsole()));
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
        SM->registerShortcut(d->toolsSearch, tr("Ctrl+S"));
        d->toolsSearch->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
        connect(d->toolsSearch, SIGNAL(triggered()), this, SLOT(slotToolsSearch()));

        d->toolsHideProgressSpace = new QAction("", this);
        d->toolsHideProgressSpace->setObjectName("toolsHideProgressSpace");

#if (!defined FREE_SPACE_BAR_C)
        d->toolsHideProgressSpace->setVisible(false);
#endif
        d->toolsHideProgressSpace->setIcon(WU->getPixmap(WulforUtil::eiFREESPACE));
        connect(d->toolsHideProgressSpace, SIGNAL(triggered()), this, SLOT(slotHideProgressSpace()));

        d->toolsHideLastStatus = new QAction("", this);
        d->toolsHideLastStatus->setObjectName("toolsHideLastStatus");
        d->toolsHideLastStatus->setIcon(WU->getPixmap(WulforUtil::eiSTATUS));
        connect(d->toolsHideLastStatus, SIGNAL(triggered()), this, SLOT(slotHideLastStatus()));

        d->toolsHideUsersStatisctics = new QAction("", this);
        d->toolsHideUsersStatisctics->setObjectName("toolsHideUsersStatisctics");
        d->toolsHideUsersStatisctics->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(d->toolsHideUsersStatisctics, SIGNAL(triggered()), this, SLOT(slotHideUsersStatistics()));

        d->toolsSwitchSpeedLimit = new QAction("", this);
        d->toolsSwitchSpeedLimit->setObjectName("toolsSwitchSpeedLimit");
        d->toolsSwitchSpeedLimit->setIcon(BOOLSETTING(THROTTLE_ENABLE)? WU->getPixmap(WulforUtil::eiSPEED_LIMIT_ON) : WU->getPixmap(WulforUtil::eiSPEED_LIMIT_OFF));
        d->toolsSwitchSpeedLimit->setCheckable(true);
        d->toolsSwitchSpeedLimit->setChecked(BOOLSETTING(THROTTLE_ENABLE));
        connect(d->toolsSwitchSpeedLimit, SIGNAL(triggered()), this, SLOT(slotToolsSwitchSpeedLimit()));

        d->chatClear = new QAction("", this);
        d->chatClear->setObjectName("chatClear");
        d->chatClear->setIcon(WU->getPixmap(WulforUtil::eiCLEAR));
        connect(d->chatClear, SIGNAL(triggered()), this, SLOT(slotChatClear()));

        d->findInWidget = new QAction("", this);
        d->findInWidget->setObjectName("findInWidget");
        SM->registerShortcut(d->findInWidget, tr("Ctrl+F"));
        d->findInWidget->setIcon(WU->getPixmap(WulforUtil::eiFIND));
        connect(d->findInWidget, SIGNAL(triggered()), this, SLOT(slotFind()));

        d->chatDisable = new QAction("", this);
        d->chatDisable->setObjectName("chatDisable");
        d->chatDisable->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
        connect(d->chatDisable, SIGNAL(triggered()), this, SLOT(slotChatDisable()));

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
                << separator0
                << d->toolsTransfers
                << d->toolsDownloadQueue
                << d->toolsQueuedUsers
                << d->toolsFinishedDownloads
                << d->toolsFinishedUploads
                << d->toolsSwitchSpeedLimit
                //<< toolsHubManager
                << separator1
                << d->toolsSpy
                << d->toolsAntiSpam
                << d->toolsIPFilter
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
                << d->toolsSpy
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

        SM->registerShortcut(d->nextTabShortCut, tr("Ctrl+PgDown"));
        SM->registerShortcut(d->prevTabShortCut, tr("Ctrl+PgUp"));
        SM->registerShortcut(d->nextMsgShortCut, tr("Ctrl+Down"));
        SM->registerShortcut(d->prevMsgShortCut, tr("Ctrl+Up"));
        SM->registerShortcut(d->closeWidgetShortCut, tr("Ctrl+W"));
        SM->registerShortcut(d->toggleMainMenuShortCut, tr("Ctrl+M"));

        connect(d->nextMsgShortCut,        SIGNAL(triggered()), this, SLOT(nextMsg()));
        connect(d->prevMsgShortCut,        SIGNAL(triggered()), this, SLOT(prevMsg()));
        connect(d->closeWidgetShortCut,    SIGNAL(triggered()), this, SLOT(slotCloseCurrentWidget()));
        connect(d->toggleMainMenuShortCut, SIGNAL(triggered()), this, SLOT(slotHideMainMenu()));
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
        connect(d->panelsWidgets, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));

        d->panelsTools = new QAction("", this);
        d->panelsTools->setCheckable(true);
        connect(d->panelsTools, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));

        d->panelsSearch = new QAction("", this);
        d->panelsSearch->setCheckable(true);
        connect(d->panelsSearch, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));
    }
    {
        d->aboutHomepage = new QAction("", this);
        connect(d->aboutHomepage, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        d->aboutSource = new QAction("", this);
        connect(d->aboutSource, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        d->aboutIssues = new QAction("", this);
        connect(d->aboutIssues, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        d->aboutWiki = new QAction("", this);
        connect(d->aboutWiki, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        d->aboutChangelog = new QAction("", this);
        connect(d->aboutChangelog, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        d->aboutClient = new QAction("", this);
        d->aboutClient->setMenuRole(QAction::AboutRole);
        d->aboutClient->setIcon(WU->getPixmap(WulforUtil::eiICON_APPL)
                    .scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        connect(d->aboutClient, SIGNAL(triggered()), this, SLOT(slotAboutClient()));

        d->aboutQt = new QAction("", this);
        d->aboutQt->setMenuRole(QAction::AboutQtRole);
        d->aboutQt->setIcon(WU->getPixmap(WulforUtil::eiQT_LOGO));
        connect(d->aboutQt, SIGNAL(triggered()), this, SLOT(slotAboutQt()));
    }
}

void MainWindow::initMenuBar(){
#ifdef Q_WS_MAC
    setMenuBar(new QMenuBar());
    menuBar()->setParent(NULL);
    connect(this, SIGNAL(destroyed()), menuBar(), SLOT(deleteLater()));
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
        d->menuAbout->addAction(d->aboutSource);
        d->menuAbout->addAction(d->aboutIssues);
        d->menuAbout->addAction(d->aboutWiki);
        d->menuAbout->addAction(sep0);
        d->menuAbout->addAction(d->aboutChangelog);
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
    d->progressSpace = new QProgressBar(this);
    d->progressSpace->setMaximum(100);
    d->progressSpace->setMinimum(0);
    d->progressSpace->setAlignment( Qt::AlignHCenter );
    d->progressSpace->setMinimumWidth(100);
    d->progressSpace->setMaximumWidth(250);
    d->progressSpace->setFixedHeight(18);
    d->progressSpace->setToolTip(tr("Space free"));
    d->progressSpace->installEventFilter(this);

    if (!WBGET(WB_SHOW_FREE_SPACE))
        d->progressSpace->hide();
#else //FREE_SPACE_BAR_C
    WBSET(WB_SHOW_FREE_SPACE, false);
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
    statusBar()->addPermanentWidget(d->progressSpace);
#endif //FREE_SPACE_BAR_C
}

void MainWindow::initSearchBar(){
    Q_D(MainWindow);
    
    d->searchLineEdit = new LineEdit(this);

    connect(d->searchLineEdit,   SIGNAL(returnPressed()), this, SLOT(slotToolsSearch()));
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

        d->fileRefreshShareHashProgress->setText(tr("Refresh share"));

        d->fileHideWindow->setText(tr("Hide window"));

        if (!WBGET(WB_TRAY_ENABLED))
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

        d->toolsSpy->setText(tr("Search Spy"));

        d->toolsAntiSpam->setText(tr("AntiSpam module"));

        d->toolsIPFilter->setText(tr("IPFilter module"));

        d->toolsHideProgressSpace->setText(tr("Hide free space bar"));

        if (!WBGET(WB_SHOW_FREE_SPACE))
            d->toolsHideProgressSpace->setText(tr("Show free space bar"));

        d->toolsHideLastStatus->setText(tr("Hide last status message"));

        if (!WBGET(WB_LAST_STATUS))
            d->toolsHideLastStatus->setText(tr("Show last status message"));

        d->toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

        if (!WBGET(WB_USERS_STATISTICS))
            d->toolsHideUsersStatisctics->setText(tr("Show users statistics"));

        d->menuAway->setTitle(tr("Away message"));

        d->toolsAwayOn->setText(tr("On"));

        d->toolsAwayOff->setText(tr("Off"));

        d->toolsAutoAway->setText(tr("Away when not visible"));

        d->toolsCopyWindowTitle->setText(tr("Copy window title"));

        d->toolsOptions->setText(tr("Preferences"));

        d->toolsSearch->setText(tr("Search"));

        d->toolsADLS->setText(tr("ADLSearch"));

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

        if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
            d->panelsWidgets->setText(tr("Widgets panel"));
        else
            d->panelsWidgets->setText(tr("Widgets side dock"));

        d->panelsTools->setText(tr("Tools panel"));

        d->panelsSearch->setText(tr("Fast search panel"));

        d->menuAbout->setTitle(tr("&Help"));

        d->aboutHomepage->setText(tr("Homepage"));

        d->aboutSource->setText(tr("Source (git)"));

        d->aboutIssues->setText(tr("Report a Bug"));

        d->aboutWiki->setText(tr("Wiki of project"));

        d->aboutChangelog->setText(tr("Changelog (git)"));

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

    QStringList enabled_actions = QString(QByteArray::fromBase64(WSGET(WS_MAINWINDOW_TOOLBAR_ACTS).toAscii())).split(";", QString::SkipEmptyParts);

    if (enabled_actions.isEmpty())
        d->fBar->addActions(d->toolBarActions);
    else {
        foreach (const QString &objName, enabled_actions){
            QAction *act = findChild<QAction*>(objName);

            if (act)
                d->fBar->addAction(act);
        }
    }

    d->fBar->setContextMenuPolicy(Qt::CustomContextMenu);
    d->fBar->setMovable(true);
    d->fBar->setFloatable(true);
    d->fBar->setAllowedAreas(Qt::AllToolBarAreas);
    d->fBar->setWindowTitle(tr("Actions"));
    d->fBar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(WIGET(TOOLBUTTON_STYLE, Qt::ToolButtonIconOnly)));

    connect(d->fBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotToolbarCustomization()));

    addToolBar(d->fBar);

    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR) && WBGET(WB_MAINWINDOW_USE_M_TABBAR)){

        MultiLineToolBar *mBar = new MultiLineToolBar(this);
        mBar->setContextMenuPolicy(Qt::CustomContextMenu);
        mBar->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));
        
        connect(d->nextTabShortCut, SIGNAL(triggered()), mBar, SIGNAL(nextTab()));
        connect(d->prevTabShortCut, SIGNAL(triggered()), mBar, SIGNAL(prevTab()));
        
        addToolBar(mBar);
    }
    else if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR) && !WBGET(WB_MAINWINDOW_USE_M_TABBAR)){

        ToolBar *tBar = new ToolBar(this);
        tBar->setObjectName("tBar");
        tBar->initTabs();
        tBar->setMovable(true);
        tBar->setFloatable(true);
        tBar->setAllowedAreas(Qt::AllToolBarAreas);
        tBar->setContextMenuPolicy(Qt::CustomContextMenu);

        addToolBar(tBar);
        
        connect(d->nextTabShortCut, SIGNAL(triggered()), tBar, SLOT(nextTab()));
        connect(d->prevTabShortCut, SIGNAL(triggered()), tBar, SLOT(prevTab()));
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
    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
        return;
    
    Q_D(MainWindow);

    d->sideDock = new QDockWidget("", this);

    d->sideDock->setWidget(new SideBarView(this));
    d->sideDock->setFeatures(d->sideDock->features() & (~QDockWidget::DockWidgetClosable));
    d->sideDock->setObjectName("sideDock");
    d->sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    d->sideDock->setContextMenuPolicy(Qt::CustomContextMenu);

    addDockWidget(Qt::LeftDockWidgetArea, d->sideDock);

    connect(d->sideDock, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSideBarDockMenu()));
}

QObject *MainWindow::getToolBar(){
    Q_D(MainWindow);
    
    if (!d->fBar)
        return NULL;

    return qobject_cast<QObject*>(reinterpret_cast<QToolBar*>(d->fBar->qt_metacast("QToolBar")));
}

ArenaWidget *MainWindow::widgetForRole(ArenaWidget::Role r) const{
    ArenaWidget *awgt = NULL;
    Q_D(const MainWindow);

    switch (r){
    case ArenaWidget::Downloads:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, DownloadQueue>();
            awgt->setToolButton(d->toolsDownloadQueue);

            break;
        }
    case ArenaWidget::FinishedUploads:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, FinishedUploads>();
            awgt->setToolButton(d->toolsFinishedUploads);

            break;
        }
    case ArenaWidget::FinishedDownloads:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, FinishedDownloads>();
            awgt->setToolButton(d->toolsFinishedDownloads);

            break;
        }
    case ArenaWidget::FavoriteHubs:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, FavoriteHubs>();
            awgt->setToolButton(d->hubsFavoriteHubs);

            break;
        }
    case ArenaWidget::FavoriteUsers:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, FavoriteUsers>();
            awgt->setToolButton(d->hubsFavoriteUsers);

            break;
        }
    case ArenaWidget::PublicHubs:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, PublicHubs>();
            awgt->setToolButton(d->hubsPublicHubs);

            break;
        }
    case ArenaWidget::Spy:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, SpyFrame>();
            awgt->setToolButton(d->toolsSpy);

            break;
        }
    case ArenaWidget::ADLS:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, ADLS>();
            awgt->setToolButton(d->toolsADLS);

            break;
        }
    case ArenaWidget::QueuedUsers:
        {
            awgt = ArenaWidgetFactory().create<dcpp::Singleton, QueuedUsers>();
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

    address = QUrl::fromPercentEncoding(address.toAscii());

    HubFrame *fr = qobject_cast<HubFrame*>(HubManager::getInstance()->getHub(address));

    if (fr){
        ArenaWidgetManager::getInstance()->activate(fr);

        return;
    }
    
    fr = ArenaWidgetFactory().create<HubFrame, QWidget*, QString, QString>(this, address, enc);

    ArenaWidgetManager::getInstance()->activate(fr);
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

    if (Notification::getInstance())
        Notification::getInstance()->setToolTip(map["DSPEED"]+tr("/s"), map["USPEED"]+tr("/s"), map["DOWN"], map["UP"]);

    int leftM=0, topM=0, rightM=0, bottomM=0;
    d->statusSPLabel->getContentsMargins(&leftM, &topM, &rightM, &bottomM);
    int boundWidth = leftM + rightM;

    d->statusSPLabel->setFixedWidth(metrics.width(speedText) > d->statusSPLabel->width()? metrics.width(speedText) + boundWidth : d->statusSPLabel->width());
    d->statusDLabel->setFixedWidth(metrics.width(downText) > d->statusDLabel->width()? metrics.width(downText) + boundWidth : d->statusDLabel->width());

    if (WBGET(WB_SHOW_FREE_SPACE)) {
#ifdef FREE_SPACE_BAR_C
        std::string s = SETTING(DOWNLOAD_DIRECTORY);
        unsigned long long available = 0;
        unsigned long long total = 0;
        if (!s.empty()) {
            if (FreeSpace::FreeDiscSpace(s, &available, &total) == false) {
                available = 0;
                total = 0;
            }
        }
        float percent = 100.0f*(total-available)/total;
        QString format = tr("Free %1")
                         .arg(WulforUtil::formatBytes(available));

        QString tooltip = tr("Free %1 of %2")
                          .arg(WulforUtil::formatBytes(available))
                          .arg(WulforUtil::formatBytes(total));

        d->progressSpace->setFormat(format);
        d->progressSpace->setToolTip(tooltip);
        d->progressSpace->setValue(static_cast<unsigned>(percent));

        d->progressSpace->setFixedWidth(metrics.width(format) > d->progressSpace->width()? metrics.width(d->progressSpace->text()) + 40 : d->progressSpace->width());
#endif //FREE_SPACE_BAR_C
    }

    if ((Util::getAway() && !d->toolsAwayOn->isChecked()) || (!Util::getAway() && d->toolsAwayOff->isChecked())){
        QAction *act = Util::getAway()? d->toolsAwayOn : d->toolsAwayOff;

        act->setChecked(true);
    }

    updateHashProgressStatus();
}

void MainWindow::updateHashProgressStatus() {
    WulforUtil *WU = WulforUtil::getInstance();
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
            if (SETTING(HASHING_START_DELAY) >= 0){
                int left = SETTING(HASHING_START_DELAY) - Util::getUpTime();
                d->progressHashing->setValue( 100 * left / SETTING(HASHING_START_DELAY) );
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
            if (SETTING(HASHING_START_DELAY) >= 0){
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

    if (m.width(msg) > d->msgLabel->width())
        pure_msg = m.elidedText(msg, Qt::ElideRight, d->msgLabel->width(), 0);

    WulforUtil::getInstance()->textToHtml(pure_msg, true);
    WulforUtil::getInstance()->textToHtml(msg, true);

    d->msgLabel->setText(pure_msg);

    d->core_msg_history.push_back(msg);

    if (WIGET(WI_STATUSBAR_HISTORY_SZ) > 0){
        while (d->core_msg_history.size() > WIGET(WI_STATUSBAR_HISTORY_SZ))
            d->core_msg_history.removeFirst();
    }
    else
        d->core_msg_history.clear();

    d->msgLabel->setToolTip(d->core_msg_history.join("\n"));
    d->msgLabel->setMaximumHeight(d->statusLabel->height());
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

    foreach (const QString &arg, args){
        if (arg.startsWith("magnet:?")){
            Magnet m(this);
            m.setLink(arg);
            if (WIGET(WI_DEF_MAGNET_ACTION) == 0) {
                m.exec();
            }
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
    if (!isVisible()){
        show();
        raise();

        redrawToolPanel();
    }

    QStringList args = data.split("\n", QString::SkipEmptyParts);

    foreach (const QString &arg, args){
        if (arg.startsWith("magnet:?xt=urn:tree:tiger:")){
            Magnet m(this);
            m.setLink(arg);
            if (WIGET(WI_DEF_MAGNET_ACTION) == 0) {
                m.exec();
            }
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
    QString file = QFileDialog::getOpenFileName(this, tr("Choose file to open"),
                QString::fromStdString(Util::getPath(Util::PATH_FILE_LISTS)),
                tr("Modern XML Filelists") + " (*.xml.bz2);;" +
                tr("Modern XML Filelists uncompressed") + " (*.xml);;" +
                tr("All files") + " (*)");

    if (file.isEmpty())
        return;

    file = QDir::toNativeSeparators(file);
    UserPtr user = DirectoryListing::getUserFromFilename(_tq(file));

    if (user) {
        ArenaWidgetFactory().create<ShareBrowser, UserPtr, QString, QString>(user, file, "");
    } else {
        setStatusMessage(tr("Unable to load file list: Invalid file list name"));
    }
}

void MainWindow::redrawToolPanel(){
    Q_D(MainWindow);
    
    QHash<QAction*, ArenaWidget*>::iterator it = d->menuWidgetsHash.begin();
    QHash<QAction*, ArenaWidget*>::iterator end = d->menuWidgetsHash.end();

    ArenaWidget *awgt = NULL;
    PMWindow *pm = NULL;
    bool has_unread = false;

    for(; it != end; ++it){//also redraw all widget menu items and change window title if needed
        it.key()->setText(it.value()->getArenaShortTitle());
        it.key()->setIcon(it.value()->getPixmap());

        pm = qobject_cast<PMWindow *>(d->arenaMap[it.value()]);
        if (pm && pm->hasNewMessages())
            has_unread = true;

        awgt = qobject_cast<ArenaWidget*> (d->arenaMap[it.value()]);
        if (awgt && d->arena->widget() && d->arena->widget() == awgt->getWidget())
            setWindowTitle(awgt->getArenaTitle() + " :: " + QString("%1").arg(EISKALTDCPP_WND_TITLE));
    }

    if (!has_unread)
        Notify->resetTrayIcon();
    
    emit redrawWidgetPanels();
}

void MainWindow::mapWidgetOnArena(ArenaWidget *awgt){
    Q_D(MainWindow);
    
    if (!(awgt && awgt->getWidget())){
        d->arena->setWidget(NULL);
        
        return;
    }
    
    if (d->arena->widget() != awgt->getWidget())
        d->arena->setWidget(awgt->getWidget());

    setWindowTitle(awgt->getArenaTitle() + " :: " + QString("%1").arg(EISKALTDCPP_WND_TITLE));

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    ArenaWidget::Role role = awgt->role();

    bool widgetWithFilter = role == ArenaWidget::Hub ||
                            role == ArenaWidget::ShareBrowser ||
                            role == ArenaWidget::PublicHubs ||
                            role == ArenaWidget::Search ||
                            role == ArenaWidget::PrivateMessage;

    d->chatClear->setEnabled(role == ArenaWidget::Hub || role == ArenaWidget::PrivateMessage);
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
    
    connect(act, SIGNAL(triggered(bool)), this, SLOT(slotWidgetsToggle()));
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
        QAction *act = reinterpret_cast<QAction*>(sender());;

        act->setCheckable(true);

        a->setToolButton(act);
    }
   
    ArenaWidgetManager::getInstance()->toggle(a);
}

void MainWindow::toggleMainMenu(bool showMenu){
    static QAction *compactMenus = NULL;

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
                compactMenus->setIcon(WICON(WulforUtil::eiEDIT));
            }
            else {
                compactMenus->menu()->deleteLater();
                compactMenus->setMenu(NULL);
            }

            QMenu *m = new QMenu(this);

            foreach (QAction *a, menuBar()->actions())
                m->addAction(a);

            compactMenus->setMenu(m);

            connect(compactMenus, SIGNAL(triggered()), this, SLOT(slotShowMainMenu()));
        }

        if (d->fBar)
            d->fBar->insertAction(d->toolBarActions.first(), compactMenus);
    }

    WBSET(WB_MAIN_MENU_VISIBLE, showMenu);
}

void MainWindow::startSocket(bool changed){
    if (changed)
        ConnectivityManager::getInstance()->updateLast();
    try {
        ConnectivityManager::getInstance()->setup(true);
    } catch (const Exception& e) {
        showPortsError(e.getError());
    }
    ClientManager::getInstance()->infoUpdated();
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
    
    for (int k = 0; k < d->arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(d->arenaMap[d->arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }

    d->toolsSwitchSpeedLimit->setChecked(BOOLSETTING(THROTTLE_ENABLE));
}

void MainWindow::slotFileOpenLogFile(){
    QString f = QFileDialog::getOpenFileName(this, tr("Open log file"),_q(SETTING(LOG_DIRECTORY)), tr("Log files (*.log);;All files (*.*)"));

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
    QString directory = QString::fromStdString(SETTING(DOWNLOAD_DIRECTORY));

    directory.prepend( directory.startsWith("/")? ("file://") : ("file:///"));

    QDesktopServices::openUrl(QUrl::fromEncoded(directory.toUtf8()));
}

void MainWindow::slotFileBrowseOwnFilelist(){
    UserPtr user = ClientManager::getInstance()->getMe();
    QString file = QString::fromStdString(ShareManager::getInstance()->getOwnListFile());

    ArenaWidgetFactory().create<ShareBrowser, UserPtr, QString, QString>(user, file, "");
}

void MainWindow::slotFileHashProgress(){
    progress_dialog()->slotAutoClose(false);
    progress_dialog()->show();
}

void MainWindow::slotFileHasher(){
    FileHasher *m = new FileHasher(MainWindow::getInstance());
    m->setModal(true);
    m->exec();
    delete m;
}

void MainWindow::slotFileRefreshShareHashProgress(){
    switch( HashProgress::getHashStatus() ) {
    case HashProgress::IDLE:
    {
        ShareManager *SM = ShareManager::getInstance();

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
    HubFrame *fr = qobject_cast<HubFrame*>(HubManager::getInstance()->activeHub());

    if (fr)
        fr->reconnect();
}

void MainWindow::slotToolsADLS(){
    toggleSingletonWidget(widgetForRole(ArenaWidget::ADLS));
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
    toggleSingletonWidget(widgetForRole(ArenaWidget::Spy));
}

void MainWindow::slotToolsAntiSpam(){
    AntiSpamFrame fr(this);

    fr.exec();
    
    Q_D(MainWindow);

    d->toolsAntiSpam->setChecked(AntiSpam::getInstance() != NULL);
}

void MainWindow::slotToolsIPFilter(){   
    IPFilterFrame fr(this);

    fr.exec();
    
    Q_D(MainWindow);

    d->toolsIPFilter->setChecked(IPFilter::getInstance() != NULL);
}

void MainWindow::slotToolsAutoAway(){
    Q_D(MainWindow);
    
    WBSET(WB_APP_AUTO_AWAY, d->toolsAutoAway->isChecked());
}

void MainWindow::slotToolsSwitchAway(){
    Q_D(MainWindow);

    if ((sender() != d->toolsAwayOff) && (sender() != d->toolsAwayOn))
        return;

    bool away = d->toolsAwayOn->isChecked();

    Util::setAway(away);
    Util::setManualAway(away);
}

void MainWindow::slotToolsJS(){
#ifdef USE_JS
    ScriptManagerDialog(this).exec();
#endif
}

void MainWindow::slotJSFileChanged(const QString &script){
#ifdef USE_JS
    if (QMessageBox::warning(this, 
                             tr("Script Engine"), 
                             QString("\'%1\' has been changed. Reload it?").arg(script), 
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        ScriptEngine::getInstance()->loadScript(script);
#endif
}


void MainWindow::slotToolsJSConsole(){
    Q_D(MainWindow);
    
#ifdef USE_JS
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
    if (!WBGET(WB_TRAY_ENABLED))
        d->fileHideWindow->setText(tr("Show/hide find frame"));
    else
        d->fileHideWindow->setText(tr("Hide window"));
}

void MainWindow::slotToolsTransfer(bool toggled){
    Q_D(MainWindow);
    
    if (toggled){
        d->transfer_dock->setVisible(true);
        d->transfer_dock->setWidget(TransferView::getInstance());
    }
    else {
        d->transfer_dock->setWidget(NULL);
        d->transfer_dock->setVisible(false);
    }
}

void MainWindow::slotToolsSwitchSpeedLimit(){
    static WulforUtil *WU = WulforUtil::getInstance();
    Q_D(MainWindow);

    SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, d->toolsSwitchSpeedLimit->isChecked());
    d->toolsSwitchSpeedLimit->setIcon(BOOLSETTING(THROTTLE_ENABLE)? WU->getPixmap(WulforUtil::eiSPEED_LIMIT_ON) : WU->getPixmap(WulforUtil::eiSPEED_LIMIT_OFF));
}

void MainWindow::slotPanelMenuActionClicked(){
    QAction *act = qobject_cast<QAction *>(sender());

    if (act == 0)
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
        
        WBSET(WB_WIDGETS_PANEL_VISIBLE, d->panelsWidgets->isChecked());
    }
    else if (act == d->panelsTools){
        d->fBar->setVisible(d->panelsTools->isChecked());
        WBSET(WB_TOOLS_PANEL_VISIBLE, d->panelsTools->isChecked());
    }
    else if (act == d->panelsSearch){
        d->sBar->setVisible(d->panelsSearch->isChecked());
        WBSET(WB_SEARCH_PANEL_VISIBLE, d->panelsSearch->isChecked());
    }
}

void MainWindow::slotChatClear(){
    Q_D(MainWindow);
    
    HubFrame *fr = qobject_cast<HubFrame *>(d->arena->widget());
    PMWindow *pm = qobject_cast<PMWindow *>(d->arena->widget());

    if (fr)
        fr->clearChat();
    else if (pm)
        pm->clearChat();
}

void MainWindow::slotFind(){
    Q_D(MainWindow);
    
    if (!d->arena->widget() || !qobject_cast<ArenaWidget*>(d->arena->widget()))
        return;

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(d->arena->widget());
    awgt->requestFilter();
}

void MainWindow::slotChatDisable(){
    HubFrame *fr = qobject_cast<HubFrame*>(HubManager::getInstance()->activeHub());

    if (fr)
        fr->disableChat();
}

void MainWindow::slotWidgetsToggle(){
    Q_D(MainWindow);
    
    QAction *act = reinterpret_cast<QAction*>(sender());
    QHash<QAction*, ArenaWidget*>::iterator it = d->menuWidgetsHash.find(act);

    if (it == d->menuWidgetsHash.end())
        return;

    ArenaWidgetManager::getInstance()->activate(it.value());
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
    
    if (!d->isUnload && isActiveWindow() && WBGET(WB_TRAY_ENABLED)) {
        hide();
    }
}

void MainWindow::slotHideProgressSpace() {
    Q_D(MainWindow);
    
    if (WBGET(WB_SHOW_FREE_SPACE)) {
        d->progressSpace->hide();
        d->toolsHideProgressSpace->setText(tr("Show free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, false);
    } else {
        d->progressSpace->show();
        d->toolsHideProgressSpace->setText(tr("Hide free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, true);
    }
}

void MainWindow::slotHideLastStatus(){
    Q_D(MainWindow);
    
    bool st = WBGET(WB_LAST_STATUS);

    st = !st;

    if (!st)
        d->toolsHideLastStatus->setText(tr("Show last status message"));
    else
        d->toolsHideLastStatus->setText(tr("Hide last status message"));

    WBSET(WB_LAST_STATUS, st);

    for (int k = 0; k < d->arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(d->arenaMap[d->arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }
}

void MainWindow::slotHideUsersStatistics(){
    Q_D(MainWindow);
    
    bool st = WBGET(WB_USERS_STATISTICS);

    st = !st;

    if (!st)
        d->toolsHideUsersStatisctics->setText(tr("Show users statistics"));
    else
        d->toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

    WBSET(WB_USERS_STATISTICS, st);

    for (int k = 0; k < d->arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(d->arenaMap[d->arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }
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

    foreach (QAction *a, toolButtonStyle->actions()){
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
        connect(&customizer, SIGNAL(done(QList<QAction*>)), this, SLOT(slotToolbarCustomizerDone(QList<QAction*>)));

        customizer.exec();
    }
    else if (ret){
        d->fBar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(ret->data().toInt()));

        WISET(TOOLBUTTON_STYLE, static_cast<int>(d->fBar->toolButtonStyle()));
    }
}

void MainWindow::slotToolbarCustomizerDone(const QList<QAction*> &enabled){
    Q_D(MainWindow);
    
    d->fBar->clear();
    
    QStringList enabled_list;

    foreach (QAction *act, enabled){
        if (!act)
            continue;

        d->fBar->addAction(act);
        enabled_list.push_back(act->objectName());
    }

    WSSET(WS_MAINWINDOW_TOOLBAR_ACTS, enabled_list.join(";").toAscii().toBase64());
}

void MainWindow::slotAboutOpenUrl(){
    Q_D(MainWindow);
    
    QAction *act = qobject_cast<QAction *>(sender());
    if (act == d->aboutHomepage){
        QDesktopServices::openUrl(QUrl("http://code.google.com/p/eiskaltdc/"));
    }
    else if (act == d->aboutSource){
        QDesktopServices::openUrl(QUrl("http://github.com/negativ/eiskaltdcpp/"));
    }
    else if (act == d->aboutIssues){
        QDesktopServices::openUrl(QUrl("http://code.google.com/p/eiskaltdc/issues/list"));
    }
    else if (act == d->aboutWiki){
        QDesktopServices::openUrl(QUrl("http://code.google.com/p/eiskaltdc/w/list"));
    }
    else if (act == d->aboutChangelog){
        // Now available: ChangeLog.txt, ChangeLog_ru.txt, ChangeLog_uk.txt
        QDesktopServices::openUrl(QUrl(tr("http://github.com/negativ/eiskaltdcpp/raw/master/ChangeLog.txt")));
    }
}

void MainWindow::slotAboutClient(){
    About a(this);

    double ratio;
    double down = static_cast<double>(SETTING(TOTAL_DOWNLOAD));
    double up   = static_cast<double>(SETTING(TOTAL_UPLOAD));

    if (down > 0)
        ratio = up / down;
    else
        ratio = 0;

    a.label->setText(QString("<b>%1</b> %2 (%3)")
                     .arg(EISKALTDCPP_WND_TITLE)
                     .arg(EISKALTDCPP_VERSION)
                     .arg(EISKALTDCPP_VERSION_SFX));

    a.label_ABOUT->setTextFormat(Qt::RichText);

    QString about_text = tr("EiskaltDC++ is a graphical client for Direct Connect and ADC protocols.<br/><br/>"
                            ""
                            "DC++ core version: %1 (modified)<br/><br/>"
                            ""
                            "Home page: <a href=\"http://code.google.com/p/eiskaltdc/\">http://code.google.com/p/eiskaltdc/</a><br/><br/>"
                            ""
                            "Total up: <b>%2</b><br/>"
                            "Total down: <b>%3</b><br/>"
                            "Ratio: <b>%4</b>"
                         ).arg(DCVERSIONSTRING)
                          .arg(WulforUtil::formatBytes(up))
                          .arg(WulforUtil::formatBytes(down))
                          .arg(ratio, 0, 'f', 3);

    a.label_ABOUT->setText(about_text);

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
               "&nbsp;Alexandr Tkachev<br/>"
               "&nbsp;&lt;tka4ev@gmail.com&gt;<br/>"
               "&nbsp;(developer 2.0.3 and later)<br/>")+
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
               "&nbsp;Boris Pek  aka  Tehnick<br/>"
               "&nbsp;&lt;tehnick-8@mail.ru&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
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
               "&nbsp;(for 2.0.2 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Ukrainian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Dmytro Demenko<br/>"
               "&nbsp;&lt;dmytro.demenko@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.3 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Serbian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Miroslav Petrovic<br/>"
               "&nbsp;&lt;miroslav031@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.3 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Spanish translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Gustavo Alvarez aka sL1pKn07<br/>"
               "&nbsp;&lt;sl1pkn07@gmail.com&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Bulgarian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Rusi Dimitrov aka PsyTrip<br/>"
               "&nbsp;&lt;dimitrov.rusi@gmail.com&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Slovak translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Martin Durisin<br/>"
               "&nbsp;&lt;martin.durisin@gmail.com&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Czech translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Uhlik<br/>"
               "&nbsp;&lt;uhlikx@seznam.cz&gt;<br/>"
               "&nbsp;(for 2.2.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>German translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Chris Leick<br/>"
               "&nbsp;&lt;c.leick@vollbio.de&gt;<br/>"
               "&nbsp;(for 2.2.3 and later)<br/>")
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
        ArenaWidgetManager::getInstance()->rem(awgt);
}

void MainWindow::slotSideBarDockMenu(){
    Q_D(MainWindow);
    
    QMenu *m = new QMenu(this);
    QAction *act = new QAction(tr("Show close buttons"), m);

    act->setCheckable(true);
    act->setChecked(WBGET(SIDEBAR_SHOW_CLOSEBUTTONS, true));

    m->addAction(act);

    if (m->exec(QCursor::pos()) != NULL){
        WBSET(SIDEBAR_SHOW_CLOSEBUTTONS, act->isChecked());

        //repaint rows!
        if (d->sideDock && act->isChecked())
            d->sideDock->resize(d->sideDock->size()+QSize(18, 0));
        else if(d->sideDock)
            d->sideDock->resize(d->sideDock->size()+QSize(-18, 0));
    }

    m->deleteLater();
}

void MainWindow::slotAboutQt(){
    QMessageBox::aboutQt(this);
}

void MainWindow::nextMsg(){
    Q_D(MainWindow);
    
    HubFrame *fr = qobject_cast<HubFrame*>(HubManager::getInstance()->activeHub());

    if (fr)
        fr->nextMsg();
    else{
        QWidget *wg = d->arena->widget();

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
    Q_D(MainWindow);
    HubFrame *fr = qobject_cast<HubFrame*>(HubManager::getInstance()->activeHub());

    if (fr)
        fr->prevMsg();
    else{
        QWidget *wg = d->arena->widget();

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

void MainWindow::on(dcpp::LogManagerListener::Message, time_t t, const std::string& m) noexcept{
    emit coreLogMessage(_q(m.c_str()));
}

void MainWindow::on(dcpp::QueueManagerListener::Finished, QueueItem *item, const std::string &dir, int64_t) noexcept{
    if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)){
        UserPtr user = item->getDownloads()[0]->getUser();
        QString listName = _q(item->getListName());

        emit coreOpenShare(user, listName, _q(dir));
    }

    const int qsize = QueueManager::getInstance()->lockQueue().size();
    QueueManager::getInstance()->unlockQueue();

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

    SettingsManager *SM = SettingsManager::getInstance();
    SM->set(SettingsManager::TOTAL_UPLOAD,   SETTING(TOTAL_UPLOAD)   + upDiff);
    SM->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downDiff);

    emit coreUpdateStats(map);
}
