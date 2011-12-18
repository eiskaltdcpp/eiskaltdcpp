/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ShareBrowser.h"
#include "WulforUtil.h"
#include "FileBrowserModel.h"
#include "MainWindow.h"
#include "SearchFrame.h"
#include "Magnet.h"
#include "ShareBrowserSearch.h"
#include "ArenaWidgetManager.h"
#include "ArenaWidgetFactory.h"
#include "DownloadToHistory.h"

#include "dcpp/SettingsManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/ClientManager.h"
#include <dcpp/ADLSearch.h>

#if (HAVE_MALLOC_TRIM)
#include <malloc.h>
#endif

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QTreeView>
#include <QModelIndex>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QtConcurrentFilter>
#include <QtConcurrentRun>
#include <QInputDialog>
#include <QDesktopServices>

#include <boost/bind.hpp>

using namespace dcpp;

AsyncRunner::AsyncRunner(QObject *parent): QThread(parent){

}

AsyncRunner::~AsyncRunner(){

}

void AsyncRunner::run(){
    runFunc();
}

void AsyncRunner::setRunFunction(const boost::function<void()> &f){
    runFunc = f;
}

ShareBrowser::Menu::Menu(){
    menu = new QMenu();

    WulforUtil *WU = WulforUtil::getInstance();

    rest_menu = new QMenu(tr("Restrictions"));
    QMenu *magnet_menu = new QMenu(tr("Magnet"), MainWindow::getInstance());

    QAction *down    = new QAction(tr("Download"), menu);
    down->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
    down_to = new QMenu(tr("Download to..."));
    down_to->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD_AS));
    QAction *sep     = new QAction(menu);
    QAction *alter   = new QAction(tr("Search for alternates"), menu);
    alter->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
    QAction *magnet  = new QAction(tr("Copy magnet"), menu);
    magnet->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));
    QAction *magnet_web  = new QAction(tr("Copy web-magnet"), menu);
    magnet_web->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));
    QAction *magnet_info  = new QAction(tr("Properties of magnet"), menu);
    magnet_info->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
    QAction *sep1    = new QAction(menu);
    QAction *add_to_fav = new QAction(tr("Add to favorites"), menu);
    add_to_fav->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));
    QAction *set_rest = new QAction(tr("Add restriction"), rest_menu);
    QAction *rem_rest = new QAction(tr("Remove restriction"), rest_menu);
    open_url = new QAction(tr("Open directory"), menu);
    QAction *sep2    = new QAction(menu);
    QAction *sep3    = new QAction(menu);

    actions.insert(down, Download);
    actions.insert(alter, Alternates);
    actions.insert(magnet, Magnet);
    actions.insert(magnet_web, MagnetWeb);
    actions.insert(magnet_info, MagnetInfo);
    actions.insert(add_to_fav, AddToFav);
    actions.insert(set_rest, AddRestrinction);
    actions.insert(rem_rest, RemoveRestriction);
    actions.insert(open_url, OpenUrl);

    magnet_menu->addActions(QList<QAction*>()
                    << magnet << magnet_web << sep3 << magnet_info);

    sep->setSeparator(true);
    sep1->setSeparator(true);
    sep2->setSeparator(true);
    sep3->setSeparator(true);

    menu->addActions(QList<QAction*>() << down << sep << alter);
    menu->addMenu(magnet_menu);
    menu->addActions(QList<QAction*>() << sep1 << add_to_fav << sep2);
    rest_menu->addActions(QList<QAction*>() << set_rest << rem_rest);
    menu->insertMenu(sep, down_to);
    menu->addMenu(rest_menu);
    menu->addAction(open_url);
}

ShareBrowser::Menu::~Menu(){
    delete menu;
    delete rest_menu;
    delete down_to;
}

