/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <dcpp/stdinc.h>
#include <dcpp/UPnP.h>

class UPnPc6 :
    public dcpp::UPnP
{
    public:
        UPnPc6() {}
        virtual ~UPnPc6() {}
    private:
        virtual bool init();

        virtual bool add(const unsigned short port, const dcpp::UPnP::Protocol protocol, const std::string& description);
        virtual bool remove(const unsigned short port, const dcpp::UPnP::Protocol protocol);
        virtual const std::string& getName() const {
            return name;
        }

        virtual std::string getExternalIP();
        static const std::string name;
        virtual bool isIpV6() { return true; }
};
