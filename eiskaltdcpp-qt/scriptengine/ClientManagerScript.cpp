/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ClientManagerScript.h"
#include "WulforUtil.h"

#include "dcpp/CID.h"

static QStringList toQStringList(const dcpp::StringList &list){
    QStringList ret;
    for (dcpp::StringList::const_iterator it = list.begin(); it != list.end(); ){
        ret.push_back(_q(*it));
    }

    return ret;
}

ClientManagerScript::ClientManagerScript(QObject *parent) :
    QObject(parent)
{
    CM = dcpp::ClientManager::getInstance();

    CM->addListener(this);
}

ClientManagerScript::~ClientManagerScript(){
    CM->removeListener(this);
}

quint64 ClientManagerScript::getUserCount() const{
    return CM->getUserCount();
}

quint64 ClientManagerScript::getAvailable() const{
    return CM->getAvailable();
}

QStringList ClientManagerScript::getHubs(const QString& cid) const{
    return toQStringList(CM->getHubs(dcpp::CID(_tq(cid)), ""));
}

QStringList ClientManagerScript::getHubNames(const QString& cid) const{
    StringList hubs = ClientManager::getInstance()->getHubNames(dcpp::CID(_tq(cid)), "");

    if (hubs.empty())
        return QStringList();
    else
        return toQStringList(hubs);
}

QStringList ClientManagerScript::getNicks(const QString& cid) const{
    const dcpp::Identity &user = CM->getOnlineUserIdentity(CM->getUser(dcpp::CID(_tq(cid))));
    QStringList lst;
    lst.push_back(_q(user.getNick()));

    return lst;
}

QStringList ClientManagerScript::getConnectedHubs() const {
    QStringList ret;
    CM->lock();
    dcpp::Client::List& clients = CM->getClients();

    for(dcpp::Client::List::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client* client = *it;

        if(!client->isConnected())
            continue;

        ret.push_back(_q(client->getHubUrl()));
    }

    CM->unlock();

    return ret;
}

QString ClientManagerScript::getConnection(const QString& cid) const{
    return _q(CM->getConnection(dcpp::CID(_tq(cid))));
}

bool ClientManagerScript::isConnected(const QString& aUrl) const{
    return CM->isConnected(_tq(aUrl));
}

bool ClientManagerScript::isActive() const{
    return CM->isActive();
}

QString ClientManagerScript::getMyCID() const{
    return _q(CM->getMyCID().toBase32());
}

void ClientManagerScript::on(ClientConnected, dcpp::Client *cl) throw(){
    emit connected(_q(cl->getHubUrl()));
}

void ClientManagerScript::on(ClientUpdated, dcpp::Client *cl) throw(){
    emit updated(_q(cl->getHubUrl()));
}

void ClientManagerScript::on(ClientDisconnected, dcpp::Client *cl) throw(){
    emit disconnected(_q(cl->getHubUrl()));
}
