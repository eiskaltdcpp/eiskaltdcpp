/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QList>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QCloseEvent>
#include <QShowEvent>
#include <QTabBar>
#include <QToolBar>
#include <QHash>
#include <QSessionManager>
#include <QShortcut>
#include <QKeySequence>
#include <QToolButton>
#include <QRegExp>
#include <QTreeView>
#include <QMetaType>
#include <QTimer>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/DownloadManager.h"
#include "dcpp/LogManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/TimerManager.h"
#include "dcpp/UploadManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Download.h"
#include "dcpp/Util.h"
#include "dcpp/version.h"

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"
#include "HistoryInterface.h"
#include "LineEdit.h"
#include "WulforSettings.h"
#include "ShortcutManager.h"

#include "ui_UIAbout.h"

class FavoriteHubs;
class DownloadQueue;
class ToolBar;
class MainWindow;
class MultiLineToolBar;

extern const char * const EISKALTDCPP_VERSION;
extern const char * const EISKALTDCPP_VERSION_SFX;
extern const char * const EISKALTDCPP_WND_TITLE;

class QProgressBar;

class About:
        public QDialog,
        public Ui::UIAbout
{
Q_OBJECT

public:
    About(QWidget *parent = NULL): QDialog(parent){ setupUi(this); }

    void printHelp() const {
        QString msg =   tr("Using:\n"
                        "  eiskaltdcpp-qt <magnet link> <dchub://link> <adc(s)://link>\n"
                        "  eiskaltdcpp-qt <Key>\n"
                        "EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.\n"
                        "\n"
                        "Keys:\n"
                        "  -h, --help\t Show this message\n"
                        "  -v, --version\t Show version string"
                        );

        printf("%s\n", msg.toUtf8().constData());
    }

    void printVersion() const {
#ifndef DCPP_REVISION
        printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
#else
        printf("%s - %s %s \n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX, DCPP_REVISION);
#endif
    }
};

class HashProgress;

class MainWindow:
        public QMainWindow,
        public  dcpp::Singleton<MainWindow>,
        private dcpp::LogManagerListener,
        private dcpp::TimerManagerListener,
        private dcpp::QueueManagerListener
{
    Q_OBJECT

friend class dcpp::Singleton<MainWindow>;

    public:

        typedef QList<QAction*> ActionList;
        typedef QList<ArenaWidget*> ArenaWidgetList;
        typedef QMap<ArenaWidget*, QWidget*> ArenaWidgetMap;

        Q_PROPERTY (QObject* ToolBar READ getToolBar);
        Q_PROPERTY (QMenuBar* MenuBar READ menuBar);

        void beginExit();

        /** */
        void newHubFrame(QString, QString);

        /** */
        void browseOwnFiles();

        /** */
        void startSocket(bool onstart, int oldmode);
        /** */
        void showPortsError(const std::string& port);
        /** */
        void autoconnect();

        /** */
        void parseCmdLine();
        /** */
        void parseInstanceLine(QString);

        /** */
        void retranslateUi();

        /** */
        void reloadSomeSettings();

        /** */
        void setUnload(bool b){ isUnload = b; }

        ArenaWidget *widgetForRole(ArenaWidget::Role) const;

    public Q_SLOTS:
        /** Allow widget to be mapped on arena*/
        void addArenaWidget(ArenaWidget*);
        /** Disallow widget to be mapped on arena*/
        void remArenaWidget(ArenaWidget*);
        /** Show widget on arena */
        void mapWidgetOnArena(ArenaWidget*);
        /** Remove widget from arena*/
        void remWidgetFromArena(ArenaWidget*);

        /** */
        void addArenaWidgetOnToolbar(ArenaWidget*, bool keepFocus = false);
        /** */
        void remArenaWidgetFromToolbar(ArenaWidget*);

        QObject *getToolBar();

        /** */
        void addActionOnToolBar(QAction*);
        /** */
        void remActionFromToolBar(QAction*);

        /** */
        void toggleMainMenu(bool);

        void slotChatClear();

        /** */
        void redrawToolPanel();

        /** */
        void setStatusMessage(QString);

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void showEvent(QShowEvent *);
        virtual void hideEvent(QHideEvent *);
        virtual bool eventFilter(QObject *, QEvent *);

    private Q_SLOTS:
        void slotFileOpenLogFile();
        void slotFileOpenDownloadDirectory();
        void slotFileBrowseFilelist();
        void slotFileHasher();
        void slotFileBrowseOwnFilelist();
        void slotFileHashProgress();
        void slotFileRefreshShareHashProgress();
        void slotHubsReconnect();
        void slotHubsFavoriteHubs();
        void slotHubsPublicHubs();
        void slotHubsFavoriteUsers();
        void slotToolsDownloadQueue();
        void slotToolsHubManager();
        void slotToolsFinishedDownloads();
        void slotToolsFinishedUploads();
        void slotToolsSpy();
        void slotToolsAntiSpam();
        void slotToolsIPFilter();
        void slotToolsSwitchAway();
        void slotToolsAutoAway();
        void slotToolsSearch();
        void slotToolsADLS();
        void slotToolsCopyWindowTitle();
        void slotToolsSettings();
        void slotToolsJS();
        void slotToolsJSConsole();
        void slotToolsTransfer(bool);
        void slotPanelMenuActionClicked();
        void slotWidgetsToggle();
        void slotQC();
        void slotHideMainMenu();
        void slotShowMainMenu();
        void slotHideWindow();
        void slotHideProgressSpace();
        void slotHideLastStatus();
        void slotHideUsersStatistics();
        void slotSidebarContextMenu();
        void slotSidebarHook(const QModelIndex&);
        void slotSelectSidebarIndex(const QModelIndex&);
        void slotExit();
        void slotToolbarCustomization();
        void slotToolbarCustomizerDone(const QList<QAction*> &enabled);

        void slotCloseCurrentWidget();

        void slotUnixSignal(int);

        void nextMsg();
        void prevMsg();

        void slotFind();
        void slotDel();
        void slotChatDisable();

        void slotAboutOpenUrl();
        void slotAboutClient();
        void slotAboutQt();

        void showShareBrowser(dcpp::UserPtr, const QString &, const QString&);
        void updateStatus(const QMap<QString,QString> &);

    Q_SIGNALS:
        void coreLogMessage(const QString&);
        void coreOpenShare(dcpp::UserPtr, const QString &, const QString&);
        void coreUpdateStats(const QMap<QString, QString> &);
        void notifyMessage(int, const QString&, const QString&);

    private:
        MainWindow (QWidget *parent=NULL);
        virtual ~MainWindow();

        /** LogManagerListener */
        virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string&) throw();
        /** TimerManagerListener */
        virtual void on(dcpp::TimerManagerListener::Second, uint32_t) throw();
        /** QueueManagerListener */
        virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem*, const std::string&, int64_t) throw();

        // Interface setup functions
        void init();
        void loadSettings();
        void saveSettings();

        void initActions();
        void initMenuBar();
        void initStatusBar();
        void initSearchBar();
        void initToolbar();
        void initSideBar();
        void initHotkeys();

        void toggleSingletonWidget(ArenaWidget *a);

        void updateHashProgressStatus();

        HashProgress *progress_dialog(); // Lazy initialization for _progress_dialog;
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

        QTreeView *sideTree;

        ArenaWidgetContainer *wcontainer;

        MultiLineToolBar *mBar; // multi-line ToolBar
        ToolBar *tBar; // default ToolBar
        ToolBar *fBar; //for actions
        ToolBar *sBar; //for fast search

        LineEdit   *searchLineEdit;
        QStringList core_msg_history;
        QLabel *statusLabel;
        QLabel *statusDSPLabel;
        QLabel *statusUSPLabel;
        QLabel *statusDLabel;
        QLabel *statusULabel;
        QLabel *statusTRLabel;
        QLabel *msgLabel;
        QProgressBar *progressSpace;
        QProgressBar *progressHashing;
        HashProgress *_progress_dialog; // Hashing progress dialog

        QMenu   *menuFile;
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
#endif

        QMenu   *menuPanels;
        QAction *panelsWidgets;
        QAction *panelsTools;
        QAction *panelsSearch;

        // Standalone shortcuts
        QShortcut *ctrl_pgup;
        QShortcut *ctrl_pgdown;
        QShortcut *ctrl_up;
        QShortcut *ctrl_down;
        QShortcut *ctrl_w;
        QShortcut *ctrl_m;
        QShortcut *del;

        QAction *chatDisable;
        QAction *findInWidget;
        QAction *chatClear;

        QMenu *menuWidgets;
        QList<QAction*> menuWidgetsActions;
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

