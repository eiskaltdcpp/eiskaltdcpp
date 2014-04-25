/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "Notification.h"

#include <QMenu>
#include <QList>
#include <QSound>
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

Notification::Notification(QObject *parent) :
    QObject(parent), tray(NULL), notify(NULL), suppressSnd(false), suppressTxt(false)
{
    switchModule(static_cast<unsigned>(WIGET(WI_NOTIFY_MODULE)));

    checkSystemTrayCounter = 0;

    reloadSounds();

    enableTray(WBGET(WB_TRAY_ENABLED));

    connect(MainWindow::getInstance(), SIGNAL(notifyMessage(int,QString,QString)), this, SLOT(showMessage(int,QString,QString)), Qt::QueuedConnection);
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

        tray = NULL;

#if defined(Q_WS_MAC)
        MainWindow::getInstance()->setUnload(false);
#else // defined(Q_WS_MAC)
        MainWindow::getInstance()->setUnload(true);
#endif // defined(Q_WS_MAC)

        //WBSET(WB_TRAY_ENABLED, false);
    }
    else {
        delete tray;

        tray = NULL;

        if (!QSystemTrayIcon::isSystemTrayAvailable() && checkSystemTrayCounter < 12){
            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(5000);

            connect(timer, SIGNAL(timeout()), this, SLOT(slotCheckTray()));

            timer->start();

            ++checkSystemTrayCounter;

            return;
        }
        else if (!QSystemTrayIcon::isSystemTrayAvailable()){
            MainWindow::getInstance()->show();

            return;
        }

        checkSystemTrayCounter = 0;

        tray = new QSystemTrayIcon(this);
        tray->setIcon(WICON(WulforUtil::eiICON_APPL)
                    .scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        QMenu *menu = new QMenu(MainWindow::getInstance());
        menu->setTitle("EiskaltDC++");

        QMenu *menuAdditional = new QMenu(tr("Additional"), MainWindow::getInstance());
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

        setup_speed_lim->setIcon(WICON(WulforUtil::eiSPEED_LIMIT_ON));
        show_hide->setIcon(WICON(WulforUtil::eiHIDEWINDOW));
        close_app->setIcon(WICON(WulforUtil::eiEXIT));

        connect(show_hide, SIGNAL(triggered()), this, SLOT(slotShowHide()));
        connect(close_app, SIGNAL(triggered()), this, SLOT(slotExit()));
        connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason)));
        connect(actSuppressTxt, SIGNAL(triggered()), this, SLOT(slotSuppressTxt()));
        connect(actSuppressSnd, SIGNAL(triggered()), this, SLOT(slotSuppressSnd()));
        connect(setup_speed_lim, SIGNAL(triggered()), this, SLOT(slotShowSpeedLimits()));

        menu->addAction(show_hide);
        menu->addAction(setup_speed_lim);
        menu->addMenu(menuAdditional);
        menu->addActions(QList<QAction*>() << sep << close_app);

        tray->setContextMenu(menu);

        tray->show();

        MainWindow::getInstance()->setUnload(false);

        //WBSET(WB_TRAY_ENABLED, true);
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
    if (WBGET(WB_NOTIFY_ENABLED) && !suppressTxt){
        do {
            if (title.isEmpty() || msg.isEmpty())
                break;

            if ((MainWindow::getInstance()->isActiveWindow() && !WBGET(WB_NOTIFY_SHOW_ON_ACTIVE)) ||
            (!MainWindow::getInstance()->isActiveWindow() && MainWindow::getInstance()->isVisible() && !WBGET(WB_NOTIFY_SHOW_ON_VISIBLE)))
                break;

            if (!(static_cast<unsigned>(WIGET(WI_NOTIFY_EVENTMAP)) & static_cast<unsigned>(t)))
                break;

#if defined(Q_WS_MAC)
            qApp->setWindowIcon(WICON(WulforUtil::eiMESSAGE_TRAY_ICON));
            qApp->alert(MainWindow::getInstance(), 0);
#else // defined(Q_WS_MAC)
            if (tray && t == PM && (!MainWindow::getInstance()->isVisible() || WBGET(WB_NOTIFY_CH_ICON_ALWAYS))){
                tray->setIcon(WICON(WulforUtil::eiMESSAGE_TRAY_ICON));

                if (MainWindow::getInstance()->isVisible())
                    QApplication::alert(MainWindow::getInstance(), 0);
            }
#endif // defined(Q_WS_MAC)

            if (notify)
                notify->showMessage(title, msg, tray);
        } while (0);
    }

    if (WBGET(WB_NOTIFY_SND_ENABLED) && !suppressSnd){
        do {
            if (!(static_cast<unsigned>(WIGET(WI_NOTIFY_SNDMAP)) & static_cast<unsigned>(t)))
                break;

            int sound_pos = getBitPos(static_cast<unsigned>(t));

            if (sound_pos >= 0 && sound_pos < sounds.size()){
                QString sound = sounds.at(sound_pos);

                if (sound.isEmpty() || !QFile::exists(sound))
                    break;

                if (!WBGET(WB_NOTIFY_SND_EXTERNAL))
                    QSound::play(sound);
                else {
                    QString cmd = WSGET(WS_NOTIFY_SND_CMD);

                    if (cmd.isEmpty())
                        break;

                    ShellCommandRunner *r = new ShellCommandRunner(cmd, QStringList() << sound, this);
                    connect(r, SIGNAL(finished(bool,QString)), this, SLOT(slotCmdFinished(bool,QString)));

                    r->start();
                }
            }
        } while (0);
    }
}

