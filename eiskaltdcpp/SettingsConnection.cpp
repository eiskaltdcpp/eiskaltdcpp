#include "SettingsConnection.h"
#include "MainWindow.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Socket.h"

#include <QLineEdit>
#include <QRadioButton>
#include <QList>
#include <QMessageBox>

using namespace dcpp;

SettingsConnection::SettingsConnection( QWidget *parent):
        QWidget(parent),
        dirty(false)
{
    setupUi(this);

    init();
}

bool SettingsConnection::eventFilter(QObject *obj, QEvent *e){
    if ((e->type() == QEvent::KeyRelease) || (e->type() == QEvent::MouseButtonRelease))//May be some settings has been changed
        dirty = true;

    return QWidget::eventFilter(obj, e);
}

#include <QtDebug>

void SettingsConnection::ok(){
    if (!dirty)
        return;

    bool active = !radioButton_PASSIVE->isChecked();
    SettingsManager *SM = SettingsManager::getInstance();

    int old_mode = SETTING(INCOMING_CONNECTIONS);

    if (active){
        QString ip = lineEdit_WANIP->text();

        qDebug() << ip;

        if (!validateIp(ip))
            ip = "";

        if (radioButton_ACTIVE->isChecked())
            SM->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_DIRECT);
        else if (radioButton_PORT->isChecked())
            SM->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_NAT);
        else
            SM->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_UPNP);

        SM->set(SettingsManager::TCP_PORT, spinBox_TCP->value());
        SM->set(SettingsManager::UDP_PORT, spinBox_UDP->value());
        SM->set(SettingsManager::TLS_PORT, spinBox_TLS->value());

        ip.replace(" ", "");
        ip = ip.trimmed();

        qDebug() << ip;

        SM->set(SettingsManager::EXTERNAL_IP, ip.toStdString());
        SM->set(SettingsManager::NO_IP_OVERRIDE, checkBox_DONTOVERRIDE->checkState() == Qt::Checked);
    }
    else {
        SM->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
    }

    bool use_socks = !radioButton_DC->isChecked();
    int type = SETTING(OUTGOING_CONNECTIONS);

    if (use_socks){
        QString ip = lineEdit_SIP->text();

        if (!validateIp(ip)){
            showMsg(tr("No valid SOCKS5 server IP found!"), NULL);

            return;
        }

        int port = lineEdit_SPORT->text().toInt();

        SM->set(SettingsManager::SOCKS_SERVER, lineEdit_SIP->text().toStdString());
        SM->set(SettingsManager::SOCKS_USER, lineEdit_SUSR->text().toStdString());
        SM->set(SettingsManager::SOCKS_PASSWORD, lineEdit_SPSWD->text().toStdString());
        SM->set(SettingsManager::SOCKS_RESOLVE, checkBox_RESOLVE->checkState() == Qt::Checked);
        SM->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_SOCKS5);

        if (port > 0 && port <= 65535)
            SM->set(SettingsManager::SOCKS_PORT, port);
    }
    else{
        SM->set(SettingsManager::OUTGOING_CONNECTIONS, SettingsManager::OUTGOING_DIRECT);
    }

    if (SETTING(OUTGOING_CONNECTIONS) != type)
        Socket::socksUpdated();

    SM->save();

    if (old_mode != SETTING(INCOMING_CONNECTIONS) || old_tcp != (SETTING(TCP_PORT))
        || old_udp != (SETTING(UDP_PORT)) || old_tls != (SETTING(TLS_PORT)))
        {
            MainWindow::getInstance()->startSocket();
        }
}

