/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "FileHasher.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QUrl>
#include <QClipboard>
#include <QFileInfo>
#include <QMessageBox>

#include <algorithm>

#include "dcpp/stdinc.h"
#include "dcpp/HashManager.h"
#include "dcpp/CID.h"
#include "dcpp/File.h"

#include "ArenaWidgetFactory.h"
#include "SearchFrame.h"
#include "WulforUtil.h"

using namespace dcpp;

static const quint64 MIN_BLOCK_SIZE = 64 * 1024;
static const size_t BUF_SIZE = 64*1024;
static const QString DIALOG_SIZE = "ui/file-hasher-dialog-size";

FileHasher::FileHasher(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    hasher = new HashThread();

    toolButton_COPY_MAGNET->setIcon(WICON(WulforUtil::eiMAGNET));
    toolButton_COPY_SEARCH_LINK->setIcon(WICON(WulforUtil::eiMAGNET));
    toolButton_BROWSE->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

    connect(hasher, SIGNAL(finished()), this, SLOT(slotDone()));
    connect(pushButton_SEARCH, SIGNAL(clicked()), this, SLOT(search()));
    connect(pushButton_RUN,    SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(toolButton_BROWSE, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    connect(toolButton_COPY_MAGNET,      SIGNAL(clicked()), this, SLOT(slotCopyMagnet()));
    connect(toolButton_COPY_SEARCH_LINK, SIGNAL(clicked()), this, SLOT(slotCopySearchString()));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveWindowSize()));

    if (WVGET(DIALOG_SIZE).isValid()) {
        resize(WVGET(DIALOG_SIZE).toSize());
    }
}

FileHasher::~FileHasher() {
    if (hasher) {
        hasher->quit();
        hasher->wait(2000);
    }

    delete hasher;
}

void FileHasher::saveWindowSize(){
    WVSET(DIALOG_SIZE, size());
}

void FileHasher::slotStart(){
    const QString &&tth = lineEdit_TTH->text();
    if (tth.size() == 39)
        return;

    QString file = lineEdit_FILE->text();
    if (!QFile::exists(file))
        return;

    pushButton_RUN->setEnabled(false);
    HashManager *HM = HashManager::getInstance();
    const TTHValue *tth_val= HM->getFileTTHif(_tq(file));
    if (tth_val) {
        lineEdit_TTH->setText(_q(tth_val->toBase32()));
        pushButton_RUN->setEnabled(true);
    } else if (hasher) {
        hasher->quit();
        hasher->wait(2000);
        hasher->setFile(file);
        hasher->start();
    }
}

void FileHasher::slotDone(){
    lineEdit_TTH->setText(hasher->getHash());

    pushButton_RUN->setEnabled(true);
}

void FileHasher::search(){
    QString size_str = lineEdit_SIZE->text();
    qulonglong size = size_str.left(size_str.indexOf(" (")).toULongLong();
    search(lineEdit_FNAME->text(), size, lineEdit_TTH->text());
}

void FileHasher::search(const QString &file, const qulonglong &, const QString &tth){
    if (!tth.isEmpty())
        FileHasher::searchTTH(tth);
    else if (!file.isEmpty())
        FileHasher::searchFile(file);
}

void FileHasher::searchTTH(const QString &tth) {
    SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame>();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchAlternates(tth);
}

void FileHasher::searchFile(const QString &file) {
    SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame>();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchFile(file);
}

void FileHasher::slotCopyMagnet(){
    const QString &&tth = lineEdit_TTH->text();
    const QString &&fname = lineEdit_FNAME->text().trimmed();
    const QString &&sizeStr = lineEdit_SIZE->text();

    if (fname.isEmpty())
        return;

    if (tth.isEmpty()) {
        slotStart();
        return;
    }

    const qulonglong &&fileSize = sizeStr.left(sizeStr.indexOf(" (")).toULongLong();
    const QString &&urlStr = WulforUtil::getInstance()->makeMagnet(fname, fileSize, tth);
    qApp->clipboard()->setText(urlStr);
}

void FileHasher::slotCopySearchString(){
    const QString &&fname = lineEdit_FNAME->text().trimmed();

    if (fname.isEmpty())
        return;

    const QString name = fname.split(QDir::separator(), QString::SkipEmptyParts).last();

    // Special searching magnet link:
    const QString &&encoded_name = _q(Util::encodeURI(name.toStdString()));
    const QString &&magnet = "magnet:?kt=" + encoded_name + "&dn=" + encoded_name;

    if (!magnet.isEmpty())
        qApp->clipboard()->setText(magnet, QClipboard::Clipboard);
}

void FileHasher::slotBrowse(){

    const QString &&file = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::homePath(), tr("All files (*.*)"));

    if (!file.isEmpty()){
        lineEdit_FILE->setText(QDir::toNativeSeparators(file));
        lineEdit_FNAME->setText(file.split("/").last().trimmed());

        const qint64 fileSize = QFileInfo(file).size();
        if (fileSize > 0)
            lineEdit_SIZE->setText(QString("%1 (%2)").arg(fileSize).arg(WulforUtil::formatBytes(fileSize)));
        else
            lineEdit_SIZE->setText("0 (0 MiB)");

        lineEdit_TTH->clear();
    }
}

HashThread::HashThread(){
}

HashThread::~HashThread(){
}

void HashThread::run(){
    calculate_tth();
}

void HashThread::setFile(const QString &f) {
    file_name = f;
}

QString HashThread::getHash(){
    return hash;
}

void HashThread::calculate_tth() {
    char TTH[40] = {0};
    char *buf = new char[BUF_SIZE];

    memset(buf, 0, BUF_SIZE);

    try {
        File f(Text::fromT(_tq(file_name)),File::READ, File::OPEN);
        TigerTree tth(max(TigerTree::calcBlockSize(f.getSize(), 10), static_cast<int64_t>(MIN_BLOCK_SIZE)));
        if(f.getSize() > 0) {
            size_t n = BUF_SIZE;
            while( (n = f.read(&buf[0], n)) > 0) {
                tth.update(&buf[0], n);
                n = BUF_SIZE;
            }
        } else {
            tth.update("", 0);
        }
        tth.finalize();
        strncpy(&TTH[0], tth.getRoot().toBase32().c_str(), sizeof(TTH)-1);
        hash = _q(TTH);
        f.close();
    } catch (...) {}

    delete [] buf;
}
