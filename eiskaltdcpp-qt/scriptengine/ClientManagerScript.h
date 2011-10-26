/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CLIENTMANAGERSCRIPT_H
#define CLIENTMANAGERSCRIPT_H

#include <QObject>
#include <QStringList>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/ClientManager.h"
#include "dcpp/Client.h"
#include "dcpp/ClientManagerListener.h"

class ClientManagerScript :
        public QObject,
        public dcpp::Singleton<ClientManagerScript>,
        public dcpp::ClientManagerListener
{
Q_OBJECT
friend class dcpp::Singleton<ClientManagerScript>;

public Q_SLOTS:
    quint64 getUserCount() const;
    quint64 getAvailable() const;

    QStringList getHubs(const QString& cid) const;
    QStringList getHubNames(const QString& cid) const;
    QStringList getNicks(const QString& cid) const;
    QStringList getConnectedHubs() const;
    QString getConnection(const QString& cid) const;

    bool isConnected(const QString& aUrl) const;
    bool isActive() const;

    void sendPM(const QString &cid, const QString &hubUrl, const QString &msg);    
    QString getMyCID() const;

Q_SIGNALS:
    void connected(const QString &url);
    void updated(const QString &url);
    void disconnected(const QString &url);

protected:
    virtual void on(ClientConnected, dcpp::Client*) throw();
    virtual void on(ClientUpdated, dcpp::Client*) throw();
    virtual void on(ClientDisconnected, dcpp::Client*) throw();

private:
    ClientManagerScript(QObject *parent = 0);
    ClientManagerScript(const ClientManagerScript&) {}
    ~ClientManagerScript();
    ClientManagerScript &operator =(const ClientManagerScript&) { return *this; }

    dcpp::ClientManager *CM;
};

#endif // CLIENTMANAGERSCRIPT_H
