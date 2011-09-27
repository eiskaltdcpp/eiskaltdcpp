/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_POINTER_H
#define DCPLUSPLUS_DCPP_POINTER_H

#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>
#include "noexcept.h"

namespace dcpp {

template<typename T>
class intrusive_ptr_base
{
public:
    void inc() noexcept {
        intrusive_ptr_add_ref(this);
    }

    void dec() noexcept {
        intrusive_ptr_release(this);
    }

    bool unique(int val = 1) const noexcept {
        return (ref <= val);
    }

protected:
    intrusive_ptr_base() noexcept : ref(0) { }
    virtual ~intrusive_ptr_base() { }
private:
    friend void intrusive_ptr_add_ref(intrusive_ptr_base* p) {++p->ref;}
    friend void intrusive_ptr_release(intrusive_ptr_base* p) { if(--p->ref == 0) { delete static_cast<T*>(p); } }

    boost::detail::atomic_count ref;
};


struct DeleteFunction {
    template<typename T>
    void operator()(const T& p) const { delete p; }
};

} // namespace dcpp

#endif // !defined(POINTER_H)
