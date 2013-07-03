/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ActionCustomizer.h"

ActionCustomizer::ActionCustomizer(const QList<QAction*> &available, const QList<QAction*> &enabled, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    setWindowTitle(tr("Customize actions"));

    foreach (QAction *act, enabled){
        if (!act)
            continue;

        QListWidgetItem *item = new QListWidgetItem(act->icon(), act->text(), listWidget_ENABLED, 0);

        if (act->isSeparator())
            item->setText(tr("-- Separator --"));

        enabled_items.insert(item, act);
    }

    foreach (QAction *act, available){
        if (!act || enabled.contains(act))
            continue;

        QListWidgetItem *item = new QListWidgetItem(act->icon(), act->text(), listWidget_AVAILABLE, 0);

        if (act->isSeparator())
            item->setText(tr("-- Separator --"));

        avail_items.insert(item, act);
    }

    connect(pushButton_DOWN,    SIGNAL(clicked()), this, SLOT(moveDown()));
    connect(pushButton_UP,      SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(pushButton_REM,     SIGNAL(clicked()), this, SLOT(moveToAvailable()));
    connect(pushButton_ADD,     SIGNAL(clicked()), this, SLOT(moveToEnabled()));
    connect(buttonBox,          SIGNAL(accepted()),this, SLOT(accepted()));
}

void ActionCustomizer::accepted() {
    QList<QAction*> enabled;

    for (int i = 0; i < listWidget_ENABLED->count(); i++)
        enabled.push_back(enabled_items[listWidget_ENABLED->item(i)]);

    emit done(enabled);
}

void ActionCustomizer::moveDown(){
    int currentRow = listWidget_ENABLED->currentRow();

    if (currentRow > listWidget_ENABLED->count()-1)
        return;

    QListWidgetItem *currentItem = listWidget_ENABLED->takeItem(currentRow);

    listWidget_ENABLED->insertItem(currentRow + 1, currentItem);
    listWidget_ENABLED->setCurrentRow(currentRow + 1);
}

void ActionCustomizer::moveUp(){
    int currentRow = listWidget_ENABLED->currentRow();

    if (!currentRow)
        return;

    QListWidgetItem *currentItem = listWidget_ENABLED->takeItem(currentRow);

    listWidget_ENABLED->insertItem(currentRow - 1, currentItem);
    listWidget_ENABLED->setCurrentRow(currentRow - 1);
}

void ActionCustomizer::moveToAvailable() {
    int currentRow = listWidget_ENABLED->currentRow();
    QListWidgetItem *currentItem = listWidget_ENABLED->takeItem(currentRow);

    if (!currentItem)
        return;

    avail_items.insert(currentItem, enabled_items[currentItem]);
    enabled_items.remove(currentItem);

    listWidget_AVAILABLE->addItem(currentItem);
}

void ActionCustomizer::moveToEnabled() {
    int currentRow = listWidget_AVAILABLE->currentRow();
    QListWidgetItem *currentItem = listWidget_AVAILABLE->takeItem(currentRow);

    if (!currentItem)
        return;

    enabled_items.insert(currentItem, avail_items[currentItem]);
    avail_items.remove(currentItem);

    listWidget_ENABLED->addItem(currentItem);
}
