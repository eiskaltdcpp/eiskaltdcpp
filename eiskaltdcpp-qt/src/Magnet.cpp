/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "Magnet.h"

#include <QUrl>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QClipboard>

#include "dcpp/stdinc.h"
#include "dcpp/Util.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/QueueManager.h"

#include "WulforUtil.h"
#include "SearchFrame.h"
#include "MainWindow.h"
#include "ArenaWidgetFactory.h"

using namespace dcpp;

Magnet::Magnet(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    pushButton_COPY->setIcon(WICON(WulforUtil::eiMAGNET));
    pushButton_BROWSE->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

    connect(pushButton_CANCEL,  SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButton_SEARCH,  SIGNAL(clicked()), this, SLOT(search()));
    connect(pushButton_DOWNLOAD,SIGNAL(clicked()), this, SLOT(download()));
    connect(pushButton_COPY,    SIGNAL(clicked()), this, SLOT(slotCopyMagnet()));
    connect(pushButton_BROWSE,  SIGNAL(clicked()), this, SLOT(slotBrowse()));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveWindowSize()));

    if (!SETTING(AUTO_SEARCH)){
        pushButton_DOWNLOAD->setToolTip(tr("Run search alternatives manually."));
    }
    else {
        pushButton_DOWNLOAD->setToolTip(tr("Download file via auto search alternatives"));
    }
    currentAction = (MagnetAction)WIGET(WI_DEF_MAGNET_ACTION);

    if (WVGET("ui/magnet-dialog-size").isValid()) {
        resize(WVGET("ui/magnet-dialog-size").toSize());
    }
}

Magnet::~Magnet() {}

void Magnet::showUI(const QString &name, const qulonglong &size, const QString &tth){

    lineEdit_SIZE->setReadOnly(true);

    lineEdit_FNAME->setText(name);

    if (size > 0)
        lineEdit_SIZE->setText(QString("%1 (%2)").arg(size).arg(WulforUtil::formatBytes(size)));
    else
        lineEdit_SIZE->setText("0 (0 MiB)");

    if (lineEdit_FNAME->text().isEmpty())
        lineEdit_FNAME->setText(tth);

    lineEdit_TTH->setText(tth);
    lineEdit_FPATH->setText(_q(SETTING(DOWNLOAD_DIRECTORY)));

    setWindowTitle(lineEdit_FNAME->text());

    if (!MainWindow::getInstance()->isVisible()){
        MainWindow::getInstance()->show();
        MainWindow::getInstance()->raise();
    }
}

void Magnet::setLink(const QString &link){
    setLink(link, currentAction);
}

void Magnet::setLink(const QString &link, MagnetAction action){
    QString name = "", tth = "";
    int64_t size = 0;

    WulforUtil::splitMagnet(link, size, tth, name);
    currentAction = action;

    switch (action) {
        case MAGNET_ACTION_SEARCH: // search
            search(name, size, tth);
            break;

        case MAGNET_ACTION_DOWNLOAD: // download
        {
            QString target;
            if (name.isEmpty())
                target = tth;
            else
                target = name;
            QString path=_q(SETTING(DOWNLOAD_DIRECTORY));
            target = path + (path.endsWith(QDir::separator())? QString("") : QDir::separator()) + target.split(QDir::separator(), QString::SkipEmptyParts).last();
                download(target, size, tth);
            break;
        }

        default: // show UI
        {
            checkBox_Remember->setChecked(false);
            showUI(name, size, tth);
        }
    }
}

int Magnet::exec() {
    if (currentAction != MAGNET_ACTION_SHOW_UI){
        accept();
        return result();
    }
    return QDialog::exec();
}

void Magnet::saveWindowSize() {
    WVSET("ui/magnet-dialog-size", size());
}

void Magnet::search(const QString &file, const qulonglong &size, const QString &tth){
    Q_UNUSED(size)

    if (!tth.isEmpty())
        Magnet::searchTTH(tth);
    else if (!file.isEmpty())
        Magnet::searchFile(file);
}

