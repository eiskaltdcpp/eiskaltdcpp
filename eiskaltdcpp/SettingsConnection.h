#ifndef SETTINGSCONNECTION_H
#define SETTINGSCONNECTION_H

#include <QWidget>
#include <QIntValidator>
#include <QEvent>
#include <QKeyEvent>

#include "ui_UISettingsConnection.h"
#include "SettingsInterface.h"

class SettingsConnection :
        public QWidget,
        private Ui::UISettingsConnection
{
    Q_OBJECT
public:
    SettingsConnection(QWidget* = NULL);

public slots:
    void ok();

protected:
    virtual bool eventFilter(QObject*, QEvent*);

private slots:
    void slotToggleIncomming();
    void slotToggleOutgoing();
    void slotThrottle();
    void slotTimeThrottle();
    void slotDownLimitTime(int a);
    void slotDownLimitNormal(int a);
    void slotUpLimitTime(int a);
    void slotUpLimitNormal(int a);

private:
    void init();

    bool validateIp(QString&);
    void showMsg(QString, QWidget* = NULL);

    bool dirty;

    int old_tcp, old_udp, old_tls;
};

#endif // SETTINGSCONNECTION_H
