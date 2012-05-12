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
    void playFile(const QString &);

public slots:
    void ok();

private slots:
    void slotBrowseFile();
    void slotTest();
    void slotToggleSndCmd(bool);
    void slotCmdFinished(bool,QString);
};
