/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef TRANSFERVIEWMODEL_H
#define TRANSFERVIEWMODEL_H

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QMultiHash>
#include <QSize>

#define COLUMN_TRANSFER_USERS       0
#define COLUMN_TRANSFER_SPEED       1
#define COLUMN_TRANSFER_STATS       2
#define COLUMN_TRANSFER_SIZE        3
#define COLUMN_TRANSFER_TLEFT       4
#define COLUMN_TRANSFER_FNAME       5
#define COLUMN_TRANSFER_HOST        6
#define COLUMN_TRANSFER_IP          7
#define COLUMN_TRANSFER_ENCRYPTION  8

class TransferViewDelegate:
        public QStyledItemDelegate
{
    Q_OBJECT

public:
    TransferViewDelegate(QObject* = NULL);
    virtual ~TransferViewDelegate();

    virtual void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;

private Q_SLOTS:
    void wsVarValueChanged(const QString&, const QVariant &);

private:
    QColor download_bar_color;
    QColor upload_bar_color;
};

class TransferViewItem
{

public:
    TransferViewItem(const QList<QVariant> &data, TransferViewItem *parent = 0);
    TransferViewItem(const TransferViewItem&);
    void operator=(const TransferViewItem&);
    virtual ~TransferViewItem();

    void appendChild(TransferViewItem *child);

    TransferViewItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TransferViewItem *parent();
    void updateColumn(int, QVariant);

    QList<TransferViewItem*> childItems;

    bool download;
    bool fail;
    bool finished;
    QString cid;
    QString tth;
    QString target;
    qlonglong dpos;
    qlonglong delta;
    double percent;
    QList<QVariant> itemData;

private:

    TransferViewItem *parentItem;
};

class TransferViewModel: public QAbstractItemModel
{
    Q_OBJECT

    typedef QMap<QString, QVariant> VarMap;

public:
    TransferViewModel(QObject* = NULL);
    virtual ~TransferViewModel();

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
    bool findTransfer(const QString &, bool, TransferViewItem**);
    /** */
    bool findParent(const QString&, TransferViewItem**, bool = true);
    /** */
    TransferViewItem *getParent(const QString &target, const VarMap &params);

    /** */
    QModelIndex createIndexForItem(TransferViewItem*);

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

    /** */
    void addConnection(const VarMap&);
    /** */
    void initTransfer(const VarMap&);
    /** */
    void updateTransfer(const VarMap&);
    /** */
    void removeTransfer(const VarMap&);
    /** */
    void updateTransferPos(const VarMap&, qint64);
    /** */
    void finishParent(const VarMap&);
    /** */
    void updateParents();
    /** Just resort*/
    virtual void sort() { sort(sortColumn, sortOrder); }

private:
    inline QString      vstr(const QVariant &var) { return var.toString(); }
    inline int          vint(const QVariant &var) { return var.toInt(); }
    inline double       vdbl(const QVariant &var) { return var.toDouble(); }
    inline qlonglong    vlng(const QVariant &var) { return var.toLongLong(); }
    inline bool         vbol(const QVariant &var) { return var.toBool(); }

    /** */
    void updateParent(TransferViewItem*);
    /** */
    void moveTransfer(TransferViewItem*, TransferViewItem*, TransferViewItem*);
    /** */
    QMultiHash<QString, TransferViewItem*> transfer_hash;
    /** */
    QMap<QString, int> column_map;
    /** */
    int sortColumn;
    /** */
    Qt::SortOrder sortOrder;
    /** */
    TransferViewItem *rootItem;
    /** */
    bool iconsScaled;
    /** */
    QSize iconsSize;
};

#endif // TRANSFERVIEWMODEL_H
