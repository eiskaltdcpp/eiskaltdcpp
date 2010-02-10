#ifndef SETTINGSPERSONAL_H
#define SETTINGSPERSONAL_H

#include <QWidget>

#include "ui_UISettingsPersonal.h"
#include "SettingsInterface.h"

class SettingsPersonal :
        public QWidget,
        private Ui::UISettingsPersonal,
        public SettingsInterface
{
    Q_OBJECT
public:
    SettingsPersonal(QWidget* = NULL);
    ~SettingsPersonal();

public slots:
    void ok();

private:
    void init();
};

#endif // SETTINGSPERSONAL_H
