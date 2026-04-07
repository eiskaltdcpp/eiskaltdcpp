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

#include "SettingsNotification.h"
#include "QtContextAware.h"
#include "QtContext.h"

#include "WulforSettings.h"
#include "WulforUtil.h"
#include "Notification.h"
#include "ShellCommandRunner.h"

#include <QFileDialog>
#ifndef NO_QT_MULTIMEDIA
#include <QSoundEffect>
#endif
#include <QUrl>
#include <QDir>

SettingsNotification::SettingsNotification(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    init();
}

void SettingsNotification::init(){
    WulforUtil *WU = qtCtx()->wulforUtil();

    {//Text
        checkBox_EXIT_CONFIRM->setChecked(qtCtx()->settings()->getBool(WB_EXIT_CONFIRM));

        groupBox->setChecked(qtCtx()->settings()->getBool(WB_NOTIFY_ENABLED));

        unsigned emap = static_cast<unsigned>(qtCtx()->settings()->getInt(WI_NOTIFY_EVENTMAP));

        checkBox_NICKSAY->setChecked(emap & Notification::NICKSAY);
        checkBox_ANY->setChecked(emap & Notification::ANY);
        checkBox_PM->setChecked(emap & Notification::PM);
        checkBox_TRDONE->setChecked(emap & Notification::TRANSFER);
        checkBox_FAVJOIN->setChecked(emap & Notification::FAVORITE);
        checkBox_MWACTIVE->setChecked(qtCtx()->settings()->getBool(WB_NOTIFY_SHOW_ON_ACTIVE));
        checkBox_MWVISIBLE->setChecked(qtCtx()->settings()->getBool(WB_NOTIFY_SHOW_ON_VISIBLE));
        checkBox_CHICON->setChecked(qtCtx()->settings()->getBool(WB_NOTIFY_CH_ICON_ALWAYS));

        if (qtCtx()->settings()->getBool(WB_NOTIFY_SHOW_ON_ACTIVE)){
            checkBox_MWVISIBLE->setChecked(true);
            checkBox_MWVISIBLE->setDisabled(true);
        }

        comboBox->setCurrentIndex(qtCtx()->settings()->getInt(WI_NOTIFY_MODULE));
    }
    {//Sound
        QString encoded = qtCtx()->settings()->getStr(WS_NOTIFY_SOUNDS);
        QString decoded = QByteArray::fromBase64(encoded.toUtf8());
        QStringList sounds = decoded.split("\n");

        if (sounds.size() == 4){
            lineEdit_SNDNICKSAY->setText(sounds.at(0));
            lineEdit_SNDPM->setText(sounds.at(1));
            lineEdit_SNDTRDONE->setText(sounds.at(2));
            lineEdit_FAV->setText(sounds.at(3));
        }

        groupBox_SND->setChecked(qtCtx()->settings()->getBool(WB_NOTIFY_SND_ENABLED));
        groupBox_SNDCMD->setChecked(qtCtx()->settings()->getBool(WB_NOTIFY_SND_EXTERNAL));

        lineEdit_SNDCMD->setText(qtCtx()->settings()->getStr(WS_NOTIFY_SND_CMD));

        unsigned emap = static_cast<unsigned>(qtCtx()->settings()->getInt(WI_NOTIFY_SNDMAP));

        groupBox_NICK->setChecked(emap & Notification::NICKSAY);
        groupBox_PM->setChecked(emap & Notification::PM);
        groupBox_TR->setChecked(emap & Notification::TRANSFER);
        groupBox_FAV->setChecked(emap & Notification::FAVORITE);

        checkBox_ACTIVEPM->setChecked(qtCtx()->settings()->getBool("notification/play-sound-with-active-pm", true));
    }

    toolButton_BRWNICK->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
    toolButton_BRWPM->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
    toolButton_BRWTR->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
    toolButton_BRWFAV->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));

    connect(toolButton_BRWNICK, &QToolButton::clicked, this, &SettingsNotification::slotBrowseFile);
    connect(toolButton_BRWPM,   &QToolButton::clicked, this, &SettingsNotification::slotBrowseFile);
    connect(toolButton_BRWTR,   &QToolButton::clicked, this, &SettingsNotification::slotBrowseFile);
    connect(toolButton_BRWFAV,  &QToolButton::clicked, this, &SettingsNotification::slotBrowseFile);

    connect(pushButton_TESTNICKSAY, &QPushButton::clicked, this, &SettingsNotification::slotTest);
    connect(pushButton_TESTPM,      &QPushButton::clicked, this, &SettingsNotification::slotTest);
    connect(pushButton_TESTTR,      &QPushButton::clicked, this, &SettingsNotification::slotTest);
    connect(pushButton_TESTFAV,     &QPushButton::clicked, this, &SettingsNotification::slotTest);

    connect(groupBox_SNDCMD, &QGroupBox::toggled, this, &SettingsNotification::slotToggleSndCmd);

#ifndef DBUS_NOTIFY
    frame->setVisible(false);
#endif
}

