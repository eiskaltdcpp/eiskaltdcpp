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
#include <QList>
#include <QMenuBar>
#include <QMenu>
#include <QCloseEvent>
#include <QShowEvent>
#include <QTabBar>
#include <QToolBar>
#include <QHash>
#include <QSessionManager>


#include "dcpp/stdinc.h"
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
#include "HistoryInterface.h"
#include "LineEdit.h"
#include "ShortcutManager.h"

#include "ui_UIAbout.h"

class FavoriteHubs;
class DownloadQueue;
class ToolBar;
class MainWindow;
class MultiLineToolBar;
#ifdef USE_JS
class ScriptConsole;
#endif

extern const char * const EISKALTDCPP_VERSION;
extern const char * const EISKALTDCPP_WND_TITLE;
extern const char * const EISKALTDCPP_VERSION_SFX;

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
        printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
    }
};

class HashProgress;
class MainWindowPrivate;

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
        void startSocket(bool changed);
        /** */
        void showPortsError(const std::string& port);
        /** */
        void autoconnect();
        /** */
        void retranslateUi();

        /** */
        void reloadSomeSettings();

        /** */
        void setUnload(bool b);

        ArenaWidget *widgetForRole(ArenaWidget::Role) const;
        
    Q_SIGNALS:
        void redrawWidgetPanels();
        
    public Q_SLOTS:
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

        /** */
        void show();

        /** */
        void parseCmdLine();
        /** */
        void parseInstanceLine(QString);

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void showEvent(QShowEvent *);
        virtual void hideEvent(QHideEvent *);
        virtual bool eventFilter(QObject *, QEvent *);

    private Q_SLOTS:
        /** Show widget on arena */
        void mapWidgetOnArena(ArenaWidget*);
        void removeWidget(ArenaWidget *awgt);
        void insertWidget(ArenaWidget *awgt);
        void updated(ArenaWidget *awgt);
    
        void slotOpenMagnet();
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
        void slotToolsQueuedUsers();
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
        void slotJSFileChanged(const QString&);
        void slotToolsTransfer(bool);
        void slotToolsSwitchSpeedLimit();
        void slotPanelMenuActionClicked();
        void slotWidgetsToggle();
        void slotQC();
        void slotHideMainMenu();
        void slotShowMainMenu();
        void slotHideWindow();
        void slotHideProgressSpace();
        void slotHideLastStatus();
        void slotHideUsersStatistics();
        void slotSideBarDockMenu();
        void slotExit();
        void slotToolbarCustomization();
        void slotToolbarCustomizerDone(const QList<QAction*> &enabled);

        void slotCloseCurrentWidget();

        void slotUnixSignal(int);

        void nextMsg();
        void prevMsg();

        void slotFind();
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
        virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string&) noexcept;
        /** TimerManagerListener */
        virtual void on(dcpp::TimerManagerListener::Second, uint64_t) noexcept;
        /** QueueManagerListener */
        virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem*, const std::string&, int64_t) noexcept;

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

        void toggleSingletonWidget(ArenaWidget *a);

        void updateHashProgressStatus();
        
        Q_DECLARE_PRIVATE(MainWindow)
        
        HashProgress *progress_dialog(); // Lazy initialization for _progress_dialog;

        MainWindowPrivate *d_ptr;
};

Q_DECLARE_METATYPE(MainWindow*)

#endif //MAINWINDOW_H_
