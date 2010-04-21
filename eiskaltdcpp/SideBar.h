#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QAbstractItemModel>
#include <QMap>

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"

class SideBarItem
{

public:
    SideBarItem(ArenaWidget *wgt = NULL, SideBarItem *parent = NULL): parentItem(parent), awgt(wgt) {}
    virtual ~SideBarItem() { qDeleteAll(childItems); }

    void appendChild(SideBarItem *i){
        if (!childItems.contains(i))
            childItems.push_back(i);
    }

    SideBarItem *child(int row){
       return childItems.value(row);
    }

    int childCount() const{
        return childItems.count();
    }

    int columnCount() const{
        return 1;
    }

    int row() const{
        if (parentItem)
            return parentItem->childItems.indexOf(const_cast<SideBarItem*>(this));

        return 0;
    }

    ArenaWidget *getWidget() const { return awgt; }
    void setWdget(ArenaWidget *awgt) { this->awgt = awgt; }

    SideBarItem *parent() {return parentItem; }

    QPixmap pixmap;
    QString title;

    QList<SideBarItem*> childItems;

private:
    SideBarItem *parentItem;
    ArenaWidget *awgt;
};

class SideBarModel :
        public QAbstractItemModel,
        public ArenaWidgetContainer
{
Q_OBJECT

public:
    explicit SideBarModel(QObject *parent = 0);
    virtual ~SideBarModel();

    /** */
    QVariant data(const QModelIndex &, int) const;
    /** */
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;
    /** */
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const;
    /** */
    QModelIndex parent(const QModelIndex &index) const;
    /** */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /** */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    /** sort list */
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void removeWidget(ArenaWidget *awgt);
    void insertWidget(ArenaWidget *awgt);
    bool hasWidget(ArenaWidget *awgt) const;
    void mapped(ArenaWidget *awgt);

public slots:
    void slotIndexClicked(const QModelIndex&);

signals:
    void mapWidget(ArenaWidget*);
    void selectIndex(const QModelIndex&);

private:
    QMap <ArenaWidget::Role, SideBarItem*> roots;
    QMap <ArenaWidget*, SideBarItem*> items;
    SideBarItem *rootItem;
};

#endif // SIDEBAR_H
