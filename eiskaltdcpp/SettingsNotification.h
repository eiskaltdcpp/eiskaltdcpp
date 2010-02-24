#ifndef SETTINGSNOTIFICATION_H
#define SETTINGSNOTIFICATION_H

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

#endif // SETTINGSNOTIFICATION_H
