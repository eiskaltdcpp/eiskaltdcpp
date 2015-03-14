/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SettingsDownloads.h"
#include "WulforUtil.h"
#include "PublicHubsList.h"

#include "dcpp/stdinc.h"
#include "dcpp/SettingsManager.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QDir>

using namespace dcpp;

SettingsDownloads::SettingsDownloads(QWidget *parent):
        QWidget(parent)
{
    setupUi(this);

    other_settings.insert(SettingsManager::PRIO_LOWEST, 0);
    other_settings.insert(SettingsManager::AUTODROP_ALL, 1);
    other_settings.insert(SettingsManager::AUTODROP_FILELISTS, 2);
    other_settings.insert(SettingsManager::AUTODROP_DISCONNECT, 3);
    other_settings.insert(SettingsManager::AUTO_SEARCH, 4);
    other_settings.insert(SettingsManager::AUTO_SEARCH_AUTO_MATCH, 5);
    other_settings.insert(SettingsManager::SKIP_ZERO_BYTE, 6);
    other_settings.insert(SettingsManager::DONT_DL_ALREADY_SHARED, 7);
    other_settings.insert(SettingsManager::DONT_DL_ALREADY_QUEUED, 8);
    other_settings.insert(SettingsManager::SFV_CHECK, 9);
    other_settings.insert(SettingsManager::KEEP_LISTS, 10);
    other_settings.insert(SettingsManager::KEEP_FINISHED_FILES, 11);
    other_settings.insert(SettingsManager::COMPRESS_TRANSFERS, 12);
    other_settings.insert(SettingsManager::SEGMENTED_DL, 13);

    init();
}

SettingsDownloads::~SettingsDownloads(){
}

void SettingsDownloads::ok(){
    SettingsManager *SM = SettingsManager::getInstance();

    QString dl_dir = lineEdit_DLDIR->text(), udl_dir = lineEdit_UNF_DL_DIR->text();

    if (!dl_dir.endsWith(PATH_SEPARATOR))
        dl_dir += PATH_SEPARATOR_STR;

    if (!udl_dir.endsWith(PATH_SEPARATOR))
        udl_dir += PATH_SEPARATOR_STR;

    SM->set(SettingsManager::NO_USE_TEMP_DIR, !checkBox_NO_USE_TEMP_DIR->isChecked());
    SM->set(SettingsManager::AUTO_SEARCH_TIME, spinBox_AUTO_SEARCH_TIME->value());
    SM->set(SettingsManager::SEGMENT_SIZE, spinBox_SEGMENT_SIZE->value());
    SM->set(SettingsManager::DOWNLOAD_DIRECTORY, _tq(dl_dir));
    SM->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, _tq(udl_dir));
    SM->set(SettingsManager::DOWNLOAD_SLOTS, spinBox_MAXDL->value());
    SM->set(SettingsManager::MAX_DOWNLOAD_SPEED, spinBox_NONEWDL->value());
    SM->set(SettingsManager::HTTP_PROXY, _tq(lineEdit_PROXY->text()));

    //Auto-priority
    SM->set(SettingsManager::PRIO_HIGHEST_SIZE, _tq(QString().setNum(spinBox_HTPMAX->value())));
    SM->set(SettingsManager::PRIO_HIGH_SIZE, _tq(QString().setNum(spinBox_HPMAX->value())));
    SM->set(SettingsManager::PRIO_NORMAL_SIZE, _tq(QString().setNum(spinBox_NPMAX->value())));
    SM->set(SettingsManager::PRIO_LOW_SIZE, _tq(QString().setNum(spinBox_LPMAX->value())));

    // Auto-drop
    SM->set(SettingsManager::AUTODROP_SPEED, _tq(QString().setNum(spinBox_DROPSB->value())));
    SM->set(SettingsManager::AUTODROP_ELAPSED, _tq(QString().setNum(spinBox_MINELAPSED->value())));
    SM->set(SettingsManager::AUTODROP_MINSOURCES, _tq(QString().setNum(spinBox_MINSRCONLINE->value())));
    SM->set(SettingsManager::AUTODROP_INTERVAL, _tq(QString().setNum(spinBox_CHECKEVERY->value())));
    SM->set(SettingsManager::AUTODROP_INACTIVITY, _tq(QString().setNum(spinBox_MAXINACT->value())));
    SM->set(SettingsManager::AUTODROP_FILESIZE, _tq(QString().setNum(spinBox_MINFSZ->value())));

    auto it = other_settings.constBegin();

    for (; it != other_settings.constEnd(); ++it)
        SM->set(it.key(), listWidget->item(it.value())->checkState() == Qt::Checked);
    
    SM->set(SettingsManager::ALLOW_SIM_UPLOADS, checkBox_ALLOW_SIM_UPLOADS->isChecked());
    SM->set(SettingsManager::ALLOW_UPLOAD_MULTI_HUB, checkBox_ALLOW_UPLOAD_MULTI_HUB->isChecked());
}

