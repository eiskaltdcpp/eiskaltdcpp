/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "SettingsUC.h"
#include "UCModel.h"

#include <QItemSelectionModel>

#include <dcpp/FavoriteManager.h>

SettingsUC::SettingsUC(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    model = new UCModel(this);
    model->loadUC();

    treeView->setModel(model);

    connect(pushButton_ADD, &QPushButton::clicked,          model, &UCModel::newUC);
    connect(pushButton_REM, &QPushButton::clicked,          this,  &SettingsUC::slotRemClicked);
    connect(pushButton_CH,  &QPushButton::clicked,          this,  &SettingsUC::slotChangeClicked);
    connect(pushButton_UP,  &QPushButton::clicked,          this,  &SettingsUC::slotUpClicked);
    connect(pushButton_DOWN, &QPushButton::clicked,          this,  &SettingsUC::slotDownClicked);
    connect(this,           &SettingsUC::remUC,     model, &UCModel::remUC);
    connect(this,           &SettingsUC::changeUC,  model, &UCModel::changeUC);
    connect(this,           &SettingsUC::upUC,      model, &UCModel::moveUp);
    connect(this,           &SettingsUC::downUC,    model, &UCModel::moveDown);
    connect(model,          &UCModel::selectIndex, this, &SettingsUC::slotSelect);
}

SettingsUC::~SettingsUC(){
    model->deleteLater();
}

void SettingsUC::ok(){

}

QModelIndex SettingsUC::selectedIndex(){
    QItemSelectionModel *s_m = treeView->selectionModel();
    QModelIndexList list = s_m->selectedRows(0);

    if (!list.isEmpty())
        return list.at(0);
    else
        return QModelIndex();
}

void SettingsUC::slotRemClicked(){
    emit remUC(selectedIndex());
}

void SettingsUC::slotChangeClicked(){
    emit changeUC(selectedIndex());
}

void SettingsUC::slotUpClicked(){
    emit upUC(selectedIndex());
}

void SettingsUC::slotDownClicked(){
    emit downUC(selectedIndex());
}

void SettingsUC::slotSelect(const QModelIndex &i){
    QItemSelectionModel *s_m = treeView->selectionModel();

    s_m->select(i, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}
