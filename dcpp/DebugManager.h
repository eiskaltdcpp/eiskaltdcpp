/* 
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

#ifndef __DEBUGMANAGER_H
#define __DEBUGMANAGER_H

#include "DCPlusPlus.h"
#include "Singleton.h"
#include "TimerManager.h"

namespace dcpp {

class DebugManagerListener {
public:
template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> DebugCommand;
	typedef X<0> DebugDetection;	

	virtual void on(DebugCommand, const string&) throw() { }
	virtual void on(DebugDetection, const string&, int, const string&) throw() { }
};

class DebugManager : public Singleton<DebugManager>, public Speaker<DebugManagerListener> {
public:
	void SendCommandMessage(const string& mess, int typeDir, const string& ip) {
		fire(DebugManagerListener::DebugCommand(), mess, typeDir, ip);
	}
	void SendDetectionMessage(const string& mess) {
		fire(DebugManagerListener::DebugDetection(), mess);
	}
	enum {
		HUB_IN, HUB_OUT, CLIENT_IN, CLIENT_OUT
	};

private:
	friend class Singleton<DebugManager>;
	DebugManager() throw() { };
	~DebugManager() throw() { };
};
#define COMMAND_DEBUG(a,b,c) DebugManager::getInstance()->SendCommandMessage(a,b,c);
#define DETECTION_DEBUG(m) DebugManager::getInstance()->SendDetectionMessage(m);

} // namespace dcpp

#endif
