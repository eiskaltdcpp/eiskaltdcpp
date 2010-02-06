#ifndef UPNPMAPPER_H
#define UPNPMAPPER_H

#include <QObject>
#include <QMap>

#include "UPnP.h"

class UPnPMapper :
        public QObject,
        public dcpp::Singleton<UPnPMapper>
{
    Q_OBJECT

friend class dcpp::Singleton<UPnPMapper>;

typedef QMap<UPnP::Port, UPnP::Protocol> UPnPMap;

public:
    void forward();
    void unmap();

private:
    UPnPMapper();
    virtual ~UPnPMapper();

    UPnPMap mapped;
};

#endif // UPNPMAPPER_H
