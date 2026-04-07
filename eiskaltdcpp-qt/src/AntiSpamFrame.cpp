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

#include "AntiSpamFrame.h"
#include "WulforSettings.h"
#include "QtContext.h"
#include "QtContextAware.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QPushButton>

AntiSpamFrame::AntiSpamFrame(QWidget *parent) : QDialog(parent) {
    setupUi(this);

    InitDocument();
}

AntiSpamFrame::~AntiSpamFrame() {
    clearTreeWidget(treeWidget_WHITE);
    clearTreeWidget(treeWidget_BLACK);
    clearTreeWidget(treeWidget_BLACK);

    if (qtCtx()->antiSpam()) {
        qtCtx()->antiSpam()->saveLists();
        qtCtx()->antiSpam()->saveSettings();
    }
}

void AntiSpamFrame::InitDocument() {
   if (qtCtx()->settings()->getBool(WB_ANTISPAM_ENABLED)) {
        if (!qtCtx()->antiSpam())
            qtCtx()->createAntiSpam();

        qtCtx()->antiSpam()->loadSettings();
        qtCtx()->antiSpam()->loadLists();

        checkBox_ASENABLE->setChecked(true);
    } else
        checkBox_ASENABLE->setChecked(false);

    loadGUIData();

    connect(checkBox_ASENABLE, &QCheckBox::clicked, this, &AntiSpamFrame::slotAntiSpamSwitch);
    connect(checkBox_ASFILTER, &QCheckBox::clicked, this, &AntiSpamFrame::slotAsFilter);
    connect(checkBox_FILTER_OPS, &QCheckBox::clicked, this, &AntiSpamFrame::slotFilterOps);

    connect(pushButton_ADDWHITE, &QPushButton::clicked, this, &AntiSpamFrame::slotAddToWhite);
    connect(pushButton_ADDBLACK, &QPushButton::clicked, this, &AntiSpamFrame::slotAddToBlack);
    connect(pushButton_ADDGRAY, &QPushButton::clicked, this, &AntiSpamFrame::slotAddToGray);
    connect(pushButton_REMWHITE, &QPushButton::clicked, this, &AntiSpamFrame::slotRemFromWhite);
    connect(pushButton_REMBLACK, &QPushButton::clicked, this, &AntiSpamFrame::slotRemFromBlack);
    connect(pushButton_REMGRAY, &QPushButton::clicked, this, &AntiSpamFrame::slotRemFromGray);
    connect(pushButton_CLRWHITE, &QPushButton::clicked, this, &AntiSpamFrame::slotClearWhite);
    connect(pushButton_CLRBLACK, &QPushButton::clicked, this, &AntiSpamFrame::slotClearBlack);
    connect(pushButton_CLRGRAY, &QPushButton::clicked, this, &AntiSpamFrame::slotClearGray);

    connect(pushButton_OK, &QPushButton::clicked, this, &AntiSpamFrame::slotAccept);

    connect(pushButton_WTOG, &QPushButton::clicked, this, &AntiSpamFrame::slotWToG);
    connect(pushButton_WTOB, &QPushButton::clicked, this, &AntiSpamFrame::slotWToB);
    connect(pushButton_BTOW, &QPushButton::clicked, this, &AntiSpamFrame::slotBToW);
    connect(pushButton_BTOG, &QPushButton::clicked, this, &AntiSpamFrame::slotBToG);
    connect(pushButton_GTOB, &QPushButton::clicked, this, &AntiSpamFrame::slotGToB);
    connect(pushButton_GTOW, &QPushButton::clicked, this, &AntiSpamFrame::slotGToW);

    connect(qtCtx()->settings(), &WulforSettings::strValueChanged, this, &AntiSpamFrame::slotSettingsChanged);

    slotAntiSpamSwitch();
}

void AntiSpamFrame::slotAntiSpamSwitch() {
    bool b = checkBox_ASENABLE->isChecked();

    qtCtx()->settings()->setBool(WB_ANTISPAM_ENABLED, b);

    checkBox_ASFILTER->setEnabled(b);
    groupBox_WHITE->setEnabled(b);
    groupBox_BLACK->setEnabled(b);
    groupBox_GRAY->setEnabled(b);

    groupBox_PHRASE->setEnabled(b);

    pushButton_WTOG->setEnabled(b);
    pushButton_WTOB->setEnabled(b);
    pushButton_BTOW->setEnabled(b);
    pushButton_BTOG->setEnabled(b);
    pushButton_GTOB->setEnabled(b);
    pushButton_GTOW->setEnabled(b);

    groupBox_WHITE->setEnabled(!qtCtx()->settings()->getBool(WB_ANTISPAM_AS_FILTER));
    groupBox_GRAY->setEnabled(!qtCtx()->settings()->getBool(WB_ANTISPAM_AS_FILTER));
    groupBox_PHRASE->setEnabled(!qtCtx()->settings()->getBool(WB_ANTISPAM_AS_FILTER));

    if (!b && qtCtx()->antiSpam()) {
        qtCtx()->antiSpam()->setAttempts(spinBox_TRYCOUNT->value());
        qtCtx()->antiSpam()->saveSettings();
        qtCtx()->antiSpam()->saveLists();

        qtCtx()->destroyAntiSpam();
    } else if (b && !qtCtx()->antiSpam()) {
        qtCtx()->createAntiSpam();

        qtCtx()->antiSpam()->loadSettings();
        qtCtx()->antiSpam()->loadLists();

        loadGUIData();
    }
}

