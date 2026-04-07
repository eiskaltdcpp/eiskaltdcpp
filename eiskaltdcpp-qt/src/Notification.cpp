/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "Notification.h"
#include "QtContext.h"
#include "QtContextAware.h"

#include <QMenu>
#include <QList>
#ifndef NO_QT_MULTIMEDIA
#include <QSoundEffect>
#endif
#include <QUrl>
#include <QFile>

#include "WulforUtil.h"
#include "WulforSettings.h"
#include "MainWindow.h"
#include "ShellCommandRunner.h"
#include "Settings.h"

static int getBitPos(unsigned eventId){
    for (unsigned i = 0; i < (sizeof(unsigned)*8); i++){
        if ((eventId >> i) == 1U)
            return static_cast<int>(i);
    }

    return -1;
}

Notification::Notification(dcpp::DCContext& ctx, QObject *parent) :
    QtContextAware(ctx),
    QObject(parent), tray(nullptr), notify(nullptr), suppressSnd(false), suppressTxt(false)
{
    switchModule(static_cast<unsigned>(qtCtx()->settings()->getInt(WI_NOTIFY_MODULE)));

    checkSystemTrayCounter = 0;

    reloadSounds();

    enableTray(qtCtx()->settings()->getBool(WB_TRAY_ENABLED));

    connect(qtCtx()->mainWindow(), &MainWindow::notifyMessage, this, &Notification::showMessage, Qt::QueuedConnection);
}

Notification::~Notification(){
    enableTray(false);
    delete notify;
}

void Notification::enableTray(bool enable){
    if (!enable){
        if (tray)
            tray->hide();

        delete tray;

        tray = nullptr;

#if defined(Q_OS_MAC)
        qtCtx()->mainWindow()->setUnload(false);
#else // defined(Q_OS_MAC)
        qtCtx()->mainWindow()->setUnload(true);
#endif // defined(Q_OS_MAC)

        //qtCtx()->settings()->setBool(WB_TRAY_ENABLED, false);
    }
    else {
        delete tray;

        tray = nullptr;

        if (!QSystemTrayIcon::isSystemTrayAvailable() && checkSystemTrayCounter < 12){
            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(5000);

            connect(timer, &QTimer::timeout, this, &Notification::slotCheckTray);

            timer->start();

            ++checkSystemTrayCounter;

            return;
        }
        else if (!QSystemTrayIcon::isSystemTrayAvailable()){
            qtCtx()->mainWindow()->show();

            return;
        }

        checkSystemTrayCounter = 0;

        tray = new QSystemTrayIcon(this);
        tray->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiICON_APPL)
                    .scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        QMenu *menu = new QMenu(qtCtx()->mainWindow());
        menu->setTitle("EiskaltDC++");

        QMenu *menuAdditional = new QMenu(tr("Additional"), qtCtx()->mainWindow());
        QAction *actSuppressSnd = new QAction(tr("Suppress sound notifications"), menuAdditional);
        QAction *actSuppressTxt = new QAction(tr("Suppress text notifications"), menuAdditional);

        actSuppressSnd->setCheckable(true);
        actSuppressSnd->setChecked(false);

        actSuppressTxt->setCheckable(true);
        actSuppressTxt->setChecked(false);

        menuAdditional->addActions(QList<QAction*>() << actSuppressTxt << actSuppressSnd);

        QAction *show_hide = new QAction(tr("Show/Hide window"), menu);
        QAction *setup_speed_lim = new QAction(tr("Setup speed limits"), menu);
        QAction *close_app = new QAction(tr("Exit"), menu);
        QAction *sep = new QAction(menu);
        sep->setSeparator(true);

        setup_speed_lim->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiSPEED_LIMIT_ON));
        show_hide->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiHIDEWINDOW));
        close_app->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEXIT));

        connect(show_hide, &QAction::triggered, this, &Notification::slotShowHide);
        connect(close_app, &QAction::triggered, this, &Notification::slotExit);
        connect(tray, &QSystemTrayIcon::activated,
                this, &Notification::slotTrayMenuTriggered);
        connect(actSuppressTxt, &QAction::triggered, this, &Notification::slotSuppressTxt);
        connect(actSuppressSnd, &QAction::triggered, this, &Notification::slotSuppressSnd);
        connect(setup_speed_lim, &QAction::triggered, this, &Notification::slotShowSpeedLimits);

        menu->addAction(show_hide);
        menu->addAction(setup_speed_lim);
        menu->addMenu(menuAdditional);
        menu->addActions(QList<QAction*>() << sep << close_app);

        tray->setContextMenu(menu);

        tray->show();

        qtCtx()->mainWindow()->setUnload(false);

        //qtCtx()->settings()->setBool(WB_TRAY_ENABLED, true);
    }
}

