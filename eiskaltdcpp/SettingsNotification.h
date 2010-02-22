#ifndef SETTINGSNOTIFICATION_H
#define SETTINGSNOTIFICATION_H

#include <QWidget>

#include "ui_UISettingsNotification.h"

class SettingsNotification :
        public QWidget,
        private Ui::UISettingsNotification
{
Q_OBJECT
public:
    explicit SettingsNotification(QWidget *parent = 0);

private:
    void init();

public slots:
    void ok();

};

#endif // SETTINGSNOTIFICATION_H
