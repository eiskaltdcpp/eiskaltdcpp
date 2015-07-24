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

#include "ui_UISettingsAdvanced.h"

class SettingsAdvanced : public QWidget, private Ui::UISettingsAdvanced
{
    Q_OBJECT
public:
    explicit SettingsAdvanced(QWidget *parent = 0);
    virtual ~SettingsAdvanced();
public slots:
    void ok();
private:
    void init();
private slots:
    void slotBrowse();
};
