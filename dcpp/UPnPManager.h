/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_UPNP_MANAGER_H
#define DCPLUSPLUS_DCPP_UPNP_MANAGER_H

#include "forward.h"
#include "Singleton.h"
#include "Thread.h"
#include "UPnP.h"

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
	void addImplementation(UPnP* impl);
	void open();
	void close();

	bool getOpened() const { return opened; }

private:
	friend class Singleton<UPnPManager>;

	typedef boost::ptr_vector<UPnP> Impls;
	Impls impls;

	bool opened;

	UPnPManager() : opened(false) { }
	virtual ~UPnPManager() throw() { join(); }

	int run();

	void close(UPnP& impl);
	void log(const string& message);
};

} // namespace dcpp

#endif
