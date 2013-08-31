/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"

#include "HttpConnection.h"

#include "BufferedSocket.h"
#include "format.h"
#include "SettingsManager.h"
#include "TimerManager.h"
#include "version.h"

namespace dcpp {

static const std::string CORAL_SUFFIX = ".nyud.net";
HttpConnection::HttpConnection(const string& aUserAgent) :
coralized(false),
userAgent(aUserAgent),
port("80"),
size(-1),
done(0),
speed(0),
lastPos(0),
lastTick(0),
connState(CONN_UNKNOWN),
socket(0)
{
}

HttpConnection::~HttpConnection() {
    abort();
}
/**
 * Downloads a file and returns it as a string
 * @todo Report exceptions
 * @todo Abort download
 * @param aUrl Full URL of file
 * @return A string with the content, or empty if download failed
 */
void HttpConnection::download() {
    prepareRequest(TYPE_GET);
}
/**
 * Initiates a basic urlencoded form submission
 * @param aFile Fully qualified file URL
 * @param aData StringMap with the args and values
 */
void HttpConnection::download(const StringMap& postData) {
    requestBody.clear();

    for(auto& i: postData)
        requestBody += "&" + Util::encodeURI(i.first) + "=" + Util::encodeURI(i.second);

    if (!requestBody.empty()) requestBody = requestBody.substr(1);
        prepareRequest(TYPE_POST);
}

void HttpConnection::abort() {
    if(socket) {
        abortRequest(true);
    }
}

void HttpConnection::prepareRequest(RequestType type) {
    dcassert(Util::findSubString(url, "http://") == 0 || Util::findSubString(url, "https://") == 0);
    //Util::sanitizeUrl(url);

    // Reset the connection states
    if(connState == CONN_OK || connState == CONN_FAILED)
            userAgent.clear();

    size = -1;
    done = 0;
    speed = 0;
    lastPos = 0;
    lastTick = GET_TICK();

    connState = CONN_UNKNOWN;
    connType = type;

    // method selection
    method = (connType == TYPE_POST) ? "POST" : "GET";

    // set download type
    if(::strcmp(url.substr(url.size() - 4).c_str(), ".bz2") == 0) {
            mimeType = "application/x-bzip2";
    } else mimeType.clear();


    string proto, query, fragment;
    if(SETTING(HTTP_PROXY).empty()) {
        Util::decodeUrl(url, proto, server, port, file, query, fragment);
        if(file.empty())
            file = "/";
    } else {
        Util::decodeUrl(SETTING(HTTP_PROXY), proto, server, port, file, query, fragment);
        file = url;
    }

    if(!query.empty())
        file += '?' + query;

    if(coralized && server.size() > CORAL_SUFFIX.size() &&
        server.compare(server.size() - CORAL_SUFFIX.size(), CORAL_SUFFIX.size(), CORAL_SUFFIX) != 0)
    {
        server += CORAL_SUFFIX;
    }

    if(port.empty())
        port = "80";

    if(userAgent.empty()) {
        if (server == "strongdc.sourceforge.net")
            userAgent = "User-Agent: StrongDC++ v2.42";
        else
            userAgent = "User-Agent: " APPNAME " v" VERSIONSTRING;
    }

    if(!socket)
        socket = BufferedSocket::getSocket(0x0a);

    socket->addListener(this);
    try {
        socket->connect(server, port, (proto == "https"), true, false);
    } catch(const Exception& e) {
        connState = CONN_FAILED;
        fire(HttpConnectionListener::Failed(), this, str(F_("%1% (%2%)") % e.getError() % url));
    }
}

void HttpConnection::abortRequest(bool disconnect) {
    dcassert(socket);

    socket->removeListener(this);
    if(disconnect) socket->disconnect();

    BufferedSocket::putSocket(socket);
    socket = NULL;
}

void HttpConnection::updateSpeed() {
    if(done > lastPos) {
        auto tick = GET_TICK();

        auto tickDelta = static_cast<double>(tick - lastTick);
        if(tickDelta > 0) {
            speed = static_cast<double>(done - lastPos) / tickDelta * 1000.0;
        }

        lastPos = done;
        lastTick = tick;
    }
}

void HttpConnection::on(BufferedSocketListener::Connected) noexcept {
    dcassert(socket);
    socket->write("GET " + file + " HTTP/1.1\r\n");

    string sRemoteServer = server;
    if(!SETTING(HTTP_PROXY).empty())
    {
        string tfile, tport, proto, query, fragment;
        Util::decodeUrl(file, proto, sRemoteServer, tport, tfile, query, fragment);
    }

#ifdef WITH_DHT
    if (sRemoteServer == "strongdc.sourceforge.net")
        socket->write("User-Agent: StrongDC++ v2.42\r\n");
    else
        socket->write("User-Agent: " APPNAME " v" VERSIONSTRING "\r\n");
#else
    socket->write("User-Agent: " APPNAME " v" VERSIONSTRING "\r\n");
#endif

    socket->write("Host: " + sRemoteServer + "\r\n");
    socket->write("Connection: close\r\n"); // we'll only be doing one request
    socket->write("Cache-Control: no-cache\r\n\r\n");
    if (connType == TYPE_POST) socket->write(requestBody);
}

void HttpConnection::on(BufferedSocketListener::Line, const string& aLine) noexcept {
    if(connState == CONN_CHUNKED && aLine.size() > 1) {
        string::size_type i;
        string chunkSizeStr;
        if((i = aLine.find(";")) == string::npos) {
            chunkSizeStr = aLine.substr(0, aLine.length() - 1);
        } else chunkSizeStr = aLine.substr(0, i);

        unsigned long chunkSize = strtoul(chunkSizeStr.c_str(), NULL, 16);
        if(chunkSize == 0 || chunkSize == ULONG_MAX) {
            abortRequest(true);

            if(chunkSize == 0) {
                    connState = CONN_OK;
                    fire(HttpConnectionListener::Complete(), this);
            } else {
                connState = CONN_FAILED;
                fire(HttpConnectionListener::Failed(), this, str(F_("Transfer-encoding error (%1%)") % url));
            }
        } else socket->setDataMode(chunkSize);

    } else if(connState == CONN_UNKNOWN) {
        if(aLine.find("200") != string::npos) {
            connState = CONN_OK;
        } else if(aLine.find("301") != string::npos || aLine.find("302") != string::npos) {
            connState = CONN_MOVED;
        } else {
            abortRequest(true);
            connState = CONN_FAILED;
            fire(HttpConnectionListener::Failed(), this, str(F_("%1% (%2%)") % aLine % url));
        }
    } else if(connState == CONN_MOVED && Util::findSubString(aLine, "Location") != string::npos) {
        abortRequest(true);

        string location = aLine.substr(10, aLine.length() - 10);
        //Util::sanitizeUrl(location);

        // make sure we can also handle redirects with relative paths
        if(location.find("://") == string::npos) {
            if(location[0] == '/') {
                string proto, query, fragment;
                Util::decodeUrl(url, proto, server, port, file, query, fragment);
                string tmp = proto + "://" + server;
                if(port != "80" || port != "443")
                    tmp += ':' + port;
                location = tmp + location;
            } else {
                auto i = url.rfind('/');
                dcassert(i != string::npos);
                location = url.substr(0, i + 1) + location;
            }
        }
        if(location == url) {
            connState = CONN_FAILED;
            fire(HttpConnectionListener::Failed(), this, str(F_("Endless redirection loop (%1%)") % url));
            return;
        }

        fire(HttpConnectionListener::Redirected(), this, location);
        url = location;
        download();

    } else if(aLine[0] == 0x0d) {
        if(size != -1) {
            socket->setDataMode(size);
        } else connState = CONN_CHUNKED;
    } else if(Util::findSubString(aLine, "Content-Length") != string::npos) {
        size = Util::toInt(aLine.substr(16, aLine.length() - 17));

    } else if(mimeType.empty()) {
        if(Util::findSubString(aLine, "Content-Encoding") != string::npos) {
            if(aLine.substr(18, aLine.length() - 19) == "x-bzip2")
                mimeType = "application/x-bzip2";
        } else if(Util::findSubString(aLine, "Content-Type") != string::npos) {
            mimeType = aLine.substr(14, aLine.length() - 15);
        }
    }
}

void HttpConnection::on(BufferedSocketListener::Failed, const string& aLine) noexcept {
    abortRequest(false);
    connState = CONN_FAILED;
    fire(HttpConnectionListener::Failed(), this, str(F_("%1% (%2%)") % aLine % url));
}

void HttpConnection::on(BufferedSocketListener::ModeChange) noexcept {
    if(connState != CONN_CHUNKED) {
        abortRequest(true);
        fire(HttpConnectionListener::Complete(), this);
    }
}
void HttpConnection::on(BufferedSocketListener::Data, uint8_t* aBuf, size_t aLen) noexcept {
    if(size != -1 && static_cast<size_t>(size - done)  < aLen) {
        abortRequest(true);
        connState = CONN_FAILED;
        fire(HttpConnectionListener::Failed(), this, str(F_("Too much data in response body (%1%)") % url));
        return;
    }
    done += aLen;
    updateSpeed();
    fire(HttpConnectionListener::Data(), this, aBuf, aLen);
}

} // namespace dcpp
