/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "AntiSpamFrame.h"
#include "WulforSettings.h"

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

    if (AntiSpam::getInstance()) {
        AntiSpam::getInstance()->saveLists();
        AntiSpam::getInstance()->saveSettings();
    }
}

void AntiSpamFrame::InitDocument() {
   if (WBGET(WB_ANTISPAM_ENABLED)) {
        if (!AntiSpam::getInstance())
            AntiSpam::newInstance();

        AntiSpam::getInstance()->loadSettings();
        AntiSpam::getInstance()->loadLists();

        checkBox_ASENABLE->setChecked(true);
    } else
        checkBox_ASENABLE->setChecked(false);

    loadGUIData();

    connect(checkBox_ASENABLE, SIGNAL(clicked()), this, SLOT(slotAntiSpamSwitch()));
    connect(checkBox_ASFILTER, SIGNAL(clicked()), this, SLOT(slotAsFilter()));
    connect(checkBox_FILTER_OPS, SIGNAL(clicked()), this, SLOT(slotFilterOps()));

    connect(pushButton_ADDWHITE, SIGNAL(clicked()), this, SLOT(slotAddToWhite()));
    connect(pushButton_ADDBLACK, SIGNAL(clicked()), this, SLOT(slotAddToBlack()));
    connect(pushButton_ADDGRAY, SIGNAL(clicked()), this, SLOT(slotAddToGray()));
    connect(pushButton_REMWHITE, SIGNAL(clicked()), this, SLOT(slotRemFromWhite()));
    connect(pushButton_REMBLACK, SIGNAL(clicked()), this, SLOT(slotRemFromBlack()));
    connect(pushButton_REMGRAY, SIGNAL(clicked()), this, SLOT(slotRemFromGray()));
    connect(pushButton_CLRWHITE, SIGNAL(clicked()), this, SLOT(slotClearWhite()));
    connect(pushButton_CLRBLACK, SIGNAL(clicked()), this, SLOT(slotClearBlack()));
    connect(pushButton_CLRGRAY, SIGNAL(clicked()), this, SLOT(slotClearGray()));

    connect(pushButton_OK, SIGNAL(clicked()), this, SLOT(slotAccept()));

    connect(pushButton_WTOG, SIGNAL(clicked()), this, SLOT(slotWToG()));
    connect(pushButton_WTOB, SIGNAL(clicked()), this, SLOT(slotWToB()));
    connect(pushButton_BTOW, SIGNAL(clicked()), this, SLOT(slotBToW()));
    connect(pushButton_BTOG, SIGNAL(clicked()), this, SLOT(slotBToG()));
    connect(pushButton_GTOB, SIGNAL(clicked()), this, SLOT(slotGToB()));
    connect(pushButton_GTOW, SIGNAL(clicked()), this, SLOT(slotGToW()));

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));

    slotAntiSpamSwitch();
}

void AntiSpamFrame::slotAntiSpamSwitch() {
    bool b = checkBox_ASENABLE->isChecked();

    WBSET(WB_ANTISPAM_ENABLED, b);

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

    groupBox_WHITE->setEnabled(!WBGET(WB_ANTISPAM_AS_FILTER));
    groupBox_GRAY->setEnabled(!WBGET(WB_ANTISPAM_AS_FILTER));
    groupBox_PHRASE->setEnabled(!WBGET(WB_ANTISPAM_AS_FILTER));

    if (!b && AntiSpam::getInstance()) {
        AntiSpam::getInstance()->setAttempts(spinBox_TRYCOUNT->value());
        AntiSpam::getInstance()->saveSettings();
        AntiSpam::getInstance()->saveLists();

        AntiSpam::deleteInstance();
    } else if (b && (AntiSpam::getInstance() == NULL)) {
        AntiSpam::newInstance();

        AntiSpam::getInstance()->loadSettings();
        AntiSpam::getInstance()->loadLists();

        loadGUIData();
    }
}

void AntiSpamFrame::slotAsFilter(){
    bool b = checkBox_ASFILTER->isChecked();

    WBSET(WB_ANTISPAM_AS_FILTER, b);

    groupBox_WHITE->setEnabled(!b);
    groupBox_GRAY->setEnabled(!b);
    groupBox_PHRASE->setEnabled(!b);
}

void AntiSpamFrame::slotFilterOps(){
    WBSET(WB_ANTISPAM_FILTER_OPS, checkBox_FILTER_OPS->isChecked());
}

