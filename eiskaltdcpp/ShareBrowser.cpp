#include "ShareBrowser.h"
#include "WulforUtil.h"
#include "FileBrowserModel.h"
#include "MainWindow.h"
#include "SearchFrame.h"

#include "dcpp/SettingsManager.h"

#if (HAVE_MALLOC_TRIM)
#include <malloc.h>
#endif

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QTreeView>
#include <QModelIndex>
#include <QClipboard>

using namespace dcpp;

ShareBrowserLoader::ShareBrowserLoader(ShareBrowserLoader::LoaderFunc *f):
        func(f)
{
}

ShareBrowserLoader::~ShareBrowserLoader(){
}

void ShareBrowserLoader::run(){
    if (func)
        func->call();

    emit finished();
}

ShareBrowser::Menu::Menu(){
    menu = new QMenu();

    WulforUtil *WU = WulforUtil::getInstance();

    QAction *down    = new QAction(tr("Download"), menu);
    down->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
    down_to = new QMenu(tr("Download to"));
    QAction *sep     = new QAction(menu);
    QAction *alter   = new QAction(tr("Search for alternates"), menu);
    alter->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
    QAction *magnet  = new QAction(tr("Copy magnet"), menu);
    magnet->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));

    actions.insert(down, Download);
    actions.insert(alter, Alternates);
    actions.insert(magnet, Magnet);

    sep->setSeparator(true);

    menu->addActions(QList<QAction*>() << down << sep << alter << magnet);
    menu->insertMenu(sep, down_to);
}

ShareBrowser::Menu::~Menu(){
    delete menu;
    delete down_to;
}

ShareBrowser::Menu::Action ShareBrowser::Menu::exec(){
    qDeleteAll(down_to->actions());
    down_to->clear();

    const QPixmap &dir_px = WulforUtil::getInstance()->getPixmap(WulforUtil::eiFOLDER_BLUE);
    QString aliases, paths;

    aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toAscii());
    paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toAscii());

    QStringList a = aliases.split("\n", QString::SkipEmptyParts);
    QStringList p = paths.split("\n", QString::SkipEmptyParts);

    if (a.size() == p.size() && !a.isEmpty()){
        for (int i = 0; i < a.size(); i++){
            QAction *act = new QAction(a.at(i), down_to);
            act->setData(p.at(i));
            act->setIcon(dir_px);

            down_to->addAction(act);
        }
    }

    QAction *browse = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Browse"), down_to);
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
        loader_func(NULL)
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
}

ShareBrowser::~ShareBrowser(){
    delete tree_model;
    delete list_model;
    delete arena_menu;

    MainWindow::getInstance()->remWidgetFromArena(this);
    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);

    Menu::deleteInstance();

#if (HAVE_MALLOC_TRIM)
    malloc_trim(0);
#endif
}

void ShareBrowser::closeEvent(QCloseEvent *e){
    save();

    e->accept();
}

void ShareBrowser::init(){
    initModels();

    buildList();

    itemsCount = listing.getRoot()->getTotalFileCount();
    share_size = listing.getRoot()->getTotalSize();

    treeView_LPANE->setModel(tree_model);
    treeView_LPANE->header()->hideSection(COLUMN_FILEBROWSER_ESIZE);
    treeView_LPANE->header()->hideSection(COLUMN_FILEBROWSER_TTH);
    treeView_LPANE->setContextMenuPolicy(Qt::CustomContextMenu);

    treeView_RPANE->setModel(list_model);
    treeView_RPANE->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RPANE->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    label_LEFT->setText(QString(tr("Total share size: %1;  Files: %2")).arg(_q(Util::formatBytes(share_size))).arg(itemsCount));

    arena_menu = new QMenu(tr("Filebrowser"));

    QAction *close_wnd = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    connect(treeView_LPANE, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotLeftPaneClicked(QModelIndex)));
    connect(treeView_LPANE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenu(QPoint)));

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));

    connect(treeView_RPANE->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotLeftPaneSelChanged(QItemSelection,QItemSelection)));
    connect(treeView_RPANE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomContextMenu(QPoint)));
    connect(treeView_RPANE->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(treeView_RPANE, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotLeftPaneClicked(QModelIndex)));

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
}

void ShareBrowser::save(){
    WSSET(WS_SHARE_LPANE_STATE, treeView_LPANE->header()->saveState().toBase64());
    WSSET(WS_SHARE_RPANE_STATE, treeView_RPANE->header()->saveState().toBase64());

    WISET(WI_SHARE_RPANE_WIDTH, treeView_RPANE->width());
    WISET(WI_SHARE_WIDTH, width());
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

        loader_func = new ShareBrowserLoader::LoaderFunc(this, &ShareBrowser::createTree, listing.getRoot(), tree_root);
        ShareBrowserLoader *loader = new ShareBrowserLoader(loader_func);

        connect(loader, SIGNAL(finished()), this, SLOT(slotLoaderFinish()));

        treeView_LPANE->blockSignals(true);
        treeView_RPANE->blockSignals(true);

        loader->start();
    }
    catch (const Exception &e){
        //TODO: add error handling
    }
}

