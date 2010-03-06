#include "SettingsLog.h"
#include "WulforUtil.h"

#include "dcpp/SettingsManager.h"

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
    lineEdit_LOGDIR->setText(_q(SETTING(LOG_DIRECTORY)));

    groupBox_MAINCHAT->setChecked(BOOLSETTING(LOG_MAIN_CHAT));
    lineEdit_CHATFMT->setText(_q(SETTING(LOG_FORMAT_MAIN_CHAT)));

    groupBox_PM->setChecked(BOOLSETTING(LOG_PRIVATE_CHAT));
    lineEdit_PMFMT->setText(_q(SETTING(LOG_FORMAT_PRIVATE_CHAT)));

    groupBox_DOWN->setChecked(BOOLSETTING(LOG_DOWNLOADS));
    lineEdit_DOWNFMT->setText(_q(SETTING(LOG_FORMAT_POST_DOWNLOAD)));

    groupBox_UP->setChecked(BOOLSETTING(LOG_UPLOADS));
    lineEdit_UPFMT->setText(_q(SETTING(LOG_FORMAT_POST_UPLOAD)));

    checkBox_FILELIST->setChecked(BOOLSETTING(LOG_SYSTEM));
    checkBox_STAT->setChecked(BOOLSETTING(LOG_STATUS_MESSAGES));
    checkBox_SYSTEM->setChecked(BOOLSETTING(LOG_SYSTEM));

    toolButton_BROWSE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFOLDER_BLUE));

    connect(toolButton_BROWSE, SIGNAL(clicked()), this, SLOT(slotBrowse()));
}

void SettingsLog::ok(){
    SettingsManager *sm = SettingsManager::getInstance();

    QString path = lineEdit_LOGDIR->text();
    if (!path.isEmpty() && !path.endsWith(QDir::separator()))
        path += QDir::separator();

    sm->set(SettingsManager::LOG_DIRECTORY, _tq(path));
    sm->set(SettingsManager::LOG_MAIN_CHAT, groupBox_MAINCHAT->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_MAIN_CHAT, _tq(lineEdit_CHATFMT->text()));
    sm->set(SettingsManager::LOG_PRIVATE_CHAT, groupBox_PM->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_PRIVATE_CHAT, _tq(lineEdit_PMFMT->text()));
    sm->set(SettingsManager::LOG_DOWNLOADS, groupBox_DOWN->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_POST_DOWNLOAD, _tq(lineEdit_DOWNFMT->text()));
    sm->set(SettingsManager::LOG_UPLOADS, groupBox_UP->isChecked());
    sm->set(SettingsManager::LOG_FORMAT_POST_UPLOAD, _tq(lineEdit_UPFMT->text()));
    sm->set(SettingsManager::LOG_SYSTEM, checkBox_SYSTEM->isChecked());
    sm->set(SettingsManager::LOG_STATUS_MESSAGES, checkBox_STAT->isChecked());
    sm->set(SettingsManager::LOG_FILELIST_TRANSFERS, checkBox_FILELIST->isChecked());

    sm->save();
}

void SettingsLog::slotBrowse(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the directory"), lineEdit_LOGDIR->text());

    if (!dir.isEmpty()){
        if (!dir.endsWith(QDir::separator()))
            dir += QDir::separator();

        lineEdit_LOGDIR->setText(dir);
    }
}
