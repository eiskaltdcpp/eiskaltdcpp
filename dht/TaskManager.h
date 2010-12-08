/*
 * Copyright (C) 2009-2010 Big Muscle, http://strongdc.sf.net
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

#ifndef _TASKMANAGER_H
#define _TASKMANAGER_H

#include "dcpp/Singleton.h"
#include "dcpp/TimerManager.h"

namespace dht
{

	class TaskManager :
		public Singleton<TaskManager>, private TimerManagerListener
	{
	public:
		TaskManager(void);
		~TaskManager(void);

	private:

		/** Time of publishing next file in queue */
		uint64_t nextPublishTime;

		/** When running searches will be processed */
		uint64_t nextSearchTime;

		/** When initiate searching for myself */
		uint64_t nextSelfLookup;

		/** When request next firewall check */
		uint64_t nextFirewallCheck;

		uint64_t lastBootstrap;

		// TimerManagerListener
		void on(TimerManagerListener::Second, uint64_t aTick) throw();
		void on(TimerManagerListener::Minute, uint64_t aTick) throw();
	};

}

#endif	// _TASKMANAGER_H
