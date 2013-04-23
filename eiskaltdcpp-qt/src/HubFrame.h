/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QMetaType>

#include "ui_HubFrame.h"

#include "dcpp/stdinc.h"
#include "dcpp/FastAlloc.h"
#include "dcpp/ClientListener.h"
#include "dcpp/ClientManager.h"
#include "dcpp/ClientManagerListener.h"
#include "dcpp/TaskQueue.h"
#include "dcpp/User.h"
#include "dcpp/Client.h"
#include "dcpp/FavoriteManagerListener.h"

#include "ArenaWidget.h"
#include "WulforUtil.h"

class ShellCommandRunner;
class PMWindow;
class HubFramePrivate;

using namespace dcpp;

class HubFrame :
        public  QWidget,
        private Ui::UIHubFrame,
        private dcpp::ClientListener,
        private FavoriteManagerListener,
        public  ArenaWidget
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    class Menu{

    public:

        enum Action{
            /** Actions for userlist */
            CopyText=0,
            SearchText,
            CopyNick,
            FindInList,
            BrowseFilelist,
            MatchQueue,
            PrivateMessage,
            FavoriteAdd,
            FavoriteRem,
            GrantSlot,
            RemoveQueue,
            UserCommands,

            /** Additional actions for chat */
            ClearChat,
            FindInChat,
            DisableChat,
            SelectAllChat,
            ZoomInChat,
            ZoomOutChat,

            None,

            /** Additional actions for userlist */
            CopyIP,
            CopyShare,
            CopyTag,

            /** Additional actions for AntiSpam */
            AntiSpamWhite,
            AntiSpamBlack
        };

        Menu();
        virtual ~Menu();

        static void newInstance();
        static void deleteInstance();
        static Menu *getInstance();

        Action execUserMenu(Client*, const QString&);
        Action execChatMenu(Client*, const QString&, bool pmw);

        QString getLastUserCmd() const;

    private:
        QMenu *menu;
        QList<QAction*> actions;           // actions list for menu
        QList<QAction*> pm_actions;        // actions list for menu in PMWindow
        QList<QAction*> ul_actions;        // actions list for menu in user list
        QList<QAction*> chat_actions;      // chat actions list for menu
        QList<QAction*> pm_chat_actions;   // chat actions list for menu in PMWindow
        QMap<QAction*, Action> chat_actions_map; //chat menu has separators and because of it all actions are mapped
        QString last_user_cmd;
        static Menu *instance;
        static unsigned counter;
    };

public:
    class LinkParser{
    public:
       static QString parseForLinks(QString, bool);
       static void parseForMagnetAlias(QString &output);//find and replace <magnet ...></magnet> sections
    };

    typedef QMap<QString, PMWindow*> PMMap;
    typedef QMap<QString, QVariant> VarMap;
    typedef QList<ShellCommandRunner*> ShellList;

    HubFrame(QWidget *parent, QString, QString);
    ~HubFrame();

    bool parseForCmd(QString, QWidget *);

    void createPMWindow(const QString&);
    void createPMWindow(const dcpp::CID&);

    bool hasCID(const dcpp::CID &, const QString &);

    inline void reconnect() { slotReconnect(); }

    // PM functions
    void addPM(QString, QString, bool keepfocus = true);

    // Arena Widget interface
    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap();
    void requestFilter() { slotHideFindFrame(); }
    void requestFocus() { plainTextEdit_INPUT->setFocus(); }
    ArenaWidget::Role role() const { return ArenaWidget::Hub; }

    QString getCIDforNick(QString nick);

Q_SIGNALS:
    void coreConnecting(QString);
    void coreConnected(QString);
    void coreUserUpdated(const dcpp::UserPtr &user, const dcpp::Identity &id);
    void coreUserRemoved(const dcpp::UserPtr &user, const dcpp::Identity &id);
    void coreStatusMsg(QString);
    void coreFollow(QString);
    void coreFailed();
    void corePassword();
    void coreMessage(const VarMap&);
    void corePrivateMsg(const VarMap&);
    void coreHubUpdated();
    void coreFavoriteUserAdded(QString);
    void coreFavoriteUserRemoved(QString);
    void closeRequest();
    void highlighted(const VarMap&);

