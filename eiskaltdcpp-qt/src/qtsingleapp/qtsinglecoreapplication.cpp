/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include "qtsinglecoreapplication.h"

#include <QTimer>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDebug>
#include <QLocalSocket>

static const unsigned int SHARED_MEM_SIZE = 2048;

QtSingleCoreApplication::QtSingleCoreApplication(int &argc, char **argv, const QString &uniqueKey)
    : QApplication(argc, argv), localServer(nullptr), sharedMemory()
{
    sharedMemory.setKey(uniqueKey);
    localServerName = QStringLiteral("edcpp-") +
        QString::fromLatin1(QCryptographicHash::hash(uniqueKey.toUtf8(), QCryptographicHash::Sha1).toHex().left(16));

    QLocalSocket socket;
    socket.connectToServer(localServerName, QIODevice::WriteOnly);
    if (socket.waitForConnected(250)) {
        _isRunning = true;
        return;
    }

    QLocalServer::removeServer(localServerName);
    localServer = new QLocalServer(this);
    if (!localServer->listen(localServerName)) {
        qWarning() << "Unable to create local single-instance server" << localServerName << localServer->errorString();
        delete localServer;
        localServer = nullptr;
    } else {
        connect(localServer, SIGNAL(newConnection()), this, SLOT(receiveLocalConnection()));
    }

    _isRunning = false;

    // Keep the legacy shared-memory channel for crash-handler cleanup and compatibility.
    QByteArray byteArray("0"); // default value to note that no message is available.
    byteArray.resize(SHARED_MEM_SIZE);

    if (!sharedMemory.create(byteArray.size()))
    {
        sharedMemory.attach();
        sharedMemory.detach();
        sharedMemory.create(byteArray.size());
    }

    if (sharedMemory.isAttached()) {
        sharedMemory.lock();
        char *to = (char*)sharedMemory.data();
        const char *from = byteArray.data();
        memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
        sharedMemory.unlock();
    }
}

QtSingleCoreApplication::~QtSingleCoreApplication(){
    if (localServer) {
        localServer->close();
        QLocalServer::removeServer(localServerName);
    }
    sharedMemory.detach();
}

bool QtSingleCoreApplication::isRunning()
{
    return _isRunning;
}


bool QtSingleCoreApplication::sendMessage(QString message)
{
    if (!_isRunning)
        return false;

    QLocalSocket socket;
    socket.connectToServer(localServerName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(1000))
        return false;

    QByteArray payload = message.toUtf8();
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << static_cast<quint32>(payload.size());
    frame.append(payload);

    if (socket.write(frame) != frame.size())
        return false;
    return socket.waitForBytesWritten(1000);
}


void QtSingleCoreApplication::receiveLocalConnection()
{
    while (localServer && localServer->hasPendingConnections()) {
        QLocalSocket *socket = localServer->nextPendingConnection();
        if (!socket)
            continue;

        while (socket->bytesAvailable() < static_cast<qint64>(sizeof(quint32)) && socket->waitForReadyRead(1000)) { }

        if (socket->bytesAvailable() >= static_cast<qint64>(sizeof(quint32))) {
            QDataStream stream(socket);
            stream.setVersion(QDataStream::Qt_5_0);
            quint32 payloadSize = 0;
            stream >> payloadSize;

            while (socket->bytesAvailable() < static_cast<qint64>(payloadSize) && socket->waitForReadyRead(1000)) { }

            if (socket->bytesAvailable() >= static_cast<qint64>(payloadSize)) {
                QByteArray payload = socket->read(payloadSize);
                if (!payload.isEmpty())
                    emit messageReceived(QString::fromUtf8(payload));
            }
        }
        socket->disconnectFromServer();
        socket->deleteLater();
    }
}

void QtSingleCoreApplication::checkForMessage()
{
    sharedMemory.lock();

    QByteArray byteArray = QByteArray((char*)sharedMemory.constData(), sharedMemory.size());

    sharedMemory.unlock();

    if (byteArray.left(1) != "1")
        return;

    byteArray.remove(0, 1);

    QString message = QString::fromUtf8(byteArray.constData());

    emit messageReceived(message);

    // remove message from shared memory.
    byteArray = "0";
    sharedMemory.lock();

    char *to = (char*)sharedMemory.data();
    const char *from = byteArray.data();

    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));

    sharedMemory.unlock();
}
