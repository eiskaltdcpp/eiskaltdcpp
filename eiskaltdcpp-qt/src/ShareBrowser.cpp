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

#include "dcpp/SettingsManager.h"
#include "dcpp/FavoriteManager.h"
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

#include <boost/bind.hpp>

using namespace dcpp;

ShareBrowserRunner::ShareBrowserRunner(QObject *parent): QThread(parent){

}

ShareBrowserRunner::~ShareBrowserRunner(){

}

void ShareBrowserRunner::run(){
    runFunc();
}

void ShareBrowserRunner::setRunFunction(const boost::function<void()> &f){
    runFunc = f;
}

ShareBrowser::Menu::Menu(){
    menu = new QMenu();

    WulforUtil *WU = WulforUtil::getInstance();

    QAction *down    = new QAction(tr("Download"), menu);
    down->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
    down_to = new QMenu(tr("Download to..."));
    down_to->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD_AS));
    QAction *sep     = new QAction(menu);
    QAction *alter   = new QAction(tr("Search for alternates"), menu);
    alter->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
    QAction *magnet  = new QAction(tr("Copy magnet"), menu);
    magnet->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));
    QAction *sep1    = new QAction(menu);
    QAction *add_to_fav = new QAction(tr("Add to favorites"), menu);
    add_to_fav->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));

    actions.insert(down, Download);
    actions.insert(alter, Alternates);
    actions.insert(magnet, Magnet);
    actions.insert(add_to_fav, AddToFav);

    sep->setSeparator(true);
    sep1->setSeparator(true);

    menu->addActions(QList<QAction*>() << down << sep << alter << magnet << sep1 << add_to_fav);
    menu->insertMenu(sep, down_to);
}

ShareBrowser::Menu::~Menu(){
    delete menu;
    delete down_to;
}

ShareBrowser::Menu::Action ShareBrowser::Menu::exec(){
    qDeleteAll(down_to->actions());
    down_to->clear();

    const QPixmap &dir_px = WICON(WulforUtil::eiFOLDER_BLUE);
    QString aliases, paths;

    aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toAscii());
    paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toAscii());

    QStringList a = aliases.split("\n", QString::SkipEmptyParts);
    QStringList p = paths.split("\n", QString::SkipEmptyParts);

    QString raw = QByteArray::fromBase64(WSGET(WS_DOWNLOAD_DIR_HISTORY).toAscii());
    QStringList temp_pathes = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

    if (!temp_pathes.isEmpty()){
        foreach (QString t, temp_pathes){
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
        listing(user),
        tree_root(NULL),
        list_root(NULL),
        tree_model(NULL),
        list_model(NULL),
        proxy(NULL)
{
    setupUi(this);

    nick = WulforUtil::getInstance()->getNicks(user->getCID());;

    if (nick.indexOf(_q(user->getCID().toBase32()) >= 0)){//User offline
        nick = _q(ClientManager::getInstance()->getNicks(user->getCID())[0]);

        QFileInfo info(file);

        nick = info.baseName().left(info.baseName().indexOf("."));

        if (nick == "files")
            title = tr("Own files");
        else
            title = tr("Listing: ") + nick;
    }

    init();

    setAttribute(Qt::WA_DeleteOnClose);
}

ShareBrowser::~ShareBrowser(){
    delete tree_model;
    delete list_model;
    delete arena_menu;

    delete proxy;

    Menu::deleteInstance();

#if (HAVE_MALLOC_TRIM)
    malloc_trim(0);
#endif
}

void ShareBrowser::closeEvent(QCloseEvent *e){
    save();

    MainWindow::getInstance()->remWidgetFromArena(this);
    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);

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

    return QWidget::eventFilter(obj, e);
}

void ShareBrowser::init(){
    frame_FILTER->setVisible(false);

    initModels();

    buildList();

    treeView_LPANE->setModel(tree_model);
    treeView_LPANE->header()->hideSection(COLUMN_FILEBROWSER_ESIZE);
    treeView_LPANE->header()->hideSection(COLUMN_FILEBROWSER_TTH);
    treeView_LPANE->setContextMenuPolicy(Qt::CustomContextMenu);

    treeView_RPANE->setModel(list_model);
    treeView_RPANE->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RPANE->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RPANE->installEventFilter(this);

    toolButton_CLOSEFILTER->setIcon(WICON(WulforUtil::eiEDITDELETE));

    arena_menu = new QMenu(tr("Filebrowser"));

    QAction *close_wnd = new QAction(WICON(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    //connect(treeView_LPANE, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotLeftPaneClicked(QModelIndex)));
    connect(treeView_LPANE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenu(QPoint)));
    connect(treeView_LPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));
    connect(toolButton_CLOSEFILTER, SIGNAL(clicked()), this, SLOT(slotFilter()));

    connect(treeView_RPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotRightPaneSelChanged(QItemSelection,QItemSelection)));
    connect(treeView_RPANE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenu(QPoint)));
    connect(treeView_RPANE->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(treeView_RPANE, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotRightPaneClicked(QModelIndex)));
    connect(this, SIGNAL(loadFinished()), this, SLOT(slotLoaderFinish()), Qt::QueuedConnection);
    connect(tree_model, SIGNAL(layoutChanged()), this, SLOT(slotLayoutUpdated()));

    setAttribute(Qt::WA_DeleteOnClose);
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
        ADLSearchManager::getInstance()->matchListing(listing);

        ShareBrowserRunner *runner = new ShareBrowserRunner(this);
        boost::function<void()> f = boost::bind(&ShareBrowser::createTree, this, listing.getRoot(), tree_root);

        runner->setRunFunction(f);
        connect(runner, SIGNAL(finished()), runner, SLOT(deleteLater()));

        runner->start();

        treeView_LPANE->blockSignals(true);
        treeView_RPANE->blockSignals(true);
    }
    catch (const Exception &e){
        QMessageBox::critical(this, file, _q(e.what()), QMessageBox::Ok);

        close();
    }
}

