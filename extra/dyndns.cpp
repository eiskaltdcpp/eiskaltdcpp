//      Copyright 2011 Eugene Petrov <dhamp@ya.ru>
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
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include "dyndns.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ClientManager.h"

namespace dcpp {

DynDNS::DynDNS()
{
    httpConnection.addListener(this);
    request = true;
    Request();
}


DynDNS::~DynDNS()
{
    httpConnection.removeListener(this);
}

void DynDNS::Request() {
    if (BOOLSETTING(DYNDNS_ENABLE)) {
	httpConnection.setCoralizeState(HttpConnection::CST_NOCORALIZE);
	httpConnection.downloadFile(SETTING(DYNDNS_SERVER));
    }
}

void DynDNS::on(TimerManagerListener::Minute, uint64_t aTick) throw() {
    if (request)
	Request();
}

void DynDNS::on(HttpConnectionListener::Data, HttpConnection*, const uint8_t* buf, size_t len) throw()
{
    html += string((const char*)buf, len);
}

void DynDNS::on(HttpConnectionListener::Complete, HttpConnection*, string const&, bool /*fromCoral*/) throw()
{
    request = false;
    string internetIP;
    if (!html.empty()){
        int start = html.find(":")+2;
        int end = html.find("</body>");


        if ((start == -1) || (end < start)) {
            internetIP = "";
        } else {
            internetIP = html.substr(start, end - start);

            //if (QHostAddress().setAddress(ip)) {
                //internetIP = ip;
            //}
        }
    }
    else
        internetIP = "";

    if (!internetIP.empty()){
        SettingsManager::getInstance()->set(SettingsManager::INTERNETIP, internetIP);
        Client::List clients = ClientManager::getInstance()->getClients();

        for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
            if((*i)->isConnected()) {
                (*i)->reloadSettings(false);
            }
        }
    }
    request = true;
}

void DynDNS::on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw(){
    Request();
}

}
