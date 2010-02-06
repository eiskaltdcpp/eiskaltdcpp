/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include <QAbstractItemModel>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QRegExp>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SearchResult.h"
#include "dcpp/SearchManager.h"

#include <boost/pool/object_pool.hpp>

#define COLUMN_COUNT            0
#define COLUMN_FILENAME         1
#define COLUMN_EXTENSION        2
#define COLUMN_SIZE             3
#define COLUMN_ESIZE            4
#define COLUMN_TTH              5
#define COLUMN_PATH             6
#define COLUMN_NICK             7
#define COLUMN_FREESLOTS        8
#define COLUMN_ALLSLOTS         9
#define COLUMN_IP               10
#define COLUMN_HUB              11
#define COLUMN_HOST             12

class SearchListException{
    public:

    enum Type{
        Sort=0,
        Add,
        Unkn
    };

        SearchListException();
        SearchListException(const SearchListException&);
        SearchListException(const QString& message, Type type);
        virtual ~SearchListException();

        SearchListException &operator=(const SearchListException&);

        QString message;
        Type type;
};

class SearchItem
{

public:
    SearchItem(const QList<QVariant> &data, SearchItem *parent = 0);
    ~SearchItem();

    void appendChild(SearchItem *child);

    SearchItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    SearchItem *parent() const;

    unsigned count;

    QString cid;

    bool isDir;

    QList<SearchItem*> childItems;
private:

    QList<QVariant> itemData;
    SearchItem *parentItem;
};

class SearchModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    SearchModel(QObject *parent = 0);
    ~SearchModel();

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
    QModelIndex createIndexForItem(SearchItem*);

    void setFilterRole(int);

    /** */
    void addResult(
            const QString &file,
            qulonglong size,
            const QString &tth,
            const QString &path,
            const QString &nick,
            const int free_slots,
            const int all_slots,
            const QString &ip,
            const QString &hub,
            const QString &host,
            const QString &cid,
            const bool isDir);

    /** */
    void addResultPtr(QMap<QString, QVariant>&);
    /** */
    int getSortColumn() const;
    /** */
    void setSortColumn(int);
    /** */
    Qt::SortOrder getSortOrder() const;
    /** */
    void setSortOrder(Qt::SortOrder);

    /** Clear model and redraw view*/
    void clearModel();
    /** */
    void removeItem(const SearchItem*);

    /** */
    void repaint();

private:
    /** */
    boost::object_pool<SearchItem> pool;
    /** */
    bool okToFind(const SearchItem*);
    /** */
    int filterRole;
    /** */
    int sortColumn;
    /** */
    Qt::SortOrder sortOrder;
    /** */
    SearchItem *rootItem;
    /** */
    QHash<QString, SearchItem*> tths;
};

#endif