ShareBrowser::Menu::Action ShareBrowser::Menu::exec(const dcpp::UserPtr &user){
    qDeleteAll(down_to->actions());
    down_to->clear();

    const QPixmap &dir_px = WICON(WulforUtil::eiFOLDER_BLUE);
    QString aliases, paths;

    aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toAscii());
    paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toAscii());

    QStringList a = aliases.split("\n", QString::SkipEmptyParts);
    QStringList p = paths.split("\n", QString::SkipEmptyParts);

    QStringList temp_pathes = DownloadToDirHistory::get();

    if (!temp_pathes.isEmpty()){
        foreach (const QString &t, temp_pathes){
            QAction *act = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), QDir(t).dirName(), down_to);
            act->setToolTip(t);
            act->setData(t);

            down_to->addAction(act);
        }

        down_to->addSeparator();
    }

    if (a.size() == p.size() && !a.isEmpty()){
        for (int i = 0; i < a.size(); i++){
            QAction *act = new QAction(a.at(i), down_to);
            act->setData(p.at(i));
            act->setIcon(dir_px);

            down_to->addAction(act);
        }

        down_to->addSeparator();
    }

    QAction *browse = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), tr("Browse"), down_to);
    browse->setIcon(dir_px);
    browse->setData("");

    down_to->addAction(browse);

    rest_menu->setEnabled(user == ClientManager::getInstance()->getMe());
    open_url->setEnabled(user == ClientManager::getInstance()->getMe());

    QAction *ret = menu->exec(QCursor::pos());

    if (actions.contains(ret))
        return actions.value(ret);
    else if (down_to->actions().contains(ret)){
        target = ret->data().toString();

        return DownloadTo;
    }

    return None;
}

ShareBrowser::ShareBrowser(UserPtr user, QString file, QString jump_to):
        QWidget(MainWindow::getInstance()),
        user(user),
        file(file),
        jump_to(jump_to),
        share_size(0),
        current_size(0),
        itemsCount(0),
        listing(HintedUser(user, "")),
        tree_root(NULL),
        list_root(NULL),
        tree_model(NULL),
        list_model(NULL),
        proxy(NULL)
{
    

    nick = WulforUtil::getInstance()->getNicks(user->getCID());;

    if (nick.indexOf(_q(user->getCID().toBase32()) >= 0)){//User offline
        nick = _q(ClientManager::getInstance()->getNicks(HintedUser(user, ""))[0]);

        QFileInfo info(file);

        nick = info.baseName().left(info.baseName().indexOf("."));

        if (nick == "files")
            title = tr("Own files");
        else
            title = tr("Listing: ") + nick;
    }
    
    AsyncRunner *runner = new AsyncRunner(this);
    boost::function<void()> f = boost::bind(&ShareBrowser::buildList, this);

    runner->setRunFunction(f);
    connect(runner, SIGNAL(finished()), this, SLOT(init()), Qt::QueuedConnection);
    connect(runner, SIGNAL(finished()), runner, SLOT(deleteLater()), Qt::QueuedConnection);

    runner->start();
    
    setState(state() | ArenaWidget::Hidden);
 }

ShareBrowser::~ShareBrowser(){
    delete tree_model;
    delete list_model;
    delete arena_menu;

    delete proxy;

    pathHistory.clear();

    Menu::deleteInstance();

#if (HAVE_MALLOC_TRIM)
    malloc_trim(0);
#endif
}

void ShareBrowser::closeEvent(QCloseEvent *e){
    save();

    QWidget::closeEvent(e);
}

bool ShareBrowser::eventFilter(QObject *obj, QEvent *e){
    QTreeView *tree_view = qobject_cast<QTreeView*>(obj);

    if (tree_view && (e->type() == QEvent::KeyRelease)){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return)
            goDown(tree_view);
        else if (k_e->key() == Qt::Key_Backspace)
            goUp(tree_view);
    }
    else if (static_cast<LineEdit*>(obj) == lineEdit_FILTER && (e->type() == QEvent::KeyRelease)){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (k_e->key() == Qt::Key_Escape){
            lineEdit_FILTER->clear();

            requestFilter();

            return true;
        }
    }

    return QWidget::eventFilter(obj, e);
}

