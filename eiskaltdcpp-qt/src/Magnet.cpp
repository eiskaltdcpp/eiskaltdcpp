#include "Magnet.h"

#include <QUrl>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/QueueManager.h"

#include "WulforUtil.h"
#include "SearchFrame.h"
#include "MainWindow.h"

using namespace dcpp;

Magnet::Magnet(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    pushButton_BROWSE->setIcon(WICON(WulforUtil::eiFOLDER_BLUE));

    connect(pushButton_CANCEL,  SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButton_SEARCH,  SIGNAL(clicked()), this, SLOT(search()));
    connect(pushButton_DOWNLOAD,SIGNAL(clicked()), this, SLOT(download()));
    connect(pushButton_BROWSE, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    if (!SETTING(AUTO_SEARCH)){
        pushButton_DOWNLOAD->setToolTip(tr("Run search alternatives manually."));
    }
    else {
        pushButton_DOWNLOAD->setToolTip(tr("Download file via auto search alternatives"));
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
    QString name = "", tth = "";
    int64_t size = 0;

    WulforUtil::splitMagnet(link, size, tth, name);

    if (WIGET(WI_DEF_MAGNET_ACTION) != 0) {
        if (WIGET(WI_DEF_MAGNET_ACTION) == 2) {
            QString target;
            if (name.isEmpty())
                target = tth;
            else
                target = name;
            QString path=_q(SETTING(DOWNLOAD_DIRECTORY));
            target = path + (path.endsWith(QDir::separator())? QString("") : QDir::separator()) + target.split(QDir::separator(), QString::SkipEmptyParts).last();
            Magnet::download(target, size, tth);
        }
        else if (WIGET(WI_DEF_MAGNET_ACTION) == 1)
            Magnet::search(tth);
    } else {
        checkBox_Remember->setChecked(false);
        Magnet::showUI(name, size, tth);
    }

}
void Magnet::search(){
    QString tth = lineEdit_TTH->text();

    if (checkBox_Remember->isChecked() && WIGET(WI_DEF_MAGNET_ACTION) != 1)
        WISET(WI_DEF_MAGNET_ACTION,1);

    if (tth.isEmpty())
        return;
    Magnet::search(tth);

    accept();
}

void Magnet::download() {
    QString tth = lineEdit_TTH->text();

    if (checkBox_Remember->isChecked() && WIGET(WI_DEF_MAGNET_ACTION) != 2)
        WISET(WI_DEF_MAGNET_ACTION,2);
    if (tth.isEmpty())
        return;
    QString fname = lineEdit_FNAME->text();
    QString path = lineEdit_FPATH->text();
    QString size_str = lineEdit_SIZE->text();

    QString name = path + (path.endsWith(QDir::separator())? QString("") : QDir::separator()) + fname.split(QDir::separator(), QString::SkipEmptyParts).last();
    qulonglong size = size_str.left(size_str.indexOf(" (")).toULongLong();
    Magnet::download(name,size,tth);

    accept();
}

void Magnet::slotBrowse(){
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
        QueueManager::getInstance()->add(_tq(name), size, TTHValue(_tq(tth)), UserPtr(), "");
    }
    catch (const std::exception& e){
        QMessageBox::critical(this, tr("Error"), tr("Some error ocurred when starting download:\n %1").arg(e.what()));
    }
}
void Magnet::search(const QString &tth) {
    SearchFrame *fr = new SearchFrame();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchAlternates(tth);
}
