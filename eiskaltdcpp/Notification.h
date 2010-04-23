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
#include <QtDBus>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

#include "WulforUtil.h"

class NotifyModule {
public:
    virtual void showMessage(const QString &, const QString &, QObject *) = 0;
};

class QtNotifyModule: public NotifyModule {
public:
    void showMessage(const QString &title, const QString &msg, QObject *obj){
        QSystemTrayIcon *tray = reinterpret_cast<QSystemTrayIcon*>(obj);

        if (tray)
            tray->showMessage(title, msg, QSystemTrayIcon::Information, 5000);
    }
};

class DBusNotifyModule: public NotifyModule {
public:
    void showMessage(const QString &title, const QString &msg, QObject *){
        QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());

        QVariantList args;
        args << QString("EiskaltDC++");
        args << QVariant(QVariant::UInt);
        args << QVariant(WulforUtil::getInstance()->getIconsPath() + "/" + "icon_appl.png");
        args << QString(title);
        args << QString(msg);
        args << QStringList();
        args << QVariantMap();
        args << 5000;

        iface.callWithArgumentList(QDBus::Block, "Notify", args);
    }
};

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
    ANY=8
};

    void enableTray(bool);
    void showMessage(Type t, const QString&, const QString&);
    void setToolTip(const QString &, const QString &, const QString &, const QString &);
    void reloadSounds();
    void resetTrayIcon(){
        if (tray)
            tray->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));
    }

public slots:
    void switchModule(int);

private slots:
    void slotExit();
    void slotShowHide();
    void slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason);
    void slotCmdFinished(bool, QString);

private:
    explicit Notification(QObject *parent = 0);
    virtual ~Notification();

    QStringList sounds;

    QSystemTrayIcon *tray;
    NotifyModule *notify;
};

#define Notify Notification::getInstance()

#endif // NOTIFICATION_H