void ShareBrowser::init(){
    setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    
    toolButton_UP->setIcon(WICON(WulforUtil::eiTOP));
    toolButton_FORVARD->setIcon(WICON(WulforUtil::eiNEXT));
    toolButton_BACK->setIcon(WICON(WulforUtil::eiPREVIOUS));
    
    frame_FILTER->setVisible(false);

    initModels();

    lineEdit_FILTER->installEventFilter(this);

    treeView_LPANE->setModel(tree_model);
    treeView_LPANE->header()->hideSection(COLUMN_FILEBROWSER_ESIZE);
    treeView_LPANE->header()->hideSection(COLUMN_FILEBROWSER_TTH);
    treeView_LPANE->setContextMenuPolicy(Qt::CustomContextMenu);

    treeView_RPANE->setModel(list_model);
    treeView_RPANE->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RPANE->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RPANE->installEventFilter(this);

    toolButton_CLOSEFILTER->setIcon(WICON(WulforUtil::eiEDITDELETE));
    toolButton_SEARCH->setIcon(WICON(WulforUtil::eiFIND));

    arena_menu = new QMenu(tr("Filebrowser"));

    QAction *close_wnd = new QAction(WICON(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    //connect(treeView_LPANE, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotLeftPaneClicked(QModelIndex)));
    connect(treeView_LPANE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenu(QPoint)));
    connect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(slotClose()));
    connect(toolButton_CLOSEFILTER, SIGNAL(clicked()), this, SLOT(slotFilter()));

    connect(treeView_RPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotRightPaneSelChanged(QItemSelection,QItemSelection)));
    connect(treeView_RPANE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenu(QPoint)));
    connect(treeView_RPANE->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(treeView_RPANE, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotRightPaneClicked(QModelIndex)));
    connect(tree_model, SIGNAL(layoutChanged()), this, SLOT(slotLayoutUpdated()));
    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));
    connect(toolButton_SEARCH, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
    connect(toolButton_BACK, SIGNAL(clicked()), this, SLOT(slotButtonBack()));
    connect(toolButton_FORVARD, SIGNAL(clicked()), this, SLOT(slotButtonForvard()));
    connect(toolButton_UP, SIGNAL(clicked()), this, SLOT(slotButtonUp()));

    continueInit();
}

void ShareBrowser::continueInit(){
    tree_model->repaint();
    list_model->repaint();

    load();

    if (user == ClientManager::getInstance()->getMe()){
        tree_model->loadRestrictions();
        list_model->setOwnList(true);
    }

    if (!jump_to.isEmpty()){
        FileBrowserItem *root = tree_model->getRootElem();

        root = root->childItems.at(0);

        FileBrowserItem *jump = tree_model->createRootForPath(jump_to, root);

        if (jump){
            QModelIndex jump_index = tree_model->createIndexForItem(jump);

            treeView_LPANE->selectionModel()->select(jump_index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
            treeView_LPANE->scrollTo(jump_index, QAbstractItemView::PositionAtCenter);
        }
    }

    pathHistory.clear();
    
    ArenaWidgetManager::getInstance()->activate(this);
}


void ShareBrowser::load(){
    int w = WIGET(WI_SHARE_WIDTH);
    int wr= WIGET(WI_SHARE_RPANE_WIDTH);

    if (w >= 0 && wr >= 0){
        QList<int> frames;

        frames << (w - wr) << wr;

        splitter->setSizes(frames);
    }

    treeView_LPANE->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SHARE_LPANE_STATE).toAscii()));
    treeView_RPANE->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SHARE_RPANE_STATE).toAscii()));

    treeView_LPANE->setSortingEnabled(true);
    treeView_RPANE->setSortingEnabled(true);

    itemsCount = listing.getRoot()->getTotalFileCount(true);
    share_size = listing.getRoot()->getTotalSize(true);

    label_LEFT->setText(QString(tr("Total share size: %1;  Files: %2")).arg(WulforUtil::formatBytes(share_size)).arg(itemsCount));
}

