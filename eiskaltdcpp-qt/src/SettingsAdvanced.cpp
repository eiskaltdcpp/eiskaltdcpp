/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SettingsAdvanced.h"
#include "MainWindow.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include <QFileDialog>

using namespace dcpp;

SettingsAdvanced::SettingsAdvanced(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    init();
}

SettingsAdvanced::~SettingsAdvanced() {

}

void SettingsAdvanced::ok() {
    SettingsManager *SM = SettingsManager::getInstance();

    SM->set(SettingsManager::MIME_HANDLER, _tq(lineEdit_MIME->text()));
}

void SettingsAdvanced::init() {
    lineEdit_MIME->setText(_q(SETTING(MIME_HANDLER)));
    pushButton_BROWSE->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

    connect(pushButton_BROWSE, SIGNAL(clicked()), SLOT(slotBrowse()));
}

void SettingsAdvanced::slotBrowse()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select mime handler binary"), QDir::homePath());

    if (file.isEmpty())
        return;

    file = QDir::toNativeSeparators(file);
    lineEdit_MIME->setText(file);
}
