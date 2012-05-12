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

private slots:
    void slotBrowse();
};