void ShareBrowser::createTree(DirectoryListing::Directory *dir, FileBrowserItem *root){
    if (!(dir && root))
        return;

    DirectoryListing::Directory::Iter it;
    FileBrowserItem *item;
    quint64 size = 0;
    QList<QVariant> data;

    size = dir->getTotalSize(true);

    data << QString::fromUtf8(dir->getName().c_str())
         << WulforUtil::formatBytes(size)
         << size
         << "";

    item = new FileBrowserItem(data, root);
    item->dir = dir;

    root->appendChild(item);

    itemsCount += dir->getFileCount();

    std::sort(dir->directories.begin(), dir->directories.end(), DirectoryListing::Directory::DirSort());

    for (it = dir->directories.begin(); it != dir->directories.end(); ++it)
        createTree(*it, item);

    if (root == tree_root)
        emit loadFinished();
}

void ShareBrowser::initModels(){
    tree_model = new FileBrowserModel();
    tree_root  = new FileBrowserItem(QList<QVariant>() << tr("Name") << tr("Size")
                                                       << tr("Exact size")
                                                       << tr("TTH"),
                                                       NULL);

    tree_model->setRootElem(tree_root, true, true);

    list_model = new FileBrowserModel();
    list_root = new FileBrowserItem(QList<QVariant>() << tr("Name") << tr("Size")
                                                      << tr("Exact size")
                                                      << tr("TTH"),
                                                      NULL);

    list_model->setRootElem(list_root, true, true);
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

    QStringList paths = label_PATH->text().split("\\", QString::SkipEmptyParts);

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

    label_PATH->setText(label_PATH->text()+"\\"+item->data(COLUMN_FILEBROWSER_NAME).toString());

    FileBrowserItem *tree_item = tree_model->createRootForPath(label_PATH->text());
    QModelIndex tree_index = tree_model->createIndexForItem(tree_item);

    treeView_LPANE->selectionModel()->select(tree_index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    treeView_LPANE->scrollTo(tree_index, QAbstractItemView::PositionAtCenter);
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

        data << QString::fromUtf8((*it)->getName().c_str())
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

        data << QString::fromUtf8((*it_file)->getName().c_str())
             << WulforUtil::formatBytes(size)
             << size
             << QString::fromUtf8((*it_file)->getTTH().toBase32().c_str());

        child = new FileBrowserItem(data, list_root);
        child->file = (*it_file);

        list_root->appendChild(child);
    }

    label_RIGHT->setText(QString(tr("Total size: %1")).arg(WulforUtil::formatBytes(current_size)));

    list_model->sort();
}

