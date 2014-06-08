/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
