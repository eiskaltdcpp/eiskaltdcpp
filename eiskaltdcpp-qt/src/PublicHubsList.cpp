/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "PublicHubsList.h"
#include "WulforUtil.h"

#include <QInputDialog>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/SettingsManager.h"

using namespace dcpp;

PublicHubsList::PublicHubsList(QWidget *parent): QDialog(parent)
{
    setupUi(this);

    listWidget->addItems(_q(SettingsManager::getInstance()->get(SettingsManager::HUBLIST_SERVERS))
                         .split(";", QString::SkipEmptyParts));

    connect(pushButton_DOWN, SIGNAL(clicked()), this, SLOT(slotDown()));
    connect(pushButton_UP,   SIGNAL(clicked()), this, SLOT(slotUp()));
    connect(pushButton_ADD,  SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(pushButton_REM,  SIGNAL(clicked()), this, SLOT(slotRem()));
    connect(pushButton_EDIT, SIGNAL(clicked()), this, SLOT(slotChange()));
    connect(this, SIGNAL(accepted()), this, SLOT(slotAccepted()));
}

void PublicHubsList::slotAccepted(){
    QString hubs = "";
    for (int i = 0; i < listWidget->count(); i++)
        hubs += (hubs.isEmpty()? "" : ";") + listWidget->item(i)->text();

    SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, _tq(hubs));
}

void PublicHubsList::slotDown(){
    int currentRow = listWidget->currentRow();

    if (currentRow > listWidget->count()-1)
        return;

    QListWidgetItem *currentItem = listWidget->takeItem(currentRow);

    listWidget->insertItem(currentRow + 1, currentItem);
    listWidget->setCurrentRow(currentRow + 1);
}

void PublicHubsList::slotUp(){
    int currentRow = listWidget->currentRow();

    if (currentRow == 0)
        return;

    QListWidgetItem *currentItem = listWidget->takeItem(currentRow);

    listWidget->insertItem(currentRow - 1, currentItem);
    listWidget->setCurrentRow(currentRow - 1);
}

void PublicHubsList::slotAdd(){
    bool ok = false;
    QString link = QInputDialog::getText(this, tr("Public hub"), tr("Link"), QLineEdit::Normal, "", &ok);

    if (ok && !link.isEmpty())
        listWidget->addItem(link);
}

void PublicHubsList::slotRem(){
    int currentRow = listWidget->currentRow();

    if (currentRow == 0)
        return;

    QListWidgetItem *currentItem = listWidget->takeItem(currentRow);

    delete currentItem;
}

void PublicHubsList::slotChange(){
    QListWidgetItem *item = listWidget->currentItem();

    if (!item)
        return;

    bool ok = false;
    QString link = QInputDialog::getText(this, tr("Public hub"), tr("Link"), QLineEdit::Normal, item->text(), &ok);

    if (ok && !link.isEmpty())
        item->setText(link);
}