public Q_SLOTS:
    void disableChat();
    void clearChat();
    void addStatus(QString);
    QString getHubUrl();
    QString getHubName();
    QString getMyNick();
    void sendMsg(const QString&);
    void disablePrivateMessages(bool disable);//disconnect corePrivateMsg from this

    void reloadSomeSettings();
    void slotHideFindFrame();
    void slotActivate();
    void nextMsg();
    void prevMsg();

    void getStatistic(quint64 &users, quint64 &share) const;
    bool isConnected() const;
    bool isOP(const QString &nick);

    void browseUserFiles(const QString&, bool=false);

protected:
    virtual bool eventFilter(QObject *, QEvent *);
    virtual void closeEvent(QCloseEvent*);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    virtual void sendChat(QString, bool, bool);
    virtual void save();
    virtual void load();

private Q_SLOTS:
    void slotUsersUpdated();
    void slotReconnect();
    void slotMapOnArena();
    void slotClose();
    void slotPMClosed(QString);
    void slotUserListMenu(const QPoint&);
    void slotChatMenu(const QPoint&);
    void slotHeaderMenu(const QPoint&);
    void slotShowWnd();
    void slotShellFinished(bool, QString);
    void slotFilterTextChanged();
    void slotFindForward() { findText(0); }
    void slotFindBackward(){ findText(QTextDocument::FindBackward); }
    void slotFindTextEdited(const QString & text);
    void slotInputTextChanged();
    void slotInputContextMenu();
    void slotFindAll();
    void slotStatusLinkOpen(const QString &url);
    void slotHubMenu(QAction*);
    void slotSmile();
    void slotSmileClicked();
    void slotSmileContextMenu();
    void slotSettingsChanged(const QString &, const QString &);
    void slotBoolSettingsChanged(const QString&, int);
    void slotCopyHubURL();
    void slotCopyHubTitle();
    void slotCopyHubIP();

    void grantSlot(const QString&);
    void addUserToFav(const QString&);
    void delUserFromFav(const QString&);
    void changeFavStatus(const QString&);
    void delUserFromQueue(const QString&);
    void addAsFavorite();

    void userUpdated(const dcpp::UserPtr&, const dcpp::Identity&);
    void userRemoved(const dcpp::UserPtr&, const dcpp::Identity&);
    void follow(QString);
    void clearUsers();
    void getPassword();
    void newMsg(const VarMap&);
    void newPm(const VarMap&);

private:
    // Chat functions
    void addOutput(QString);

    // GUI setup functions
    void init();
    void initMenu();

    QString getUserInfo(UserListItem *item);

    void pmUserOffline(QString);
    void pmUserEvent(QString, QString);

    void findText(QTextDocument::FindFlags );

    void updateStyles();

    /** Extracts data from user identity */
    void getParams(VarMap &, const Identity &);

    // FavoriteManagerListener
    virtual void on(FavoriteManagerListener::UserAdded, const FavoriteUser& /*aUser*/) noexcept;
    virtual void on(FavoriteManagerListener::UserRemoved, const FavoriteUser& /*aUser*/) noexcept;

    // ClientListener interface
    virtual void on(ClientListener::Connecting, Client*) noexcept;
    virtual void on(ClientListener::Connected, Client*) noexcept;
    virtual void on(ClientListener::UserUpdated, Client*, const OnlineUser&) noexcept;
    virtual void on(ClientListener::UsersUpdated, Client*, const OnlineUserList&) noexcept;
    virtual void on(ClientListener::UserRemoved, Client*, const OnlineUser&) noexcept;
    virtual void on(ClientListener::Redirect, Client*, const string&) noexcept;
    virtual void on(ClientListener::Failed, Client*, const string&) noexcept;
    virtual void on(ClientListener::GetPassword, Client*) noexcept;
    virtual void on(ClientListener::HubUpdated, Client*) noexcept;
    virtual void on(ClientListener::Message, Client*, const ChatMessage&) noexcept;
    virtual void on(ClientListener::StatusMessage, Client*, const string&, int = ClientListener::FLAG_NORMAL) noexcept;
    virtual void on(ClientListener::NickTaken, Client*) noexcept;
    virtual void on(ClientListener::SearchFlood, Client*, const string&) noexcept;

    Q_DECLARE_PRIVATE(HubFrame);

    HubFramePrivate *d_ptr;
};

Q_DECLARE_METATYPE(HubFrame*)