Q_DECLARE_METATYPE(MainWindow*)

class EiskaltEventFilter: public QObject{
Q_OBJECT
public:
    EiskaltEventFilter(): has_activity(true), counter(0) {
        timer.setInterval(60000);

        connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));

        timer.start();
    }

    virtual ~EiskaltEventFilter() {}

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event){
        if ((event->type() == QEvent::MouseButtonPress) ||
            (event->type() == QEvent::MouseButtonRelease) ||
            (event->type() == QEvent::MouseButtonDblClick) ||
            (event->type() == QEvent::MouseMove) ||
            (event->type() == QEvent::KeyPress) ||
            (event->type() == QEvent::KeyRelease) ||
            (event->type() == QEvent::Wheel))
        {
            has_activity = true;
            counter = 0;

            if (WBGET(WB_APP_AUTOAWAY_BY_TIMER)){
                dcpp::Util::setAway(false);
                dcpp::Util::setManualAway(false);
            }
        }
        else {
            has_activity = false;
        }

        return QObject::eventFilter(obj, event);
    }

private Q_SLOTS:
    void tick(){
        if (!has_activity)
            ++counter;

        if (WBGET(WB_APP_AUTOAWAY_BY_TIMER)){
            int mins = WIGET(WI_APP_AUTOAWAY_INTERVAL);

            if (!mins)
                return;

            int mins_done = (counter*timer.interval()/1000)/60;

            if (mins <= mins_done){
                dcpp::Util::setAway(true);
                dcpp::Util::setManualAway(true);
            }
        }
    }

private:
    QTimer timer;
    int counter;
    bool has_activity;
};

class EiskaltApp: public QApplication{
Q_OBJECT
public:
    EiskaltApp(int argc, char *argv[]): QApplication(argc, argv){
        setOrganizationName("EiskaltDC++ Team");
        setApplicationName("EiskaltDC++");
        setApplicationVersion(EISKALTDCPP_VERSION);

        installEventFilter(&ef);
    }

    void commitData(QSessionManager& manager){
        if (MainWindow::getInstance()){
            MainWindow::getInstance()->beginExit();
            MainWindow::getInstance()->close();
        }

        manager.release();
    }

    void saveState(QSessionManager &){ /** Do nothing */ }

private:
    EiskaltEventFilter ef;
};

#endif //MAINWINDOW_H_
