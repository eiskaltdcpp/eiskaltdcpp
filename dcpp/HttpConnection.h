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

#pragma once

#include <string>
#include "BufferedSocketListener.h"
#include "HttpConnectionListener.h"
#include "GetSet.h"
#include "Speaker.h"
#include "Util.h"

namespace dcpp {
using std::string;

class HttpConnection : BufferedSocketListener, public Speaker<HttpConnectionListener>, boost::noncopyable
{
public:
    HttpConnection(const string& aUserAgent = Util::emptyString);
    virtual ~HttpConnection();

    void download();
    void download(const StringMap& postData);

    void abort();
    const string& getMimeType() const { return mimeType; }

    int64_t getSize() const { return size; }
    int64_t getDone() const { return done; }
    double getSpeed() const { return speed; }

    GETSET(string, url, Url)
    GETSET(bool, coralized, Coralized)

private:
    enum RequestType { TYPE_GET, TYPE_POST };
    enum ConnectionStates { CONN_UNKNOWN, CONN_OK, CONN_FAILED, CONN_MOVED, CONN_CHUNKED };

    string userAgent;
    string method;
    string file;
    string server;
    string port;
    string requestBody;
    string mimeType;
    int64_t size;
    int64_t done;
    double speed;

    // counters to compute a best-effort speed
    int64_t lastPos;
    uint64_t lastTick;

    ConnectionStates connState;
    RequestType connType;

    BufferedSocket* socket;
    void prepareRequest(RequestType type);
    void abortRequest(bool disconnect);

    void updateSpeed();

    // BufferedSocketListener
    void on(Connected) noexcept;
    void on(Line, const string&) noexcept;
    void on(Data, uint8_t*, size_t) noexcept;
    void on(ModeChange) noexcept;
    void on(Failed, const string&) noexcept;

};

} // namespace dcpp
