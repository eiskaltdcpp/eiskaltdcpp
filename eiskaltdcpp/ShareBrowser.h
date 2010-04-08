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

#include "ArenaWidget.h"
#include "Func.h"
#include "WulforUtil.h"
#include "ui_UIShareBrowser.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/User.h"
#include "dcpp/DirectoryListing.h"
#include "dcpp/Singleton.h"

class MainWindow;

class FileBrowserModel;
class FileBrowserItem;
class ShareBrowser;

class QModelIndex;

class ShareBrowserLoader: public QThread
{
    Q_OBJECT

    public:
        typedef Func2<ShareBrowser, dcpp::DirectoryListing::Directory*, FileBrowserItem*> LoaderFunc;

        ShareBrowserLoader(LoaderFunc *);
        virtual ~ShareBrowserLoader();

        virtual void run();
    private:
        LoaderFunc *func;
};

class ShareBrowser : public  QWidget,
                     public  ArenaWidget,
                     private Ui::UIShareBrowser
{
    Q_OBJECT

    class Menu : public dcpp::Singleton<Menu>{

    friend class dcpp::Singleton<Menu>;

    public:
        enum Action {
            Download=0,
            DownloadTo,
            Alternates,
            Magnet,
            None
        };

        Action exec();

        QString getTarget() { return target; }

    private:
        Menu();
        virtual ~Menu();

        QMap<QAction*, Action> actions;
        QMenu *menu;
        QMenu *down_to;
        QString target;
    };

public:
    ShareBrowser(dcpp::UserPtr, QString file, QString jump_to);
    virtual ~ShareBrowser();

    QString  getArenaTitle();
    QString  getArenaShortTitle();
    QWidget *getWidget();
    QMenu   *getMenu();
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiOWN_FILELIST); }

    bool isFindFrameActivated();

public slots:
    void slotFilter();

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void slotLeftPaneClicked(const QModelIndex&);
    void slotLeftPaneSelChanged(const QItemSelection&, const QItemSelection&);
    void slotCustomContextMenu(const QPoint&);
    void slotHeaderMenu();
    void slotLoaderFinish();

private:

    void init();

    void load();
    void save();

    void buildList();
    void createTree(dcpp::DirectoryListing::Directory*, FileBrowserItem*);
    void initModels();

    void download(dcpp::DirectoryListing::Directory*, const QString &);
    void download(dcpp::DirectoryListing::File*, const QString &);

    void changeRoot(dcpp::DirectoryListing::Directory*);

    ShareBrowserLoader::LoaderFunc *loader_func;

    QMenu *arena_menu;

    QSortFilterProxyModel *proxy;

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
