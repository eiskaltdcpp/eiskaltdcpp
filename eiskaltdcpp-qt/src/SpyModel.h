/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SPYMODEL_H
#define SPYMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QHash>

#include <boost/pool/object_pool.hpp>

#define COLUMN_SPY_COUNT        0
#define COLUMN_SPY_STRING       1

class SpyItem
{

public:
    SpyItem(const QList<QVariant> &data, SpyItem *parent = 0);
    ~SpyItem();

    void appendChild(SpyItem *child);

    SpyItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    SpyItem *parent() const;

    unsigned count;

    bool isTTH;

    QList<SpyItem*> childItems;
private:

    QList<QVariant> itemData;
    SpyItem *parentItem;
};

class SpyModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    SpyModel(QObject *parent = 0);
    virtual ~SpyModel();

    /** */
    QVariant data(const QModelIndex &, int) const;
    /** */
    Qt::ItemFlags flags(const QModelIndex &) const;
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
    virtual void sort();

    /** Clear model and redraw view*/
    void clearModel();

private Q_SLOTS:
    /** */
    void addResult(const QString &file, bool isTTH);

private:
    /** */
    boost::object_pool<SpyItem> pool;
    /** */
    SpyItem *rootItem;
    int sortColumn;
    Qt::SortOrder sortOrder;
    /** */
    QHash<QString, SpyItem*> hashes;
};

#endif // SPYMODEL_H