void AntiSpamFrame::slotAsFilter(){
    bool b = checkBox_ASFILTER->isChecked();

    qtCtx()->settings()->setBool(WB_ANTISPAM_AS_FILTER, b);

    groupBox_WHITE->setEnabled(!b);
    groupBox_GRAY->setEnabled(!b);
    groupBox_PHRASE->setEnabled(!b);
}

void AntiSpamFrame::slotFilterOps(){
    qtCtx()->settings()->setBool(WB_ANTISPAM_FILTER_OPS, checkBox_FILTER_OPS->isChecked());
}

void AntiSpamFrame::loadGUIData() {
    if (qtCtx()->antiSpam()) {
        lineEdit_PHRASE->setText(qtCtx()->antiSpam()->getPhrase());

        checkBox_ASFILTER->setChecked(qtCtx()->settings()->getBool(WB_ANTISPAM_AS_FILTER));

        spinBox_TRYCOUNT->setValue(qtCtx()->antiSpam()->getAttempts());

        checkBox_FILTER_OPS->setChecked(qtCtx()->settings()->getBool(WB_ANTISPAM_FILTER_OPS));

        QList<QString> keys = qtCtx()->antiSpam()->getKeys();
        QString words = "";

        for (int i = 0; i < keys.size(); i++)
            words += keys.at(i) + "|";

        if (words.right(1) == "|")
            words = words.left(words.length()-1);

        lineEdit_KEY->setText(words);

        loadBlackList();
        loadGrayList();
        loadWhiteList();

    }
}

void AntiSpamFrame::loadBlackList() {
    QList<QString> list = qtCtx()->antiSpam()->getBlack();
    loadList(treeWidget_BLACK, list);
}

void AntiSpamFrame::loadGrayList() {
    QList<QString> list = qtCtx()->antiSpam()->getGray();
    loadList(treeWidget_GRAY, list);
}

void AntiSpamFrame::loadWhiteList() {
    QList<QString> list = qtCtx()->antiSpam()->getWhite();
    loadList(treeWidget_WHITE, list);
}

void AntiSpamFrame::loadList(QTreeWidget *tree, QList<QString> &list) {
    if (!tree)
        return;

    clearTreeWidget(tree);

    for (int i = 0; i < list.size(); i++) {
        QTreeWidgetItem *it = new QTreeWidgetItem(tree);

        it->setText(0, list.at(i));
    }
}

void AntiSpamFrame::clearTreeWidget(QTreeWidget *tree) {
    QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::NotHidden);

    while (*it) {
        delete *it;
        ++it;
    }

    tree->clear();
}

bool AntiSpamFrame::addToList(AntiSpamObjectState state, const QString &nick) {
    if (nick.isEmpty())
        return false;

    if (!qtCtx()->antiSpam())
        return false;

    if (qtCtx()->antiSpam()->isInAny(nick)) {//nick already in lists
        AntiSpamObjectState e;

        if (qtCtx()->antiSpam()->isInBlack(nick))
            e = eIN_BLACK;
        else if (qtCtx()->antiSpam()->isInGray(nick))
            e = eIN_GRAY;
        else
            e = eIN_WHITE;

        if (e == state)//in some list
            return false;

        qtCtx()->antiSpam()->move(nick, state);

        QTreeWidget *tree = nullptr;

        switch (e) {
            case eIN_BLACK:
                tree = treeWidget_BLACK;
                break;
            case eIN_GRAY:
                tree = treeWidget_GRAY;
                break;
            case eIN_WHITE:
                tree = treeWidget_WHITE;
                break;
        }
        remItemFromTree(tree, nick);

        tree = nullptr;
        switch (state) {
            case eIN_BLACK:
                tree = treeWidget_BLACK;
                break;
            case eIN_GRAY:
                tree = treeWidget_GRAY;
                break;
            case eIN_WHITE:
                tree = treeWidget_WHITE;
                break;
        }
        addItemToTree(tree, nick);

        return true;
    }

    QTreeWidget *tree = nullptr;

    (*qtCtx()->antiSpam()) << state << nick;

    switch (state) {
        case eIN_BLACK:
            tree = treeWidget_BLACK;
            break;
        case eIN_GRAY:
            tree = treeWidget_GRAY;
            break;
        case eIN_WHITE:
            tree = treeWidget_WHITE;
    }

    addItemToTree(tree, nick);

    return true;
}

void AntiSpamFrame::addItemToTree(QTreeWidget *tree, const QString &text) {
    if (!tree || text.isEmpty())
        return;

    QTreeWidgetItem *it = new QTreeWidgetItem(tree);

    it->setText(0, text);
}

