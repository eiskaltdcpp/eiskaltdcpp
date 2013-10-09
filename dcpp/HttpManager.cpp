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

#include "stdinc.h"
#include "HttpManager.h"

#include "format.h"
#include "HttpConnection.h"
#include "SettingsManager.h"
#include "Streams.h"

namespace dcpp {

HttpManager::HttpManager() {
    TimerManager::getInstance()->addListener(this);
}

HttpManager::~HttpManager() {
}

HttpConnection* HttpManager::download(string url, OutputStream* stream) {
    auto conn = makeConn(move(url), SETTING(CORAL), stream);
    conn->download();
    return conn;
}

HttpConnection* HttpManager::download(string url, const StringMap& postData, OutputStream* stream) {
    auto conn = makeConn(move(url), false, stream);
    conn->download(postData);
    return conn;
}

void HttpManager::disconnect(const string& url) {
    HttpConnection* c = nullptr;

    {
        Lock l(cs);
        for(auto& conn: conns) {
            if(conn.c->getUrl() == url) {
                if(!conn.remove) {
                    c = conn.c;
                }
                break;
            }
        }
    }

    if(c) {
        c->abort();
        resetStream(c);
        fire(HttpManagerListener::Failed(), c, _("Disconnected"));
        removeLater(c);
    }
}

void HttpManager::shutdown() {
    TimerManager::getInstance()->removeListener(this);

    Lock l(cs);
    for(auto& conn: conns) {
        delete conn.c;
        if(conn.manageStream) {
            delete conn.stream;
        }
    }
}

HttpConnection* HttpManager::makeConn(string&& url, bool coralized, OutputStream* stream) {
    auto c = new HttpConnection();
    {
        Lock l(cs);
        Conn conn { c, stream ? stream : new StringOutputStream(), !stream, 0 };
        conns.push_back(move(conn));
    }
    c->addListener(this);
    c->setUrl(move(url));
    c->setCoralized(coralized);
    fire(HttpManagerListener::Added(), c);
    return c;
}

HttpManager::Conn* HttpManager::findConn(HttpConnection* c) {
    for(auto& conn: conns) {
        if(conn.c == c) {
            return &conn;
        }
    }
    return nullptr;
}

void HttpManager::resetStream(HttpConnection* c) {
    OutputStream* stream = nullptr;
    {
        Lock l(cs);
        auto conn = findConn(c);
        if(conn->manageStream) {
                stream = conn->stream;
        }
    }
    if(stream) {
        static_cast<StringOutputStream*>(stream)->stringRef().clear();
    } else {
        fire(HttpManagerListener::ResetStream(), c);
    }
}

void HttpManager::removeLater(HttpConnection* c) {
    auto later = GET_TICK() + 60 * 1000;
    Lock l(cs);
    findConn(c)->remove = later;
}

void HttpManager::on(HttpConnectionListener::Data, HttpConnection* c, const uint8_t* data, size_t len) noexcept {
    OutputStream* stream;
    {
        Lock l(cs);
        stream = findConn(c)->stream;
    }
    stream->write(data, len);
    fire(HttpManagerListener::Updated(), c);
}

void HttpManager::on(HttpConnectionListener::Failed, HttpConnection* c, const string& str) noexcept {
    resetStream(c);

    if(c->getCoralized()) {
        fire(HttpManagerListener::Removed(), c);
        c->setCoralized(false);
        fire(HttpManagerListener::Added(), c);
        c->download();
        return;
    }

    fire(HttpManagerListener::Failed(), c, str);
    removeLater(c);
}

void HttpManager::on(HttpConnectionListener::Complete, HttpConnection* c) noexcept {
    OutputStream* stream;
    {
        Lock l(cs);
        stream = findConn(c)->stream;
    }
    stream->flush();
    fire(HttpManagerListener::Complete(), c, stream);
    removeLater(c);
}

void HttpManager::on(HttpConnectionListener::Redirected, HttpConnection* c, const string& redirect) noexcept {
    resetStream(c);

    fire(HttpManagerListener::Removed(), c);
    c->setUrl(redirect);
    fire(HttpManagerListener::Added(), c);
}

void HttpManager::on(TimerManagerListener::Minute, uint64_t tick) noexcept {
    vector<pair<HttpConnection*, OutputStream*>> removed;

    {
        Lock l(cs);
        conns.erase(std::remove_if(conns.begin(), conns.end(), [tick, &removed](const Conn& conn) -> bool {
            if(conn.remove && tick > conn.remove) {
                removed.emplace_back(conn.c, conn.manageStream ? conn.stream : nullptr);
                return true;
            }
            return false;
        }), conns.end());
    }

    for(auto& rem: removed) {
        fire(HttpManagerListener::Removed(), rem.first);
        delete rem.first;
        if(rem.second) {
            delete rem.second;
        }
    }
}

} // namespace dcpp
