/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QSize>

#include "dcpp/stdinc.h"
#include "dcpp/User.h"
#include "dcpp/DirectoryListing.h"

static const unsigned COLUMN_DOWNLOADQUEUE_NAME     = 0;
static const unsigned COLUMN_DOWNLOADQUEUE_STATUS   = 1;
static const unsigned COLUMN_DOWNLOADQUEUE_SIZE     = 2;
static const unsigned COLUMN_DOWNLOADQUEUE_DOWN     = 3;
static const unsigned COLUMN_DOWNLOADQUEUE_PRIO     = 4;
static const unsigned COLUMN_DOWNLOADQUEUE_USER     = 5;
static const unsigned COLUMN_DOWNLOADQUEUE_PATH     = 6;
static const unsigned COLUMN_DOWNLOADQUEUE_ESIZE    = 7;
static const unsigned COLUMN_DOWNLOADQUEUE_ERR      = 8;
static const unsigned COLUMN_DOWNLOADQUEUE_ADDED    = 9;
static const unsigned COLUMN_DOWNLOADQUEUE_TTH      = 10;

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
    const QVariant &data(int column) const;
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

class DownloadQueueModelPrivate;

class DownloadQueueModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    DownloadQueueModel(QObject* = NULL);
    virtual ~DownloadQueueModel();

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
    virtual void sort() { sort(getSortColumn(), getSortOrder()); }

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
    void needExpand(const QModelIndex &item);
    void updateStats(quint64 files, quint64 size);

private:
    /** */
    DownloadQueueItem *findTarget(const DownloadQueueItem*, const QString&);

    Q_DECLARE_PRIVATE(DownloadQueueModel)

    DownloadQueueModelPrivate *d_ptr;
};