void AntiSpamFrame::loadGUIData() {
    if (AntiSpam::getInstance()) {
        lineEdit_PHRASE->setText(AntiSpam::getInstance()->getPhrase());

        checkBox_ASFILTER->setChecked(WBGET(WB_ANTISPAM_AS_FILTER));

        spinBox_TRYCOUNT->setValue(AntiSpam::getInstance()->getAttempts());

        checkBox_FILTER_OPS->setChecked(WBGET(WB_ANTISPAM_FILTER_OPS));

        QList<QString> keys = AntiSpam::getInstance()->getKeys();
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
    QList<QString> list = AntiSpam::getInstance()->getBlack();
    loadList(treeWidget_BLACK, list);
}

void AntiSpamFrame::loadGrayList() {
    QList<QString> list = AntiSpam::getInstance()->getGray();
    loadList(treeWidget_GRAY, list);
}

void AntiSpamFrame::loadWhiteList() {
    QList<QString> list = AntiSpam::getInstance()->getWhite();
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
        if (*it)
            delete *it;
        ++it;
    }

    tree->clear();
}

bool AntiSpamFrame::addToList(AntiSpamObjectState state, QString nick) {
    if (nick == "")
        return false;

    if (!AntiSpam::getInstance())
        return false;

    if (AntiSpam::getInstance()->isInAny(nick)) {//nick already in lists
        AntiSpamObjectState e;

        if (AntiSpam::getInstance()->isInBlack(nick))
            e = eIN_BLACK;
        else if (AntiSpam::getInstance()->isInGray(nick))
            e = eIN_GRAY;
        else
            e = eIN_WHITE;

        if (e == state)//in some list
            return false;

        AntiSpam::getInstance()->move(nick, state);

        QTreeWidget *tree = NULL;

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

        tree = NULL;
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

    QTreeWidget *tree = NULL;

    (*AntiSpam::getInstance()) << state << nick;

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

void AntiSpamFrame::addItemToTree(QTreeWidget *tree, QString text) {
    if (tree == NULL || text == "")
        return;

    QTreeWidgetItem *it = new QTreeWidgetItem(tree);

    it->setText(0, text);
}

void AntiSpamFrame::remItemFromTree(QTreeWidget *tree, QString text) {
    if (tree == NULL || text == "")
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

    if (AntiSpam::getInstance())
        AntiSpam::getInstance()->clearBlack();
}

void AntiSpamFrame::slotClearGray() {
    clearTreeWidget(treeWidget_GRAY);

    if (AntiSpam::getInstance())
        AntiSpam::getInstance()->clearGray();
}

void AntiSpamFrame::slotClearWhite() {
    clearTreeWidget(treeWidget_WHITE);

    if (AntiSpam::getInstance())
        AntiSpam::getInstance()->clearWhite();
}

void AntiSpamFrame::slotRemFromBlack() {
    QTreeWidgetItem *it = treeWidget_BLACK->currentItem();

    if (it) {
        QString nick = it->text(0);

        remItemFromTree(treeWidget_BLACK, nick);

        if (AntiSpam::getInstance())
            AntiSpam::getInstance()->remFromBlack(QList<QString > () << nick);
    }
}

void AntiSpamFrame::slotRemFromGray() {
    QTreeWidgetItem *it = treeWidget_GRAY->currentItem();

    if (it) {
        QString nick = it->text(0);

        remItemFromTree(treeWidget_GRAY, nick);

        if (AntiSpam::getInstance())
            AntiSpam::getInstance()->remFromGray(QList<QString > () << nick);
    }
}

void AntiSpamFrame::slotRemFromWhite() {
    QTreeWidgetItem *it = treeWidget_WHITE->currentItem();

    if (it) {
        QString nick = it->text(0);

        remItemFromTree(treeWidget_WHITE, nick);

        if (AntiSpam::getInstance())
            AntiSpam::getInstance()->remFromWhite(QList<QString > () << nick);
    }
}

void AntiSpamFrame::slotAccept() {
    if (!AntiSpam::getInstance()) {
        accept();
        return;
    }

    QString phrase = lineEdit_PHRASE->text();
    QList<QString> keys = lineEdit_KEY->text().split("|");

    for (int i = 0; i < keys.size(); i++){
        if (keys.at(i) == ""){
            keys.removeAt(i);
            i = 0;
        }
    }

    AntiSpam::getInstance()->setPhrase(phrase);
    AntiSpam::getInstance()->setKeys(keys);
    AntiSpam::getInstance()->setAttempts(spinBox_TRYCOUNT->value());

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
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}
