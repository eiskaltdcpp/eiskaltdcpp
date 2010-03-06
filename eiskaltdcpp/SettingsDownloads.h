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

private:
    void init();

    QMap< dcpp::SettingsManager::IntSetting, int > other_settings;
};

#endif // SETTINGSDOWNLOADS_H
