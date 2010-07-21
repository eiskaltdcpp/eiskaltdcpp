/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SINGLEINSTANCERUNNER_H
#define SINGLEINSTANCERUNNER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

class SingleInstanceRunner: public QObject
{
Q_OBJECT
public:
    explicit SingleInstanceRunner();
    ~SingleInstanceRunner() {}

    bool isServerRunning(const QStringList&);
    void servStop() { serv.close(); }

private slots:
    void slotNewConnection();
    void slotReadyRead();

private:
    QTcpServer serv;
    unsigned EISKALTPORT;
};

#endif // SINGLEINSTANCERUNNER_H
