/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dyndns.h"
#include <functional>
#include "dcpp/HttpManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Streams.h"
#include "dcpp/ClientManager.h"

namespace dcpp {

DynDNS::DynDNS() {
    HttpManager::getInstance()->addListener(this);
    Request();
}


DynDNS::~DynDNS() {
    HttpManager::getInstance()->removeListener(this);
}

void DynDNS::on(HttpManagerListener::Failed, HttpConnection* c, const string& str) noexcept {
    if(c != this->c) { return; }
    c = nullptr;
    completeDownload(false, str);
}

void DynDNS::on(HttpManagerListener::Complete, HttpConnection* c, OutputStream* stream) noexcept {
    if(c != this->c) { return; }
    c = nullptr;

    auto str = static_cast<StringOutputStream*>(stream)->getString();
    completeDownload(true, str);
}

void DynDNS::Request() {
    if (BOOLSETTING(DYNDNS_ENABLE)) {
        string tmps = !SETTING(DYNDNS_SERVER).compare(0,7,"http://") ? SETTING(DYNDNS_SERVER) : "http://" + SETTING(DYNDNS_SERVER);
        c = HttpManager::getInstance()->download(tmps);
    }
}

void DynDNS::completeDownload(bool success, const string& html) {
    if (success) {
        string internetIP;
        if (!html.empty()) {
            int start = html.find(":")+2;
            int end = html.find("</body>");


            if ((start == -1) || (end < start)) {
                internetIP = "";
            } else {
                internetIP = html.substr(start, end - start);
            }
        }
        else
            internetIP = "";

        if (!internetIP.empty()) {
            SettingsManager::getInstance()->set(SettingsManager::INTERNETIP, internetIP);
            auto clients = ClientManager::getInstance()->getClients();

            for(auto i : clients) {
                if(i->isConnected()) {
                    i->reloadSettings(false);
                }
            }
        }
    } else {
        Request();
    }
}

void DynDNS::on(TimerManagerListener::Minute, uint64_t aTick) noexcept {
    Request();
}

}
