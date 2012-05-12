/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#pragma once

#include <QDialog>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

#include "ui_UISearchBlacklist.h"

class SearchBlackListDelegate:
        public QStyledItemDelegate
{
    Q_OBJECT

public:
    SearchBlackListDelegate(QObject* = NULL);
    virtual ~SearchBlackListDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
};

class SearchBlackListItem{

public:
    SearchBlackListItem(SearchBlackListItem* = NULL);
    virtual ~SearchBlackListItem();

    void appendChild(SearchBlackListItem *child);

    SearchBlackListItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    SearchBlackListItem *parent();
    QList<SearchBlackListItem*> childItems;
    QVariant data(int column) const;

    QString title;
    int argument;
private:
    SearchBlackListItem *parentItem;
    QList<QVariant> itemData;
};

#define COLUMN_SBL_KEY          0
#define COLUMN_SBL_TYPE         1

class SearchBlackListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    SearchBlackListModel(QObject * parent = 0);
    virtual ~SearchBlackListModel();

    virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex addEmptyItem();

    void repaint() { emit layoutChanged(); }
    void save();

    int  getSortColumn() const;
    void setSortColumn(int);

private:
    SearchBlackListItem *rootItem;
    Qt::SortOrder sortOrder;
    int sortColumn;
};

class SearchBlackListDialog:
        public QDialog,
        protected Ui::UISearchBlacklistDialog
{
    Q_OBJECT
public:
    SearchBlackListDialog(QWidget* = NULL);
    virtual ~SearchBlackListDialog();

protected:
    virtual void resizeEvent(QResizeEvent *);

private Q_SLOTS:
    void ok();
    void slotContextMenu();

private:
    SearchBlackListModel *model;
};
