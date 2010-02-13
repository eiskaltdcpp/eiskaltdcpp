#ifndef SETTINGSGUI_H
#define SETTINGSGUI_H

#include <QWidget>
#include "ui_UISettingsGUI.h"

class SettingsGUI :
        public QWidget,
        private Ui::UISettingsGUI
{
Q_OBJECT
public:
    explicit SettingsGUI(QWidget *parent = 0);
    virtual ~SettingsGUI();

private:
    void init();

public slots:
    void ok();
};

#endif // SETTINGSGUI_H
