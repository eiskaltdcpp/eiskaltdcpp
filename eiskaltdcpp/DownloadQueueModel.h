/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOWNLOADQUEUEMODEL_H
#define DOWNLOADQUEUEMODEL_H

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QSize>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"
#include "dcpp/DirectoryListing.h"

#define COLUMN_DOWNLOADQUEUE_NAME       0
#define COLUMN_DOWNLOADQUEUE_STATUS     1
#define COLUMN_DOWNLOADQUEUE_SIZE       2
#define COLUMN_DOWNLOADQUEUE_DOWN       3
#define COLUMN_DOWNLOADQUEUE_PRIO       4
#define COLUMN_DOWNLOADQUEUE_USER       5
#define COLUMN_DOWNLOADQUEUE_PATH       6
#define COLUMN_DOWNLOADQUEUE_ESIZE      7
#define COLUMN_DOWNLOADQUEUE_ERR        8
#define COLUMN_DOWNLOADQUEUE_ADDED      9
#define COLUMN_DOWNLOADQUEUE_TTH        10

class DownloadQueueDelegate:
        public QStyledItemDelegate
{
    Q_OBJECT

public:
    DownloadQueueDelegate(QObject* = NULL);
    virtual ~DownloadQueueDelegate();

    virtual void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
};

class DownloadQueueItem
{

public:
    DownloadQueueItem(const QList<QVariant> &data, DownloadQueueItem *parent = 0);
    DownloadQueueItem(const DownloadQueueItem&);
    void operator=(const DownloadQueueItem&);
    virtual ~DownloadQueueItem();

    void appendChild(DownloadQueueItem *child);

    DownloadQueueItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    DownloadQueueItem *parent();
    void updateColumn(int, QVariant);

    QList<DownloadQueueItem*> childItems;

    DownloadQueueItem *nextSibling();

    bool dir;
private:

    QList<QVariant> itemData;
    DownloadQueueItem *parentItem;
};

class DownloadQueueModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    DownloadQueueModel(QObject* = NULL);
    ~DownloadQueueModel();

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

    /** */
    DownloadQueueItem *addItem(const QMap<QString, QVariant> &);
    /** */
    void updItem(const QMap<QString, QVariant> &);
    /** */
    bool remItem(const QMap<QString, QVariant> &);

    /** */
    void setRootElem(DownloadQueueItem *root, bool delete_old = true, bool controlNull = true);
    /** */
    DownloadQueueItem *getRootElem() const;
    /** */
    void setIconsScaled(bool, const QSize&);

    /** */
    QModelIndex createIndexForItem(DownloadQueueItem*);
    /** */
    DownloadQueueItem *createPath(const QString&);

    /** */
    int getSortColumn() const;
    /** */
    void setSortColumn(int);
    /** */
    Qt::SortOrder getSortOrder() const;
    /** */
    void setSortOrder(Qt::SortOrder);

    /** */
    void clear();

public Q_SLOTS:
    void repaint();

Q_SIGNALS:
    void rowRemoved(const QModelIndex &parent);

private:
    DownloadQueueItem *findTarget(const DownloadQueueItem*, const QString&);
    /** */
    int sortColumn;
    /** */
    Qt::SortOrder sortOrder;
    /** */
    DownloadQueueItem *rootItem;
    /** */
    bool iconsScaled;
    /** */
    QSize iconsSize;
};

#endif //DOWNLOADQUEUEMODEL_H
