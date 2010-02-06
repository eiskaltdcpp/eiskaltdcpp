/*
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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
#include "Thread.h"

namespace dcpp {

template<typename T>
class intrusive_ptr_base
{
public:
	bool unique() throw() {
		return (ref == 1);
	}

protected:
	intrusive_ptr_base() throw() : ref(0) { }

private:
	friend void intrusive_ptr_add_ref(intrusive_ptr_base* p) { Thread::safeInc(p->ref); }
	friend void intrusive_ptr_release(intrusive_ptr_base* p) { if(Thread::safeDec(p->ref) == 0) { delete static_cast<T*>(p); } }

	volatile long ref;
};


struct DeleteFunction {
	template<typename T>
	void operator()(const T& p) const { delete p; }
};

} // namespace dcpp

#endif // !defined(POINTER_H)
