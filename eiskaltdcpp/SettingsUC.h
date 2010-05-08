#ifndef SETTINGSUC_H
#define SETTINGSUC_H

#include <QWidget>
#include "ui_UISettingsUC.h"

class UCModel;

class SettingsUC :
        public QWidget,
        private Ui::UISettingsUC
{
Q_OBJECT
public:
    explicit SettingsUC(QWidget *parent = 0);
    virtual ~SettingsUC();

signals:
    void remUC(const QModelIndex&);

public slots:
    void ok();

private slots:
    void slotRemClicked();

private:
    QModelIndex selectedIndex();

    UCModel *model;
};

#endif // SETTINGSUC_H
