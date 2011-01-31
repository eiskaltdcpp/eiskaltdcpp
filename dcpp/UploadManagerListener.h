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

#ifndef UPLOADMANAGERLISTENER_H_
#define UPLOADMANAGERLISTENER_H_

#include "forward.h"

namespace dcpp {

class UploadManagerListener {
public:
	virtual ~UploadManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Complete;
	typedef X<1> Failed;
	typedef X<2> Starting;
	typedef X<3> Tick;
	typedef X<4> WaitingAddFile;
	typedef X<5> WaitingRemoveUser;

	virtual void on(Starting, Upload*) throw() { }
	virtual void on(Tick, const UploadList&) throw() { }
	virtual void on(Complete, Upload*) throw() { }
	virtual void on(Failed, Upload*, const string&) throw() { }
	virtual void on(WaitingAddFile, const HintedUser&, const string&) throw() { }
	virtual void on(WaitingRemoveUser, const HintedUser&) throw() { }

};

} // namespace dcpp

#endif /*UPLOADMANAGERLISTENER_H_*/
