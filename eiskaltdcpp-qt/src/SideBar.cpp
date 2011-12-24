/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SideBar.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "MainWindow.h"
#include "ArenaWidgetManager.h"
#include "ArenaWidgetFactory.h"
#include "QuickConnect.h"
#include "SearchFrame.h"
#include "ShareBrowser.h"
#include "GlobalTimer.h"

#include "PMWindow.h"

#include <QPainter>
#include <QEvent>
#include <QMenu>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

#define CREATE_ROOT_EL(a, b, c, d, e) \
    do { \
        SideBarItem *root = new SideBarItem(NULL, (a)); \
        root->pixmap = WU->getPixmap(WulforUtil::b); \
        root->title  = (c); \
        (d).insert(ArenaWidget::e, root); \
        (a)->appendChild(root); \
    } while (0)

#define RETRANSLATE_ROOT_EL(text, map, type_el) \
    do { \
        if ((map).contains(ArenaWidget::type_el) && map[ArenaWidget::type_el]) \
            map[ArenaWidget::type_el]->title = text; \
    } while (0)

static const QString &SIDEBAR_SHOW_CLOSEBUTTONS = "mainwindow/sidebar-with-close-buttons";
    
SideBarModel::SideBarModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    rootItem = new SideBarItem(NULL, NULL);

    WulforUtil *WU = WulforUtil::getInstance();

    CREATE_ROOT_EL(rootItem, eiSERVER,      tr("Hubs"),             roots,  Hub);
    CREATE_ROOT_EL(rootItem, eiUSERS,       tr("Private Messages"), roots,  PrivateMessage);
    CREATE_ROOT_EL(rootItem, eiFILEFIND,    tr("Search"),           roots,  Search);
    CREATE_ROOT_EL(rootItem, eiOWN_FILELIST,tr("Share Browsers"),   roots,  ShareBrowser);
    CREATE_ROOT_EL(rootItem, eiADLS,        tr("ADLSearch"),        roots,  ADLS);
    CREATE_ROOT_EL(rootItem, eiDOWNLOAD,    tr("Download Queue"),   roots,  Downloads);
    CREATE_ROOT_EL(rootItem, eiUSERS,       tr("Queued Users"),     roots,  QueuedUsers);
    CREATE_ROOT_EL(rootItem, eiUPLIST,      tr("Finished Uploads"), roots,  FinishedUploads);
    CREATE_ROOT_EL(rootItem, eiDOWNLIST,    tr("Finished Downloads"),roots, FinishedDownloads);
    CREATE_ROOT_EL(rootItem, eiFAVSERVER,   tr("Favorite Hubs"),    roots,  FavoriteHubs);
    CREATE_ROOT_EL(rootItem, eiFAVUSERS,    tr("Favorite Users"),   roots,  FavoriteUsers);
    CREATE_ROOT_EL(rootItem, eiSERVER,      tr("Public Hubs"),      roots,  PublicHubs);
    CREATE_ROOT_EL(rootItem, eiSPY,         tr("Spy"),              roots,  Spy);
    //CREATE_ROOT_EL(rootItem, eiSERVER,      tr("Hub Manager"),      roots,  HubManager);
    CREATE_ROOT_EL(rootItem, eiGUI,         tr("Other Widgets"),    roots,  CustomWidget);

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));
}

SideBarModel::~SideBarModel()
{
    if (rootItem)
        delete rootItem;
}

int SideBarModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<SideBarItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant SideBarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    SideBarItem *item = static_cast<SideBarItem*>(index.internalPointer());

    if (index.column() == 1)
        return QVariant();

    switch(role) {
        case Qt::DecorationRole:
        {
            if (!item->getWidget())
                return item->pixmap.scaled(18, 18);
            else if (item->getWidget())
                return item->getWidget()->getPixmap().scaled(18, 18);
        }
        case Qt::DisplayRole:
        {
            if (!item->getWidget())
                return item->title;
            else if (item->getWidget())
                return item->getWidget()->getArenaShortTitle();
        }
        case Qt::TextAlignmentRole:
        {
            return static_cast<uint>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::ForegroundRole:
        case Qt::BackgroundColorRole:
        case Qt::ToolTipRole:
        {
            if (!item->getWidget())
                return item->title;
            else if (item->getWidget())
                return WulforUtil::getInstance()->compactToolTipText(item->getWidget()->getArenaTitle(), 60, "\n");
        }
            break;
    }

    return QVariant();
}

