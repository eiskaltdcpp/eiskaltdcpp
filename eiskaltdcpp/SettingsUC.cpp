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
    connect(this,           SIGNAL(remUC(QModelIndex)), model, SLOT(remUC(QModelIndex)));
}

SettingsUC::~SettingsUC(){
    model->deleteLater();
}

void SettingsUC::ok(){

}

QModelIndex SettingsUC::selectedIndex(){
    QItemSelectionModel *s_m = treeView->selectionModel();
    QModelIndexList list = s_m->selectedRows(0);

    if (list.size() > 0)
        return list.at(0);
    else
        return QModelIndex();
}

void SettingsUC::slotRemClicked(){
    emit remUC(selectedIndex());
}
