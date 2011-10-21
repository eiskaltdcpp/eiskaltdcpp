#ifndef SETTINGSHISTORY_H
#define SETTINGSHISTORY_H

#include <QWidget>

#include "ui_UISettingsHistory.h"

class SettingsHistory: public QWidget, protected Ui::UISettingsHistory
{
Q_OBJECT

public:
    explicit SettingsHistory(QWidget* = NULL);
    virtual ~SettingsHistory();

public Q_SLOTS:
    void ok();

private Q_SLOTS:
    void slotClearSearchHistory();
    void slotClearDirectoriesHistory();
};

#endif // SETTINGSHISTORY_H