QVariant SideBarModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    return (QList<QVariant>() << tr("Widgets"));
}

QModelIndex SideBarModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SideBarItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SideBarItem*>(parent.internalPointer());

    SideBarItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SideBarModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SideBarItem *childItem = static_cast<SideBarItem*>(index.internalPointer());
    SideBarItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int SideBarModel::rowCount(const QModelIndex &parent) const
{
    SideBarItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SideBarItem*>(parent.internalPointer());

    return parentItem->childCount();
}


void SideBarModel::sort(int column, Qt::SortOrder order) {
    return;
}

void SideBarModel::insertWidget(ArenaWidget *awgt){
    if (items.contains(awgt) || !awgt)
        return;

    QModelIndex ind;

    switch (awgt->role()){
    case ArenaWidget::Hub:
    case ArenaWidget::PrivateMessage:
    case ArenaWidget::Search:
    case ArenaWidget::ShareBrowser:
    case ArenaWidget::CustomWidget:
        {
            SideBarItem *i = new SideBarItem(awgt, roots[awgt->role()]);
            roots[awgt->role()]->appendChild(i);

            items.insert(awgt, i);

            ind = index(i->row(), 0, index(roots[awgt->role()]->row(), 0, QModelIndex()));

            break;
        }
    default:
        roots[awgt->role()]->setWdget(awgt);
        
        SideBarItem *root  = roots[awgt->role()];
        
        ind = index(root->row(), 0, QModelIndex());
        
        if (root->getWidget() && root->getWidget()->toolButton())
            root->getWidget()->toolButton()->setChecked(true);

        break;
    }

    emit layoutChanged();
}

void SideBarModel::removeWidget(ArenaWidget *awgt){
    if (!items.contains(awgt) || !awgt)
        return;

    switch (awgt->role()){
    case ArenaWidget::Hub:
    case ArenaWidget::PrivateMessage:
    case ArenaWidget::Search:
    case ArenaWidget::ShareBrowser:
    case ArenaWidget::CustomWidget:
        {
            SideBarItem *root  = roots[awgt->role()];
            SideBarItem *child = items[awgt];

            items.remove(awgt);

            QModelIndex par_root = index(root->row(), 0, QModelIndex());

            beginRemoveRows(par_root, child->row(), child->row());
            {
                root->childItems.removeAt(root->childItems.indexOf(child));

                delete child;
            }
            endRemoveRows();

            if (!historyAtTop(awgt))
                historyPurge(awgt);

            historyPop();

            break;
        }
    default:
        {
           SideBarItem *root  = roots[awgt->role()];
           
           if (root->getWidget() && root->getWidget()->toolButton())
               root->getWidget()->toolButton()->setChecked(false);
           
           historyPop();
        }
    }
}

void SideBarModel::toggled ( ArenaWidget* awgt ) {
    if (!awgt)
        return;
    
    if (awgt->toolButton()){
        awgt->toolButton()->setChecked(false);
        awgt->toolButton()->setCheckable(false);
    }
    
    ArenaWidgetManager::getInstance()->activate(awgt);
}

bool SideBarModel::hasWidget(ArenaWidget *awgt) const{
    if (!awgt)
        return false;

    bool inRoot = false;
    QMap<ArenaWidget::Role, SideBarItem*>::const_iterator it = roots.begin();
    SideBarItem *item = NULL;

    for(; it != roots.end(); ++it){
        item = it.value();

        if (item->getWidget() == awgt && awgt->getWidget()->isVisible()){
            inRoot = true;

            break;
        }
    }

    return (items.contains(awgt) || inRoot);
}