void SettingsNotification::playFile(const QString &file){
    if (qtCtx()->settings()->getBool(WB_NOTIFY_SND_ENABLED) || groupBox_SND->isChecked()){
        if (file.isEmpty() || !QFile::exists(file))
            return;

        if (!qtCtx()->settings()->getBool(WB_NOTIFY_SND_EXTERNAL)) {
#ifndef NO_QT_MULTIMEDIA
            auto *effect = new QSoundEffect(this);
            effect->setSource(QUrl::fromLocalFile(file));
            connect(effect, &QSoundEffect::playingChanged, effect, [effect]() {
                if (!effect->isPlaying()) effect->deleteLater();
            });
            effect->play();
#endif
        } else {
            QString cmd = lineEdit_SNDCMD->text();

            if (cmd.isEmpty())
                return;

            ShellCommandRunner *r = new ShellCommandRunner(cmd, QStringList() << file, this);
            connect(r, &ShellCommandRunner::finished, this, &SettingsNotification::slotCmdFinished);

            r->start();
        }
    }
}

void SettingsNotification::ok(){
    {//Text
        qtCtx()->settings()->setBool(WB_NOTIFY_ENABLED, groupBox->isChecked());
        qtCtx()->settings()->setBool(WB_NOTIFY_CH_ICON_ALWAYS, checkBox_CHICON->isChecked());
        qtCtx()->settings()->setBool(WB_NOTIFY_SHOW_ON_ACTIVE, checkBox_MWACTIVE->isChecked());
        qtCtx()->settings()->setBool(WB_NOTIFY_SHOW_ON_VISIBLE, checkBox_MWVISIBLE->isChecked());

        qtCtx()->settings()->setBool(WB_EXIT_CONFIRM, checkBox_EXIT_CONFIRM->isChecked());

        unsigned emap = 0;

        if (checkBox_ANY->isChecked())
            emap |= Notification::ANY;

        if (checkBox_TRDONE->isChecked())
            emap |= Notification::TRANSFER;

        if (checkBox_NICKSAY->isChecked())
            emap |= Notification::NICKSAY;

        if (checkBox_PM->isChecked())
            emap |= Notification::PM;

        if (checkBox_FAVJOIN->isChecked())
            emap |= Notification::FAVORITE;

        qtCtx()->settings()->setInt(WI_NOTIFY_EVENTMAP, emap);
        qtCtx()->settings()->setInt(WI_NOTIFY_MODULE, comboBox->currentIndex());

        qtCtx()->notification()->switchModule(comboBox->currentIndex());
    }
    {//Sound
        QString sounds = "";

        sounds += lineEdit_SNDNICKSAY->text() + "\n";
        sounds += lineEdit_SNDPM->text() + "\n";
        sounds += lineEdit_SNDTRDONE->text() + "\n";
        sounds += lineEdit_FAV->text();

        qtCtx()->settings()->setStr(WS_NOTIFY_SOUNDS, sounds.toUtf8().toBase64());
        qtCtx()->settings()->setBool(WB_NOTIFY_SND_ENABLED, groupBox_SND->isChecked());
        qtCtx()->settings()->setBool("notification/play-sound-with-active-pm", checkBox_ACTIVEPM->isChecked());

        qtCtx()->notification()->reloadSounds();

        if (qtCtx()->settings()->getBool(WB_NOTIFY_SND_EXTERNAL))
            qtCtx()->settings()->setStr(WS_NOTIFY_SND_CMD, lineEdit_SNDCMD->text());

        unsigned emap = 0;

        if (groupBox_TR->isChecked())
            emap |= Notification::TRANSFER;

        if (groupBox_NICK->isChecked())
            emap |= Notification::NICKSAY;

        if (groupBox_PM->isChecked())
            emap |= Notification::PM;

        if (groupBox_FAV->isChecked())
            emap |= Notification::FAVORITE;

        qtCtx()->settings()->setInt(WI_NOTIFY_SNDMAP, emap);
    }
}

void SettingsNotification::slotBrowseFile(){
    static QString defaultPath = QDir::homePath();

    QString f = QFileDialog::getOpenFileName(this, tr("Select file"), defaultPath, tr("All files (*.*)"));

    if (f.isEmpty())
        return;

    f = QDir::toNativeSeparators(f);

    defaultPath = f.left(f.lastIndexOf(QDir::separator()));

    QToolButton *btn = reinterpret_cast<QToolButton*>(sender());

    if (btn == toolButton_BRWNICK)
        lineEdit_SNDNICKSAY->setText(f);
    else if (btn == toolButton_BRWPM)
        lineEdit_SNDPM->setText(f);
    else if (btn == toolButton_BRWTR)
        lineEdit_SNDTRDONE->setText(f);
    else if (btn == toolButton_BRWFAV)
        lineEdit_FAV->setText(f);
}

void SettingsNotification::slotTest(){
    QPushButton *btn = reinterpret_cast<QPushButton*>(sender());

    if (btn == pushButton_TESTNICKSAY)
        playFile(lineEdit_SNDNICKSAY->text());
    else if (btn == pushButton_TESTPM)
        playFile(lineEdit_SNDPM->text());
    else if (btn == pushButton_TESTTR)
        playFile(lineEdit_SNDTRDONE->text());
    else if (btn == pushButton_TESTFAV)
        playFile(lineEdit_FAV->text());
}

void SettingsNotification::slotToggleSndCmd(bool checked){
    qtCtx()->settings()->setBool(WB_NOTIFY_SND_EXTERNAL, checked);
}

void SettingsNotification::slotCmdFinished(bool, QString){
    ShellCommandRunner *r = reinterpret_cast<ShellCommandRunner*>(sender());

    r->exit(0);
    r->wait(100);

    if (r->isRunning())
        r->terminate();

    delete r;
}
