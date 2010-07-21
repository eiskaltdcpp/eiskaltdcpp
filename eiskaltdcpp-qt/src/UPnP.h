/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UPNP_H_
#define UPNP_H_

#include <QObject>
#include <QtXml>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMap>
#include <QMutex>

#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <upnp/ixml.h>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class UPnP :
        public QObject,
        public dcpp::Singleton<UPnP>
{
    Q_OBJECT

friend class dcpp::Singleton<UPnP>;

    struct UPnPService{
        QString serviceid;
        QString servicetype;
        QString controlurl;
        QString eventsuburl;
        QString scpdurl;
    };

    struct UPnPDevice{
        QString friendlyName;
        QString manufacturer;
        QString modelDescription;
        QString modelName;
        QString modelNumber;

        QList<UPnPService*> services;
    };

    typedef QList<UPnPDevice*> UPnPDeviceList;
    typedef QList<UPnPService*> UPnPServiceList;
    typedef QList<QDomNode> DomNodeList;
    typedef QMap<QString, UPnPDeviceList> DeviceLocationMap;

public:

    typedef enum _Protocol{
        TCP=0,
        UDP
    } Protocol;

    typedef unsigned short Port;

    void addNewDevice(const QString&, const QString&);

    void start();
    void stop();

    bool forward(Port, Protocol);
    bool unmap(Port, Protocol);
    QString getExternalIP();

private:
    UPnP();
    virtual ~UPnP();

    QDomNode findDeviceSection(const QDomNode&);
    QDomNode findDeviceListSection(const QDomNode&);

    UPnPDevice  *extractDevice (const QDomNode &);
    UPnPService *extractService(const QDomNode &);
    QString      extractValueFromXmlStr(const QString&, const QString&);

    QDomNode findSectionByName(const QDomNode&, const QString&);
    DomNodeList getSubSectionsByName(const QDomNode&, const QString&);

    bool getFirstForwardingService(QString&, UPnPService**);

    void parseNode(const QDomNode &, UPnPDeviceList&);

    UpnpClient_Handle hndl;

    DeviceLocationMap deviceMap;

    QMutex deviceMapMutex;

    bool initialized;
    bool ctrl_registered;
};

#endif // UPNP_H
