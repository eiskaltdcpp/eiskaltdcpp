#include "SettingsDownloads.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SettingsManager.h"

#include <QFileDialog>

using namespace dcpp;

#define CSTD(a) (QString::fromStdString((a)))
#define CQST(a) (a.toStdString())

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

    SM->set(SettingsManager::DOWNLOAD_DIRECTORY, CQST(dl_dir));
    SM->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, CQST(udl_dir));
    SM->set(SettingsManager::DOWNLOAD_SLOTS, spinBox_MAXDL->value());
    SM->set(SettingsManager::MAX_DOWNLOAD_SPEED, spinBox_NONEWDL->value());
    SM->set(SettingsManager::HTTP_PROXY, CQST(lineEdit_PROXY->text()));

    //Auto-priority
    SM->set(SettingsManager::PRIO_HIGHEST_SIZE, CQST(QString().setNum(spinBox_HTPMAX->value())));
    SM->set(SettingsManager::PRIO_HIGH_SIZE, CQST(QString().setNum(spinBox_HPMAX->value())));
    SM->set(SettingsManager::PRIO_NORMAL_SIZE, CQST(QString().setNum(spinBox_NPMAX->value())));
    SM->set(SettingsManager::PRIO_LOW_SIZE, CQST(QString().setNum(spinBox_LPMAX->value())));

    // Auto-drop
    SM->set(SettingsManager::AUTODROP_SPEED, CQST(QString().setNum(spinBox_DROPSB->value())));
    SM->set(SettingsManager::AUTODROP_ELAPSED, CQST(QString().setNum(spinBox_MINELAPSED->value())));
    SM->set(SettingsManager::AUTODROP_MINSOURCES, CQST(QString().setNum(spinBox_MINSRCONLINE->value())));
    SM->set(SettingsManager::AUTODROP_INTERVAL, CQST(QString().setNum(spinBox_CHECKEVERY->value())));
    SM->set(SettingsManager::AUTODROP_INACTIVITY, CQST(QString().setNum(spinBox_MAXINACT->value())));
    SM->set(SettingsManager::AUTODROP_FILESIZE, CQST(QString().setNum(spinBox_MINFSZ->value())));

    QMap< dcpp::SettingsManager::IntSetting, int >::const_iterator it = other_settings.constBegin();

    for (; it != other_settings.constEnd(); ++it)
        SM->set(it.key(), listWidget->item(it.value())->checkState() == Qt::Checked);

    SM->save();
}

void SettingsDownloads::init(){
    {//Downloads
        lineEdit_DLDIR->setText(CSTD(SETTING(DOWNLOAD_DIRECTORY)));
        lineEdit_UNF_DL_DIR->setText(CSTD(SETTING(TEMP_DOWNLOAD_DIRECTORY)));
        lineEdit_PROXY->setText(CSTD(SETTING(HTTP_PROXY)));

        spinBox_MAXDL->setValue(SETTING(DOWNLOAD_SLOTS));
        spinBox_NONEWDL->setValue(SETTING(MAX_DOWNLOAD_SPEED));

        connect(pushButton_BROWSE, SIGNAL(clicked()), SLOT(slotBrowse()));
        connect(pushButton_BROWSE1, SIGNAL(clicked()), SLOT(slotBrowse()));
    }
    {//Download to
        /**
          TODO: implement
        */
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

        QMap< dcpp::SettingsManager::IntSetting, int >::const_iterator it = other_settings.constBegin();

        for (; it != other_settings.constEnd(); ++it)
            listWidget->item(it.value())->setCheckState(((bool)SettingsManager::getInstance()->get(it.key()))? Qt::Checked : Qt::Unchecked);
    }
}

void SettingsDownloads::slotBrowse(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

    if (dir.isEmpty())
        return;

    if (sender() == pushButton_BROWSE)
        lineEdit_DLDIR->setText(dir);
    else if (sender() == pushButton_BROWSE1)
        lineEdit_UNF_DL_DIR->setText(dir);
}

