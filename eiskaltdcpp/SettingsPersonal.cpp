#include "SettingsPersonal.h"

#include <QComboBox>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SettingsManager.h"

#include "WulforUtil.h"
#include "WulforSettings.h"

using namespace dcpp;

SettingsPersonal::SettingsPersonal(QWidget *parent):
        QWidget(parent)
{
    setupUi(this);

    init();
}

SettingsPersonal::~SettingsPersonal(){

}

void SettingsPersonal::ok(){
    SettingsManager *SM = SettingsManager::getInstance();

    SM->set(SettingsManager::NICK, lineEdit_NICK->text().toStdString());
    SM->set(SettingsManager::EMAIL, lineEdit_EMAIL->text().toStdString());
    SM->set(SettingsManager::DESCRIPTION, lineEdit_DESC->text().toStdString());
    SM->set(SettingsManager::UPLOAD_SPEED, SettingsManager::connectionSpeeds[comboBox_SPEED->currentIndex()]);

    WSSET(WS_DEFAULT_LOCALE, comboBox_ENC->currentText());

    SM->save();

    WulforSettings::getInstance()->save();
}

void SettingsPersonal::init(){
    lineEdit_NICK->setText(SETTING(NICK).c_str());
    lineEdit_EMAIL->setText(SETTING(EMAIL).c_str());
    lineEdit_DESC->setText(SETTING(DESCRIPTION).c_str());
    lineEdit_AWAYMSG->setText(SETTING(DEFAULT_AWAY_MESSAGE).c_str());

    for (StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i){
        comboBox_SPEED->addItem((*i).c_str());

        if (SETTING(UPLOAD_SPEED) == *i)
            comboBox_SPEED->setCurrentIndex(i - SettingsManager::connectionSpeeds.begin());
    }

    QStringList encodings = WulforUtil::getInstance()->encodings();

    comboBox_ENC->addItem(tr("System default"));
    comboBox_ENC->addItems(encodings);

    QString default_enc = WulforSettings::getInstance()->getStr(WS_DEFAULT_LOCALE);

    if (encodings.contains(default_enc))
        comboBox_ENC->setCurrentIndex(encodings.indexOf(default_enc)+1);
    else
        comboBox_ENC->setCurrentIndex(0);

    lineEdit_AWAYMSG->installEventFilter(this);
    lineEdit_DESC->installEventFilter(this);
    lineEdit_EMAIL->installEventFilter(this);
    lineEdit_NICK->installEventFilter(this);

    comboBox_SPEED->installEventFilter(this);
    comboBox_ENC->installEventFilter(this);
}