void Notification::setToolTip(const QString &DSPEED, const QString &USPEED, const QString &DOWN, const QString &UP){
    if (!WBGET(WB_TRAY_ENABLED) || !tray)
        return;

#if defined(Q_WS_X11)
    QString out = tr("<b>Speed</b><br/>"
                     "Download: <font_color=\"green\">%1</font> "
                     "Upload: <font_color=\"red\">%2</font><br/>"
                     "<b>Statistics</b><br/>"
                     "Downloaded: <font_color=\"green\">%3</font> "
                     "Uploaded: <font_color=\"red\">%4</font>")
            .arg(DSPEED).arg(USPEED).arg(DOWN).arg(UP);

    out.replace(" ","&nbsp;");
    out.replace("_"," ");
#else
    QString out = tr("Speed\n"
                     "Download: %1 "
                     "Upload: %2\n"
                     "Statistics\n"
                     "Downloaded: %3 "
                     "Uploaded: %4")
            .arg(DSPEED).arg(USPEED).arg(DOWN).arg(UP);
#endif

    tray->setToolTip(out);
}

void Notification::reloadSounds(){
    QString encoded = WSGET(WS_NOTIFY_SOUNDS);
    QString decoded = QByteArray::fromBase64(encoded.toLatin1());

    sounds = decoded.split("\n");
}

void Notification::slotExit(){
    if (WBGET(WB_EXIT_CONFIRM))
        MainWindow::getInstance()->show();

    MainWindow::getInstance()->setUnload(true);
    MainWindow::getInstance()->close();
}

void Notification::slotShowHide(){
    MainWindow *MW = MainWindow::getInstance();

    if (MW->isVisible()){
#if defined(Q_WS_WIN)
        MW->hide();
#elif defined(Q_WS_MAC)
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
        MW->raise();
#if defined(Q_WS_MAC)
        MW->redrawToolPanel();
#else // defined(Q_WS_MAC)
        if (tray)
            MW->redrawToolPanel();
#endif // defined(Q_WS_MAC)
    }
}

void Notification::slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason r){
    if (r == QSystemTrayIcon::Trigger)
        slotShowHide();
}

void Notification::slotShowSpeedLimits(){
    MainWindow::getInstance()->show();
    MainWindow::getInstance()->raise();

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
        tray->setIcon(WICON(WulforUtil::eiICON_APPL)
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
    args << QVariant(WulforUtil::getInstance()->getIconsPath() + "/" + "icon_appl_big.png");
    args << QString(title);
    args << QString(msg);
    args << QStringList();
    args << QVariantMap();
    args << 5000;

    iface.callWithArgumentList(QDBus::NoBlock, "Notify", args);
}
#endif