void SideBarModel::mapped(ArenaWidget *awgt){
    if (!awgt)
        return;

    QModelIndex s;

    if (items.contains(awgt)){
        SideBarItem *root  = roots[awgt->role()];
        SideBarItem *child = items[awgt];

        QModelIndex par_root = index(root->row(), 0, QModelIndex());
        s = index(child->row(), 0, par_root);
    }
    else {
        SideBarItem *root  = roots[awgt->role()];

        s = index(root->row(), 0, QModelIndex());
    }

    historyPush(awgt);

    emit selectIndex(s);
}

void SideBarModel::updated ( ArenaWidget* awgt ) {
    if (!(awgt  && (awgt->state() & ArenaWidget::Singleton) && !historyStack.isEmpty()))
        return;
    
    if ((awgt->state() & ArenaWidget::Hidden) && (historyStack.last() == awgt))
        historyPop();
}

bool SideBarModel::isRootItem(const SideBarItem *item) const{
    QMap<ArenaWidget::Role, SideBarItem*>::const_iterator it = roots.begin();

    for(; it != roots.end(); ++it){
        if (it.value() == item)
            return true;
    }

    return false;
}

ArenaWidget::Role SideBarModel::rootItemRole(const SideBarItem *item) const{
    QMap<ArenaWidget::Role, SideBarItem*>::const_iterator it = roots.begin();

    for(; it != roots.end(); ++it){
        if (it.value() == item)
            return it.key();
    }

    return ArenaWidget::CustomWidget;
}

void SideBarModel::historyPop(){
    if (historyStack.empty())
        return;

    historyStack.pop();//remove last widget

    if (historyStack.isEmpty())
        return;

    ArenaWidget *awgt = historyStack.pop();

    emit mapWidget(awgt);
}

void SideBarModel::historyPush(ArenaWidget *awgt){
    historyPurge(awgt);

    historyStack.push(awgt);
}

bool SideBarModel::historyAtTop(ArenaWidget *awgt){
    return (!historyStack.isEmpty() && historyStack.top() == awgt);
}

void SideBarModel::historyPurge(ArenaWidget *awgt){
    if (historyStack.indexOf(awgt) >= 0)
        historyStack.remove(historyStack.indexOf(awgt));
}

void SideBarModel::slotIndexClicked(const QModelIndex &i){
   if (!(i.isValid() && i.internalPointer()))
       return;

   SideBarItem *item = reinterpret_cast<SideBarItem*>(i.internalPointer());
   ArenaWidget *awgt = item->getWidget();

   if (items.contains(awgt))
       emit mapWidget(awgt);
   else {
       ArenaWidget::Role role = roots.key(item);
       ArenaWidget *awgt = MainWindow::getInstance()->widgetForRole(role);

       emit mapWidget(awgt);
   }
}

void SideBarModel::slotSettingsChanged(const QString &key, const QString &value){
    if (key == WS_TRANSLATION_FILE){
        RETRANSLATE_ROOT_EL(tr("Hubs"),             roots,  Hub);
        RETRANSLATE_ROOT_EL(tr("Private Messages"), roots,  PrivateMessage);
        RETRANSLATE_ROOT_EL(tr("Search"),           roots,  Search);
        RETRANSLATE_ROOT_EL(tr("Share Browsers"),   roots,  ShareBrowser);
        RETRANSLATE_ROOT_EL(tr("ADLSearch"),        roots,  ADLS);
        RETRANSLATE_ROOT_EL(tr("Download Queue"),   roots,  Downloads);
        RETRANSLATE_ROOT_EL(tr("Finished Uploads"), roots,  FinishedUploads);
        RETRANSLATE_ROOT_EL(tr("Finished Downloads"),roots, FinishedDownloads);
        RETRANSLATE_ROOT_EL(tr("Favorite Hubs"),    roots,  FavoriteHubs);
        RETRANSLATE_ROOT_EL(tr("Favorite Users"),   roots,  FavoriteUsers);
        RETRANSLATE_ROOT_EL(tr("Public Hubs"),      roots,  PublicHubs);
        RETRANSLATE_ROOT_EL(tr("Spy"),              roots,  Spy);
        RETRANSLATE_ROOT_EL(tr("Other Widgets"),    roots,  CustomWidget);
        RETRANSLATE_ROOT_EL(tr("Queued Users"),     roots,  QueuedUsers);
    }
}

