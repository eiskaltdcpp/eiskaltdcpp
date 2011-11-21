/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SHAREBROWSER_H
#define SHAREBROWSER_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QMap>
#include <QItemSelectionModel>
#include <QThread>
#include <QCloseEvent>
#include <QSortFilterProxyModel>

#include <boost/function.hpp>

#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "ui_UIShareBrowser.h"

#include "dcpp/stdinc.h"
#include "dcpp/ClientManager.h"
#include "dcpp/User.h"
#include "dcpp/DirectoryListing.h"
#include "dcpp/Singleton.h"

class MainWindow;

class FileBrowserModel;
class FileBrowserItem;
class ShareBrowser;

class QModelIndex;

class AsyncRunner: public QThread{
Q_OBJECT
public:
    AsyncRunner(QObject * = NULL);
    virtual ~AsyncRunner();

    virtual void run();

    void setRunFunction(const boost::function<void()> &f);

private:
    boost::function<void()> runFunc;
};

class ShareBrowser : public  QWidget,
                     public  ArenaWidget,
                     private Ui::UIShareBrowser
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    class Menu : public dcpp::Singleton<Menu>{

    friend class dcpp::Singleton<Menu>;

    public:
        enum Action {
            Download=0,
            DownloadTo,
            Alternates,
            Magnet,
            MagnetWeb,
            MagnetInfo,
            AddToFav,
            AddRestrinction,
            RemoveRestriction,
            OpenUrl,
            None
        };

        Action exec(const dcpp::UserPtr& = dcpp::UserPtr(NULL));

        QString getTarget() { return target; }

    private:
        Menu();
        virtual ~Menu();

        QMap<QAction*, Action> actions;
        QMenu *menu;
        QMenu *down_to;
        QMenu *rest_menu;
        QString target;
        QAction *open_url;
    };

public:
    ShareBrowser(dcpp::UserPtr, QString file, QString jump_to);
    virtual ~ShareBrowser();

    QString  getArenaTitle();
    QString  getArenaShortTitle();
    QWidget *getWidget();
    QMenu   *getMenu();
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiOWN_FILELIST); }
    void requestFilter() { slotFilter(); }
    ArenaWidget::Role role() const { return ArenaWidget::ShareBrowser; }

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

private Q_SLOTS:
    void init();
    void slotFilter();
    void slotRightPaneClicked(const QModelIndex&);
    void slotRightPaneSelChanged(const QItemSelection&, const QItemSelection&);
    void slotLeftPaneSelChanged(const QItemSelection&, const QItemSelection&);
    void slotCustomContextMenu(const QPoint&);
    void slotHeaderMenu();
    void slotLayoutUpdated();
    void slotSettingsChanged(const QString&, const QString&);
    void slotStartSearch();
    void slotSearchJumpTo(FileBrowserItem*);
    void slotButtonBack();
    void slotButtonForvard();
    void slotButtonUp();
    void slotClose();

private:
    void continueInit();

    void load();
    void save();

    void buildList();
    void initModels();

    void download(dcpp::DirectoryListing::Directory*, const QString &);
    void download(dcpp::DirectoryListing::File*, const QString &);

    void changeRoot(dcpp::DirectoryListing::Directory*);

    void goUp(QTreeView *);
    void goDown(QTreeView *);

    struct SelPair
    {
        dcpp::DirectoryListing::Directory *dir;
        QString path_tesxt;
        QModelIndex index;
    };

    QMenu *arena_menu;

    QSortFilterProxyModel *proxy;

    QVector<SelPair>::iterator pathHistory_iter;
    QVector <SelPair> pathHistory;

    QString nick;
    QString file;
    QString title;
    QString jump_to;
    dcpp::DirectoryListing listing;
    dcpp::UserPtr user;
    quint64 share_size;
    quint64 current_size;
    quint64 itemsCount;

    FileBrowserModel *tree_model;
    FileBrowserModel *list_model;
    FileBrowserItem  *tree_root;
    FileBrowserItem  *list_root;
};

#endif // SHAREBROWSER_H
