/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SETTINGSDOWNLOADS_H
#define SETTINGSDOWNLOADS_H

#include <QWidget>

#include "ui_UISettingsDownloads.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SettingsManager.h"

class SettingsDownloads :
        public QWidget,
        private Ui::UISettingsDownloads
{
    Q_OBJECT
public:
    SettingsDownloads(QWidget* = NULL);
    virtual ~SettingsDownloads();

public slots:
    void  ok();

private slots:
    void slotBrowse();
    void slotDownloadTo();
    void slotCfgPublic();

private:
    void init();

    QMap< dcpp::SettingsManager::IntSetting, int > other_settings;
};

#endif // SETTINGSDOWNLOADS_H