void Notification::switchModule(int m){
    Module t = static_cast<Module>(m);

    delete notify;

    if (t == QtNotify)
        notify = new QtNotifyModule();
#ifdef DBUS_NOTIFY
    else
        notify = new DBusNotifyModule();
#else
    else
        notify = new QtNotifyModule();
#endif
}

void Notification::showMessage(int t, const QString &title, const QString &msg){
    // On Mac OS X, the Growl notification system must be installed for this function to display messages.
    if (qtCtx()->settings()->getBool(WB_NOTIFY_ENABLED) && !suppressTxt){
        do {
            if (title.isEmpty() || msg.isEmpty())
                break;

            if ((qtCtx()->mainWindow()->isActiveWindow() && !qtCtx()->settings()->getBool(WB_NOTIFY_SHOW_ON_ACTIVE)) ||
            (!qtCtx()->mainWindow()->isActiveWindow() && qtCtx()->mainWindow()->isVisible() && !qtCtx()->settings()->getBool(WB_NOTIFY_SHOW_ON_VISIBLE)))
                break;

            if (!(static_cast<unsigned>(qtCtx()->settings()->getInt(WI_NOTIFY_EVENTMAP)) & static_cast<unsigned>(t)))
                break;

#if defined(Q_OS_MAC)
            qApp->setWindowIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiMESSAGE_TRAY_ICON));
            qApp->alert(qtCtx()->mainWindow(), 0);
#else // defined(Q_OS_MAC)
            if (tray && t == PM && (!qtCtx()->mainWindow()->isVisible() || qtCtx()->settings()->getBool(WB_NOTIFY_CH_ICON_ALWAYS))){
                tray->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiMESSAGE_TRAY_ICON));

                if (qtCtx()->mainWindow()->isVisible())
                    QApplication::alert(qtCtx()->mainWindow(), 0);
            }
#endif // defined(Q_OS_MAC)

            if (notify)
                notify->showMessage(title, msg, tray);
        } while (false);
    }

    if (qtCtx()->settings()->getBool(WB_NOTIFY_SND_ENABLED) && !suppressSnd){
        do {
            if (!(static_cast<unsigned>(qtCtx()->settings()->getInt(WI_NOTIFY_SNDMAP)) & static_cast<unsigned>(t)))
                break;

            int sound_pos = getBitPos(static_cast<unsigned>(t));

            if (sound_pos >= 0 && sound_pos < sounds.size()){
                QString sound = sounds.at(sound_pos);

                if (sound.isEmpty() || !QFile::exists(sound))
                    break;

                if (!qtCtx()->settings()->getBool(WB_NOTIFY_SND_EXTERNAL)) {
#ifndef NO_QT_MULTIMEDIA
                    auto *effect = new QSoundEffect(this);
                    effect->setSource(QUrl::fromLocalFile(sound));
                    connect(effect, &QSoundEffect::playingChanged, effect, [effect]() {
                        if (!effect->isPlaying()) effect->deleteLater();
                    });
                    effect->play();
#endif
                } else {
                    QString cmd = qtCtx()->settings()->getStr(WS_NOTIFY_SND_CMD);

                    if (cmd.isEmpty())
                        break;

                    ShellCommandRunner *r = new ShellCommandRunner(cmd, QStringList() << sound, this);
                    connect(r, &ShellCommandRunner::finished, this, &Notification::slotCmdFinished);

                    r->start();
                }
            }
        } while (false);
    }
}