void SettingsConnection::init(){
    lineEdit_WANIP->setText(QString::fromStdString(SETTING(EXTERNAL_IP)));

    spinBox_TCP->setValue(old_tcp = SETTING(TCP_PORT));
    spinBox_UDP->setValue(old_udp = SETTING(UDP_PORT));
    spinBox_TLS->setValue(old_tls = SETTING(TLS_PORT));

    checkBox_DONTOVERRIDE->setCheckState( SETTING(NO_IP_OVERRIDE)? Qt::Checked : Qt::Unchecked );

    switch (SETTING(INCOMING_CONNECTIONS)){
        case SettingsManager::INCOMING_DIRECT:
        {
            radioButton_ACTIVE->setChecked(true);

            break;
        }
        case SettingsManager::INCOMING_FIREWALL_NAT:
        {
            radioButton_PORT->setChecked(true);

            break;
        }
        case SettingsManager::INCOMING_FIREWALL_PASSIVE:
        {
            radioButton_PASSIVE->setChecked(true);

            break;
        }
        case SettingsManager::INCOMING_FIREWALL_UPNP:
        {
            radioButton_UPNP->setChecked(true);

            break;
        }
    }

    lineEdit_SIP->setText(QString::fromStdString(SETTING(SOCKS_SERVER)));
    lineEdit_SUSR->setText(QString::fromStdString(SETTING(SOCKS_USER)));
    lineEdit_SPORT->setText(QString().setNum(SETTING(SOCKS_PORT)));
    lineEdit_SPSWD->setText(QString::fromStdString(SETTING(SOCKS_PASSWORD)));

    checkBox_RESOLVE->setCheckState( SETTING(SOCKS_RESOLVE)? Qt::Checked : Qt::Unchecked );

    switch (SETTING(OUTGOING_CONNECTIONS)){
        case SettingsManager::OUTGOING_DIRECT:
        {
            radioButton_DC->toggle();

            break;
        }
        case SettingsManager::OUTGOING_SOCKS5:
        {
            radioButton_SOCKS->toggle();

            break;
        }
    }

    slotToggleIncomming();
    slotToggleOutgoing();

    connect(radioButton_ACTIVE, SIGNAL(toggled(bool)), this, SLOT(slotToggleIncomming()));
    connect(radioButton_PORT, SIGNAL(toggled(bool)), this, SLOT(slotToggleIncomming()));
    connect(radioButton_PASSIVE, SIGNAL(toggled(bool)), this, SLOT(slotToggleIncomming()));
    connect(radioButton_UPNP, SIGNAL(toggled(bool)), this, SLOT(slotToggleIncomming()));

    connect(radioButton_DC, SIGNAL(toggled(bool)), this, SLOT(slotToggleOutgoing()));
    connect(radioButton_SOCKS, SIGNAL(toggled(bool)), this, SLOT(slotToggleOutgoing()));

    lineEdit_SIP->installEventFilter(this);
    lineEdit_SPORT->installEventFilter(this);
    lineEdit_SPSWD->installEventFilter(this);
    lineEdit_SUSR->installEventFilter(this);
    lineEdit_WANIP->installEventFilter(this);

    spinBox_TCP->installEventFilter(this);
    spinBox_UDP->installEventFilter(this);
    spinBox_TLS->installEventFilter(this);

    radioButton_ACTIVE->installEventFilter(this);
    radioButton_DC->installEventFilter(this);
    radioButton_PASSIVE->installEventFilter(this);
    radioButton_PORT->installEventFilter(this);
    radioButton_SOCKS->installEventFilter(this);
    radioButton_UPNP->installEventFilter(this);

    checkBox_DONTOVERRIDE->installEventFilter(this);
    checkBox_RESOLVE->installEventFilter(this);
}

void SettingsConnection::slotToggleIncomming(){
    bool b = !radioButton_PASSIVE->isChecked();

    frame->setEnabled(b);
}

void SettingsConnection::slotToggleOutgoing(){
    bool b = !radioButton_DC->isChecked();

    frame_2->setEnabled(b);
}

bool SettingsConnection::validateIp(QString &ip){
    if (ip.isEmpty() || ip.isNull())
        return false;

    QStringList l = ip.split(".", QString::SkipEmptyParts);

    if (l.size() != 4)
        return false;

    QIntValidator v(0, 255, this);

    bool valid = true;
    int pos = 0;

    foreach (QString s, l)
        valid = valid && (v.validate(s, pos) == QValidator::Acceptable);

    return valid;
}

void SettingsConnection::showMsg(QString msg, QWidget *focusTo){
    QMessageBox::warning(this, tr("Warning"), msg);

    if (focusTo)
        focusTo->setFocus();
}

