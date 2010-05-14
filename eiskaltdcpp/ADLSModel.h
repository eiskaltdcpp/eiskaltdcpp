/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADLSModel_H
#define ADLSModel_H

#include <QAbstractItemModel>

#define COLUMN_CHECK            0
#define COLUMN_SSTRING          1
#define COLUMN_TYPE             2
#define COLUMN_DIRECTORY    3
#define COLUMN_MAXSIZE      4
#define COLUMN_MINSIZE          5

class ADLSItem{

public:

    ADLSItem(const QList<QVariant> &data, ADLSItem *parent = NULL);
    ~ADLSItem();

    void appendChild(ADLSItem *child);

    ADLSItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    ADLSItem *parent();

    QList<ADLSItem*> childItems;

    void updateColumn(unsigned, QVariant);

private:
    QList<QVariant> itemData;
    ADLSItem *parentItem;
};

class ADLSModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ADLSModel(QObject *parent = 0);
    virtual ~ADLSModel();

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

    virtual Qt::DropActions supportedDragActions() const { return Qt::MoveAction; }

    virtual bool removeRow(int row, const QModelIndex &parent);
    virtual bool insertRow(int row, const QModelIndex &parent);
    virtual bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    virtual bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

    /** */
    void addResult(QList<QVariant> &data);
    /** */
    const QList<ADLSItem*> &getItems();

    /** Clear the model and redraw it*/
    void clearModel();

    /** */
    bool removeItem(const QModelIndex&);
    /** */
    bool removeItem(const ADLSItem*);

    QModelIndex moveUp(const QModelIndex &);
    QModelIndex moveDown(const QModelIndex &);

    /** */
    void repaint();

private:
    Qt::DropActions supportedDropActions() const;

    ADLSItem *rootItem;

    int sortColumn;
    Qt::SortOrder sortOrder;
};

#endif // ADLSModel_H
