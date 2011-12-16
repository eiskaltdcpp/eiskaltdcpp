/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QList>

#include "QuickConnect.h"
#include "HubFrame.h"
#include "MainWindow.h"
#include "WulforSettings.h"

QuickConnect::QuickConnect(QWidget *parent) : QDialog(parent) {
    setupUi(this);

    comboBox_HUB->addItems(WulforSettings::getInstance()->getStr(WS_QCONNECT_HISTORY).split(" ", QString::SkipEmptyParts));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotAccept()));
    connect(comboBox_HUB, SIGNAL(activated(int)), this, SLOT(slotAccept()));

    comboBox_HUB->setFocus();
}

QuickConnect::~QuickConnect() {

}

void QuickConnect::slotAccept() {
    QString hub = comboBox_HUB->currentText();
    QString encoding = "";

    hub.replace(" ", "");

    if (hub.isEmpty()){
        accept();

        return;
    }

    if (hub.startsWith("adc://") || hub.startsWith("adcs://"))
        encoding = "UTF-8";
    if (!hub.isEmpty()) {
        if (encoding.isEmpty()){//Has favorite entry for hub?
            FavoriteHubEntry* entry = FavoriteManager::getInstance()->getFavoriteHubEntry(_tq(hub));

            if (entry)
                encoding = WulforUtil::getInstance()->dcEnc2QtEnc(_q(entry->getEncoding()));
        }

        MainWindow::getInstance()->newHubFrame(hub, (encoding.isEmpty())? (WSGET(WS_DEFAULT_LOCALE)) : (encoding));

        QStringList list = WulforSettings::getInstance()->getStr(WS_QCONNECT_HISTORY).split(" ", QString::SkipEmptyParts);

        if (!list.contains(hub))
            list.push_back(hub);
        else{
            accept();

            return;
        }
        QString hist = "";

        foreach (const QString &i, list)
            hist += (i + " ");

        WulforSettings::getInstance()->setStr(WS_QCONNECT_HISTORY, hist);
    }

    accept();
}

