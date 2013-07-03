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

#include "WulforUtil.h"

using namespace dcpp;

static const quint64 MIN_BLOCK_SIZE = 64 * 1024;
static const size_t BUF_SIZE = 64*1024;

FileHasher::FileHasher(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    hasher = new HashThread();

    connect(hasher, SIGNAL(finished()), this, SLOT(slotDone()));
    pushButton_BROWSE->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

    connect(pushButton_RUN,    SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(pushButton_BROWSE, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    connect(pushButton_MAGNET, SIGNAL(clicked()), this, SLOT(slotMagnet()));
}

FileHasher::~FileHasher() {
    if (hasher)
        hasher->terminate();


    delete hasher;
}

void FileHasher::slotStart(){

    QString file = lineEdit_FILE->text();

    if (!QFile::exists(file))
        return;

    pushButton_RUN->setEnabled(false);
    HashManager  *HM = HashManager::getInstance();
    const TTHValue *tth= HM->getFileTTHif(_tq(file));
    if (tth) {
        lineEdit_HASH->setText(_q(tth->toBase32()));
        pushButton_RUN->setEnabled(true);
    } else {
        if (hasher){
            hasher-> terminate();
            hasher->setFile(file);
            hasher->start();
        }
    }
}

void FileHasher::slotDone(){
    lineEdit_HASH->setText(hasher->getHash());

    pushButton_RUN->setEnabled(true);
}

void FileHasher::slotMagnet(){
    QString tthstring = lineEdit_HASH->text(), file = lineEdit_FILE->text();

    if (tthstring.isEmpty()){
        slotStart();
        return;
    }

    qlonglong filesize = QFile(file).size();
    QStringList list = file.split("/");

    file = list.last();

    QString urlStr = WulforUtil::getInstance()->makeMagnet(file, filesize, tthstring);
    qApp->clipboard()->setText(urlStr);
}

void FileHasher::slotBrowse(){

    QString file = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::homePath(), tr("All files (*.*)"));

    if (!file.isEmpty()){
        file = QDir::toNativeSeparators(file);
        lineEdit_FILE->setText(file);
        lineEdit_HASH->setText("");
    }
}

HashThread::HashThread(){
}

HashThread::~HashThread(){
}

void HashThread::run(){
    calculate_tth();
}

void HashThread::setFile(QString f){
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
        strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
        hash = _q(TTH);
        f.close();
    } catch (...) {}

    delete [] buf;
}
