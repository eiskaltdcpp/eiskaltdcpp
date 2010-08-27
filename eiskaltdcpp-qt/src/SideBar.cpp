#include "SideBar.h"
#include "WulforUtil.h"
#include "WulforSettings.h"
#include "MainWindow.h"

#include "PMWindow.h"

#include <QPainter>

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
    CREATE_ROOT_EL(rootItem, eiADLS,           tr("ADLSearch"),     roots,  ADLS);
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

    if (!(typeid(*awgt) == typeid(PMWindow) && WBGET(WB_CHAT_KEEPFOCUS)))
        emit mapWidget(awgt);

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
        break;
    }
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

SideBarDelegate::SideBarDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{
}

SideBarDelegate::~SideBarDelegate(){
}

void SideBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    bool showCloseBtn = false;
    SideBarItem *item = reinterpret_cast<SideBarItem*>(index.internalPointer());
    ArenaWidget *awgt = item->getWidget();

    if (awgt){
        switch (awgt->role()){
            case ArenaWidget::Hub:
            case ArenaWidget::PrivateMessage:
            case ArenaWidget::Search:
            case ArenaWidget::ShareBrowser:
                showCloseBtn = true;
            default:
                break;
        }
    }

    if (!((option.state & (QStyle::State_MouseOver | QStyle::State_Selected)) && showCloseBtn)){
        if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)){
            drawBackground(painter, option, index);

            QStyleOptionViewItem opt = option;
            opt.state = opt.state & ~QStyle::State_Selected;

            QStyledItemDelegate::paint(painter, opt, index);
        }
        else
            QStyledItemDelegate::paint(painter, option, index);

        return;
    }

    QPixmap px = WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QRect pxRect = option.rect;
    pxRect.setX(pxRect.x()+option.rect.width()-px.width()-4);
    pxRect.setWidth(px.width());

    QStyleOptionButton btnOption;
    btnOption.state         = option.state & ~(QStyle::State_Selected | QStyle::State_MouseOver);
    btnOption.direction     = QApplication::layoutDirection();
    btnOption.rect          = pxRect;
    btnOption.fontMetrics   = QApplication::fontMetrics();
    btnOption.features      = QStyleOptionButton::Flat;
    btnOption.icon          = px;
    btnOption.iconSize      = px.size();
    btnOption.palette       = option.palette;

    QStyleOptionViewItem opt = option;
    drawBackground(painter, option, index);

    opt.rect.setHeight(option.rect.height());
    opt.rect.setWidth(opt.rect.width()-px.width());
    opt.state = opt.state & ~(QStyle::State_Selected | QStyle::State_MouseOver);//skip flags because background already painted

    QStyledItemDelegate::paint(painter, opt, index);
    QApplication::style()->drawControl(QStyle::CE_PushButton, &btnOption, painter);
}

void SideBarDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    QPoint p1 = option.rect.topLeft() + QPoint(option.rect.width()/2, 0);
    QPoint p2 = option.rect.topLeft() + QPoint(option.rect.width()/2, option.rect.height());

    QPainterPath path;
    QRect rect = option.rect;
    QRect roundRect(0, 0, 4, 4);

    path.moveTo(rect.topLeft()+QPoint(0, 2));
    roundRect.moveCenter(rect.topLeft()+QPoint(2, 2));
    path.arcTo(roundRect, 180.0, -90.0);
    path.lineTo(rect.topRight()-QPoint(2, 0));
    roundRect.moveCenter(rect.topRight()+QPoint(-2, 2));
    path.arcTo(roundRect, 90.0, -90.0);
    path.lineTo(rect.topRight()+QPoint(0, rect.height()-2));
    roundRect.moveCenter(rect.bottomRight()+QPoint(-2, -2));
    path.arcTo(roundRect, 0.0, -90.0);
    path.lineTo(rect.bottomLeft()+QPoint(2, 0));
    roundRect.moveCenter(rect.bottomLeft()+QPoint(2, -2));
    path.arcTo(roundRect, 270.0, -90.0);
    path.closeSubpath();

    QLinearGradient gradient(p1, p2);

    if (option.state & QStyle::State_Selected){
        gradient.setColorAt(0, option.palette.highlight().color().lighter());
        gradient.setColorAt(1, option.palette.highlight().color());
    }
    else if (option.state & QStyle::State_MouseOver){
        gradient.setColorAt(0, option.palette.highlight().color().lighter().lighter());
        gradient.setColorAt(1, option.palette.highlight().color().lighter().lighter());
    }
    else
        return;

    QBrush brush(gradient);

    brush.setColor(option.palette.highlight().color());
    brush.setTransform(option.palette.highlight().transform());

    painter->fillPath(path, brush);
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