void ShareBrowser::slotRightPaneSelChanged(const QItemSelection &, const QItemSelection &){
    QItemSelectionModel *selection_model = treeView_RPANE->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);
    quint64 selected_size = 0;

    foreach (QModelIndex i, list)
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
        FileBrowserItem *item = static_cast<FileBrowserItem*>(index.internalPointer());

        changeRoot(item->dir);

        label_PATH->setText(tree_model->createRemotePath(item));

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

void ShareBrowser::slotCustomContextMenu(const QPoint &){
    QTreeView *view = dynamic_cast<QTreeView*>(sender());

    if (!view)
        return;

    QItemSelectionModel *selection_model = view->selectionModel();
    QModelIndexList list;
    QModelIndexList selected  = selection_model->selectedRows(0);

    if (view == treeView_RPANE && treeView_RPANE->model() == proxy){
        foreach (QModelIndex i, selected)
            list.push_back(proxy->mapToSource(i));
    }
    else
        list = selected;

    if (!Menu::getInstance())
        Menu::newInstance();

    Menu::Action act = Menu::getInstance()->exec();
    QString target = _q(SETTING(DOWNLOAD_DIRECTORY));

    switch (act){
        case Menu::None:
        {
            break;
        }
        case Menu::Download:
        {
            foreach (QModelIndex index, list){
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

            QString raw = QByteArray::fromBase64(WSGET(WS_DOWNLOAD_DIR_HISTORY).toAscii());
            QStringList temp_pathes = raw.replace("\r","").split('\n', QString::SkipEmptyParts);

            if (!temp_pathes.contains(target)){
                temp_pathes.push_front(target);

                if (temp_pathes.count() > 5)
                    temp_pathes.removeLast();

                QString raw = temp_pathes.join("\n");
                WSSET(WS_DOWNLOAD_DIR_HISTORY, raw.toAscii().toBase64());
            }

            if (!target.isEmpty()){
                foreach (QModelIndex index, list){
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
            foreach (QModelIndex index, list){
                FileBrowserItem *item = reinterpret_cast<FileBrowserItem*>(index.internalPointer());

                if (item->file){//search alternates only for files
                    QString tth = item->data(COLUMN_FILEBROWSER_TTH).toString();
                    SearchFrame *sf = new SearchFrame();

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

            foreach (QModelIndex index, list){
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
        case Menu::AddToFav:
        {
            if (user && user != ClientManager::getInstance()->getMe())
                FavoriteManager::getInstance()->addFavoriteUser(user);

            break;
        }
    }
}

void ShareBrowser::slotLoaderFinish(){
    treeView_LPANE->blockSignals(false);
    treeView_RPANE->blockSignals(false);

    tree_model->repaint();
    list_model->repaint();

    load();

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
    else {
        QModelIndex index = tree_model->index(0, 0, QModelIndex());

        treeView_LPANE->selectionModel()->select(index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
        treeView_LPANE->expand(index);
    }

    /*treeView_LPANE->resizeColumnToContents(0);
    treeView_LPANE->resizeColumnToContents(1);*/

    MainWindow::getInstance()->addArenaWidget(this);
    MainWindow::getInstance()->addArenaWidgetOnToolbar(this);
    MainWindow::getInstance()->mapWidgetOnArena(this);

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

bool ShareBrowser::isFindFrameActivated(){
    return (frame_FILTER->isVisible() && lineEdit_FILTER->hasFocus());
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
