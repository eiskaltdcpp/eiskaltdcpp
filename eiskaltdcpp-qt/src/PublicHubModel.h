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
#include <QSortFilterProxyModel>

#include "dcpp/stdinc.h"
#include "dcpp/FavoriteManager.h"

class PublicHubProxyModel: public QSortFilterProxyModel {
    Q_OBJECT

public:
    virtual void sort(int column, Qt::SortOrder order);
};

#define COLUMN_PHUB_NAME                0
#define COLUMN_PHUB_DESC                1
#define COLUMN_PHUB_USERS               2
#define COLUMN_PHUB_ADDRESS             3
#define COLUMN_PHUB_COUNTRY             4
#define COLUMN_PHUB_SHARED              5
#define COLUMN_PHUB_MINSHARE            6
#define COLUMN_PHUB_MINSLOTS            7
#define COLUMN_PHUB_MAXHUBS             8
#define COLUMN_PHUB_MAXUSERS            9
#define COLUMN_PHUB_REL                 10
#define COLUMN_PHUB_RATING              11


class PublicHubItem{

public:

    PublicHubItem(const QList<QVariant> &data, PublicHubItem *parent = NULL);
    ~PublicHubItem();

    void appendChild(PublicHubItem *child);

    PublicHubItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    PublicHubItem *parent();

    QList<PublicHubItem*> childItems;

    void updateColumn(unsigned, QVariant);

    dcpp::HubEntry *entry;

private:
    QList<QVariant> itemData;
    PublicHubItem *parentItem;
};

class PublicHubModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    PublicHubModel(QObject *parent = 0);
    virtual ~PublicHubModel();

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

    /** */
    void addResult(const QList<QVariant> &data, dcpp::HubEntry *);

    /** Clear the model and redraw it*/
    void clearModel();

private:

    PublicHubItem *rootItem;

    int sortColumn;
    Qt::SortOrder sortOrder;
};