SideBarDelegate::SideBarDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{
}

SideBarDelegate::~SideBarDelegate(){
}

void SideBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.column() == 0){
        QStyledItemDelegate::paint(painter, option, index);

        return;
    }

    bool showCloseBtn = false;
    SideBarItem *item = reinterpret_cast<SideBarItem*>(index.internalPointer());
    ArenaWidget *awgt = item->getWidget();

    if (awgt){
        switch (awgt->role()){
            case ArenaWidget::Hub:
            case ArenaWidget::PrivateMessage:
            case ArenaWidget::Search:
            case ArenaWidget::ShareBrowser:
            case ArenaWidget::CustomWidget:
                showCloseBtn = true;
            default:
                break;
        }
    }

    QStyledItemDelegate::paint(painter, option, index);

    if ((option.state & (QStyle::State_MouseOver | QStyle::State_Selected)) && showCloseBtn){
        QPixmap px = WICON(WulforUtil::eiEDITDELETE).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        painter->drawPixmap(option.rect.x() + (option.rect.width() - 16)/2,
                            option.rect.y() + (option.rect.height() - 16)/2,
                            16, 16, px);

        return;
    }
}

QSize SideBarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    static const int MARGIN = 1;
    const int PXHEIGHT = option.fontMetrics.height() > 16? option.fontMetrics.height() : 16;
    const int HEIGHT = PXHEIGHT+MARGIN*4;

    return QSize( 200, HEIGHT );
}

