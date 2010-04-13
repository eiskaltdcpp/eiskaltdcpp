#include "Notification.h"

#include <QMenu>
#include <QList>
#include <QSound>
#include <QFile>

#include "WulforUtil.h"
#include "WulforSettings.h"
#include "MainWindow.h"
#include "ShellCommandRunner.h"

static int getBitPos(unsigned eventId){
    for (unsigned i = 0; i < sizeof(unsigned); i++){
        if ((eventId >> i) == 1U)
            return static_cast<int>(i);
    }

    return -1;
}

Notification::Notification(QObject *parent) :
    QObject(parent), tray(NULL), notify(NULL)
{
    switchModule(static_cast<unsigned>(WIGET(WI_NOTIFY_MODULE)));

    reloadSounds();
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

        MainWindow::getInstance()->setUnload(true);

        WBSET(WB_TRAY_ENABLED, false);
    }
    else {
        delete tray;

        if (!QSystemTrayIcon::isSystemTrayAvailable()){
            WBSET(WB_TRAY_ENABLED, false);

            return;
        }

        tray = new QSystemTrayIcon(this);
        tray->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));

        QMenu *menu = new QMenu();
        menu->setTitle("EiskaltDC++");

        QAction *show_hide = new QAction(tr("Show/Hide window"), menu);
        QAction *close_app = new QAction(tr("Exit"), menu);
        QAction *sep = new QAction(menu);
        sep->setSeparator(true);

        show_hide->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiHIDEWINDOW));
        close_app->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEXIT));

        connect(show_hide, SIGNAL(triggered()), this, SLOT(slotShowHide()));
        connect(close_app, SIGNAL(triggered()), this, SLOT(slotExit()));
        connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason)));

        menu->addActions(QList<QAction*>() << show_hide << sep << close_app);

        tray->setContextMenu(menu);

        tray->show();

        MainWindow::getInstance()->setUnload(false);

        WBSET(WB_TRAY_ENABLED, true);
    }
}

void Notification::switchModule(int m){
    Module t = static_cast<Module>(m);

    delete notify;

    if (t == QtNotify)
        notify = new QtNotifyModule();
    else
        notify = new DBusNotifyModule();
}

void Notification::showMessage(Notification::Type t, const QString &title, const QString &msg){
    if (WBGET(WB_NOTIFY_ENABLED)){
        do {
            if (title.isEmpty() || msg.isEmpty())
                break;

            if (MainWindow::getInstance()->isActiveWindow() && !WBGET(WB_NOTIFY_SHOW_ON_ACTIVE) ||
		!MainWindow::getInstance()->isActiveWindow() && MainWindow::getInstance()->isVisible() && !WBGET(WB_NOTIFY_SHOW_ON_VISIBLE))
                break;

            if (!(static_cast<unsigned>(WIGET(WI_NOTIFY_EVENTMAP)) & static_cast<unsigned>(t)))
                break;

            if (tray && t == PM && (!MainWindow::getInstance()->isVisible() || WBGET(WB_NOTIFY_CH_ICON_ALWAYS)))
                tray->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiMESSAGE_TRAY_ICON));

            if (notify)
                notify->showMessage(title, msg, tray);
        } while (0);
    }

    if (WBGET(WB_NOTIFY_SND_ENABLED)){
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
    if (!WBGET(WB_TRAY_ENABLED))
        return;

    QString out = tr("<b>Speed</b><br/>"
                     "Download: <font color=\"green\">%1</font> Upload: <font color=\"red\">%2</font><br/>"
                     "<b>Statistics</b><br/>"
                     "Downloaded: <font color=\"green\">%3</font> Uploaded: <font color=\"red\">%4</font>")
                  .arg(DSPEED).arg(USPEED).arg(DOWN).arg(UP);

    tray->setToolTip(out);
}

void Notification::reloadSounds(){
    QString encoded = WSGET(WS_NOTIFY_SOUNDS);
    QString decoded = QByteArray::fromBase64(encoded.toAscii());

    sounds = decoded.split("\n");
}

void Notification::slotExit(){
    MainWindow::getInstance()->setUnload(true);
    MainWindow::getInstance()->close();
}

void Notification::slotShowHide(){
    MainWindow *MW = MainWindow::getInstance();

    if (MW->isVisible()){
        MW->hide();
    }
    else{
        MW->show();
        MW->raise();

        if (tray)
            tray->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));
    }
}

void Notification::slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason r){
    if (r == QSystemTrayIcon::Trigger)
        slotShowHide();
}

void Notification::slotCmdFinished(bool, QString){
    ShellCommandRunner *r = reinterpret_cast<ShellCommandRunner*>(sender());

    r->exit(0);
    r->wait(100);

    if (r->isRunning())
        r->terminate();

    delete r;
}
