#include "SettingsNotification.h"

#include "WulforSettings.h"
#include "Notification.h"

SettingsNotification::SettingsNotification(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    init();
}

void SettingsNotification::init(){
    {//Text
        groupBox->setChecked(WBGET(WB_NOTIFY_ENABLED));

        unsigned emap = static_cast<unsigned>(WIGET(WI_NOTIFY_EVENTMAP));

        checkBox_NICKSAY->setChecked(emap & Notification::NICKSAY);
        checkBox_ANY->setChecked(emap & Notification::ANY);
        checkBox_PM->setChecked(emap & Notification::PM);
        checkBox_TRDONE->setChecked(emap & Notification::TRANSFER);

        comboBox->setCurrentIndex(WIGET(WI_NOTIFY_MODULE));
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

    WulforSettings::getInstance()->save();
}
