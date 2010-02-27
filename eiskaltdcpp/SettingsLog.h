#ifndef SETTINGSLOG_H
#define SETTINGSLOG_H

#include <QWidget>
#include "ui_UISettingsLog.h"

class SettingsLog :
        public QWidget,
        private Ui::UISettingsLog
{
Q_OBJECT
public:
    explicit SettingsLog(QWidget *parent = 0);

private:
    void init();

public slots:
    void ok();
};

#endif // SETTINGSLOG_H