void Magnet::search(){
    if (checkBox_Remember->isChecked() && WIGET(WI_DEF_MAGNET_ACTION) != 1)
        WISET(WI_DEF_MAGNET_ACTION,1);

    QString size_str = lineEdit_SIZE->text();
    qulonglong size = size_str.left(size_str.indexOf(" (")).toULongLong();
    search(lineEdit_FNAME->text(), size, lineEdit_TTH->text());

    accept();
}

void Magnet::download() {
    const QString &&tth = lineEdit_TTH->text();

    if (checkBox_Remember->isChecked() && WIGET(WI_DEF_MAGNET_ACTION) != 2)
        WISET(WI_DEF_MAGNET_ACTION,2);
    if (tth.isEmpty())
        return;

    const QString &&fname = lineEdit_FNAME->text();
    const QString &&path = lineEdit_FPATH->text();
    const QString &&size_str = lineEdit_SIZE->text();

    const QString &&name = path + (path.endsWith(QDir::separator())? QString("") : QDir::separator()) + fname.split(QDir::separator(), QString::SkipEmptyParts).last();
    const qulonglong size = size_str.left(size_str.indexOf(" (")).toULongLong();

    Magnet::download(name,size,tth);

    accept();
}

void Magnet::slotCopyMagnet(){
    const QString &&tth = lineEdit_TTH->text();
    const QString &&fname = lineEdit_FNAME->text().trimmed();
    const QString &&size_str = lineEdit_SIZE->text();

    if (fname.isEmpty())
        return;

    const QString name = fname.split(QDir::separator(), QString::SkipEmptyParts).last();
    const qulonglong size = size_str.left(size_str.indexOf(" (")).toULongLong();

    QString magnet;
    if (tth.isEmpty()) {
        // Special searching magnet link:
        const QString &&encoded_name = _q(Util::encodeURI(name.toStdString()));
        magnet = "magnet:?kt=" + encoded_name + "&dn=" + encoded_name;
    } else {
        magnet = WulforUtil::getInstance()->makeMagnet(name, size, tth);
    }

    if (!magnet.isEmpty())
        qApp->clipboard()->setText(magnet, QClipboard::Clipboard);
}

void Magnet::slotBrowse(){
    QMenu *down_to = nullptr;
    QString aliases, paths;

    aliases = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_ALIASES).toUtf8());
    paths   = QByteArray::fromBase64(WSGET(WS_DOWNLOADTO_PATHS).toUtf8());

    QStringList a = aliases.split("\n", QString::SkipEmptyParts);
    QStringList p = paths.split("\n", QString::SkipEmptyParts);

    if (a.size() == p.size() && !a.isEmpty()){
        down_to = new QMenu();

        for (int i = 0; i < a.size(); i++){
            QAction *act = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), a.at(i), down_to);
            act->setData(p.at(i));

            down_to->addAction(act);
        }

        down_to->addSeparator();

        QAction *browse = new QAction(WICON(WulforUtil::eiFOLDER_BLUE), tr("Browse"), down_to);
        browse->setData("");

        down_to->addAction(browse);
    }

    if (down_to){
        QAction *act = down_to->exec(frame->mapToGlobal(pushButton_BROWSE->pos())+QPoint(0, pushButton_BROWSE->height()));

        down_to->deleteLater();

        if (act && !act->data().toString().isEmpty()){
            lineEdit_FPATH->setText(act->data().toString() + PATH_SEPARATOR_STR);

            return;
        }
        else if (!act)
            return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

    if (dir.isEmpty())
        return;

    dir = QDir::toNativeSeparators(dir);

    lineEdit_FPATH->setText(dir + PATH_SEPARATOR_STR);
}

void Magnet::download(const QString &name, const qulonglong &size, const QString &tth) {
    if (tth.isEmpty())
        return;
    try {
        QueueManager::getInstance()->add(_tq(name), size, TTHValue(_tq(tth)));
    }
    catch (const std::exception& e){
        QMessageBox::critical(this, tr("Error"), tr("Some error ocurred when starting download:\n %1").arg(e.what()));
    }
}

void Magnet::searchTTH(const QString &tth) {
    SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame>();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchAlternates(tth);
}

void Magnet::searchFile(const QString &file) {
    SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame>();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchFile(file);
}
