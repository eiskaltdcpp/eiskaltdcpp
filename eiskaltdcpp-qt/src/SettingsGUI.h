/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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

private Q_SLOTS:
    void slotChatColorItemClicked(QListWidgetItem *);
    void slotTestAppTheme();
    void slotThemeChanged();
    void slotBrowseFont();
    void slotBrowseLng();
    void slotLngIndexChanged(int);
    void slotUsersChanged();
    void slotIconsChanged();
    void slotGetColor();
    void slotSetTransparency(int);
    void slotResetTransferColors();

Q_SIGNALS:
    void saveFonts();

public Q_SLOTS:
    void ok();

private:
    bool custom_style;

    // clean colors (without transparency)
    QColor h_color;
    QColor shared_files_color;
    QColor chat_background_color;
    QColor downloads_clr;
    QColor uploads_clr;
};

#endif // SETTINGSGUI_H
