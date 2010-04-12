#include "UPnP.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/forward.h"
#include "dcpp/Util.h"

#define _DEBUG_UPNP_ 0

#if _DEBUG_UPNP_
#include <QtDebug>
#endif

static int callback_Upnp(IN Upnp_EventType type, IN void *event, IN void *){
    switch (type){
        case UPNP_DISCOVERY_SEARCH_RESULT:
        {
            const Upnp_Discovery *d_e = (const Upnp_Discovery*)event;

            IXML_Document *DescDoc = NULL;

            int ret = -1;

            ret = UpnpDownloadXmlDoc( d_e->Location, &DescDoc );

            if (ret != UPNP_E_SUCCESS ) {
#if _DEBUG_UPNP_
                printf("Error obtaining device description from %s -- error = %d", d_e->Location, ret );
#endif
                ixmlDocument_free(DescDoc);
                return 0;
            }
#if _DEBUG_UPNP_
            qDebug() << "New device located at " << d_e->Location;
#endif

            UPnP::getInstance()->addNewDevice(QString::fromUtf8(d_e->Location), QString::fromUtf8(ixmlDocumenttoString(DescDoc)));

            ixmlDocument_free(DescDoc);

            break;
        }
        default:
            break;
    }

    return 0;
}


UPnP::UPnP():
        initialized(false),
        ctrl_registered(false)
{
}

UPnP::~UPnP(){
    stop();
}

void UPnP::start(){
    if (initialized && ctrl_registered)
        return;

    short port = 3030;
    const char *ip = NULL;

    int ret = -1;

    ret = UpnpInit(ip, port);

    if (ret != UPNP_E_SUCCESS){
        UpnpFinish();

        return;
    }
    else
        initialized = true;

#if _DEBUG_UPNP_
    qDebug() << QString("OK. Server running on %1:%2").arg(UpnpGetServerIpAddress()).arg(UpnpGetServerPort());
#endif

    ret = UpnpRegisterClient(callback_Upnp, NULL, &hndl);

    if (ret != UPNP_E_SUCCESS){
#if _DEBUG_UPNP_
        qDebug() << "Error registering UPnP control point";
#endif
        UpnpFinish();

        initialized = false;

        return;
    }
    else
        ctrl_registered = true;

    ret = UpnpSearchAsync(hndl, 2, "upnp:rootdevice", NULL);

#if _DEBUG_UPNP_
    if (ret != UPNP_E_SUCCESS){
        qDebug() << "Error searching UPnP devices";
    }
#endif
}

void UPnP::stop(){
    initialized = ctrl_registered = false;

    UpnpUnRegisterClient(hndl);
    UpnpFinish();

    DeviceLocationMap::iterator it = deviceMap.begin();

    for (; it != deviceMap.end(); ++ it){
        UPnPDeviceList devices = it.value();

        foreach(UPnPDevice *dev, devices){
            qDeleteAll(dev->services);
            delete dev;
        }
    }

    deviceMap.clear();
}

bool UPnP::forward(UPnP::Port port, UPnP::Protocol proto){
    if (port == 0)
        return false;

    UPnPService *srv = NULL;
    QString controlURL = "";

    if (!getFirstForwardingService(controlURL, &srv) || !srv || controlURL.isEmpty()){
#if _DEBUG_UPNP_
        qDebug() << "No valid forwarding services found.";
#endif
        return false;
    }

    WulforUtil *WU = WulforUtil::getInstance();

    IXML_Document *doc = NULL;
    IXML_Document *out = new IXML_Document;

    QString proto_desc = (proto == TCP) ? "TCP" : "UDP";
    QString internal_ip = (WU->getLocalIPs().size() > 0) ? (WU->getLocalIPs()[0]) : (dcpp::Util::getLocalIp().c_str());
    QString desc = QString("EiskaltDC++ %1 (%2)").arg(port).arg(proto_desc);
    QString p = QString().setNum(port);
    int ret = -1;

    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewExternalPort",  p.toAscii().constData());
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewInternalPort",  p.toAscii().constData());
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewProtocol",      proto_desc.toAscii().constData());
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewInternalClient",internal_ip.toAscii().constData());
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewEnabled",       "1");
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewPortMappingDescription", desc.toAscii().constData());
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewLeaseDuration", "0");
    UpnpAddToAction(&doc, "AddPortMapping", srv->servicetype.toAscii().constData(), "NewRemoteHost",    internal_ip.toAscii().constData());

#if _DEBUG_UPNP_
    qDebug() << QString("Forwarding %1 to %2 (%3).").arg(port).arg(internal_ip).arg(controlURL).toAscii().constData();
#endif

    ret = UpnpSendAction(hndl, controlURL.toAscii().constData(), srv->servicetype.toAscii().constData(), NULL, doc, &out);

    ixmlDocument_free(doc);
    delete out;

    return (ret == UPNP_E_SUCCESS);
}

