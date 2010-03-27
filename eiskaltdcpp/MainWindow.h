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
#include "dcpp/version.h"

#include "ArenaWidget.h"
#include "HistoryInterface.h"
#include "Func.h"

#include "ui_UIAbout.h"

using namespace dcpp;

class FavoriteHubs;
class DownloadQueue;
class ToolBar;
class MainWindow;

class QProgressBar;

class MainWindowCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1210);

    MainWindowCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~MainWindowCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class About:
        public QDialog,
        public Ui::UIAbout
{
Q_OBJECT

public:
    About(QWidget *parent): QDialog(parent){ setupUi(this); }
};

class MainWindow:
        public QMainWindow,
        public dcpp::Singleton<MainWindow>,
        private LogManagerListener,
        private TimerManagerListener,
        private QueueManagerListener
{
    Q_OBJECT

friend class dcpp::Singleton<MainWindow>;

    public:

        typedef QList<QAction*> ActionList;
        typedef QList<ArenaWidget*> ArenaWidgetList;
        typedef QMap<ArenaWidget*, QWidget*> ArenaWidgetMap;

        /** Allow widget to be mapped on arena*/
        void addArenaWidget(ArenaWidget*);
        /** Disallow widget to be mapped on arena*/
        void remArenaWidget(ArenaWidget*);
        /** Show widget on arena */
        void mapWidgetOnArena(ArenaWidget*);
        /** Remove widget from arena*/
        void remWidgetFromArena(ArenaWidget*);

        /** */
        void setStatusMessage(QString);

        /** */
        void newHubFrame(QString, QString);

        /** */
        void addArenaWidgetOnToolbar(ArenaWidget*, bool keepFocus = false);
        /** */
        void remArenaWidgetFromToolbar(ArenaWidget*);

        /** */
        void browseOwnFiles();

        /** */
        void redrawToolPanel();

        /** */
        void startSocket();

        /** */
        void autoconnect();

        /** */
        void parseCmdLine();
        /** */
        void parseInstanceLine(QString);

        /** */
        void retranslateUi();

        /** */
        void setUnload(bool b){ isUnload = b; }

    public slots:
        void slotChatClear();

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void showEvent(QShowEvent *);
        virtual void customEvent(QEvent *);
        virtual bool eventFilter(QObject *, QEvent *);

    private slots:
        void slotFileOpenLogFile();
        void slotFileBrowseFilelist();
        void slotFileBrowseOwnFilelist();
        void slotFileRefreshShare();
        void slotFileHashProgress();
        void slotHubsReconnect();
        void slotHubsFavoriteHubs();
        void slotHubsPublicHubs();
        void slotHubsFavoriteUsers();
        void slotToolsDownloadQueue();
        void slotToolsFinishedDownloads();
        void slotToolsFinishedUploads();
        void slotToolsSpy();
        void slotToolsAntiSpam();
        void slotToolsIPFilter();
        void slotToolsSearch();
        void slotToolsSettings();
        void slotToolsTransfer(bool);
        void slotWidgetsToggle();
        void slotQC();
        void slotHideWindow();
        void slotHideProgressSpace();
        void slotExit();

        void slotCloseCurrentWidget();

        void slotUnixSignal(int);

        void slotFindInChat();
        void slotChatDisable();

        void slotAboutClient();
        void slotAboutQt();

    private:
        MainWindow (QWidget *parent=NULL);
        virtual ~MainWindow();

        /** LogManagerListener */
        virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string&) throw();
        /** TimerManagerListener */
        virtual void on(dcpp::TimerManagerListener::Second, uint32_t) throw();
        /** QueueManagerListener */
        virtual void on(dcpp::QueueManagerListener::Finished, QueueItem*, const std::string&, int64_t) throw();
        //
        void showShareBrowser(dcpp::UserPtr, QString, QString);

        // Interface setup functions
        void init();
        void loadSettings();
        void saveSettings();

        void initActions();
        void initMenuBar();
        void initStatusBar();
        void initToolbar();
        void initHotkeys();

        void toggleSingletonWidget(ArenaWidget *a);

        void updateStatus(QMap<QString,QString>);
#ifdef FREE_SPACE_BAR_C
        static bool FreeDiscSpace ( std::string path, unsigned long long * res, unsigned long long * res2);
#endif
        bool isUnload;
        bool exitBegin;

        // Widgets
        QDockWidget *arena;
        QDockWidget *transfer_dock;

        ToolBar *tBar;//for tabs
        ToolBar *fBar;//for actions

        QLabel *statusLabel;
        QLabel *statusDSPLabel;
        QLabel *statusUSPLabel;
        QLabel *statusDLabel;
        QLabel *statusULabel;

        QLabel *statusTRLabel;
        QLabel *msgLabel;
        QProgressBar *progressSpace;

        QMenu   *menuFile;
        QAction *fileFileListBrowser;
        QAction *fileFileListBrowserLocal;
        QAction *fileFileListRefresh;
        QAction *fileHashProgress;
        QAction *fileOpenLogFile;
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
        QAction *toolsTransfers;
        QAction *toolsDownloadQueue;
        QAction *toolsFinishedDownloads;
        QAction *toolsFinishedUploads;
        QAction *toolsSpy;
        QAction *toolsAntiSpam;
        QAction *toolsIPFilter;
        QAction *toolsHideProgressSpace;
        QAction *toolsOptions;

        QShortcut *ctrl_pgup;
        QShortcut *ctrl_pgdown;
        QShortcut *ctrl_w;

        QAction *chatDisable;
        QAction *findInChat;
        QAction *chatClear;

        QMenu *menuWidgets;
        QList<QAction*> menuWidgetsActions;
        QHash<QAction*, ArenaWidget*> menuWidgetsHash;

        QMenu   *menuAbout;
        QAction *aboutClient;
        QAction *aboutQt;

        ActionList toolBarActions;
        ActionList fileMenuActions;
        ActionList hubsMenuActions;
        ActionList toolsMenuActions;
        ArenaWidgetList arenaWidgets;
        ArenaWidgetMap arenaMap;

        HistoryInterface<QWidget*> history;
};

class EiskaltApp: public QApplication{
Q_OBJECT
public:
    EiskaltApp(int argc, char *argv[]): QApplication(argc, argv){}

    void commitData(QSessionManager& manager){
        if (MainWindow::getInstance()){
            MainWindow::getInstance()->setUnload(true);
            MainWindow::getInstance()->close();
        }

        manager.release();
    }

    void saveState(QSessionManager &){ /** Do nothing */ }
};

#endif //MAINWINDOW_H_
