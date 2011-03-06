/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UCMODEL_H
#define UCMODEL_H

#include <QObject>
#include <QDialog>
#include <QAbstractItemModel>

#include "ui_UIUserCommands.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include <dcpp/FavoriteManager.h>

class UCDialog: public QDialog, public Ui::UIUserCommands{
Q_OBJECT

public:
    UCDialog(QWidget *parent = NULL);

    unsigned long getCtx()  const;
    unsigned long getType();

    QString getName() const;
    QString getHub()  const;
    QString getCmd()  const;

    int type;

public Q_SLOTS:
    void updateLines();
    void updateType();
};

class UCItem{

public:

    UCItem(UCItem *parent = NULL);
    ~UCItem();

    void appendChild(UCItem *child);

    UCItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    UCItem *parent();

    QList<UCItem*> childItems;

    QString name;
    QString comm;
    QString hub;
    QString to;
    unsigned long ctx;
    unsigned long type;
    unsigned long id;
private:
    UCItem *parentItem;
};

class UCModel : public QAbstractItemModel
{
Q_OBJECT
public:
    explicit UCModel(QObject *parent = 0);
    virtual ~UCModel();

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

    void loadUC();
    void addUC(const dcpp::UserCommand &cmd);

signals:
    void selectIndex(const QModelIndex&);

public slots:
    void newUC();
    void changeUC(const QModelIndex&);
    void remUC(const QModelIndex&);
    void moveUp(const QModelIndex&);
    void moveDown(const QModelIndex&);

private:
    void initDlgFromItem(UCDialog&, const UCItem&);

    UCItem *rootItem;
};

#endif // UCMODEL_H
