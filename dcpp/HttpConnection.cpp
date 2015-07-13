/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2015 Eugene Petrov, dhamp on ya point ru
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
#include "format.h"
#include "SettingsManager.h"
#include "TimerManager.h"
#include "version.h"

#include "extra/fossa.h"
#include <thread>

namespace dcpp {

static void dcpp_HttpConnection_ev_handler(struct ns_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    HttpConnection* c = ((HttpConnection*)nc->mgr->user_data);

    switch (ev) {
        case NS_CLOSE: {
//          printf("Address %s Close connection\n", c->getUrl().c_str() ); fflush(stdout);
          break;
        }
        case NS_RECV: {
//          printf("Address %s Received %lu \n", c->getUrl().c_str(), nc->recv_mbuf.len); fflush(stdout);
//          printf("Data: %s", string(nc->recv_mbuf.buf, nc->recv_mbuf.len).c_str());fflush(stdout);
          break;
        }
        case NS_CONNECT: {
            if (* (int *) ev_data != 0) {
//                printf("Download failed\n"); fflush(stdout);
                c->fire(HttpConnectionListener::Failed(), c, str(F_("%1%") % (strerror(* (int *) ev_data))));
                c->poll = false;
            }
            break;
        }
        case NS_HTTP_REPLY: {
//            printf("Address %s Download finished\n", c->getUrl().c_str()); fflush(stdout);
            nc->flags |= NSF_CLOSE_IMMEDIATELY;
            c->poll = false;
            string check(nc->recv_mbuf.buf, nc->recv_mbuf.len);
            if (check.find("301") != string::npos || check.find("302") != string::npos) {
                size_t start = check.find("Location");
                if (start != string::npos) {
                    size_t end = check.find("\r\n", start);
                    if (end == string::npos) {
                        break;
                    }
//                    printf("Document moved\n"); fflush(stdout);
                    string location = check.substr(start + 10, end - start - 10);
//                    printf("Locaton: %s\n", location.c_str()); fflush(stdout);
                    if (location.compare(c->getUrl()) != 0) {
//                        printf("Continue download\n"); fflush(stdout);
                        c->setUrl(location);
                        c->download();
                    }
                }
            } else {
                ns_str buf;
                buf = hm->body;
                c->fire(HttpConnectionListener::Complete(), c, buf);
            }
            break;
        }
        default:
          break;
    }
}

HttpConnection::HttpConnection(const string& aUserAgent) :
    poll(false),
    userAgent(aUserAgent)
{
    ns_mgr_init(&mgr, this);
}

HttpConnection::~HttpConnection() {
    ns_mgr_free(&mgr);
}

void HttpConnection::poll_func(void* ptr)
{
    HttpConnection* c = (HttpConnection*) ptr;
    while(c->poll) {
        ns_mgr_poll(&(c->mgr), 1000);
    }
}

void HttpConnection::download(const string& post_data) {
    string extraHeaders;

    if(userAgent.empty()) {
        userAgent = "User-Agent: " APPNAME " v" VERSIONSTRING "\r\n";
    }

    extraHeaders.append(userAgent);

    if(::strcmp(url.substr(url.size() - 4).c_str(), ".bz2") == 0) {
                mimeType = "application/x-bzip2";
    } else mimeType.clear();

    ns_connect_http(&mgr, &dcpp_HttpConnection_ev_handler, url.c_str(),
                    (extraHeaders.empty() ? NULL : extraHeaders.c_str()),
                    (post_data.empty() ? NULL : post_data.c_str()));

//    printf("Start download: %s\n", url.c_str());fflush(stdout);

    if (!poll) {
        poll = true;
        std::thread t(poll_func, this);
        t.detach();
    }
}

} // namespace dcpp
