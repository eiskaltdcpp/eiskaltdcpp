#ifndef HUBMANAGER_H
#define HUBMANAGER_H

#include <QObject>
#include <QHash>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class HubFrame;

class HubManager :
        public QObject,
        public dcpp::Singleton<HubManager>
{
Q_OBJECT
friend class dcpp::Singleton<HubManager>;
friend class HubFrame;
typedef QHash<QString, HubFrame*> HubHash;

public:
    HubFrame *getHub(const QString &);
    QList<HubFrame*> getHubs() const;
    HubFrame *activeHub() const;

private:
    explicit HubManager();
    virtual ~HubManager();

    void registerHubUrl(const QString &, HubFrame *);
    void unregisterHubUrl(const QString &);
    void setActiveHub(HubFrame*);

    HubHash hubs;
    HubFrame *active;
};

#endif // HUBMANAGER_H
