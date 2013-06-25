/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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

    connect(pushButton_ADD, SIGNAL(clicked()),          model, SLOT(newUC()));
    connect(pushButton_REM, SIGNAL(clicked()),          this,  SLOT(slotRemClicked()));
    connect(pushButton_CH,  SIGNAL(clicked()),          this,  SLOT(slotChangeClicked()));
    connect(pushButton_UP,  SIGNAL(clicked()),          this,  SLOT(slotUpClicked()));
    connect(pushButton_DOWN,SIGNAL(clicked()),          this,  SLOT(slotDownClicked()));
    connect(this,           SIGNAL(remUC(QModelIndex)),     model, SLOT(remUC(QModelIndex)));
    connect(this,           SIGNAL(changeUC(QModelIndex)),  model, SLOT(changeUC(QModelIndex)));
    connect(this,           SIGNAL(upUC(QModelIndex)),      model, SLOT(moveUp(QModelIndex)));
    connect(this,           SIGNAL(downUC(QModelIndex)),    model, SLOT(moveDown(QModelIndex)));
    connect(model,          SIGNAL(selectIndex(QModelIndex)), this, SLOT(slotSelect(QModelIndex)));
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