void ShareBrowser::save(){
    WSSET(WS_SHARE_LPANE_STATE, treeView_LPANE->header()->saveState().toBase64());
    WSSET(WS_SHARE_RPANE_STATE, treeView_RPANE->header()->saveState().toBase64());

    WISET(WI_SHARE_RPANE_WIDTH, treeView_RPANE->width());
    WISET(WI_SHARE_WIDTH, treeView_RPANE->width() + treeView_LPANE->width());
}

QString ShareBrowser::getArenaTitle(){
    return title;
}

QString ShareBrowser::getArenaShortTitle(){
    return getArenaTitle();
}

QWidget *ShareBrowser::getWidget(){
    return this;
}

QMenu  *ShareBrowser::getMenu(){
    return arena_menu;
}

void ShareBrowser::buildList(){
    try {
        listing.loadFile(file.toStdString());
        listing.getRoot()->setName(nick.toStdString());
        ADLSearchManager::getInstance()->matchListing(listing);
    }
    catch (const Exception &e){
        LogManager::getInstance()->message(e.what());
    }
}

void ShareBrowser::initModels(){
    tree_model = new FileBrowserModel();
    tree_model->setListing(&listing);
    tree_model->fetchMore(QModelIndex());
    tree_root  = tree_model->getRootElem();

    list_model = new FileBrowserModel();
    list_root = list_model->getRootElem();
}

void ShareBrowser::goDown(QTreeView *view){
    if (view != treeView_RPANE)
        return;

    QItemSelectionModel *selection_model = view->selectionModel();
    QModelIndexList selected  = selection_model->selectedRows(0);

    if (selected.size() > 1 || selected.empty())
        return;

    QModelIndex index = selected.at(0);
    FileBrowserItem *item = NULL;

    if (view->model() == proxy)
        item = static_cast<FileBrowserItem*>(proxy->mapToSource(index).internalPointer());
    else
        item = static_cast<FileBrowserItem*>(index.internalPointer());

    if (!item || item->file)
        return;

    slotRightPaneClicked(index);

    treeView_RPANE->setFocus();
}

