#include "Notification.h"

#include <QMenu>
#include <QList>

#include "WulforUtil.h"
#include "WulforSettings.h"
#include "MainWindow.h"

Notification::Notification(QObject *parent) :
    QObject(parent), tray(NULL), notify(NULL)
{
    switchModule(static_cast<unsigned>(WIGET(WI_NOTIFY_MODULE)));
}

Notification::~Notification(){
    enableTray(false);
    delete notify;
}

void Notification::enableTray(bool enable){
    if (!enable){
        if (tray)
            tray->hide();

        delete tray->contextMenu();
        delete tray;

        tray = NULL;

        MainWindow::getInstance()->setUnload(true);

        WBSET(WB_TRAY_ENABLED, false);
    }
    else {
        tray = new QSystemTrayIcon(this);
        tray->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));

        QMenu *menu = new QMenu();
        menu->setTitle("EiskaltDC++");

        QAction *show_hide = new QAction(tr("Show/Hide window"), menu);
        QAction *close_app = new QAction(tr("Exit"), menu);
        QAction *sep = new QAction(menu);
        sep->setSeparator(true);

        connect(show_hide, SIGNAL(triggered()), this, SLOT(slotShowHide()));
        connect(close_app, SIGNAL(triggered()), this, SLOT(slotExit()));
        connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason)));

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
    if (MainWindow::getInstance()->isActiveWindow())
        return;

    if (title.isEmpty() || msg.isEmpty() || !WBGET(WB_NOTIFY_ENABLED))
        return;

    if (!(static_cast<unsigned>(WIGET(WI_NOTIFY_EVENTMAP)) & static_cast<unsigned>(t)))
        return;

    if (notify)
        notify->showMessage(title, msg, tray);
}

void Notification::slotExit(){
    MainWindow::getInstance()->setUnload(true);
    MainWindow::getInstance()->close();
}

void Notification::slotShowHide(){
    MainWindow *MW = MainWindow::getInstance();

    if (MW->isVisible())
        MW->hide();
    else{
        MW->show();
        MW->raise();
    }
}

void Notification::slotTrayMenuTriggered(QSystemTrayIcon::ActivationReason r){
    if (r == QSystemTrayIcon::Trigger)
        slotShowHide();
}
