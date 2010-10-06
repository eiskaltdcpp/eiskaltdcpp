#include "FileHasher.h"

#include <QFile>
#include <QFileDialog>
#include <QUrl>
#include <QClipboard>
#include <QFileInfo>
#include <QMessageBox>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/HashManager.h"
#include "dcpp/CID.h"

#include "WulforUtil.h"
#include "MainWindow.h"

using namespace dcpp;

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
    if (tth != NULL) {
        lineEdit_HASH->setText(_q(tth->toBase32()));
        pushButton_RUN->setEnabled(true);
    } else {
        if (hasher)
            hasher-> terminate();
        hasher->setFile(file);
        hasher->start();
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
    string TTH;
    char *buf = new char[512*1024];
    try {
		File f(Text::fromT(_tq(file_name)),File::READ, File::OPEN);
		TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
		if(f.getSize() > 0) {
				size_t n = 512*1024;
				while( (n = f.read(&buf[0], n)) > 0) {
					tth.update(&buf[0], n);
					n = 512*1024;
				}
		} else {
			tth.update("", 0);
		}
		tth.finalize();
		strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
        hash = _q(TTH);
		f.close();
    } catch (...) {}
}
