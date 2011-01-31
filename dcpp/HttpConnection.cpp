/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "HttpConnection.h"

#include "SettingsManager.h"
#include "version.h"

namespace dcpp {

static const std::string CORAL_SUFFIX = ".nyud.net";

/**
 * Downloads a file and returns it as a string
 * @todo Report exceptions
 * @todo Abort download
 * @param aUrl Full URL of file
 * @return A string with the content, or empty if download failed
 */
void HttpConnection::downloadFile(const string& aUrl) {
    dcassert(Util::findSubString(aUrl, "http://") == 0);
    currentUrl = aUrl;
    // Trim spaces
    while(currentUrl[0] == ' ')
        currentUrl.erase(0, 1);
    while(currentUrl[currentUrl.length() - 1] == ' ') {
        currentUrl.erase(currentUrl.length()-1);
    }
    // reset all settings (as in constructor), moved here from onLine(302) because ok was not reset properly
    moved302 = false;
    ok = false;
    size = -1;
    // set download type
    if(Util::stricmp(currentUrl.substr(currentUrl.size() - 4), ".bz2") == 0) {
        fire(HttpConnectionListener::TypeBZ2(), this);
    } else {
        fire(HttpConnectionListener::TypeNormal(), this);
    }

    if(SETTING(HTTP_PROXY).empty()) {
        Util::decodeUrl(currentUrl, server, port, file);
        if(file.empty())
            file = "/";
    } else {
        Util::decodeUrl(SETTING(HTTP_PROXY), server, port, file);
        file = currentUrl;
    }

        if(BOOLSETTING(CORAL) && coralizeState != CST_NOCORALIZE) {
        if(server.length() > CORAL_SUFFIX.length() && server.compare(server.length() - CORAL_SUFFIX.length(), CORAL_SUFFIX.length(), CORAL_SUFFIX) !=0) {
            server += CORAL_SUFFIX;
        } else {
            coralizeState = CST_NOCORALIZE;
        }

    }

    if(port == 0)
        port = 80;

    if(!socket) {
        socket = BufferedSocket::getSocket(0x0a);
    }
    socket->addListener(this);
    try {
        socket->connect(server, port, false, false, false);
    } catch(const Exception& e) {
        fire(HttpConnectionListener::Failed(), this, e.getError() + " (" + currentUrl + ")");
    }
}

void HttpConnection::on(BufferedSocketListener::Connected) throw() {
    dcassert(socket);
    socket->write("GET " + file + " HTTP/1.1\r\n");
    socket->write("User-Agent: " APPNAME " v" VERSIONSTRING "\r\n");

    string sRemoteServer = server;
    if(!SETTING(HTTP_PROXY).empty())
    {
        string tfile;
        uint16_t tport;
        Util::decodeUrl(file, sRemoteServer, tport, tfile);
    }
    socket->write("Host: " + sRemoteServer + "\r\n");
    socket->write("Connection: close\r\n"); // we'll only be doing one request
    socket->write("Cache-Control: no-cache\r\n\r\n");
        if (coralizeState == CST_DEFAULT) coralizeState = CST_CONNECTED;
}

void HttpConnection::on(BufferedSocketListener::Line, const string& aLine) throw() {
    if(!ok) {
                dcdebug("%s\n",aLine.c_str());
        if(aLine.find("200") == string::npos) {
            if(aLine.find("301") != string::npos || aLine.find("302") != string::npos){
                moved302 = true;
            } else {
                socket->disconnect();
                socket->removeListener(this);
                BufferedSocket::putSocket(socket);
                socket = NULL;
                                if(SETTING(CORAL) && coralizeState != CST_NOCORALIZE) {
                                        fire(HttpConnectionListener::Retried(), this, coralizeState == CST_CONNECTED);
                                        coralizeState = CST_NOCORALIZE;
                                        dcdebug("HTTP error with Coral, retrying : %s\n",currentUrl.c_str());
                                        downloadFile(currentUrl);
                                        return;
                                }
                fire(HttpConnectionListener::Failed(), this, aLine + " (" + currentUrl + ")");
                coralizeState = CST_DEFAULT;
                return;
            }
        }
        ok = true;
    } else if(moved302 && Util::findSubString(aLine, "Location") != string::npos){
        dcassert(socket);
        socket->removeListener(this);
        socket->disconnect();
        BufferedSocket::putSocket(socket);
        socket = NULL;

        string location302 = aLine.substr(10, aLine.length() - 11);
        // make sure we can also handle redirects with relative paths
        if(Util::strnicmp(location302.c_str(), "http://", 7) != 0) {
            if(location302[0] == '/') {
                Util::decodeUrl(currentUrl, server, port, file);
                string tmp = "http://" + server;
                if(port != 80)
                    tmp += ':' + Util::toString(port);
                location302 = tmp + location302;
            } else {
                string::size_type i = currentUrl.rfind('/');
                dcassert(i != string::npos);
                location302 = currentUrl.substr(0, i + 1) + location302;
            }
        }
        fire(HttpConnectionListener::Redirected(), this, location302);

        coralizeState = CST_DEFAULT;
        downloadFile(location302);

    } else if(aLine == "\x0d") {
        socket->setDataMode(size);
    } else if(Util::findSubString(aLine, "Content-Length") != string::npos) {
        size = Util::toInt(aLine.substr(16, aLine.length() - 17));
    } else if(Util::findSubString(aLine, "Content-Encoding") != string::npos) {
        if(aLine.substr(18, aLine.length() - 19) == "x-bzip2")
            fire(HttpConnectionListener::TypeBZ2(), this);
    }
}

void HttpConnection::on(BufferedSocketListener::Failed, const string& aLine) throw() {
    socket->removeListener(this);
    BufferedSocket::putSocket(socket);
    socket = NULL;
        if(SETTING(CORAL) && coralizeState != CST_NOCORALIZE) {
                fire(HttpConnectionListener::Retried(), this, coralizeState == CST_CONNECTED);
        coralizeState = CST_NOCORALIZE;
        dcdebug("Coralized address failed, retrying : %s\n",currentUrl.c_str());
        downloadFile(currentUrl);
        return;
    }
    coralizeState = CST_DEFAULT;
    fire(HttpConnectionListener::Failed(), this, aLine + " (" + currentUrl + ")");
}

void HttpConnection::on(BufferedSocketListener::ModeChange) throw() {
    socket->removeListener(this);
    socket->disconnect();
    BufferedSocket::putSocket(socket);
    socket = NULL;
        fire(HttpConnectionListener::Complete(), this, currentUrl, BOOLSETTING(CORAL) && coralizeState != CST_NOCORALIZE);
    coralizeState = CST_DEFAULT;
}
void HttpConnection::on(BufferedSocketListener::Data, uint8_t* aBuf, size_t aLen) throw() {
    fire(HttpConnectionListener::Data(), this, aBuf, aLen);
}

} // namespace dcpp
