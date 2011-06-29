/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FBMODEL_H
#define FBMODEL_H

#include <QAbstractItemModel>
#include <QSize>
#include <QHash>
#include <QMap>

#include "PoolItem.h"

#include "dcpp/stdinc.h"
#include "dcpp/User.h"
#include "dcpp/DirectoryListing.h"

#define COLUMN_FILEBROWSER_NAME     0
#define COLUMN_FILEBROWSER_SIZE     1
#define COLUMN_FILEBROWSER_ESIZE    2
#define COLUMN_FILEBROWSER_TTH      3


class FileBrowserItem: public PoolItem<FileBrowserItem>
{

public:
    FileBrowserItem(const QList<QVariant> &data, FileBrowserItem *parent = 0);
    FileBrowserItem(const FileBrowserItem&);
    void operator=(const FileBrowserItem&);
    virtual ~FileBrowserItem();

    void appendChild(FileBrowserItem *child);

    FileBrowserItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    FileBrowserItem *parent();
    void updateColumn(int, QVariant);

    QList<FileBrowserItem*> childItems;

    FileBrowserItem *nextSibling();

    dcpp::DirectoryListing::Directory *dir;
    dcpp::DirectoryListing::File *file;
    bool isDuplicate;
private:

    QList<QVariant> itemData;
    FileBrowserItem *parentItem;
};

class FileBrowserModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    FileBrowserModel(QObject* = NULL);
    virtual ~FileBrowserModel();

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
    virtual void sort() { sort(sortColumn, sortOrder); }

    /** */
    void setRootElem(FileBrowserItem *root, bool delete_old = true, bool controlNull = true);
    /** */
    FileBrowserItem *getRootElem() const;
    /** */
    void setIconsScaled(bool, const QSize&);

    /** */
    QString createRemotePath(FileBrowserItem *) const;
    /** */
    FileBrowserItem *createRootForPath(const QString&, FileBrowserItem *pathRoot = NULL);
    /** */
    QModelIndex createIndexForItem(FileBrowserItem*);

    /** */
    void loadRestrictions();
    /** */
    void updateRestriction(QModelIndex&, unsigned);

    /** */
    int getSortColumn() const;
    /** */
    void setSortColumn(int);
    /** */
    Qt::SortOrder getSortOrder() const;
    /** */
    void setSortOrder(Qt::SortOrder);

    /** */
    void highlightDuplicates();

    /** */
    void clear();
    /** */
    void repaint();

signals:
    void rootChanged(FileBrowserItem*,FileBrowserItem*);

private:
    /** */
    int sortColumn;
    /** */
    Qt::SortOrder sortOrder;
    /** */
    FileBrowserItem *rootItem;
    /** */
    bool iconsScaled;
    /** */
    QSize iconsSize;
    /** */
    QHash<QString, dcpp::DirectoryListing::File*> hash;
    /** */
    QMap<QString, unsigned> restrict_map;
    /** */
    bool restrictionsLoaded;
};

#endif // FBMODEL_H
