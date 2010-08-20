/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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

typedef QMap<LibUPnP::Port, LibUPnP::Protocol> UPnPMap;

public:
    void forward();
    void unmap();

private:
    UPnPMapper();
    virtual ~UPnPMapper();

    UPnPMap mapped;
};

#endif // UPNPMAPPER_H
