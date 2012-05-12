/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

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

private:
    void init();

    bool validateIp(QString&);
    void showMsg(QString, QWidget* = NULL);

    bool dirty;

    int old_tcp, old_udp, old_tls, old_dht;
};