void ShareBrowser::goUp(QTreeView *view){
    if (view != treeView_RPANE)
        return;

    QStringList paths = lineEdit_PATH->text().split("\\", QString::SkipEmptyParts);

    if (paths.empty())//is it possible?
        return;
    else
        paths.removeLast();

    FileBrowserItem *tree_item = tree_model->createRootForPath(paths.join("\\"));
    QModelIndex tree_index = tree_model->createIndexForItem(tree_item);

    treeView_LPANE->selectionModel()->select(tree_index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    treeView_LPANE->scrollTo(tree_index, QAbstractItemView::PositionAtCenter);

    treeView_RPANE->setFocus();
}

void ShareBrowser::download(dcpp::DirectoryListing::Directory *dir, const QString &target){
    if (dir && !target.isEmpty()){
        try {
            listing.download(dir, target.toStdString(), false);
        }
        catch (const Exception &e){
            MainWindow::getInstance()->setStatusMessage(_q(e.getError()));
        }
    }
}

void ShareBrowser::download(dcpp::DirectoryListing::File *file, const QString &target){
    if (file && !target.isEmpty()){
        QString name = _q(file->getName());

        try {
            listing.download(file, (target+name).toStdString(), false, false);
        }
        catch (const Exception &e){
            MainWindow::getInstance()->setStatusMessage(_q(e.getError()));
        }
    }
}

void ShareBrowser::slotRightPaneClicked(const QModelIndex &index){
    if (!index.isValid())
        return;

    FileBrowserItem *item = NULL;

    if (treeView_RPANE->model() == proxy)
        item = static_cast<FileBrowserItem*>(proxy->mapToSource(index).internalPointer());
    else
        item = static_cast<FileBrowserItem*>(index.internalPointer());

    if (!item)
        return;

    if (item->file){
        download(item->file, _q(SETTING(DOWNLOAD_DIRECTORY)));

        return;
    }

    QString parent_path = lineEdit_PATH->text();
    QModelIndex parent_index = tree_model->createIndexForItem(tree_model->createRootForPath(parent_path));

    parent_path = parent_path +"\\"+item->data(COLUMN_FILEBROWSER_NAME).toString();
    parent_index = tree_model->createIndexForItem(tree_model->createRootForPath(parent_path));

    if (!parent_index.isValid())
        return;

    if (tree_model->canFetchMore(parent_index))
        tree_model->fetchMore(parent_index);

    treeView_LPANE->selectionModel()->clear();
    treeView_LPANE->selectionModel()->select(parent_index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    treeView_LPANE->scrollTo(parent_index, QAbstractItemView::PositionAtCenter);
}

void ShareBrowser::changeRoot(dcpp::DirectoryListing::Directory *root){
    if (!root)
        return;

    list_model->clear();

    current_size = 0;

    DirectoryListing::Directory::Iter it;

    for (it = root->directories.begin(); it != root->directories.end(); ++it){
        FileBrowserItem *child;
        quint64 size = 0;
        QList<QVariant> data;

        size = (*it)->getTotalSize(true);
        current_size += size;

        data << _q((*it)->getName())
             << WulforUtil::formatBytes(size)
             << size
             << "";

        child = new FileBrowserItem(data, list_root);
        child->dir = *it;

        list_root->appendChild(child);
    }

    DirectoryListing::File::List *files = &(root->files);
    DirectoryListing::File::Iter it_file;

    for (it_file = files->begin(); it_file != files->end(); ++it_file){
        FileBrowserItem *child;
        quint64 size = 0;
        QList<QVariant> data;

        size = (*it_file)->getSize();
        current_size += size;

        data << _q((*it_file)->getName())
             << WulforUtil::formatBytes(size)
             << size
             << _q((*it_file)->getTTH().toBase32());

        child = new FileBrowserItem(data, list_root);
        child->file = (*it_file);

        list_root->appendChild(child);
    }

    label_RIGHT->setText(QString(tr("Total size: %1")).arg(WulforUtil::formatBytes(current_size)));

    list_model->highlightDuplicates();

    list_model->sort();
}

void ShareBrowser::slotRightPaneSelChanged(const QItemSelection &, const QItemSelection &){
    QItemSelectionModel *selection_model = treeView_RPANE->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);
    quint64 selected_size = 0;

    foreach (const QModelIndex &i, list)
        selected_size += (reinterpret_cast<FileBrowserItem*>(i.internalPointer()))->data(COLUMN_FILEBROWSER_ESIZE).toULongLong();

    QString status = QString(tr("Total size: %1")).arg(WulforUtil::formatBytes(current_size));

    if (selected_size)
        status += QString(tr("; Selected: %1")).arg(WulforUtil::formatBytes(selected_size));

    label_RIGHT->setText(status);
}

static bool onlyFirstColumn(const QModelIndex &index){
    return (index.column() == 0);
}

void ShareBrowser::slotLeftPaneSelChanged(const QItemSelection &sel, const QItemSelection &des){


    QItemSelectionModel *selection_model = treeView_LPANE->selectionModel();
    QModelIndexList selected  = selection_model->selectedRows(0);

    if (selected.size() > 1 || selected.empty())
        return;

    QModelIndex index = selected.at(0);

    if (index.isValid()){

        SelPair p;

        FileBrowserItem *item = static_cast<FileBrowserItem*>(index.internalPointer());

        changeRoot(item->dir);
        p.dir = item->dir;
        p.index = index;

        lineEdit_PATH->setText(tree_model->createRemotePath(item));
        p.path_tesxt = tree_model->createRemotePath(item);

        pathHistory.append(p);
        pathHistory_iter = pathHistory.end();

        QModelIndexList deselected_idx = des.indexes();
        QFuture<QModelIndex> dsel_filter    = QtConcurrent::filtered(deselected_idx, onlyFirstColumn);

        deselected_idx  = dsel_filter.results();

        if (deselected_idx.size() != 1)
            return;

        QModelIndex old_index = deselected_idx.at(0);
        bool switchedToParent = (old_index.parent() == index);

        if (switchedToParent){
            FileBrowserItem *old_item = static_cast<FileBrowserItem*>(old_index.internalPointer());
            QString old_path = old_item->data(COLUMN_FILEBROWSER_NAME).toString();

            FileBrowserItem *list_item = list_model->createRootForPath(old_path);

            if (list_item){
                QModelIndex i = list_model->index(list_item->row(), 0, QModelIndex());

                if (i.isValid()){
                    treeView_RPANE->selectionModel()->select(i, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
                    treeView_RPANE->selectionModel()->setCurrentIndex(i, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
                }
            }
        }
        else {
            QModelIndex i = list_model->index(0, 0, QModelIndex());

            if (i.isValid()){
                treeView_RPANE->selectionModel()->select(i, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
                treeView_RPANE->selectionModel()->setCurrentIndex(i, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
            }
        }        
    }
}

void ShareBrowser::slotButtonUp(){

    QItemSelectionModel *selection_model = treeView_LPANE->selectionModel();
    QModelIndexList selected  = selection_model->selectedRows(0);

    if (selected.size() > 1 || selected.empty())
        return;

    QModelIndex index = selected.at(0);

    if (index.isValid()){

        FileBrowserItem *item = static_cast<FileBrowserItem*>(index.internalPointer());

        if (NULL != item->parent()){

            disconnect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                    this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));

            SelPair sparent;
            sparent.index = index.parent();

            changeRoot(item->parent()->dir);
            sparent.dir = item->parent()->dir;

            sparent.path_tesxt = tree_model->createRemotePath(item->parent());
            lineEdit_PATH->setText(tree_model->createRemotePath(item->parent()));

            slotRightPaneClicked(index.parent());

            pathHistory.append(sparent);
            pathHistory_iter = pathHistory.end();

            connect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                    this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));
        }
    }
}

void ShareBrowser::slotButtonBack(){
    if ( (pathHistory_iter != NULL)
            && (pathHistory.size() >0)){

        disconnect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));

        if(pathHistory.end() == pathHistory_iter || pathHistory.begin() != pathHistory_iter)
            --pathHistory_iter;

        SelPair sp= *pathHistory_iter;
        changeRoot(sp.dir);
        lineEdit_PATH->setText(sp.path_tesxt);

        slotRightPaneClicked(sp.index);

        connect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));
    }
}

