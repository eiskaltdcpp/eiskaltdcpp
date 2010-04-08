#ifndef SEARCHFRAME_H
#define SEARCHFRAME_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QMap>
#include <QList>
#include <QMenu>
#include <QTimer>
#include <QCloseEvent>
#include <QTimer>
#include <QCustomEvent>

#include "ui_UISearchFrame.h"
#include "ArenaWidget.h"
#include "Func.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SearchResult.h"
#include "dcpp/SearchManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ClientManagerListener.h"
#include "dcpp/Singleton.h"

using namespace dcpp;

class SearchModel;
class SearchItem;

class SearchCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1203);

    SearchCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~SearchCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class SearchFrame : public QWidget,
                    public ArenaWidget,
                    private Ui::SearchFrame,
                    private SearchManagerListener,
                    private ClientManagerListener
{
    Q_OBJECT


    typedef QMap<QString, QVariant> VarMap;

    class Menu : public Singleton<Menu>{
    friend class Singleton<Menu>;

    public:
        enum Action{
            Download=0,
            DownloadTo,
            DownloadWholeDir,
            DownloadWholeDirTo,
            SearchTTH,
            Magnet,
            Browse,
            MatchQueue,
            SendPM,
            AddToFav,
            GrantExtraSlot,
            RemoveFromQueue,
            Remove,
            UserCommands,
            None
        };

        Action exec(QStringList);
        QMenu *buildUserCmdMenu(QList<QString> hubs);
        QMap<QString, QString> ucParams;
        QString getDownloadToPath() {return downToPath; }

    private:
        Menu();
        virtual ~Menu();

        QMap<QAction*, Action> actions;
        QList<QAction*> action_list;

        QString downToPath;

        QMenu *menu;
        QMenu *down_to;
        QMenu *down_wh_to;
    };

    class HubInfo{

    public:
        HubInfo(Client* client, QListWidget *list):
                client(client),
                url(QString::fromStdString(client->getHubUrl())),
                name(QString::fromStdString(client->getHubName())),
                op(client->getMyIdentity().isOp()),
                item(NULL)
        {
            item = new QListWidgetItem(url + ": " + name, list);
            item->setCheckState(Qt::Checked);
        }

        ~HubInfo(){
            delete item;
        }

        const QString& getText() const {
            return name;
        }

        QString url;
        QString name;
        QListWidgetItem *item;
        Client *client;
        bool op;
    };

public:
    enum AlreadySharedAction{
        None=0,
        Filter,
        Highlight
    };

    SearchFrame(QWidget* = NULL);
    virtual ~SearchFrame();

    QWidget *getWidget();
    QString  getArenaTitle();
    QString  getArenaShortTitle();
    QMenu   *getMenu();
    const QPixmap &getPixmap();

    void searchAlternates(const QString &);
    void searchFile(const QString &);
    void fastSearch(const QString &, bool);

protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void customEvent(QEvent *);

private slots:
    void timerTick();
    void slotClear();
    void slotTimer();
    void slotResultDoubleClicked(const QModelIndex&);
    void slotContextMenu(const QPoint&);
    void slotHeaderMenu(const QPoint&);
    void slotToggleSidePanel();
    void slotStartSearch();

private:
    void init();
    void initSecond();

    void load();
    void save();

    void getParams(VarMap&, const dcpp::SearchResultPtr&);
    bool getDownloadParams(VarMap&, SearchItem*);
    bool getWholeDirParams(VarMap&, SearchItem*);
    void addResult(VarMap map);

    void download(const VarMap&);
    void getFileList(const VarMap&, bool = false);
    void addToFav(const QString&);
    void grant(const VarMap&);
    void removeSource(const VarMap&);

    QString arena_title;
    QString token;

    TStringList currentSearch;

    qulonglong dropped;
    qulonglong results;
    AlreadySharedAction filterShared;
    bool withFreeSlots;

    QMap<QListWidgetItem*, HubInfo*> hub_items;
    QMap<Client*, HubInfo*> hub_list;

    QList<QString> searchHistory;

    QTimer *timer;
    QTimer *timer1;

    QMenu *arena_menu;

    bool saveFileType;

    SearchModel *model;

    bool isHash;
    int left_pane_old_size;

    // SearchManagerListener
    virtual void on(SearchManagerListener::SR, const SearchResultPtr& aResult) throw();

    // ClientManagerListener
    virtual void on(ClientConnected, Client* c) throw();
    virtual void on(ClientUpdated, Client* c) throw();
    virtual void on(ClientDisconnected, Client* c) throw();

    void onHubAdded(HubInfo* info);
    void onHubChanged(HubInfo* info);
    void onHubRemoved(HubInfo* info);
};

#endif // SEARCHFRAME_H
