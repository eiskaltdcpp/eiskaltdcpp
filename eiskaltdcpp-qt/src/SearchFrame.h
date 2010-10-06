/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
#include <QCompleter>
#include <QMetaType>
#include <QStringListModel>

#include "ui_UISearchFrame.h"
#include "ArenaWidget.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SearchResult.h"
#include "dcpp/SearchManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ClientManagerListener.h"
#include "dcpp/Singleton.h"

using namespace dcpp;

class SearchModel;
class SearchProxyModel;
class SearchItem;

class SearchStringListModel: public QStringListModel{
public:
    SearchStringListModel(QObject *parent = NULL): QStringListModel(parent){}
    virtual ~SearchStringListModel(){}

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &) const { return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable); }

private:
   QList<QString> checked;
};

class SearchFrame : public QWidget,
                    public ArenaWidget,
                    private Ui::SearchFrame,
                    private SearchManagerListener,
                    private ClientManagerListener
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

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
        void addTempPath(const QString &path);

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
    ArenaWidget::Role role() const { return ArenaWidget::Search; }

    void requestFilter() { slotFilter(); }

    bool isFindFrameActivated();

public Q_SLOTS:
    void searchAlternates(const QString &);
    void searchFile(const QString &);
    void fastSearch(const QString &, bool);

    void slotFilter();

protected:
    virtual void closeEvent(QCloseEvent*);

Q_SIGNALS:
    /** SearchManager signals */
    void coreSR(const VarMap&);

    /** ClienManager signals */
    void coreClientConnected(const QString &info);
    void coreClientUpdated(const QString &info);
    void coreClientDisconnected(const QString &info);

private Q_SLOTS:
    void timerTick();
    void slotClear();
    void slotTimer();
    void slotResultDoubleClicked(const QModelIndex&);
    void slotContextMenu(const QPoint&);
    void slotHeaderMenu(const QPoint&);
    void slotToggleSidePanel();
    void slotStartSearch();
    void slotChangeProxyColumn(int);

    void onHubAdded(const QString &info);
    void onHubChanged(const QString &info);
    void onHubRemoved(const QString &info);

    void addResult(const VarMap &map);

private:
    void init();
    void initSecond();

    void load();
    void save();

    void getParams(VarMap&, const dcpp::SearchResultPtr&);
    bool getDownloadParams(VarMap&, SearchItem*);
    bool getWholeDirParams(VarMap&, SearchItem*);

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

    QStringList hubs;
    QStringList searchHistory;

    QList<dcpp::Client*> client_list;

    QTimer *timer;
    QTimer *timer1;

    QCompleter *completer;

    QMenu *arena_menu;

    bool saveFileType;

    SearchModel *model;
    SearchStringListModel *str_model;
    SearchProxyModel *proxy;

    bool isHash;
    int left_pane_old_size;

    // SearchManagerListener
    virtual void on(SearchManagerListener::SR, const SearchResultPtr& aResult) throw();

    // ClientManagerListener
    virtual void on(ClientConnected, Client* c) throw();
    virtual void on(ClientUpdated, Client* c) throw();
    virtual void on(ClientDisconnected, Client* c) throw();
};

Q_DECLARE_METATYPE(SearchFrame*)

#endif // SEARCHFRAME_H
