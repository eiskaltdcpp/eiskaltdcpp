#include "SideBar.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "MainWindow.h"

#include "PMWindow.h"

#define CREATE_ROOT_EL(a, b, c, d, e) \
    do { \
        SideBarItem *root = new SideBarItem(NULL, (a)); \
        root->pixmap = WU->getPixmap(WulforUtil::b); \
        root->title  = (c); \
        (d).insert(ArenaWidget::e, root); \
        (a)->appendChild(root); \
    } while (0)

SideBarModel::SideBarModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    rootItem = new SideBarItem(NULL, NULL);

    WulforUtil *WU = WulforUtil::getInstance();

    CREATE_ROOT_EL(rootItem, eiSERVER,      tr("Hubs"),             roots,  Hub);
    CREATE_ROOT_EL(rootItem, eiUSERS,       tr("Private Messages"), roots,  PrivateMessage);
    CREATE_ROOT_EL(rootItem, eiFILEFIND,    tr("Search"),           roots,  Search);
    CREATE_ROOT_EL(rootItem, eiOWN_FILELIST,tr("Share Browsers"),   roots,  ShareBrowser);
    CREATE_ROOT_EL(rootItem, eiDOWNLOAD,    tr("Download Queue"),   roots,  Downloads);
    CREATE_ROOT_EL(rootItem, eiUPLIST,      tr("Finished Uploads"), roots,  FinishedUploads);
    CREATE_ROOT_EL(rootItem, eiDOWNLIST,    tr("Finished Downloads"),roots, FinishedDownloads);
    CREATE_ROOT_EL(rootItem, eiFAVSERVER,   tr("Favorite Hubs"),    roots,  FavoriteHubs);
    CREATE_ROOT_EL(rootItem, eiFAVUSERS,    tr("Favorite Users"),   roots,  FavoriteUsers);
    CREATE_ROOT_EL(rootItem, eiSERVER,      tr("Public Hubs"),      roots,  PublicHubs);
    CREATE_ROOT_EL(rootItem, eiSPY,         tr("Spy"),              roots,  Spy);
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
        case Qt::ForegroundRole:
        case Qt::BackgroundColorRole:
        case Qt::ToolTipRole:
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
        {
            SideBarItem *i = new SideBarItem(awgt, roots[awgt->role()]);
            roots[awgt->role()]->appendChild(i);

            items.insert(awgt, i);

            ind = index(i->row(), 0, index(roots[awgt->role()]->row(), 0, QModelIndex()));

            break;
        }
    default:
        roots[awgt->role()]->setWdget(awgt);
        ind = index(roots[awgt->role()]->row(), 0, QModelIndex());

        break;
    }

    if (!(typeid(*awgt) == typeid(PMWindow) && WBGET(WB_CHAT_KEEPFOCUS))){
        emit mapWidget(awgt);
        emit selectIndex(ind);
    }

    emit layoutChanged();
}

void SideBarModel::removeWidget(ArenaWidget *awgt){
    if (!items.contains(awgt) || !awgt)
        return;

    emit layoutAboutToBeChanged();

    switch (awgt->role()){
    case ArenaWidget::Hub:
    case ArenaWidget::PrivateMessage:
    case ArenaWidget::Search:
    case ArenaWidget::ShareBrowser:
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

            break;
        }
    default:
        break;
    }

    emit layoutChanged();
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
