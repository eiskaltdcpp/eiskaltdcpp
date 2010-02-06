#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>

#include "ui_UISettings.h"

class Settings :
        public QDialog,
        private Ui::UISettings
{
    Q_OBJECT

    typedef QMap<QListWidgetItem*, int> WidgetMap;
public:
    Settings();
    virtual ~Settings();

private slots:
    void slotItemActivated(QListWidgetItem*);

private:
    void init();

    WidgetMap widgets;
};

#endif // SETTINGS_H
