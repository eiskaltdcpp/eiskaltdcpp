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

    QColor h_color;
};

#endif // SETTINGSGUI_H