bool UPnP::unmap(UPnP::Port port, UPnP::Protocol proto){
    if (port == 0)
        return false;

    UPnPService *srv = NULL;
    QString controlURL = "";

    if (!getFirstForwardingService(controlURL, &srv) || !srv || controlURL.isEmpty()){
#if _DEBUG_UPNP_
        qDebug() << "No valid forwarding services found.";
#endif
        return false;
    }
#if _DEBUG_UPNP_
    else{
        qDebug() << "Trying to unmap port: " << port << ", proto: " << ((proto == TCP) ? "TCP" : "UDP");
    }
#endif

    IXML_Document *doc = NULL;
    IXML_Document *out = new IXML_Document;
    WulforUtil *WU = WulforUtil::getInstance();

    QString proto_desc = (proto == TCP) ? "TCP" : "UDP";
    QString internal_ip = (WU->getLocalIPs().size() > 0) ? (WU->getLocalIPs()[0]) : (dcpp::Util::getLocalIp().c_str());
    QString p = QString().setNum(port);
    int ret = -1;

    UpnpAddToAction(&doc, "DeletePortMapping", srv->servicetype.toAscii().constData(), "NewExternalPort",  p.toAscii().constData());
    UpnpAddToAction(&doc, "DeletePortMapping", srv->servicetype.toAscii().constData(), "NewRemoteHost",    internal_ip.toAscii().constData());
    UpnpAddToAction(&doc, "DeletePortMapping", srv->servicetype.toAscii().constData(), "NewProtocol",      proto_desc.toAscii().constData());

    ret = UpnpSendAction(hndl, controlURL.toAscii().constData(), srv->servicetype.toAscii().constData(), NULL, doc, &out);

    ixmlDocument_free(doc);
    delete out;

#if _DEBUG_UPNP_
    qDebug() << ((ret == UPNP_E_SUCCESS)? "Ok":"Fail");
#endif

    return (ret == UPNP_E_SUCCESS);
}

QString UPnP::getExternalIP(){
    UPnPService *srv = NULL;
    QString controlURL = "";

    if (!getFirstForwardingService(controlURL, &srv) || !srv || controlURL.isEmpty()){
#if _DEBUG_UPNP_
        qDebug() << "No valid forwarding services found.";
#endif
        return false;
    }

    IXML_Document *doc = NULL;
    IXML_Document *out = new IXML_Document;

    int ret = -1;

    doc = UpnpMakeAction("GetExternalIPAddress", srv->servicetype.toAscii().constData(), 0, NULL);
    ret = UpnpSendAction(hndl, controlURL.toAscii().constData(), srv->servicetype.toAscii().constData(), NULL, doc, &out);

    ixmlDocument_free(doc);

    QString ext = "";

    if (out && (ret == UPNP_E_SUCCESS)){
        ext = extractValueFromXmlStr(ixmlDocumenttoString(out), "newexternalipaddress");
    }

    delete out;

#if _DEBUG_UPNP_
    qDebug() << "Returned new exteranl IP address: " << ext;
#endif

    return ext;
}

bool UPnP::getFirstForwardingService(QString &controlUrl, UPnP::UPnPService **serv){
    if (!serv)
        return false;

    DeviceLocationMap::iterator it = deviceMap.begin();

    for (; it != deviceMap.end(); ++ it){
        UPnPDeviceList devices = it.value();

        foreach(UPnPDevice *dev, devices){
            foreach(UPnPService *srv, dev->services){
                if (srv->servicetype.contains("WANIPConnection") || srv->servicetype.contains("WANPPPConnection")){
                    *serv = srv;

                    char *abs_path = new char[512];
                    int ret = -1;

                    memset(abs_path, 0x0, 512);

                    ret = UpnpResolveURL(it.key().toAscii().constData(), srv->controlurl.toAscii().constData(), abs_path);

                    abs_path[511] = '\0';

                    if (ret == UPNP_E_SUCCESS){
                        controlUrl = QString::fromUtf8(abs_path);

                        return true;
                    }
                    else
                        return false;
                }
            }
        }
    }

    return false;
}

void UPnP::addNewDevice(const QString &location, const QString &doc){
    if (location.isEmpty() || doc.isEmpty())
        return;

    deviceMapMutex.lock();

    if (!deviceMap.contains(location)){
        QDomDocument dom;
        UPnPDeviceList devices;

        if (dom.setContent(doc)){
            QDomElement dom_el = dom.documentElement();

            parseNode(findDeviceSection(dom_el), devices);
            parseNode(findDeviceListSection(dom_el), devices);

            QDomNode baseUrlNode = findSectionByName(dom_el, "baseurl");
            QString upnpLocationUrl = location;

            if (!baseUrlNode.isNull() && baseUrlNode.isElement())
                upnpLocationUrl = baseUrlNode.toElement().text().isEmpty()? location : baseUrlNode.toElement().text();

            deviceMap.insert(upnpLocationUrl, devices);

#if _DEBUG_UPNP_
            qDebug() << QString("Registered new UPnP device at %1:").arg(upnpLocationUrl).toAscii().constData();

            foreach (UPnPDevice *dev, devices){
                qDebug() << QString("  %1 : %2").arg(dev->friendlyName).arg(dev->modelDescription).toAscii().constData();

                foreach (UPnPService *srv, dev->services){
                    qDebug() << QString("    %1 : %2").arg(srv->serviceid).arg(srv->servicetype).toAscii().constData();
                }
            }
#endif
        }
    }

    deviceMapMutex.unlock();
}

