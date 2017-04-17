/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "CmdDebug.h"
#include "dcpp/Text.h"
#include <QStringList>

CmdDebug::CmdDebug(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    maxLines = spinBoxLines->value();
    plainTextEdit_DEBUG->document()->setMaximumBlockCount(maxLines);
    plainTextEdit_DEBUG->setReadOnly(true);
    plainTextEdit_DEBUG->setMouseTracking(true);
    connect(this, SIGNAL(coreDebugCommand(const QString&, const QString&)), this, SLOT(addOutput(const QString&, const QString&)), Qt::QueuedConnection);
    connect(spinBoxLines, SIGNAL(valueChanged(int)), this, SLOT(maxLinesChanged(int)));
    DebugManager::getInstance()->addListener(this);

    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

CmdDebug::~CmdDebug()
{
    DebugManager::getInstance()->removeListener(this);
}

QWidget *CmdDebug::getWidget() {
    return this;
}

QString CmdDebug::getArenaTitle() {
    return tr("CmdDebug");
}

QString CmdDebug::getArenaShortTitle() {
    return getArenaTitle();
}

QMenu *CmdDebug::getMenu() {
    return NULL;
}

void CmdDebug::on(DebugManagerListener::DebugDetection, const string &com) noexcept {
    // NOTE: we no use this in core
}

void CmdDebug::on(DebugManagerListener::DebugCommand, const string &mess, int typedir, const string &ip) noexcept {
    QString qmess = _q(mess);

    switch(typedir) {
        case dcpp::DebugManager::HUB_IN :
            if(checkBoxHUB_IN->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Hub: [Incoming][" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::HUB_OUT :
            if(checkBoxHUB_OUT->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Hub: [Outgoing][" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::CLIENT_IN:
            if(checkBoxCL_IN->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Client: [Incoming]["  + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::CLIENT_OUT:
            if(checkBoxCL_OUT->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Client: [Outgoing][" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
#ifdef WITH_DHT
        case dcpp::DebugManager::DHT_IN:
            if(checkBoxDHT_IN->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "DHT: [Incoming]["  + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::DHT_OUT:
            if(checkBoxDHT_OUT->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "DHT: [Outgoing][" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
#endif
        default: break;
    }
}

void CmdDebug::addOutput(const QString& msg, const QString& ip) {
    if (checkBoxFilterIP->isChecked()) {
        QStringList list = ip.split(":");
        if (list.isEmpty()) return;
        if (list.at(0).compare(lineEditIP->text().trimmed()) != 0 ) return;
    }
    QString tmp = msg;
    tmp.replace("\r", "");
    plainTextEdit_DEBUG->appendPlainText(tmp);
}

void CmdDebug::maxLinesChanged(int value)
{
    plainTextEdit_DEBUG->document()->setMaximumBlockCount(value);
}
