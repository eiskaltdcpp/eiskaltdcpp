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

#pragma once

#include <string>
#include "HttpConnectionListener.h"
#include "GetSet.h"
#include "Speaker.h"
#include "Util.h"

#include "extra/noncopyable.h"
#include "extra/fossa.h"
#include <atomic>

namespace dcpp {

using std::string;

class HttpConnection : public Speaker<HttpConnectionListener>, ::noncopyable
{
public:
    HttpConnection(const string& aUserAgent = Util::emptyString);
    virtual ~HttpConnection();

    void download(const string& post_data = Util::emptyString);

    GETSET(string, url, Url)
    std::atomic<bool> poll;
    const string& getMimeType() const { return mimeType; }

private:
    struct ns_mgr mgr;

    string userAgent;
    string mimeType;

    static void poll_func(void *ptr);

};

} // namespace dcpp
