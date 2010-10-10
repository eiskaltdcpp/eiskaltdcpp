/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QAbstractItemModel>
#include <QMap>
#include <QStack>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"

class SideBarDelegate:
        public QStyledItemDelegate
{
    Q_OBJECT

public:
    SideBarDelegate(QObject* = NULL);
    virtual ~SideBarDelegate();

    virtual void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
};

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
        return childItems.size();
    }

    int columnCount() const{
        return 2;
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
    bool isRootItem(const SideBarItem *) const;
    ArenaWidget::Role rootItemRole(const SideBarItem *) const;
    void mapped(ArenaWidget *awgt);
    void redraw() { emit layoutChanged(); }

public slots:
    void slotIndexClicked(const QModelIndex&);

signals:
    void mapWidget(ArenaWidget*);
    void selectIndex(const QModelIndex&);

private:
    void historyPop();
    void historyPush(ArenaWidget*);
    bool historyAtTop(ArenaWidget*);
    void historyPurge(ArenaWidget*);

    QMap <ArenaWidget::Role, SideBarItem*> roots;
    QMap <ArenaWidget*, SideBarItem*> items;
    SideBarItem *rootItem;

    QStack<ArenaWidget*> historyStack;
};

#endif // SIDEBAR_H
