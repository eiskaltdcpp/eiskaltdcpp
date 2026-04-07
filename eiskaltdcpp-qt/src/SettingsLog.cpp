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

#include "SettingsLog.h"
#include "QtContextAware.h"
#include "QtContext.h"
#include "WulforUtil.h"

#include "dcpp/SettingsManager.h"
#include "dcpp/DCPlusPlus.h"

#include <QDir>
#include <QFileDialog>

using namespace dcpp;

SettingsLog::SettingsLog(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    init();
}

void SettingsLog::init(){
    lineEdit_LOGDIR->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_DIRECTORY, true)));

    groupBox_MAINCHAT->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_MAIN_CHAT, true));
    lineEdit_CHATFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_MAIN_CHAT, true)));
    lineEdit_FILE_CHATFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_MAIN_CHAT, true)));

    groupBox_PM->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_PRIVATE_CHAT, true));
    lineEdit_PMFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_PRIVATE_CHAT, true)));
    lineEdit_FILE_PMFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_PRIVATE_CHAT, true)));

    groupBox_DOWN->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_DOWNLOADS, true));
    lineEdit_DOWNFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_POST_DOWNLOAD, true)));
    lineEdit_FILE_DOWNFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_DOWNLOAD, true)));

    groupBox_UP->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_UPLOADS, true));
    lineEdit_UPFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_POST_UPLOAD, true)));
    lineEdit_FILE_UPFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_UPLOAD, true)));

    groupBox_FINISH_DOWN->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_FINISHED_DOWNLOADS, true));
    lineEdit_FINISH_DOWNFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_POST_FINISHED_DOWNLOAD, true)));
    lineEdit_FILE_FINISH_DOWNFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_FINISHED_DOWNLOAD, true)));

    checkBox_FILELIST->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_FILELIST_TRANSFERS, true));
    checkBox_STAT->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_STATUS_MESSAGES, true));
    checkBox_SYSTEM->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_SYSTEM, true));
    checkBox_REPORT_ALTERNATES->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::REPORT_ALTERNATES, true));

    groupBox_SPYLOG->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_SPY, true));
    lineEdit_SPYFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_SPY, true)));
    lineEdit_FILE_SPYFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_SPY, true)));

    groupBox_CMD_DEBUG->setChecked(qtCtx()->dcCtx().getSettingsManager()->getBool(SettingsManager::LOG_CMD_DEBUG, true));
    lineEdit_CMD_DEBUGFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FORMAT_CMD_DEBUG, true)));
    lineEdit_FILE_CMD_DEBUGFMT->setText(_q(qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::LOG_FILE_CMD_DEBUG, true)));

    toolButton_BROWSE->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiFOLDER_BLUE));

    connect(toolButton_BROWSE, &QToolButton::clicked, this, &SettingsLog::slotBrowse);
}

void SettingsLog::ok(){
    SettingsManager *sm = qtCtx()->dcCtx().getSettingsManager();

    QString path = lineEdit_LOGDIR->text();
    if (!path.isEmpty() && !path.endsWith(QDir::separator()))
        path += QDir::separator();

    sm->set(SettingsManager::LOG_DIRECTORY, _tq(path));
    sm->set(SettingsManager::LOG_MAIN_CHAT, groupBox_MAINCHAT->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_MAIN_CHAT, _tq(lineEdit_CHATFMT->text()));
    sm->set(SettingsManager::LOG_FILE_MAIN_CHAT, _tq(lineEdit_FILE_CHATFMT->text()));
    sm->set(SettingsManager::LOG_PRIVATE_CHAT, groupBox_PM->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_PRIVATE_CHAT, _tq(lineEdit_PMFMT->text()));
    sm->set(SettingsManager::LOG_FILE_PRIVATE_CHAT, _tq(lineEdit_FILE_PMFMT->text()));
    sm->set(SettingsManager::LOG_DOWNLOADS, groupBox_DOWN->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_POST_DOWNLOAD, _tq(lineEdit_DOWNFMT->text()));
    sm->set(SettingsManager::LOG_FILE_DOWNLOAD, _tq(lineEdit_FILE_DOWNFMT->text()));
    sm->set(SettingsManager::LOG_UPLOADS, groupBox_UP->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_POST_UPLOAD, _tq(lineEdit_UPFMT->text()));
    sm->set(SettingsManager::LOG_FILE_UPLOAD, _tq(lineEdit_FILE_UPFMT->text()));
    sm->set(SettingsManager::LOG_FINISHED_DOWNLOADS, groupBox_FINISH_DOWN->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_POST_FINISHED_DOWNLOAD, _tq(lineEdit_FINISH_DOWNFMT->text()));
    sm->set(SettingsManager::LOG_FILE_FINISHED_DOWNLOAD, _tq(lineEdit_FILE_FINISH_DOWNFMT->text()));
    sm->set(SettingsManager::LOG_CMD_DEBUG, groupBox_CMD_DEBUG->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_CMD_DEBUG, _tq(lineEdit_CMD_DEBUGFMT->text()));
    sm->set(SettingsManager::LOG_FILE_CMD_DEBUG, _tq(lineEdit_FILE_CMD_DEBUGFMT->text()));
    sm->set(SettingsManager::LOG_SYSTEM, checkBox_SYSTEM->isChecked());
    sm->set(SettingsManager::LOG_STATUS_MESSAGES, checkBox_STAT->isChecked());
    sm->set(SettingsManager::LOG_FILELIST_TRANSFERS, checkBox_FILELIST->isChecked());
    sm->set(SettingsManager::LOG_SPY, groupBox_SPYLOG->isChecked());
    sm->set(SettingsManager::REPORT_ALTERNATES, checkBox_REPORT_ALTERNATES->isChecked());
}

void SettingsLog::slotBrowse(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the directory"), lineEdit_LOGDIR->text());

    if (!dir.isEmpty()){
        dir = QDir::toNativeSeparators(dir);

        if (!dir.endsWith(QDir::separator()))
            dir += QDir::separator();

        lineEdit_LOGDIR->setText(dir);
    }
}
