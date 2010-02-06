#ifndef SETTINGSPERSONAL_H
#define SETTINGSPERSONAL_H

#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include "ui_UISettingsPersonal.h"
#include "SettingsInterface.h"

class SettingsPersonal :
        public QWidget,
        private Ui::UISettingsPersonal,
        public SettingsInterface
{
    Q_OBJECT
public:
    SettingsPersonal(QWidget* = NULL);
    ~SettingsPersonal();

public slots:
    void ok();

protected:
    virtual bool eventFilter(QObject*, QEvent*);

private:
    void init();

    bool dirty;
};

#endif // SETTINGSPERSONAL_H
