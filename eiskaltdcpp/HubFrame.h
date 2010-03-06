#ifndef HUBFRAME_H
#define HUBFRAME_H

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

#include "ui_HubFrame.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientListener.h"
#include "dcpp/ClientManager.h"
#include "dcpp/ClientManagerListener.h"
#include "dcpp/TaskQueue.h"
#include "dcpp/User.h"
#include "dcpp/Client.h"
#include "dcpp/FavoriteManagerListener.h"

#include "UserListModel.h"
#include "ArenaWidget.h"
#include "Func.h"

class ShellCommandRunner;
class PMWindow;

using namespace dcpp;

class UserUpdatedEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1200);

    UserUpdatedEvent(const dcpp::UserPtr &ptr, bool join):
            QEvent(Event),
            ptr(ptr),
            join(join)
    {}
    virtual ~UserUpdatedEvent()
    {}

    const dcpp::UserPtr &getUser() const { return ptr; }
    bool getJoin() const {return join; }
    QHash<QString, QVariant> &getMap() { return map; }

private:
    dcpp::UserPtr ptr;
    bool join;
    QHash<QString, QVariant> map;
};

class UserRemovedEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1201);

    UserRemovedEvent(const dcpp::UserPtr &user, qlonglong share):QEvent(Event), user(user), share(share)
    {}
    virtual ~UserRemovedEvent()
    {}

    const dcpp::UserPtr &getUser() const { return user; }
    qulonglong getShare() const { return share; }

private:
    qulonglong share;
    dcpp::UserPtr user;
};

class UserCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1202);

    UserCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~UserCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class HubFrame :
        public QWidget,
        private Ui::UIHubFrame,
        private dcpp::ClientListener,
        public ArenaWidget
{
    Q_OBJECT

    class Menu{

    public:

        enum Action{
            /** Actions for userlist */
            CopyText=0,
            CopyNick=1,
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

            None
        };

        Menu();
        virtual ~Menu();

        static void newInstance();
        static void deleteInstance();
        static Menu *getInstance();
        static unsigned counter;

        Action execUserMenu(Client*, const QString&);
        Action execChatMenu(Client*, const QString&);
        QMenu *buildUserCmdMenu(const QString&, const QString&);

        QString getLastUserCmd() const;

    private:
        QMenu *menu;
        QList<QAction*> actions;
        QList<QAction*> chat_actions;
        QMap<QAction*, Action> chat_actions_map;//chat menu has separators and because of it all actions are mapped
        QString last_user_cmd;
        static Menu *instance;
    };

    class LinkParser{
    public:
       static QString parseForLinks(QString);

    private:
       static QStringList link_types;
    };

public:
    typedef QMap<QString, PMWindow*> PMMap;
    typedef QHash<QString, QVariant > VarMap;
    typedef QList<ShellCommandRunner*> ShellList;

    HubFrame(QWidget *parent, QString, QString);
    ~HubFrame();

    void addStatus(QString);
    void createPMWindow(const QString&);
    void createPMWindow(const dcpp::CID&);
    bool hasCID(const dcpp::CID &, const QString &);
    inline void reconnect() { slotReconnect(); }

    // Arena Widget interface
    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap();

    QString getUserInfo(UserListItem *item);

protected:
    virtual bool eventFilter(QObject *, QEvent *);
    virtual void closeEvent(QCloseEvent*);
    virtual void customEvent(QEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    virtual void sendChat(QString, bool, bool);
    virtual void save();
    virtual void load();

private slots:
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
    void slotFilterTextChanged(const QString & text);
    void slotFindForward() { findText(0); }
    void slotFindBackward(){ findText(QTextDocument::FindBackward); }
    void slotHideFindFrame();
    void slotFindTextEdited(const QString & text);

private:
    // Chat functions
    void addOutput(QString);
    bool parseForCmd(QString);

    // GUI setup functions
    void init();
    void initMenu();

    void browseUserFiles(const QString&, bool=false);
    void grantSlot(const QString&);
    void addUserToFav(const QString&);
    void delUserFromFav(const QString&);
    void delUserFromQueue(const QString&);
    void addAsFavorite();

    void newMsg(VarMap);
    void newPm(VarMap);

    void clearUsers();

    void pmUserOffline(QString);
    void pmUserEvent(QString, QString);

    void getPassword();

    void follow(string);

    void findText(QTextDocument::FindFlags );
    void nickCompletion();

    /** Extracts data from user identity */
    void getParams(UserMap &, const Identity &);
    void on_userUpdated(const VarMap&, const UserPtr&, bool);

    // PM functions
    void addPM(QString, QString);

    // ClientListener interface
    virtual void on(ClientListener::Connecting, Client*) throw();
    virtual void on(ClientListener::Connected, Client*) throw();
    virtual void on(ClientListener::UserUpdated, Client*, const OnlineUser&) throw();
    virtual void on(ClientListener::UsersUpdated, Client*, const OnlineUserList&) throw();
    virtual void on(ClientListener::UserRemoved, Client*, const OnlineUser&) throw();
    virtual void on(ClientListener::Redirect, Client*, const string&) throw();
    virtual void on(ClientListener::Failed, Client*, const string&) throw();
    virtual void on(ClientListener::GetPassword, Client*) throw();
    virtual void on(ClientListener::HubUpdated, Client*) throw();
    virtual void on(ClientListener::Message, Client*, const OnlineUser&, const string&, bool = false) throw();
    virtual void on(ClientListener::StatusMessage, Client*, const string&, int = ClientListener::FLAG_NORMAL) throw();
    virtual void on(ClientListener::PrivateMessage, Client*, const OnlineUser&, const OnlineUser&, const OnlineUser&, const string&, bool = false) throw();
    virtual void on(ClientListener::NickTaken, Client*) throw();
    virtual void on(ClientListener::SearchFlood, Client*, const string&) throw();

    QTimer *updater;

    QMenu *arenaMenu;

    Client *client;

    // Work data
    QTextCodec *codec;

    quint64 total_shared;
    QString hub_title;

    QString last_hyperlink;

    bool chatDisabled;
    bool hasMessages;

    PMMap pm;
    ShellList shell_list;

    // Userlist data and some helpful functions
    UserListModel *model;
    QSortFilterProxyModel *proxy;
};

#endif // HUBFRAME_H
