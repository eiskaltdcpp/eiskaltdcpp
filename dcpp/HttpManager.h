/*
 * Copyright (C) 2001-2013 Jacek Sieka, arnetheduck on gmail point com
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

#pragma once

#include <string>

#include "forward.h"
#include "CriticalSection.h"
#include "HttpConnectionListener.h"
#include "HttpManagerListener.h"
#include "Singleton.h"
#include "Speaker.h"
#include "TimerManager.h"
#include "typedefs.h"

namespace dcpp {

using std::string;

class HttpManager :
    public Singleton<HttpManager>,
    public Speaker<HttpManagerListener>,
    private HttpConnectionListener,
    private TimerManagerListener
{
public:
    /** Send a GET request to the designated url. If unspecified, the stream defaults to a
    StringOutputStream. */
    HttpConnection* download(string url, OutputStream* stream = nullptr);
    /** Send a POST request to the designated url. If unspecified, the stream defaults to a
    StringOutputStream. */
    HttpConnection* download(string url, const StringMap& postData, OutputStream* stream = nullptr);

    void disconnect(const string& url);

    void shutdown();

private:
    struct Conn { HttpConnection* c; OutputStream* stream; bool manageStream; uint64_t remove; };

    friend class Singleton<HttpManager>;

    HttpManager();
    virtual ~HttpManager();

    HttpConnection* makeConn(string&& url, bool coralized, OutputStream* stream);
    Conn* findConn(HttpConnection* c);
    void resetStream(HttpConnection* c);
    void removeLater(HttpConnection* c);

    // HttpConnectionListener
    void on(HttpConnectionListener::Data, HttpConnection*, const uint8_t*, size_t) noexcept;
    void on(HttpConnectionListener::Failed, HttpConnection*, const string&) noexcept;
    void on(HttpConnectionListener::Complete, HttpConnection*) noexcept;
    void on(HttpConnectionListener::Redirected, HttpConnection*, const string&) noexcept;

    // TimerManagerListener
    void on(TimerManagerListener::Minute, uint64_t tick) noexcept;

    mutable CriticalSection cs;
    vector<Conn> conns;
};

} // namespace dcpp
