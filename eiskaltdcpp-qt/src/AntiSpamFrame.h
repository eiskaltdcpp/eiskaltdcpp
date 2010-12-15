/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef DCANTISPAMFRAME_H
#define DCANTISPAMFRAME_H

#include <QDialog>
#include <QWidget>
#include <QTreeWidget>

#include "ui_UIAntiSpam.h"
#include "Antispam.h"

class AntiSpamFrame : public QDialog, private Ui::UIAntiSpam {
    Q_OBJECT

public:

    AntiSpamFrame(QWidget *parent = 0);
    virtual ~AntiSpamFrame();

private:
    void InitDocument();
    void loadGUIData();

    void loadWhiteList();
    void loadBlackList();
    void loadGrayList();
    void loadList(QTreeWidget *, QList<QString>&);

    void addItemToTree(QTreeWidget *, QString);
    void remItemFromTree(QTreeWidget *, QString);

    void clearTreeWidget(QTreeWidget*);

    bool addToList(AntiSpamObjectState state, QString);

private Q_SLOTS:
    void slotAntiSpamSwitch();
    void slotAsFilter();
    void slotFilterOps();

    void slotAddToWhite();
    void slotAddToBlack();
    void slotAddToGray();
    void slotRemFromWhite();
    void slotRemFromBlack();
    void slotRemFromGray();
    void slotClearWhite();
    void slotClearBlack();
    void slotClearGray();

    void slotSettingsChanged(const QString &key, const QString &value);

    void slotAccept();

    void slotWToG(); //White -> Gray
    void slotWToB(); //White -> Black
    void slotBToW(); //Black -> White
    void slotBToG(); //Black -> Gray
    void slotGToB(); //Gray -> Black
    void slotGToW(); //Gray -> White

};

#endif
