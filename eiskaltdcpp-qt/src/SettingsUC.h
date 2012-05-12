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
#include "ui_UISettingsUC.h"

class UCModel;

class SettingsUC :
        public QWidget,
        private Ui::UISettingsUC
{
Q_OBJECT
public:
    explicit SettingsUC(QWidget *parent = 0);
    virtual ~SettingsUC();

signals:
    void remUC(const QModelIndex&);
    void changeUC(const QModelIndex&);
    void upUC(const QModelIndex&);
    void downUC(const QModelIndex&);

public slots:
    void ok();

private slots:
    void slotRemClicked();
    void slotChangeClicked();
    void slotDownClicked();
    void slotUpClicked();
    void slotSelect(const QModelIndex&);

private:
    QModelIndex selectedIndex();

    UCModel *model;
};
