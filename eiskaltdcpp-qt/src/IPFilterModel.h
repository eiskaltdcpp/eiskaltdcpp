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
#include <QHash>

#define COLUMN_RULE_NAME        0
#define COLUMN_RULE_DIRECTION   1

class IPFilterModelItem{

public:

    IPFilterModelItem(const QList<QVariant> &data, IPFilterModelItem *parent = nullptr);
    ~IPFilterModelItem();

    void appendChild(IPFilterModelItem *child);

    IPFilterModelItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    IPFilterModelItem *parent();

    QList<IPFilterModelItem*> childItems;

    void updateColumn(int, QVariant);

private:
    QList<QVariant> itemData;
    IPFilterModelItem *parentItem;
};

class IPFilterModel : public QAbstractItemModel{
Q_OBJECT

public:
    IPFilterModel(QObject *parent = nullptr);
    ~IPFilterModel() override;

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

    /** */
    void addResult(const QString &, const QString &);

    /** */
    void clearModel();

    /** */
    void repaint();

    /** */
    void moveUp(const QModelIndex &);
    /** */
    void moveDown(const QModelIndex &);

    /** */
    void removeItem(IPFilterModelItem*);

private:

    /** */
    void moveIndex(const QModelIndex &, bool down);

    QHash<QString, IPFilterModelItem*> rules;
    IPFilterModelItem *rootItem;

    void reset();
};
