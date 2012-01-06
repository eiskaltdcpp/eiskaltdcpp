/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SETTINGSHISTORY_H
#define SETTINGSHISTORY_H

#include <QWidget>

#include "ui_UISettingsHistory.h"

class SettingsHistory: public QWidget, protected Ui::UISettingsHistory
{
Q_OBJECT

public:
    explicit SettingsHistory(QWidget* = nullptr);
    virtual ~SettingsHistory();

public Q_SLOTS:
    void ok();

private Q_SLOTS:
    void slotClearSearchHistory();
    void slotClearDirectoriesHistory();
};

#endif // SETTINGSHISTORY_H
