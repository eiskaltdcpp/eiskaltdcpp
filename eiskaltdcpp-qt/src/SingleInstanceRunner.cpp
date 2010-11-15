/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SingleInstanceRunner.h"
#include "MainWindow.h"

#ifndef WIN32
#include <stdlib.h>
#include <QHash>
#endif

SingleInstanceRunner::SingleInstanceRunner()
{
#ifndef WIN32
    char *user = getenv("USER");
    QString login = user;
    quint32 hash = qHash(login);
    EISKALTPORT = (hash >> 16) + 4098;

    printf("Internal server running on %i\n", EISKALTPORT);
#else
    EISKALTPORT = 33561;
#endif
}

bool SingleInstanceRunner::isServerRunning(const QStringList &list){
    QTcpSocket sock(NULL);

    sock.connectToHost(QHostAddress("127.0.0.1"), EISKALTPORT, QIODevice::ReadWrite);

    if (!sock.waitForConnected(1000)){
        connect(&serv, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
        serv.listen(QHostAddress("127.0.0.1"), EISKALTPORT);

        return false;
    }

    if (sock.isWritable() && sock.isOpen()){
        QString data = "";

        foreach(QString s, list)
            data += s + "\n";

        sock.write(data.toAscii());

        sock.flush();

        sock.close();

        return true;
    }
    else {
        connect(&serv, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
        serv.listen(QHostAddress("127.0.0.1"), EISKALTPORT);

        return false;
    }
}

void SingleInstanceRunner::slotNewConnection(){
    QTcpSocket *sock = NULL;

    while (serv.hasPendingConnections()){
        sock = serv.nextPendingConnection();
        connect(sock, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    }
}

void SingleInstanceRunner::slotReadyRead(){
    QTcpSocket *sock = reinterpret_cast<QTcpSocket*>(sender());

    QString data = sock->readAll();

    if (!data.isEmpty())
        MainWindow::getInstance()->parseInstanceLine(data);
}
