//      Copyright 2011 Eugene Petrov <dhamp@ya.ru>
//      Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "dyndns.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ClientManager.h"
#include "dcpp/DCPlusPlus.h"

namespace dcpp {

DynDNS::DynDNS(DCContext& ctx) :
    ContextAware(ctx),
    request(false),
    minutesCounter(0),
    httpConnection(this->ctx())
{
    httpConnection.addListener(this);
}

DynDNS::~DynDNS() {
    httpConnection.removeListener(this);
}

void DynDNS::load()
{
    request = true;
    minutesCounter = 0;
    Request();
}

void DynDNS::stop()
{
    request = false;
}

void DynDNS::Request() {
    if (CTX_BOOLSETTING(DYNDNS_ENABLE)) {
        string tmps = CTX_SETTING(DYNDNS_SERVER);
        if (!CTX_SETTING(DYNDNS_SERVER).compare(0,7,"http://") &&
                !CTX_SETTING(DYNDNS_SERVER).compare(0,8,"https://")) {
            tmps = "http://" + CTX_SETTING(DYNDNS_SERVER);
        }
        httpConnection.downloadFile(tmps);
    }
}

void DynDNS::on(TimerManagerListener::Minute, uint64_t) noexcept {
    ++minutesCounter;
    if (minutesCounter < 2) {
        return;
    }
    else {
        minutesCounter = 0;
    }

    if (request) {
        Request();
    }
}

void DynDNS::on(HttpConnectionListener::Data, HttpConnection*, const uint8_t* buf, size_t len) noexcept {
    html += string((const char*)buf, len);
}

void DynDNS::on(HttpConnectionListener::Complete, HttpConnection*, string const&) noexcept {
    request = false;
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
    else {
        internetIP = "";
    }

    if (!internetIP.empty()) {
        ctx().getSettingsManager()->set(SettingsManager::INTERNETIP, internetIP);
        Client::List clients = ctx().getClientManager()->getClients();

        for(auto c : clients) {
            if(c->isConnected()) {
                c->reloadSettings(false);
            }
        }
    }
    request = true;
}

void DynDNS::on(HttpConnectionListener::Failed, HttpConnection*, const string&) noexcept {
    if (request) {
        Request();
    }
}

}