void Notification::setToolTip(const QString &DSPEED, const QString &USPEED, const QString &DOWN, const QString &UP){
    if (!qtCtx()->settings()->getBool(WB_TRAY_ENABLED) || !tray)
        return;

    const QString &&out = tr("Speed\n"
                     "Download: %1 "
                     "Upload: %2\n"
                     "Statistics\n"
                     "Downloaded: %3 "
                     "Uploaded: %4")
            .arg(DSPEED).arg(USPEED).arg(DOWN).arg(UP);

    tray->setToolTip(out);
}

void Notification::reloadSounds(){
    QString encoded = qtCtx()->settings()->getStr(WS_NOTIFY_SOUNDS);
    QString decoded = QByteArray::fromBase64(encoded.toUtf8());

    sounds = decoded.split("\n");
}

void Notification::slotExit(){
    if (qtCtx()->settings()->getBool(WB_EXIT_CONFIRM))
        qtCtx()->mainWindow()->show();

    qtCtx()->mainWindow()->setUnload(true);
    qtCtx()->mainWindow()->close();
}

void Notification::slotShowHide(){
    MainWindow *MW = qtCtx()->mainWindow();

    if (MW->isVisible()){
#if defined(Q_OS_WIN)
        MW->hide();
#elif defined(Q_OS_MAC)
        if (!MW->isActiveWindow()){
            MW->activateWindow();
            MW->raise();
        }
#else // Linux, FreeBSD, Haiku, Hurd
        if (MW->isMinimized())
            MW->show();

        if (!MW->isActiveWindow()){
            MW->activateWindow();
            MW->raise();
        }
        else {
            MW->hide();
        }
#endif
    }
    else{
        MW->show();
#if !defined(Q_OS_WIN)
        MW->activateWindow();
#endif
        MW->raise();

#if defined(Q_OS_MAC)
        MW->redrawToolPanel();
#else // defined(Q_OS_MAC)
        if (tray)
            MW->redrawToolPanel();
#endif // defined(Q_OS_MAC)
    }
}

void Notification::slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason r){
    if (r == QSystemTrayIcon::Trigger)
        slotShowHide();
}

void Notification::slotShowSpeedLimits(){
    qtCtx()->mainWindow()->show();
    qtCtx()->mainWindow()->raise();

    Settings settings;
    settings.navigate(Settings::Page::Connection, 1);

    settings.exec();
}

void Notification::slotCmdFinished(bool, QString){
    ShellCommandRunner *r = reinterpret_cast<ShellCommandRunner*>(sender());

    r->exit(0);
    r->wait(100);

    if (r->isRunning())
        r->terminate();

    delete r;
}

void Notification::slotCheckTray(){
    QTimer *timer = qobject_cast<QTimer*>(sender());

    if (!timer)
        return;

    enableTray(true);

    timer->deleteLater();
}

void Notification::slotSuppressTxt(){
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
        setSuppressTxt(act->isChecked());
}

void Notification::slotSuppressSnd(){
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
        setSuppressSnd(act->isChecked());
}

void Notification::resetTrayIcon(){
    if (tray)
        tray->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiICON_APPL)
                    .scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void QtNotifyModule::showMessage(const QString &title, const QString &msg, QObject *obj) {
    QSystemTrayIcon *tray = reinterpret_cast<QSystemTrayIcon*>(obj);

    if (tray)
        tray->showMessage(title, ((msg.length() > 400)? (msg.left(400) + "...") : msg), QSystemTrayIcon::Information, 5000);
}

#ifdef DBUS_NOTIFY
void DBusNotifyModule::showMessage(const QString &title, const QString &msg, QObject *) {
    QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());

    QVariantList args;
    args << QString("EiskaltDC++");
    args << QVariant(QVariant::UInt);
    args << QVariant(qtCtx()->wulforUtil()->getAppIconsPath() + "/" + "icon_appl_big.png");
    args << QString(title);
    args << QString(msg);
    args << QStringList();
    args << QVariantMap();
    args << 5000;

    iface.callWithArgumentList(QDBus::NoBlock, "Notify", args);
}
#endif
