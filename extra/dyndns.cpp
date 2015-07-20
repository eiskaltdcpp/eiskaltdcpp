/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dyndns.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ClientManager.h"
#include "dcpp/HttpConnection.h"

namespace dcpp {

DynDNS::DynDNS() {
    Request();
}

DynDNS::~DynDNS() {
}

void DynDNS::on(HttpConnectionListener::Failed, HttpConnection* c, const string& str) noexcept {
    if(c != this->c.get()) { return; }
    c->removeListener(this);
    this->c.release();
    completeDownload(false, str);
}

void DynDNS::on(HttpConnectionListener::Complete, HttpConnection* c, const string& data) noexcept {
    if(c != this->c.get()) { return; }
    c->removeListener(this);
    this->c.release();
    completeDownload(true, data);
}

void DynDNS::Request() {
    if (BOOLSETTING(DYNDNS_ENABLE)) {
        string tmps = !SETTING(DYNDNS_SERVER).compare(0,7,"http://") ? SETTING(DYNDNS_SERVER) : "http://" + SETTING(DYNDNS_SERVER);
        c.reset(new HttpConnection());
        c->addListener(this);
        c->setUrl(tmps);
        c->download();
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
