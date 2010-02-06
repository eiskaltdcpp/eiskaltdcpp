#ifndef SETTINGSSHARING_H
#define SETTINGSSHARING_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "ui_UISettingsSharing.h"

class SettingsSharing :
        public QWidget,
        private Ui::UISettingsSharing
{
    Q_OBJECT
public:
    SettingsSharing(QWidget* = NULL);
    virtual ~SettingsSharing();
public slots:
    void ok();

private slots:
    void slotRecreateShare();
    void slotContextMenu(const QPoint&);
    void slotShareHidden(bool);

private:
    void init();
    void updateShareView();
};

#endif // SETTINGSSHARING_H