void UPnP::parseNode(const QDomNode &node, UPnPDeviceList &devices){
    if (node.isNull() || !node.isElement())
        return;

    QDomElement domElement = node.toElement();
    QString tag = domElement.tagName().toLower();

    if (tag != "device" && tag != "devicelist")
        return;

    if (tag == "device"){
        UPnPDevice *dev = extractDevice(node);

        if (!dev)
            return;

        devices.append(dev);

        QDomNode serviceListNode = findSectionByName(node, "servicelist");
        DomNodeList serviceNodes = getSubSectionsByName(serviceListNode, "service");

        foreach (QDomNode n, serviceNodes){
            UPnPService *srv = extractService(n);

            if (srv)
                dev->services.append(srv);
        }

        parseNode(findDeviceListSection(node), devices);
    }
    else {
        DomNodeList deviceList = getSubSectionsByName(node, "device");

        foreach (QDomNode n, deviceList)
            parseNode(n, devices);
    }
}

UPnP::UPnPDevice *UPnP::extractDevice (const QDomNode &node){
    if (node.isNull() || !node.isElement() || (node.toElement().tagName().toLower() != "device"))
        return NULL;

    QDomNode domNode = node.firstChild();
    UPnPDevice *dev = new UPnPDevice;

    while (!domNode.isNull()){
        if (domNode.isElement()){
            QDomElement domElement = domNode.toElement();

            if (!domElement.isNull()){
                QString name = domElement.tagName().toLower();
                QString text = domElement.text();

                if (name == "friendlyname")
                    dev->friendlyName = text;
                else if (name == "manufacturer")
                    dev->manufacturer = text;
                else if (name == "modeldescription")
                    dev->modelDescription = text;
                else if (name == "modelname")
                    dev->modelName = text;
                else if (name == "modelnumber")
                    dev->modelNumber = text;
            }
        }

        domNode = domNode.nextSibling();
    }

    return dev;
}

UPnP::UPnPService *UPnP::extractService(const QDomNode &node){
    if (node.isNull() || !node.isElement() || (node.toElement().tagName().toLower() != "service"))
        return NULL;

    QDomNode domNode = node.firstChild();
    UPnPService *srv = new UPnPService;

    while (!domNode.isNull()){
        if (domNode.isElement()){
            QDomElement domElement = domNode.toElement();

            if (!domElement.isNull()){
                QString name = domElement.tagName().toLower();
                QString text = domElement.text();

                if (name == "serviceid")
                    srv->serviceid = text;
                else if (name == "servicetype")
                    srv->servicetype = text;
                else if (name == "controlurl")
                    srv->controlurl = text;
                else if (name == "eventsuburl")
                    srv->eventsuburl = text;
                else if (name == "scpdurl")
                    srv->scpdurl = text;
            }
        }

        domNode = domNode.nextSibling();
    }

    return srv;
}

QString UPnP::extractValueFromXmlStr(const QString &raw_xml, const QString &val_name){
    if (raw_xml.isEmpty() || val_name.isEmpty())
        return "";

    QDomDocument dom;

    if (dom.setContent(raw_xml)){
        QDomElement dom_el = dom.documentElement();

        QDomNode valNode = findSectionByName(dom_el, val_name);

        if (!valNode.isNull() && valNode.isElement())
            return valNode.toElement().text();
    }

    return "";
}

QDomNode UPnP::findDeviceListSection(const QDomNode &node){
    return findSectionByName(node, "devicelist");
}

QDomNode UPnP::findDeviceSection(const QDomNode &node){
    return findSectionByName(node, "device");
}

QDomNode UPnP::findSectionByName(const QDomNode &node, const QString &name){
    QDomNode domNode = node.firstChild();

    while (!domNode.isNull()){
        if (domNode.isElement()){
            QDomElement domElement = domNode.toElement();

            if (!domElement.isNull() && domElement.tagName().toLower() == name.toLower())
                return domNode;
        }

        domNode = domNode.nextSibling();
    }

    return QDomNode();
}

UPnP::DomNodeList UPnP::getSubSectionsByName(const QDomNode &node, const QString &name){
    QDomNode domNode = node.firstChild();
    DomNodeList list;

    while (!domNode.isNull()){
        if (domNode.isElement()){
            QDomElement domElement = domNode.toElement();

            if (!domElement.isNull() && domElement.tagName().toLower() == name.toLower())
                list << domNode;
        }

        domNode = domNode.nextSibling();
    }

    return list;
}
