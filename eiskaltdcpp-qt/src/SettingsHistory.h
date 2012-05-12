/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

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
