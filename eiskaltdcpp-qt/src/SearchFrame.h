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
#include <QModelIndex>
#include <QMap>
#include <QList>
#include <QMenu>
#include <QCloseEvent>
#include <QMetaType>
#include <QStringListModel>

#include <memory>

#include "ui_UISearchFrame.h"
#include "ArenaWidget.h"

#include "dcpp/stdinc.h"
#include "dcpp/SearchResult.h"
#include "dcpp/SearchManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ClientManagerListener.h"
#include "dcpp/Singleton.h"

using namespace dcpp;

class SearchItem;
class SearchFramePrivate;

class SearchStringListModel: public QStringListModel{
public:
    SearchStringListModel(QObject *parent = nullptr): QStringListModel(parent){}
    ~SearchStringListModel() override {}

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &) const override { return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable); }

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

    typedef QVariantMap VarMap;

    class Menu : public dcpp::Singleton<Menu> {
        friend class dcpp::Singleton<Menu>;
    public:
        enum Action {
            Download=0,
            DownloadTo,
            DownloadWholeDir,
            DownloadWholeDirTo,
            SearchTTH,
            Magnet,
            MagnetWeb,
            MagnetInfo,
            Browse,
            MatchQueue,
            SendPM,
            AddToFav,
            GrantExtraSlot,
            RemoveFromQueue,
            Remove,
            UserCommands,
            Blacklist,
            AddToBlacklist,
            None
        };

        Action exec(const QStringList &);
        QMenu *buildUserCmdMenu(QList<QString> hubs);
        QString getDownloadToPath() { return downToPath; }
        int getCommandId() { return uc_cmd_id; }
        void addTempPath(const QString &path);

    private:
        Menu();
        virtual ~Menu();

        QMap<QAction*, Action> actions;
        QList<QAction*> action_list;

        QString downToPath;

        int uc_cmd_id;

        QMenu *menu;
        QMenu *magnet_menu;
        QMenu *down_to;
        QMenu *down_wh_to;
        QMenu *black_list_menu;
    };

public:
    enum AlreadySharedAction{
        None=0,
        Filter,
        Highlight
    };

    SearchFrame(QWidget* = nullptr);
    ~SearchFrame() override;

    QWidget *getWidget() override;
    QString  getArenaTitle() override;
    QString  getArenaShortTitle() override;
    QMenu   *getMenu() override;
    const QPixmap &getPixmap() override;
    ArenaWidget::Role role() const override { return ArenaWidget::Search; }

    void requestFilter() override { slotFilter(); }
    void requestFocus() override { lineEdit_SEARCHSTR->setFocus(); }

public Q_SLOTS:
    void searchAlternates(const QString &);
    void searchFile(const QString &);
    void fastSearch(const QString &, bool);

protected:
    void closeEvent(QCloseEvent*) override;
    bool eventFilter(QObject *, QEvent *) override;

Q_SIGNALS:
    /** SearchManager signals */
    void coreSR(const VarMap&);

    /** ClienManager signals */
    void coreClientConnected(const QString &info);
    void coreClientUpdated(const QString &info);
    void coreClientDisconnected(const QString &info);

private Q_SLOTS:
    void slotFilter();
    void timerTick();
    void slotClear();
    void slotTimer();
    void slotResultDoubleClicked(const QModelIndex&);
    void slotContextMenu(const QPoint&);
    void slotHeaderMenu(const QPoint&);
    void slotToggleSidePanel();
    void slotStartSearch();
    void slotStopSearch();
    void slotChangeProxyColumn(int);
    void slotClose();

    void slotSettingsChanged(const QString &key, const QString &value);

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

    // SearchManagerListener
    void on(SearchManagerListener::SR, const SearchResultPtr& aResult) noexcept override;

    // ClientManagerListener
    void on(ClientConnected, Client* c) noexcept override;
    void on(ClientUpdated, Client* c) noexcept override;
    void on(ClientDisconnected, Client* c) noexcept override;

    Q_DECLARE_PRIVATE (SearchFrame)
    SearchFramePrivate* d_ptr;
};

Q_DECLARE_METATYPE(SearchFrame*)
