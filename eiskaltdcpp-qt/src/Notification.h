/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QSystemTrayIcon>
#ifdef DBUS_NOTIFY
#include <QtDBus>
#endif
#include <QTimer>
#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"

#include "WulforUtil.h"

class NotifyModule {
public:
    virtual void showMessage(const QString &, const QString &, QObject *) = 0;
    virtual ~NotifyModule() { }
};

class QtNotifyModule: public NotifyModule {
public:
    void showMessage(const QString &title, const QString &msg, QObject *obj);
};
#ifdef DBUS_NOTIFY
class DBusNotifyModule: public NotifyModule {
public:
    void showMessage(const QString &title, const QString &msg, QObject *);
};
#endif
class Notification :
        public QObject,
        public dcpp::Singleton<Notification>
{
Q_OBJECT
friend class dcpp::Singleton<Notification>;

public:

enum Module{
    QtNotify=0,
    DBusNotify
};

enum Type{
    NICKSAY=1,
    PM=2,
    TRANSFER=4,
    FAVORITE=8,
    ANY=16
};

    void enableTray(bool);
    void setToolTip(const QString &, const QString &, const QString &, const QString &);
    void reloadSounds();
    void resetTrayIcon();

public Q_SLOTS:
    void switchModule(int);
    void showMessage(int t, const QString&, const QString&);

private Q_SLOTS:
    void slotExit();
    void slotShowHide();
    void slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason);
    void slotCmdFinished(bool, QString);
    void slotCheckTray();
    void slotShowSpeedLimits();
    void slotSupress();

private:
    explicit Notification(QObject *parent = 0);
    virtual ~Notification();

    QStringList sounds;

    QSystemTrayIcon *tray;
    NotifyModule *notify;

    bool supressSnd;
    bool supressTxt;

    int checkSystemTrayCounter;
};

#define Notify Notification::getInstance()

Q_DECLARE_METATYPE(Notification*)
Q_DECLARE_METATYPE(Notification::Type)

#endif // NOTIFICATION_H
