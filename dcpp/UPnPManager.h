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

#include "forward.h"
#include "Singleton.h"
#include "Thread.h"
#include "UPnP.h"
#include "Atomic.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace dcpp {

class UPnPManager :
    public Singleton<UPnPManager>,
    private Thread
{
public:
    /**
    * add an implementation, derived from the base UPnP class.
    * must be allocated on the heap; its deletion will be managed by UPnPManager.
    * first added impl will be tried first.
    */

    void runMiniUPnP();
    void addImplementation(UPnP* impl);
    bool open();
    void close();

    bool getOpened() const { return opened; }

private:
    friend class Singleton<UPnPManager>;

    typedef boost::ptr_vector<UPnP> Impls;
    Impls impls;

    bool opened;
    Atomic<bool,memory_ordering_strong> portMapping;

    UPnPManager() : opened(false), portMapping(false) { }
    virtual ~UPnPManager() noexcept { join(); }

    int run();

    void close(UPnP& impl);
    void log(const string& message);
};

} // namespace dcpp