void ShareBrowser::createTree(DirectoryListing::Directory *dir, FileBrowserItem *root){
    if (!(dir && root))
        return;

    DirectoryListing::Directory::Iter it;
    FileBrowserItem *item;
    quint64 size = 0;
    QList<QVariant> data;

    size = dir->getTotalSize();

    data << QString::fromUtf8(dir->getName().c_str())
         << _q(Util::formatBytes(size))
         << size
         << "";

    item = new FileBrowserItem(data, root);
    item->dir = dir;

    root->appendChild(item);

    itemsCount += dir->getFileCount();

    std::sort(dir->directories.begin(), dir->directories.end(), DirectoryListing::Directory::DirSort());

    for (it = dir->directories.begin(); it != dir->directories.end(); ++it)
        createTree(*it, item);
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

void ShareBrowser::slotLeftPaneClicked(const QModelIndex &index){
    if (!index.isValid())
        return;

    FileBrowserItem *item = static_cast<FileBrowserItem*>(index.internalPointer());

    if (!(item && item->dir))
        return;

    if (sender() == treeView_LPANE){
        label_PATH->setText(tree_model->createRemotePath(item));
    }
    else{
        label_PATH->setText(label_PATH->text()+"\\"+item->data(COLUMN_FILEBROWSER_NAME).toString());

        FileBrowserItem *parent = tree_model->createRootForPath(label_PATH->text());

        if (parent){
            QModelIndex index = tree_model->createIndexForItem(parent);
            QStack<QModelIndex> stack;

            while (index.isValid()){
                stack.push(index);

                index = index.parent();
            }

            while (!stack.isEmpty()){
                index = stack.pop();

                treeView_LPANE->expand(index);
            }

            treeView_LPANE->selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            treeView_LPANE->scrollTo(index, QAbstractItemView::PositionAtCenter);
        }
    }

    changeRoot(item->dir);
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

        size = (*it)->getTotalSize();
        current_size += size;

        data << QString::fromUtf8((*it)->getName().c_str())
             << _q(Util::formatBytes(size))
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
             << _q(Util::formatBytes(size))
             << size
             << QString::fromUtf8((*it_file)->getTTH().toBase32().c_str());

        child = new FileBrowserItem(data, list_root);
        child->file = (*it_file);

        list_root->appendChild(child);
    }

    label_RIGHT->setText(QString(tr("Total size: %1")).arg(_q(Util::formatBytes(current_size))));

    list_model->repaint();
}

void ShareBrowser::slotLeftPaneSelChanged(const QItemSelection&, const QItemSelection&){
    QItemSelectionModel *selection_model = treeView_RPANE->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);
    quint64 selected_size = 0;

    foreach (QModelIndex i, list)
        selected_size += (reinterpret_cast<FileBrowserItem*>(i.internalPointer()))->data(COLUMN_FILEBROWSER_ESIZE).toULongLong();

    QString status = QString(tr("Total size: %1")).arg(_q(Util::formatBytes(current_size)));

    if (selected_size)
        status += QString(tr("; Selected: %1")).arg(_q(Util::formatBytes(selected_size)));

    label_RIGHT->setText(status);
}

void ShareBrowser::slotCustomContextMenu(const QPoint &){
    QTreeView *view = dynamic_cast<QTreeView*>(sender());

    if (!view)
        return;

    QItemSelectionModel *selection_model = view->selectionModel();
    QModelIndexList list = selection_model->selectedRows(0);

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

            if (!target.endsWith(QDir::separator()))
                target += QDir::separator();

            old_target = target;

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
    }
}

void ShareBrowser::slotLoaderFinish(){
    ShareBrowserLoader *loader = static_cast<ShareBrowserLoader*>(sender());

    if (!loader)
        return;

    loader->exit(0);
    loader->wait();
    loader->terminate();

    treeView_LPANE->blockSignals(false);
    treeView_RPANE->blockSignals(false);

    tree_model->repaint();
    list_model->repaint();

    delete loader;
    delete loader_func;

    load();

    if (!jump_to.isEmpty()){
        FileBrowserItem *root = tree_model->getRootElem();

        root = root->childItems.at(0);

        FileBrowserItem *jump = tree_model->createRootForPath(jump_to, root);

        if (jump){
            QModelIndex jump_index = tree_model->createIndexForItem(jump);

            slotLeftPaneClicked(jump_index);

            QModelIndex index = jump_index;

            while (index.isValid()){
                treeView_LPANE->expand(index);

                index = index.parent();
            }

            treeView_LPANE->selectionModel()->select(jump_index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
            treeView_LPANE->scrollTo(jump_index, QAbstractItemView::PositionAtCenter);
        }
    }
    else {
        QModelIndex index = tree_model->index(0, 0, QModelIndex());

        slotLeftPaneClicked(index);

        treeView_LPANE->expand(index);
    }

    /*treeView_LPANE->resizeColumnToContents(0);
    treeView_LPANE->resizeColumnToContents(1);*/

    MainWindow::getInstance()->addArenaWidget(this);
    MainWindow::getInstance()->mapWidgetOnArena(this);
    MainWindow::getInstance()->addArenaWidgetOnToolbar(this);

}

void ShareBrowser::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView_RPANE);
}
