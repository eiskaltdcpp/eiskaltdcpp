/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
