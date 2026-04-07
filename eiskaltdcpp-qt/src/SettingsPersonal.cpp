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

#include "SettingsPersonal.h"
#include "QtContextAware.h"
#include "QtContext.h"

#include <QComboBox>

#include "dcpp/stdinc.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/DCPlusPlus.h"

#include "WulforUtil.h"
#include "WulforSettings.h"

using namespace dcpp;

SettingsPersonal::SettingsPersonal(QWidget *parent):
        QWidget(parent)
{
    setupUi(this);

    init();
}

SettingsPersonal::~SettingsPersonal(){

}

void SettingsPersonal::ok(){
    SettingsManager *SM = qtCtx()->dcCtx().getSettingsManager();

    SM->set(SettingsManager::NICK, lineEdit_NICK->text().toStdString());
    SM->set(SettingsManager::EMAIL, lineEdit_EMAIL->text().toStdString());
    SM->set(SettingsManager::DESCRIPTION, lineEdit_DESC->text().toStdString());
    SM->set(SettingsManager::UPLOAD_SPEED, SettingsManager::connectionSpeeds[comboBox_SPEED->currentIndex()]);
    SM->set(SettingsManager::DEFAULT_AWAY_MESSAGE, lineEdit_AWAYMSG->text().toStdString());

    QString enc = comboBox_ENC->currentText();

    qtCtx()->settings()->setStr(WS_DEFAULT_LOCALE, enc);
    enc = qtCtx()->wulforUtil()->qtEnc2DcEnc(comboBox_ENC->currentText());

    if (enc.indexOf(" ") > 0){
        enc = enc.left(enc.indexOf(" "));
        enc.replace(" ", "");
    }

    Text::hubDefaultCharset = _tq(enc);

    qtCtx()->settings()->setBool(WB_APP_AUTOAWAY_BY_TIMER, checkBox_AUTOAWAY->isChecked());
    qtCtx()->settings()->setInt(WI_APP_AUTOAWAY_INTERVAL, spinBox->value());

    SM->save();

    qtCtx()->settings()->save();
}

void SettingsPersonal::init(){
    lineEdit_NICK->setText(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::NICK, true).c_str());
    lineEdit_EMAIL->setText(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::EMAIL, true).c_str());
    lineEdit_DESC->setText(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::DESCRIPTION, true).c_str());
    lineEdit_AWAYMSG->setText(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::DEFAULT_AWAY_MESSAGE, true).c_str());

    for (auto i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i){
        comboBox_SPEED->addItem((*i).c_str());

        if (qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::UPLOAD_SPEED, true) == *i)
            comboBox_SPEED->setCurrentIndex(i - SettingsManager::connectionSpeeds.begin());
    }

    QStringList encodings = qtCtx()->wulforUtil()->encodings();

    comboBox_ENC->addItem(tr("System default"));
    comboBox_ENC->addItems(encodings);

    QString default_enc = qtCtx()->settings()->getStr(WS_DEFAULT_LOCALE);

    if (encodings.contains(default_enc))
        comboBox_ENC->setCurrentIndex(encodings.indexOf(default_enc)+1);
    else
        comboBox_ENC->setCurrentIndex(0);

    checkBox_AUTOAWAY->setChecked(qtCtx()->settings()->getBool(WB_APP_AUTOAWAY_BY_TIMER));
    spinBox->setValue(qtCtx()->settings()->getInt(WI_APP_AUTOAWAY_INTERVAL));
}