void SettingsDownloads::init(){
    {//Downloads
        lineEdit_DLDIR->setText(_q(SETTING(DOWNLOAD_DIRECTORY)));
        lineEdit_UNF_DL_DIR->setText(_q(SETTING(TEMP_DOWNLOAD_DIRECTORY)));
        lineEdit_PROXY->setText(_q(SETTING(HTTP_PROXY)));

        checkBox_NO_USE_TEMP_DIR->setChecked(!(((bool)SettingsManager::getInstance()->get(SettingsManager::NO_USE_TEMP_DIR))? Qt::Checked : Qt::Unchecked));
        spinBox_AUTO_SEARCH_TIME->setValue(SETTING(AUTO_SEARCH_TIME));
        spinBox_SEGMENT_SIZE->setValue(SETTING(SEGMENT_SIZE));
        spinBox_MAXDL->setValue(SETTING(DOWNLOAD_SLOTS));
        spinBox_NONEWDL->setValue(SETTING(MAX_DOWNLOAD_SPEED));

        pushButton_BROWSE->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));
        pushButton_BROWSE1->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

        connect(pushButton_BROWSE, SIGNAL(clicked()), SLOT(slotBrowse()));
        connect(pushButton_BROWSE1, SIGNAL(clicked()), SLOT(slotBrowse()));
        connect(pushButton_CFGLISTS, SIGNAL(clicked()), SLOT(slotCfgPublic()));
    }
    {//Download to
        QString aliases, paths;

        aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toUtf8());
        paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toUtf8());

        QStringList a = aliases.split("\n", QString::SkipEmptyParts);
        QStringList p = paths.split("\n", QString::SkipEmptyParts);

        if (a.size() == p.size() && !a.isEmpty()){
            for (int i = 0; i < a.size(); i++){
                QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

                item->setText(0, p.at(i));
                item->setText(1, a.at(i));
            }
        }

        treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotDownloadTo()));
    }
    {//Queue
        //Auto-priority
        spinBox_HTPMAX->setValue(SETTING(PRIO_HIGHEST_SIZE));
        spinBox_HPMAX->setValue(SETTING(PRIO_HIGH_SIZE));
        spinBox_NPMAX->setValue(SETTING(PRIO_NORMAL_SIZE));
        spinBox_LPMAX->setValue(SETTING(PRIO_LOW_SIZE));

        //Auto-drop
        spinBox_DROPSB->setValue(SETTING(AUTODROP_SPEED));
        spinBox_MINELAPSED->setValue(SETTING(AUTODROP_ELAPSED));
        spinBox_MINSRCONLINE->setValue(SETTING(AUTODROP_MINSOURCES));
        spinBox_CHECKEVERY->setValue(SETTING(AUTODROP_INTERVAL));
        spinBox_MAXINACT->setValue(SETTING(AUTODROP_INACTIVITY));
        spinBox_MINFSZ->setValue(SETTING(AUTODROP_FILESIZE));

        auto it = other_settings.constBegin();

        for (; it != other_settings.constEnd(); ++it)
            listWidget->item(it.value())->setCheckState(((bool)SettingsManager::getInstance()->get(it.key()))? Qt::Checked : Qt::Unchecked);
    }
    {
        checkBox_ALLOW_SIM_UPLOADS->setCheckState(SETTING(ALLOW_SIM_UPLOADS)? Qt::Checked : Qt::Unchecked);
        checkBox_ALLOW_UPLOAD_MULTI_HUB->setCheckState(SETTING(ALLOW_UPLOAD_MULTI_HUB)? Qt::Checked : Qt::Unchecked);
    }
}

void SettingsDownloads::slotBrowse(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

    if (dir.isEmpty())
        return;

    dir = QDir::toNativeSeparators(dir);

    if (sender() == pushButton_BROWSE)
        lineEdit_DLDIR->setText(dir);
    else if (sender() == pushButton_BROWSE1)
        lineEdit_UNF_DL_DIR->setText(dir);
}

void SettingsDownloads::slotDownloadTo(){
    QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();

    QMenu *m = new QMenu(this);
    QAction *new_alias = new QAction(tr("New"), m);
    new_alias->setIcon(WICON(WulforUtil::eiEDITADD));

    m->addAction(new_alias);

    if (!selected.isEmpty())
        m->addAction(WICON(WulforUtil::eiEDITDELETE), tr("Delete"));

    QAction *ret = m->exec(QCursor::pos());

    delete m;

    if (ret == new_alias){
        QString alias = QInputDialog::getText(this, tr("Enter alias for directory"), tr("Alias"));

        if (alias.isEmpty())
            return;

        QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

        if (dir.isEmpty())
            return;

        dir = QDir::toNativeSeparators(dir);

        QString aliases, paths;

        aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toUtf8());
        paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toUtf8());

        aliases += alias + "\n";
        paths   += dir + "\n";

        WSSET(WS_DOWNLOADTO_ALIASES, aliases.toUtf8().toBase64());
        WSSET(WS_DOWNLOADTO_PATHS, paths.toUtf8().toBase64());

        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

        item->setText(0, dir);
        item->setText(1, alias);
    }
    else if (ret){
        QString aliases, paths;
        aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toUtf8());
        paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toUtf8());

        for (const auto &i : selected){
            QString alias = i->text(1);
            QString path  = i->text(0);

            aliases.replace(alias+"\n", "");
            paths.replace(path+"\n", "");

            delete i;
        }

        WSSET(WS_DOWNLOADTO_ALIASES, aliases.toUtf8().toBase64());
        WSSET(WS_DOWNLOADTO_PATHS, paths.toUtf8().toBase64());
    }
}

void SettingsDownloads::slotCfgPublic(){
    PublicHubsList h;

    h.exec();
}
