#include "SettingsNotification.h"

#include "WulforSettings.h"
#include "WulforUtil.h"
#include "Notification.h"
#include "ShellCommandRunner.h"

#include <QFileDialog>
#include <QSound>

SettingsNotification::SettingsNotification(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    init();
}

void SettingsNotification::init(){
    WulforUtil *WU = WulforUtil::getInstance();

    {//Text
        groupBox->setChecked(WBGET(WB_NOTIFY_ENABLED));

        unsigned emap = static_cast<unsigned>(WIGET(WI_NOTIFY_EVENTMAP));

        checkBox_NICKSAY->setChecked(emap & Notification::NICKSAY);
        checkBox_ANY->setChecked(emap & Notification::ANY);
        checkBox_PM->setChecked(emap & Notification::PM);
        checkBox_TRDONE->setChecked(emap & Notification::TRANSFER);

        comboBox->setCurrentIndex(WIGET(WI_NOTIFY_MODULE));
    }
    {//Sound
        QString encoded = WSGET(WS_NOTIFY_SOUNDS);
        QString decoded = QByteArray::fromBase64(encoded.toAscii());
        QStringList sounds = decoded.split("\n");

        if (sounds.size() == 3){
            lineEdit_SNDNICKSAY->setText(sounds.at(0));
            lineEdit_SNDPM->setText(sounds.at(1));
            lineEdit_SNDTRDONE->setText(sounds.at(2));
        }

        groupBox_SND->setChecked(WBGET(WB_NOTIFY_SND_ENABLED));
        groupBox_SNDCMD->setChecked(WBGET(WB_NOTIFY_SND_EXTERNAL));

        lineEdit_SNDCMD->setText(WSGET(WS_NOTIFY_SND_CMD));
    }

    toolButton_BRWNICK->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
    toolButton_BRWPM->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
    toolButton_BRWTR->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));

    connect(toolButton_BRWNICK, SIGNAL(clicked()), this, SLOT(slotBrowseFile()));
    connect(toolButton_BRWPM,   SIGNAL(clicked()), this, SLOT(slotBrowseFile()));
    connect(toolButton_BRWTR,   SIGNAL(clicked()), this, SLOT(slotBrowseFile()));

    connect(pushButton_TESTNICKSAY, SIGNAL(clicked()), this, SLOT(slotTest()));
    connect(pushButton_TESTPM,      SIGNAL(clicked()), this, SLOT(slotTest()));
    connect(pushButton_TESTTR,      SIGNAL(clicked()), this, SLOT(slotTest()));

    connect(groupBox_SNDCMD, SIGNAL(toggled(bool)), this, SLOT(slotToggleSndCmd(bool)));
}

void SettingsNotification::playFile(const QString &file){
    if (WBGET(WB_NOTIFY_SND_ENABLED)){
        if (file.isEmpty() || !QFile::exists(file))
            return;

        if (!WBGET(WB_NOTIFY_SND_EXTERNAL))
            QSound::play(file);
        else {
            QString cmd = WSGET(WS_NOTIFY_SND_CMD);

            if (cmd.isEmpty())
                return;

            ShellCommandRunner *r = new ShellCommandRunner(cmd + " " + file, this);
            connect(r, SIGNAL(finished(bool,QString)), this, SLOT(slotCmdFinished(bool,QString)));

            r->start();
        }
    }
}

void SettingsNotification::ok(){
    {//Text
        WBSET(WB_NOTIFY_ENABLED, groupBox->isChecked());

        unsigned emap = 0;

        if (checkBox_ANY->isChecked())
            emap |= Notification::ANY;

        if (checkBox_TRDONE->isChecked())
            emap |= Notification::TRANSFER;

        if (checkBox_NICKSAY->isChecked())
            emap |= Notification::NICKSAY;

        if (checkBox_PM->isChecked())
            emap |= Notification::PM;

        WISET(WI_NOTIFY_EVENTMAP, emap);
        WISET(WI_NOTIFY_MODULE, comboBox->currentIndex());

        Notification::getInstance()->switchModule(comboBox->currentIndex());
    }
    {//Sound
        QString sounds = "";

        sounds += lineEdit_SNDNICKSAY->text() + "\n";
        sounds += lineEdit_SNDPM->text() + "\n";
        sounds += lineEdit_SNDTRDONE->text();

        WSSET(WS_NOTIFY_SOUNDS, sounds.toAscii().toBase64());
        WBSET(WB_NOTIFY_SND_ENABLED, groupBox_SND->isChecked());

        Notification::getInstance()->reloadSounds();
    }

    WulforSettings::getInstance()->save();
}

void SettingsNotification::slotBrowseFile(){
    QString f = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::homePath(), tr("All files (*.*)"));

    if (f.isEmpty())
        return;

    QToolButton *btn = reinterpret_cast<QToolButton*>(sender());

    if (btn == toolButton_BRWNICK)
        lineEdit_SNDNICKSAY->setText(f);
    else if (btn == toolButton_BRWPM)
        lineEdit_SNDPM->setText(f);
    else if (btn == toolButton_BRWTR)
        lineEdit_SNDTRDONE->setText(f);
}

void SettingsNotification::slotTest(){
    QPushButton *btn = reinterpret_cast<QPushButton*>(sender());

    if (btn == pushButton_TESTNICKSAY)
        playFile(lineEdit_SNDNICKSAY->text());
    else if (btn == pushButton_TESTPM)
        playFile(lineEdit_SNDPM->text());
    else if (btn == pushButton_TESTTR)
        playFile(lineEdit_SNDTRDONE->text());
}

void SettingsNotification::slotToggleSndCmd(bool checked){
    WBSET(WB_NOTIFY_SND_EXTERNAL, checked);
    WSSET(WS_NOTIFY_SND_CMD, lineEdit_SNDCMD->text());
}

void SettingsNotification::slotCmdFinished(bool, QString){
    ShellCommandRunner *r = reinterpret_cast<ShellCommandRunner*>(sender());

    r->exit(0);
    r->wait(100);

    if (r->isRunning())
        r->terminate();

    delete r;
}
