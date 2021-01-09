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
#include <QList>
#include <QHash>

#define COLUMN_SPY_COUNT        0
#define COLUMN_SPY_STRING       1

class SpyItem
{

public:
    SpyItem(const QList<QVariant> &data, SpyItem *parent = nullptr);
    ~SpyItem();

    void appendChild(SpyItem *child);
    void insertChild(SpyItem *item, int pos = 0);
    void moveUp(SpyItem *child);

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

    SpyModel(QObject *parent = nullptr);
    ~SpyModel() override;

    /** */
    QVariant data(const QModelIndex &, int) const override;
    /** */
    Qt::ItemFlags flags(const QModelIndex &) const override;
    /** */
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    /** */
    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const override;
    /** */
    QModelIndex parent(const QModelIndex &index) const override;
    /** */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /** */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /** sort list */
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    virtual void sort();
    /** set autosort list */
    void setSort(bool sort);

    /** Clear model and redraw view*/
    void clearModel();

private Q_SLOTS:
    /** */
    void addResult(const QString &file, bool isTTH);

private:
    /** */
    SpyItem *rootItem;
    bool isSort;
    int sortColumn;
    Qt::SortOrder sortOrder;
    /** */
    QHash<QString, SpyItem*> hashes;

    void reset();
};
