#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QSystemTrayIcon>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class Notification :
        public QObject,
        public dcpp::Singleton<Notification>
{
Q_OBJECT
friend class dcpp::Singleton<Notification>;

public:
    void enableTray(bool);

private slots:
    void slotExit();
    void slotShowHide();
    void slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason);

private:
    explicit Notification(QObject *parent = 0);
    virtual ~Notification();

    QSystemTrayIcon *tray;
};

#define Notify Notification::getInstance()

#endif // NOTIFICATION_H
