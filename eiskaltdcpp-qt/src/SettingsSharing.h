/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QShowEvent>
#include <QHeaderView>

#include "ui_UISettingsSharing.h"

class ShareDirModel: public QFileSystemModel {
    Q_OBJECT
public:

    ShareDirModel(QObject* = nullptr);
    virtual ~ShareDirModel();

    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);

    void setAlias(const QModelIndex&, const QString &);
    void beginExpanding();
    QString filePath( const QModelIndex & index ) const;

Q_SIGNALS:
    void getName(QModelIndex);
    void expandMe(QModelIndex);
private:
    QSet<QString> checked;
};

class SettingsSharing :
        public QWidget,
        private Ui::UISettingsSharing
{
    Q_OBJECT
public:
    SettingsSharing(QWidget* = nullptr);
    virtual ~SettingsSharing();
protected:
    virtual void showEvent(QShowEvent *);

public Q_SLOTS:
    void ok();

private slots:
    void slotRecreateShare();
    void slotShareHidden(bool);
    void slotGetName(QModelIndex);
    void slotHeaderMenu();
    void slotAddExeption();
    void slotEditExeption();
    void slotDeleteExeption();
    void slotAddDirExeption();
    void slotSimpleShareModeChanged();
    void slotContextMenu(const QPoint&);

private:
    void init();
    void updateShareView();

    ShareDirModel *model;
};
