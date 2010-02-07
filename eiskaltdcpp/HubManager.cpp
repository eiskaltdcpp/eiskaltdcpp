#include "HubManager.h"
#include "HubFrame.h"

#include <QtDebug>

HubManager::HubManager(){
}

HubManager::~HubManager(){
}

void HubManager::registerHubUrl(const QString &url, HubFrame *hub){
    HubHash::const_iterator it = hubs.find(url);

    if (it != hubs.constEnd() || !hub)
        return;

    hubs.insert(url, hub);

    qDebug() << QString("Registerd new hub: %1 <-> %2").arg(url).arg((unsigned long)hub);
}

void HubManager::unregisterHubUrl(const QString &url){
    HubHash::iterator it = hubs.find(url);

    if (it != hubs.constEnd())
        hubs.erase(it);

    qDebug() << QString("Unregisterd hub: %1").arg(url);
}

HubFrame *HubManager::getHub(const QString &url){
    HubHash::const_iterator it = hubs.find(url);

    if (it != hubs.constEnd())
        return it.value();

    return NULL;
}

QList<HubFrame*> HubManager::getHubs() const {
    QList<HubFrame*> list;

    HubHash::const_iterator it = hubs.constBegin();

    for(; it != hubs.constEnd(); ++it)
        list << const_cast<HubFrame*>(it.value());

    return list;
}
