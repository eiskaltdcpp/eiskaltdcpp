#include "HubManager.h"
#include "HubFrame.h"


HubManager::HubManager():
        active(NULL)
{
}

HubManager::~HubManager(){
}

void HubManager::registerHubUrl(const QString &url, HubFrame *hub){
    HubHash::const_iterator it = hubs.find(url);

    if (it != hubs.constEnd() || !hub)
        return;

    hubs.insert(url, hub);

    connect(hub, SIGNAL(newMessage(HubFrame*,QString,QString,QString,QString)), this, SIGNAL(newMessage(HubFrame*,QString,QString,QString,QString)));
}

void HubManager::unregisterHubUrl(const QString &url){
    HubHash::iterator it = hubs.find(url);

    if (it != hubs.constEnd()){
        hubs.erase(it);

        disconnect(*it, SIGNAL(newMessage(HubFrame*,QString,QString,QString,QString)));
    }
}

void HubManager::setActiveHub(HubFrame *f){
    active = f;
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

HubFrame *HubManager::activeHub() const {
    return active;
}

QObject *HubManager::getHubObject(){
    return qobject_cast<QObject*>(activeHub());
}
