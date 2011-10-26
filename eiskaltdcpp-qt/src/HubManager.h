/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef HUBMANAGER_H
#define HUBMANAGER_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QMetaType>

#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
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

Q_SIGNALS:
    void hubRegistered(QObject*);
    void hubUnregistered(QObject*);

public Q_SLOTS:
    QObject *getHubObject();
    QList<QObject*> getHubs() const;
    QObject *getHub(const QString &);
    QObject *activeHub() const;

private:
    explicit HubManager();
    virtual ~HubManager();

    void registerHubUrl(const QString &, HubFrame *);
    void unregisterHubUrl(const QString &);
    void setActiveHub(HubFrame*);

    HubHash hubs;
    HubFrame *active;
};

Q_DECLARE_METATYPE(HubManager*)

#endif // HUBMANAGER_H