void AntiSpamFrame::remItemFromTree(QTreeWidget *tree, const QString &text) {
    if (!tree || text.isEmpty())
        return;

    QTreeWidgetItemIterator it(tree, QTreeWidgetItemIterator::NotHidden);

    while (*it) {
        if ((*it)->text(0) == text) {
            tree->removeItemWidget(*it, 0);

            delete (*it);

            tree->repaint();

            break;
        }
        ++it;
    }
}

void AntiSpamFrame::slotAddToBlack() {
    addToList(eIN_BLACK, lineEdit_BLACK->text());
    lineEdit_BLACK->setText("");
    lineEdit_BLACK->setFocus();
}

void AntiSpamFrame::slotAddToGray() {
    addToList(eIN_GRAY, lineEdit_GRAY->text());
    lineEdit_GRAY->setText("");
    lineEdit_GRAY->setFocus();
}

void AntiSpamFrame::slotAddToWhite() {
    addToList(eIN_WHITE, lineEdit_WHITE->text());
    lineEdit_WHITE->setText("");
    lineEdit_WHITE->setFocus();
}

void AntiSpamFrame::slotClearBlack() {
    clearTreeWidget(treeWidget_BLACK);

    if (qtCtx()->antiSpam())
        qtCtx()->antiSpam()->clearBlack();
}

void AntiSpamFrame::slotClearGray() {
    clearTreeWidget(treeWidget_GRAY);

    if (qtCtx()->antiSpam())
        qtCtx()->antiSpam()->clearGray();
}

void AntiSpamFrame::slotClearWhite() {
    clearTreeWidget(treeWidget_WHITE);

    if (qtCtx()->antiSpam())
        qtCtx()->antiSpam()->clearWhite();
}

void AntiSpamFrame::slotRemFromBlack() {
    QTreeWidgetItem *it = treeWidget_BLACK->currentItem();

    if (it) {
        QString nick = it->text(0);

        remItemFromTree(treeWidget_BLACK, nick);

        if (qtCtx()->antiSpam())
            qtCtx()->antiSpam()->remFromBlack(QList<QString > () << nick);
    }
}

void AntiSpamFrame::slotRemFromGray() {
    QTreeWidgetItem *it = treeWidget_GRAY->currentItem();

    if (it) {
        QString nick = it->text(0);

        remItemFromTree(treeWidget_GRAY, nick);

        if (qtCtx()->antiSpam())
            qtCtx()->antiSpam()->remFromGray(QList<QString > () << nick);
    }
}

void AntiSpamFrame::slotRemFromWhite() {
    QTreeWidgetItem *it = treeWidget_WHITE->currentItem();

    if (it) {
        QString nick = it->text(0);

        remItemFromTree(treeWidget_WHITE, nick);

        if (qtCtx()->antiSpam())
            qtCtx()->antiSpam()->remFromWhite(QList<QString > () << nick);
    }
}

void AntiSpamFrame::slotAccept() {
    if (!qtCtx()->antiSpam()) {
        accept();
        return;
    }

    QString phrase = lineEdit_PHRASE->text();
    QList<QString> keys = lineEdit_KEY->text().split("|");

    for (int i = 0; i < keys.size(); i++){
        if (keys.at(i).isEmpty()){
            keys.removeAt(i);
            i = 0;
        }
    }

    qtCtx()->antiSpam()->setPhrase(phrase);
    qtCtx()->antiSpam()->setKeys(keys);
    qtCtx()->antiSpam()->setAttempts(spinBox_TRYCOUNT->value());

    accept();
}

void AntiSpamFrame::slotWToG() {
    QTreeWidgetItem *it = treeWidget_WHITE->currentItem();

    if (it) {
        QString nick = it->text(0);

        addToList(eIN_GRAY, nick);
    }
}

void AntiSpamFrame::slotWToB() {
    QTreeWidgetItem *it = treeWidget_WHITE->currentItem();

    if (it) {
        QString nick = it->text(0);

        addToList(eIN_BLACK, nick);
    }
}

void AntiSpamFrame::slotBToW() {
    QTreeWidgetItem *it = treeWidget_BLACK->currentItem();

    if (it) {
        QString nick = it->text(0);

        addToList(eIN_WHITE, nick);
    }
}

void AntiSpamFrame::slotBToG() {
    QTreeWidgetItem *it = treeWidget_BLACK->currentItem();

    if (it) {
        QString nick = it->text(0);

        addToList(eIN_GRAY, nick);
    }
}

void AntiSpamFrame::slotGToB() {
    QTreeWidgetItem *it = treeWidget_GRAY->currentItem();

    if (it) {
        QString nick = it->text(0);

        addToList(eIN_BLACK, nick);
    }
}

void AntiSpamFrame::slotGToW() {
    QTreeWidgetItem *it = treeWidget_GRAY->currentItem();

    if (it) {
        QString nick = it->text(0);

        addToList(eIN_WHITE, nick);
    }
}

void AntiSpamFrame::slotSettingsChanged(const QString &key, const QString &value){
    Q_UNUSED(value)
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}
