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
#include <QTabBar>
#include <QToolBar>
#include <QHash>

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

#include "ArenaWidget.h"
#include "HistoryInterface.h"
#include "Func.h"

#include "ui_UIAbout.h"

using namespace dcpp;

class FavoriteHubs;
class DownloadQueue;
class ToolBar;

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
        private Ui::UIAbout
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
        void addArenaWidgetOnToolbar(ArenaWidget*);
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

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void customEvent(QEvent *);

    private slots:
        void slotFileBrowseFilelist();
        void slotFileBrowseOwnFilelist();
        void slotFileReconnect();
        void slotFileRefreshShare();
        void slotFileHashProgress();
        void slotFileFavoriteHubs();
        void slotFileFavoriteUsers();
        void slotFileDownloadQueue();
        void slotFileFinishedDownloads();
        void slotFileFinishedUploads();
        void slotFileAntiSpam();
        void slotFileIPFilter();
        void slotFileSearch();
        void slotFileSettings();
        void slotFileTransfer(bool);
        void slotWidgetsToggle();
        void slotQC();
        void slotHideWindow();
        void slotExit();

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

        void toggleSingletonWidget(ArenaWidget *a);

        void updateStatus(QMap<QString,QString>);

        bool isUnload;

        // Widgets
        QDockWidget *arena;
        QDockWidget *transfer_dock;

        ToolBar *tBar;//for tabs
        ToolBar *fBar;//for actions

        QLabel *statusLabel;
        QLabel *msgLabel;

        QMenu   *menuFile;
        QAction *fileOptions;
        QAction *fileQuickConnect;
        QAction *fileHubReconnect;
        QAction *fileFileListBrowser;
        QAction *fileFileListBrowserLocal;
        QAction *fileFileListRefresh;
        QAction *fileHashProgress;
        QAction *fileFavoriteHubs;
        QAction *fileFavoriteUsers;
        QAction *fileTransfers;
        QAction *fileDownloadQueue;
        QAction *fileFinishedDownloads;
        QAction *fileFinishedUploads;
        QAction *fileAntiSpam;
        QAction *fileIPFilter;
        QAction *fileSearch;
        QAction *fileHideWindow;
        QAction *fileQuit;

        QMenu *menuWidgets;
        QList<QAction*> menuWidgetsActions;
        QHash<QAction*, ArenaWidget*> menuWidgetsHash;

        QMenu   *menuAbout;
        QAction *aboutClient;
        QAction *aboutQt;

        ActionList fileMenuActions;
        ArenaWidgetList arenaWidgets;
        ArenaWidgetMap arenaMap;

        HistoryInterface<QWidget*> history;
};

#endif //MAINWINDOW_H_
