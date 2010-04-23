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

private slots:
    void slotChatColorItemClicked(QListWidgetItem *);
    void slotTestAppTheme();
    void slotThemeChanged();
    void slotBrowseFont();
    void slotBrowseLng();
    void slotLngIndexChanged(int);
    void slotIconsChanged();
    void slotGetColor();
    void slotSetTransparency(int);

public slots:
    void ok();

private:
    bool custom_style;

    QColor h_color; // clean color (without transparency)
};

#endif // SETTINGSGUI_H
