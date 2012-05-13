/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "HubManager.h"
#include "HubFrame.h"

#include <QtDebug>

HubManager::HubManager():
        active(NULL)
{
}

HubManager::~HubManager(){
}

void HubManager::registerHubUrl(const QString &url, HubFrame *hub){
    auto it = hubs.find(url);

    if (it != hubs.constEnd() || !hub)
        return;

    hubs.insert(url, hub);

    emit hubRegistered(hub);
}

void HubManager::unregisterHubUrl(const QString &url){
    auto it = hubs.find(url);

    if (it != hubs.end()){
        emit hubUnregistered(it.value());
        
        hubs.erase(it);
    }
}

void HubManager::setActiveHub(HubFrame *f){
    active = f;
}

QObject *HubManager::getHub(const QString &url){
    auto it = hubs.find(url);

    if (it != hubs.constEnd()){
        return it.value();
    }

    return NULL;
}

QList<QObject*> HubManager::getHubs() const {
    QList<QObject*> list;

    auto it = hubs.constBegin();

    for(; it != hubs.constEnd(); ++it)
        list << qobject_cast<QObject*>(const_cast<HubFrame*>(it.value()));

    return list;
}

QObject *HubManager::activeHub() const {
    return active;
}

QObject *HubManager::getHubObject(){
    return qobject_cast<QObject*>(activeHub());
}