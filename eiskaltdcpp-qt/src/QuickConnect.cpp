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

#include <QList>
#include "dcpp/DCPlusPlus.h"
#include "QtContextAware.h"
#include "QtContext.h"

#include "QuickConnect.h"
#include "HubFrame.h"
#include "MainWindow.h"
#include "WulforSettings.h"

QuickConnect::QuickConnect(QWidget *parent) : QDialog(parent) {
    setupUi(this);

    comboBox_HUB->addItems(qtCtx()->settings()->getStr(WS_QCONNECT_HISTORY).split(" ", Qt::SkipEmptyParts));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QuickConnect::slotAccept);
    connect(comboBox_HUB, qOverload<int>(&QComboBox::activated), this, &QuickConnect::slotAccept);

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
            FavoriteHubEntry* entry = qtCtx()->dcCtx().getFavoriteManager()->getFavoriteHubEntry(_tq(hub));

            if (entry)
                encoding = qtCtx()->wulforUtil()->dcEnc2QtEnc(_q(entry->getEncoding()));
        }

        qtCtx()->mainWindow()->newHubFrame(hub, (encoding.isEmpty())? (qtCtx()->settings()->getStr(WS_DEFAULT_LOCALE)) : (encoding));

        QStringList list = qtCtx()->settings()->getStr(WS_QCONNECT_HISTORY).split(" ", Qt::SkipEmptyParts);

        if (!list.contains(hub))
            list.push_back(hub);
        else{
            accept();

            return;
        }
        QString hist = "";

        for (const auto &i : list)
            hist += (i + " ");

        qtCtx()->settings()->setStr(WS_QCONNECT_HISTORY, hist);
    }

    accept();
}

