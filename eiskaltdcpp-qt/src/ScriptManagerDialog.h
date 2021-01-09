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
#include <QStringList>

#include "ui_UIScriptManager.h"

class ScriptManagerModel;

class ScriptManagerDialog :
        public QDialog,
        private Ui::UIScriptManager
{
Q_OBJECT
public:
    explicit ScriptManagerDialog(QWidget *parent = nullptr);
    ~ScriptManagerDialog() override;

private Q_SLOTS:
    void slotSetChangedAction(int index);

private:
    ScriptManagerModel *model;
};

class ScriptManagerItem{

public:
    ScriptManagerItem(ScriptManagerItem* = nullptr);
    virtual ~ScriptManagerItem();

    void appendChild(ScriptManagerItem *child);

    ScriptManagerItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    ScriptManagerItem *parent();
    QList<ScriptManagerItem*> childItems;

    bool isOn;
    QString desc;
    QString auth;
    QString name;
    QString path;
    QIcon   icon;
private:
    ScriptManagerItem *parentItem;
};

class ScriptManagerModel: public QAbstractItemModel{
    Q_OBJECT
public:
    ScriptManagerModel(QObject* = nullptr);
    ~ScriptManagerModel() override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int, Qt::Orientation, int) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & parent) const override;

public Q_SLOTS:
    void save();

private:
    void load();
    void loadDir(const QString&);

    ScriptManagerItem *rootItem;
    QStringList enabled;
};
