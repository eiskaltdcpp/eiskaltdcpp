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
    SearchBlackListDelegate(QObject* = nullptr);
    ~SearchBlackListDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
};

class SearchBlackListItem{

public:
    SearchBlackListItem(SearchBlackListItem* = nullptr);
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
    SearchBlackListModel(QObject * parent = nullptr);
    ~SearchBlackListModel() override;

    int rowCount(const QModelIndex & index = QModelIndex()) const override;
    int columnCount(const QModelIndex & index = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

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
    SearchBlackListDialog(QWidget* = nullptr);
    ~SearchBlackListDialog() override;

protected:
    void resizeEvent(QResizeEvent *) override;

private Q_SLOTS:
    void ok();
    void slotContextMenu();

private:
    SearchBlackListModel *model;
};