void ShareBrowser::slotButtonForvard(){
    if ( (pathHistory_iter != NULL)
            && (pathHistory.size() >0)){

        disconnect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));

        if (pathHistory.end() == pathHistory_iter)
            --pathHistory_iter;
        else if (pathHistory_iter != &pathHistory.last())
            ++pathHistory_iter;

        SelPair sp= *pathHistory_iter;
        changeRoot(sp.dir);
        lineEdit_PATH->setText(sp.path_tesxt);

        slotRightPaneClicked(sp.index);

        connect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));
    }
}

void ShareBrowser::slotCustomContextMenu(const QPoint &){
    QTreeView *view = dynamic_cast<QTreeView*>(sender());

    if (!view)
        return;

    QItemSelectionModel *selection_model = view->selectionModel();
    QModelIndexList list;
    QModelIndexList selected  = selection_model->selectedRows(0);

    if (view == treeView_RPANE && treeView_RPANE->model() == proxy){
        foreach (const QModelIndex &i, selected)
            list.push_back(proxy->mapToSource(i));
    }
    else
        list = selected;

    if (!Menu::getInstance())
        Menu::newInstance();

    Menu::Action act = Menu::getInstance()->exec(view == treeView_LPANE? user : dcpp::UserPtr(NULL));
    QString target = _q(SETTING(DOWNLOAD_DIRECTORY));

    switch (act){
        case Menu::None:
        {
            break;
        }
        case Menu::Download:
        {
            foreach (const QModelIndex &index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                if (item->file)
                    download(item->file, target);
                else if (item->dir)
                    download(item->dir, target);
            }

            break;
        }
        case Menu::DownloadTo:
        {
            static QString old_target = QDir::homePath();
            target = Menu::getInstance()->getTarget();

            if (!QDir(target).exists() || target.isEmpty())
                target = QFileDialog::getExistingDirectory(this, tr("Select directory"), old_target);

            if (target.isEmpty())
                break;

            target = QDir::toNativeSeparators(target);

            if (!target.endsWith(QDir::separator()))
                target += QDir::separator();

            old_target = target;

            QStringList temp_pathes = DownloadToDirHistory::get();
            temp_pathes.push_front(target);
            
            DownloadToDirHistory::put(temp_pathes);

            if (!target.isEmpty()){
                foreach (const QModelIndex &index, list){
                    FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                    if (item->file)
                        download(item->file, target);
                    else if (item->dir)
                        download(item->dir, target);
                }
            }

            break;
        }
        case Menu::Alternates:
        {
            foreach (const QModelIndex &index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                if (item->file){//search alternates only for files
                    QString tth = item->data(COLUMN_FILEBROWSER_TTH).toString();
                    SearchFrame *sf = ArenaWidgetFactory().create<SearchFrame>();

                    sf->searchAlternates(tth);

                    break;//just one file
                }
            }

            break;
        }
        case Menu::Magnet:
        {
            QString magnets = "";
            QString path, tth, magnet;
            qlonglong size;

            foreach (const QModelIndex &index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                path = item->data(COLUMN_FILEBROWSER_NAME).toString();
                tth  = item->data(COLUMN_FILEBROWSER_TTH).toString();
                size = item->data(COLUMN_FILEBROWSER_ESIZE).toLongLong();

                magnet = WulforUtil::getInstance()->makeMagnet(path, size, tth);

                if (!magnet.isEmpty())
                    magnets += (magnet + "\n");
            }

            magnets = magnets.trimmed();

            qApp->clipboard()->setText(magnets, QClipboard::Clipboard);

            break;
        }
        case Menu::MagnetWeb:
        {
            QString magnets = "";
            QString path, tth, magnet;
            qlonglong size;

            foreach (const QModelIndex &index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                path = item->data(COLUMN_FILEBROWSER_NAME).toString();
                tth  = item->data(COLUMN_FILEBROWSER_TTH).toString();
                size = item->data(COLUMN_FILEBROWSER_ESIZE).toLongLong();

                magnet = "[magnet=\"" + WulforUtil::getInstance()->makeMagnet(path, size, tth) + "\"]"+path+"[/magnet]";

                if (!magnet.isEmpty())
                    magnets += (magnet + "\n");
            }

            magnets = magnets.trimmed();

            qApp->clipboard()->setText(magnets, QClipboard::Clipboard);

            break;
        }
        case Menu::MagnetInfo:
        {
            QString path, tth, magnet;
            qlonglong size;

            foreach (const QModelIndex &index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                path = item->data(COLUMN_FILEBROWSER_NAME).toString();
                tth  = item->data(COLUMN_FILEBROWSER_TTH).toString();
                size = item->data(COLUMN_FILEBROWSER_ESIZE).toLongLong();

                magnet = WulforUtil::getInstance()->makeMagnet(path, size, tth);

                if (!magnet.isEmpty()){
                    Magnet m(this);
                    m.setLink(magnet + "\n");
                    m.exec();
                }
            }

            break;
        }
        case Menu::AddToFav:
        {
            if (user && user != ClientManager::getInstance()->getMe())
                FavoriteManager::getInstance()->addFavoriteUser(user);

            break;
        }
        case Menu::AddRestrinction:
        {
            bool ok = false;
            unsigned share_sz = QInputDialog::getInt(this, tr("Enter restriction size (in GB)"), "Size", 0, 0, 1024, 1, &ok);

            if (!ok)
                break;

            foreach (QModelIndex index, list)
                tree_model->updateRestriction(index, share_sz);

            break;
        }
        case Menu::RemoveRestriction:
        {
            foreach (QModelIndex index, list)
                tree_model->updateRestriction(index, 0);

            break;
        }
        case Menu::OpenUrl:
        {
            ShareManager *SM = ShareManager::getInstance();

            foreach (const QModelIndex &index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                if (!item)
                    continue;

                DirectoryListing::AdlDirectory *adl_dir = dynamic_cast<DirectoryListing::AdlDirectory*>(item->dir);
                dcpp::StringList lst;

                try {
                    if (!adl_dir)
                        lst  = listing.getLocalPaths(item->dir);
                    else{
                        QStringList path_lst = _q(adl_dir->getFullPath()).split('\\');

                        if (path_lst.size() < 1)
                            break;

                        path_lst.removeFirst();//remove root element

                        QString path = path_lst.join("\\")+'\\';

                        lst = SM->getRealPaths(Util::toAdcFile(_tq(path)));
                    }
                }
                catch ( ... ){ }

                dcpp::StringIter it = lst.begin();
                for (; it != lst.end(); ++it){
                    if (QDir(_q(*it)).exists())
#ifndef Q_WS_WIN
                        QDesktopServices::openUrl(QUrl("file://"+_q(*it)));
#else
                        QDesktopServices::openUrl(QUrl("file:///"+_q(*it)));
#endif
                }
            }

            break;
        }
        default: break;
    }
}

