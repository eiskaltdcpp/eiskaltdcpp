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

signals:
    void timeToDie();

private slots:
    void slotItemActivated(QListWidgetItem*);
    void dirty();

private:
    void init();

    WidgetMap widgets;
    bool is_dirty;
};

#endif // SETTINGS_H
