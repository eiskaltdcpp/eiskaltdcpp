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

#pragma once

#include "dcpp/stdinc.h"
#include "dcpp/HttpManagerListener.h"
#include "dcpp/Singleton.h"
#include "dcpp/TimerManager.h"

namespace dcpp {

class DynDNS : public Singleton<DynDNS>, private HttpManagerListener
{
    public:
        DynDNS();
        ~DynDNS();

    private:
        HttpConnection* c;
        void Request();
        void completeDownload(bool success, const string& html);

        // HttpManagerListener
        void on(HttpManagerListener::Failed, HttpConnection*, const string&) noexcept;
        void on(HttpManagerListener::Complete, HttpConnection*, OutputStream*) noexcept;

        // TimerManagerListener
        void on(TimerManagerListener::Minute, uint64_t aTick) noexcept;
};
}