void ShareBrowser::slotLayoutUpdated(){
    QItemSelectionModel *selection_model = treeView_LPANE->selectionModel();
    QModelIndexList selected  = selection_model->selectedRows(0);

    if (selected.size() > 1 || selected.empty())
        return;

    QModelIndex index = selected.at(0);

    treeView_LPANE->clearSelection();
    treeView_LPANE->selectionModel()->select(index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
}

void ShareBrowser::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView_RPANE);
}

void ShareBrowser::slotFilter(){
    if (frame_FILTER->isVisible()){
        treeView_RPANE->setModel(list_model);

        disconnect(lineEdit_FILTER, SIGNAL(textChanged(QString)), proxy, SLOT(setFilterFixedString(QString)));

        delete proxy;
        proxy = NULL;
    }
    else {
        proxy = new QSortFilterProxyModel(NULL);
        proxy->setDynamicSortFilter(true);
        proxy->setFilterFixedString(lineEdit_FILTER->text());
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(COLUMN_FILEBROWSER_NAME);
        proxy->setSourceModel(list_model);

        treeView_RPANE->setModel(proxy);

        connect(lineEdit_FILTER, SIGNAL(textChanged(QString)), proxy, SLOT(setFilterFixedString(QString)));

        lineEdit_FILTER->setFocus();

        if (!lineEdit_FILTER->text().isEmpty())
            lineEdit_FILTER->selectAll();
    }

    frame_FILTER->setVisible(!frame_FILTER->isVisible());
}

void ShareBrowser::slotStartSearch(){
    ShareBrowserSearch *sb_search = new ShareBrowserSearch(tree_model, this);

    sb_search->setSearchRoot(tree_root);
    connect(sb_search, SIGNAL(indexClicked(FileBrowserItem*)), this, SLOT(slotSearchJumpTo(FileBrowserItem*)));
}

void ShareBrowser::slotSearchJumpTo(FileBrowserItem *tree_item){
    if (!tree_item)
        return;

    QModelIndex tree_index = tree_model->createIndexForItem(tree_item);

    treeView_LPANE->selectionModel()->select(tree_index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    treeView_LPANE->scrollTo(tree_index, QAbstractItemView::PositionAtCenter);
}

void ShareBrowser::slotSettingsChanged(const QString &key, const QString&){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void ShareBrowser::slotClose() {
    ArenaWidgetManager::getInstance()->rem(this);
}