SideBarView::SideBarView ( QWidget* parent ) : QTreeView(parent), _model(NULL) {
    installEventFilter(this);

    _model = new SideBarModel(this);
    
    setModel(_model);
    setItemsExpandable(true);
    setHeaderHidden(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setItemDelegate(new SideBarDelegate(this));
    expandAll();
    
    connect(ArenaWidgetManager::getInstance(), SIGNAL(activated(ArenaWidget*)), this,   SLOT(activated(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(added(ArenaWidget*)),     this,   SLOT(added(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(removed(ArenaWidget*)),   this,   SLOT(removed(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(activated(ArenaWidget*)), _model, SLOT(mapped(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(toggled(ArenaWidget*)),   _model, SLOT(toggled(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(updated(ArenaWidget*)),   _model, SLOT(updated(ArenaWidget*)));
    
    connect(this, SIGNAL(doubleClicked(QModelIndex)),           this,   SLOT(slotSideBarDblClicked(QModelIndex)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),   this,   SLOT(slotSidebarContextMenu()));
    connect(this, SIGNAL(clicked(QModelIndex)),                 this,   SLOT(slotSidebarHook(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)),                 _model, SLOT(slotIndexClicked(QModelIndex)));
   
    
    connect(GlobalTimer::getInstance(), SIGNAL(second()), _model, SLOT(redraw()));
    
    connect(_model,    SIGNAL(mapWidget(ArenaWidget*)),     ArenaWidgetManager::getInstance(),  SLOT(activate(ArenaWidget*)));
    connect(_model,    SIGNAL(selectIndex(QModelIndex)),    this,                               SLOT(slotWidgetActivated(QModelIndex)));
}

SideBarView::~SideBarView() {

}

bool SideBarView::eventFilter ( QObject *obj, QEvent *e) {
    if (obj == this && e->type() == QEvent::Resize) {
        if (WBGET(SIDEBAR_SHOW_CLOSEBUTTONS, true)){
            header()->resizeSection(0, contentsRect().width() - 20);
            header()->resizeSection(1, 18);
        }
        else{
            header()->resizeSection(0, contentsRect().width());
            header()->resizeSection(1, 0);
        }
    }
    
    return QObject::eventFilter (obj , e);
}

void SideBarView::activated ( ArenaWidget *awgt ) {
    _model->mapped(awgt);
}

void SideBarView::added ( ArenaWidget *awgt ) {
    _model->insertWidget(awgt);
}

void SideBarView::removed ( ArenaWidget *awgt ) {
    _model->removeWidget(awgt);
}

void SideBarView::slotSidebarContextMenu(){
    QItemSelectionModel *s_m = selectionModel();
    QModelIndexList selected = s_m->selectedRows(0);

    if (selected.size() < 1)
        return;

    SideBarItem *item = reinterpret_cast<SideBarItem*>(selected.at(0).internalPointer());

    QMenu *menu = NULL;
    
    if (item && item->childCount() > 0){
        menu = new QMenu(this);
        menu->addAction(WICON(WulforUtil::eiEDITDELETE), tr("Close all"));

        if (menu->exec(QCursor::pos())){
            QList<SideBarItem*> childs = item->childItems;

            foreach (SideBarItem *i, childs){
                if (i && i->getWidget())
                    ArenaWidgetManager::getInstance()->rem(i->getWidget());
            }
        }
        
        menu->deleteLater();
    }
    else if (item && item->getWidget()){
        menu = item->getWidget()->getMenu();
        
        if (!menu && (item->getWidget()->state() & ArenaWidget::Singleton))
            return;

        if(!menu){
            menu = new QMenu(this);
            menu->addAction(WICON(WulforUtil::eiEDITDELETE), tr("Close"));

            if (menu->exec(QCursor::pos()))
                ArenaWidgetManager::getInstance()->rem(item->getWidget());
            
            menu->deleteLater();
        }
        else
            menu->exec(QCursor::pos());
    }
}

void SideBarView::slotSidebarHook(const QModelIndex &index){
    if (index.column() == 1){
        SideBarItem *item = reinterpret_cast<SideBarItem*>(index.internalPointer());

        if (item->getWidget()){
            switch (item->getWidget()->role()){
            case ArenaWidget::Hub:
            case ArenaWidget::PrivateMessage:
            case ArenaWidget::Search:
            case ArenaWidget::ShareBrowser:
            case ArenaWidget::CustomWidget:
                ArenaWidgetManager::getInstance()->rem(item->getWidget());
                break;
            default:
                break;
            }
        }
    }
}

void SideBarView::slotSideBarDblClicked(const QModelIndex &index){
    if (index.column() != 0)
        return;

    SideBarItem *item = reinterpret_cast<SideBarItem*>(index.internalPointer());

    if (!_model->isRootItem(item) || item->childCount() > 0)
        return;

    switch (_model->rootItemRole(item)){
    case ArenaWidget::Search:
        {
            SearchFrame *sf = ArenaWidgetFactory().create<SearchFrame>();

            break;
        }
    case ArenaWidget::Hub:
        {
            QuickConnect qc;

            qc.exec();

            break;
        }
    //FIXME: Next code duplicates methods from MainWindow
    case ArenaWidget::ShareBrowser:
        {
           QString file = QFileDialog::getOpenFileName ( this, tr ( "Choose file to open" ),
                       QString::fromStdString ( Util::getPath ( Util::PATH_FILE_LISTS ) ),
                       tr ( "Modern XML Filelists" ) + " (*.xml.bz2);;" +
                       tr ( "Modern XML Filelists uncompressed" ) + " (*.xml);;" +
                       tr ( "All files" ) + " (*)" );

            if ( file.isEmpty() )
                return;

            file = QDir::toNativeSeparators ( file );
            UserPtr user = dcpp::DirectoryListing::getUserFromFilename ( _tq ( file ) );

            if ( user )
                ArenaWidgetFactory().create<ShareBrowser, UserPtr, QString, QString> ( user, file, "" );
            
            break;
        }
    case ArenaWidget::PrivateMessage:
        {
             QString f = QFileDialog::getOpenFileName(this, tr("Open log file"),_q(SETTING(LOG_DIRECTORY)), tr("Log files (*.log);;All files (*.*)"));

            if ( !f.isEmpty() ) {
                f = QDir::toNativeSeparators ( f );

                if ( f.startsWith ( "/" ) )
                    f = "file://" + f;
                else
                    f = "file:///" + f;

                QDesktopServices::openUrl ( QUrl(f) );
            }
            
            break;
        }
    default:
        break;
    }

    setExpanded(index, true);
}

void SideBarView::slotWidgetActivated ( QModelIndex i ) {
    selectionModel()->clearSelection();
    selectionModel()->select(i, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
}
