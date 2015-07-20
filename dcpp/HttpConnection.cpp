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

#include <thread>

namespace dcpp {

void HttpConnection::ev_handler(struct ns_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    HttpConnection* c = ((HttpConnection*)nc->mgr->user_data);
    if (c->curr_ns_con != nc) { return; }

    switch (ev) {
        case NS_CLOSE: {
            c->poll = false;
//            printf("NS_CLOSE Address %s Close connection\n", c->getUrl().c_str() ); fflush(stdout);
            break;
        }
        case NS_RECV: {
            c->received += nc->recv_mbuf.len - c->received;
//            printf("NS_RECV Address %s Received %u \n", c->getUrl().c_str(), c->received.load()); fflush(stdout);
//            printf("Data: %s\n", string(nc->recv_mbuf.buf, nc->recv_mbuf.len).c_str());fflush(stdout);
            break;
        }
        case NS_CONNECT: {
            if (* (int *) ev_data != 0) {
//                printf("NS_CONNECT Connect failed\n"); fflush(stdout);
                c->fire(HttpConnectionListener::Failed(), c, str(F_("%1%") % (strerror(* (int *) ev_data))));
                c->poll = false;
            } else {
//                printf("NS_CONNECT Connect successful\n"); fflush(stdout);
            }
            break;
        }
        case NS_HTTP_REPLY: {
//            printf("NS_HTTP_REPLY Address %s Download finished\n", c->getUrl().c_str()); fflush(stdout);
            nc->flags |= NSF_CLOSE_IMMEDIATELY;
            if (hm->resp_code == 301 || hm->resp_code == 302) {
                string location;
                for(auto i = 0; i < NS_MAX_HTTP_HEADERS; ++i) {
                    if (ns_vcmp(&hm->header_names[i], "Location") == 0) {
                        location.assign(hm->header_values[i].p, hm->header_values[i].len);
                        break;
                    }
                }
                if (location.empty()) {
                    break;
                }
//                printf("Document moved\n"); fflush(stdout);
//                printf("Locaton: %s\n", location.c_str()); fflush(stdout);
                if (location.compare(c->getUrl()) != 0) {
//                    printf("Continue download\n"); fflush(stdout);
                    c->setUrl(location);
                    c->download();
                }
            } else if (hm->resp_code == 404) {
                c->fire(HttpConnectionListener::Failed(), c, _("No data received"));
            } else {
                string data(hm->body.p, hm->body.len);
                c->fire(HttpConnectionListener::Complete(), c, data);
            }
            break;
        }
        default: break;
    }
}

HttpConnection::HttpConnection(const string& aUserAgent) :
    poll(false),
    userAgent(aUserAgent),
    received(0)
{
}

void HttpConnection::download_async(void* ptr, const string& post_data)
{
    struct ns_mgr mgr;
    HttpConnection* c = (HttpConnection*) ptr;

    ns_mgr_init(&mgr, ptr);

    c->poll_time = 0;

    string extraHeaders;

    if(c->userAgent.empty()) {
        c->userAgent = "User-Agent: " APPNAME " v" VERSIONSTRING "\r\n";
    }

    extraHeaders.append(c->userAgent);

    if(::strcmp(c->url.substr(c->url.size() - 4).c_str(), ".bz2") == 0) {
                c->mimeType = "application/x-bzip2";
    } else c->mimeType.clear();

    c->curr_ns_con = ns_connect_http(&mgr, &ev_handler, c->url.c_str(),
                    (extraHeaders.empty() ? NULL : extraHeaders.c_str()),
                    (post_data.empty() ? NULL : post_data.c_str()));

//    printf("Start download: %s\n", c->url.c_str());fflush(stdout);

    c->poll = true;

    while(c->poll) {
        ns_mgr_poll(&mgr, 1000);
        c->poll_time += 1000;
        if (c->poll_time == 1000*60 && c->received == 0) {
            c->curr_ns_con->flags |= NSF_CLOSE_IMMEDIATELY;
            c->fire(HttpConnectionListener::Failed(), c, _("No data received"));
        }
    }
    ns_mgr_free(&mgr);
}

void HttpConnection::download(const string& post_data) {
    std::thread t(download_async, this, post_data);
    t.detach();
}

} // namespace dcpp
