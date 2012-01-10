/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SETTINGSSHORTCUTS_H
#define SETTINGSSHORTCUTS_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QHash>

#include "ui_UISettingsShortcuts.h"
#include "ShortcutEdit.h"

class ShortcutItem{

public:
    ShortcutItem(ShortcutItem* = NULL);
    virtual ~ShortcutItem();

    void appendChild(ShortcutItem *child);

    ShortcutItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    ShortcutItem *parent();
    QList<ShortcutItem*> childItems;

    QString title;
    QString shortcut;
private:
    ShortcutItem *parentItem;
};

class ShortcutsModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ShortcutsModel(QObject * parent = 0);
    virtual ~ShortcutsModel();

    virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void repaint() { emit layoutChanged(); }
    void save();

private:
    ShortcutItem *rootItem;
    QHash<ShortcutItem*, QString> items;
};

class SettingsShortcuts : public QWidget, private Ui::UISettingsShortcuts
{
    Q_OBJECT
public:
    explicit SettingsShortcuts(QWidget *parent = 0);
    virtual ~SettingsShortcuts();

public Q_SLOTS:
    void ok();

private Q_SLOTS:
    void slotIndexClicked(const QModelIndex&);

private:
    ShortcutsModel *model;
};

#endif // SETTINGSSHORTCUTS_H
