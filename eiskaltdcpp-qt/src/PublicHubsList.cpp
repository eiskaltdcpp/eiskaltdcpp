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

#include "PublicHubsList.h"
#include "QtContext.h"
#include "WulforUtil.h"

#include <QInputDialog>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/DCPlusPlus.h"

using namespace dcpp;

PublicHubsList::PublicHubsList(QWidget *parent): QDialog(parent)
{
    setupUi(this);

    listWidget->addItems(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::HUBLIST_SERVERS))
                         .split(";", Qt::SkipEmptyParts));

    connect(pushButton_DOWN, &QPushButton::clicked, this, &PublicHubsList::slotDown);
    connect(pushButton_UP,   &QPushButton::clicked, this, &PublicHubsList::slotUp);
    connect(pushButton_ADD,  &QPushButton::clicked, this, &PublicHubsList::slotAdd);
    connect(pushButton_REM,  &QPushButton::clicked, this, &PublicHubsList::slotRem);
    connect(pushButton_EDIT, &QPushButton::clicked, this, &PublicHubsList::slotChange);
    connect(this, &QDialog::accepted, this, &PublicHubsList::slotAccepted);
}

void PublicHubsList::slotAccepted(){
    QString hubs = "";
    for (int i = 0; i < listWidget->count(); i++)
        hubs += (hubs.isEmpty()? "" : ";") + listWidget->item(i)->text();

    qtCtx()->dcCtx().getSettingsManager()->set(SettingsManager::HUBLIST_SERVERS, _tq(hubs));
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

    if (!currentRow)
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

    if (!currentRow)
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
