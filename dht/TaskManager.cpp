/*
* Copyright (C) 2009-2010 Big Muscle, http://strongdc.sourceforge.net/
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#include "BootstrapManager.h"
#include "DHT.h"
#include "IndexManager.h"
#include "SearchManager.h"
#include "TaskManager.h"
#include "Utils.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/TimerManager.h"
#include "dcpp/DCPlusPlus.h"

namespace dht
{

    TaskManager::TaskManager(DHT& dht) :
        dht_(dht), nextPublishTime(GET_TICK()), nextSearchTime(GET_TICK()), nextSelfLookup(GET_TICK() + 3*60*1000),
        nextFirewallCheck(GET_TICK() + FWCHECK_TIME), lastBootstrap(0)
    {
        dht_.ctx().getTimerManager()->addListener(this);
    }

    TaskManager::~TaskManager(void)
    {
        dht_.ctx().getTimerManager()->removeListener(this);
    }

    // TimerManagerListener
    void TaskManager::on(TimerManagerListener::Second, uint64_t aTick) throw()
    {
        if(dht_.isConnected() && dht_.getNodesCount() >= K)
        {
            if(!dht_.isFirewalled() && dht_.getIndexManager().getPublish() && aTick >= nextPublishTime)
            {
                // publish next file
                dht_.getIndexManager().publishNextFile();
                nextPublishTime = aTick + PUBLISH_TIME;
            }
        }
        else
        {
            if(aTick - lastBootstrap > 15000 || (dht_.getNodesCount() == 0 && aTick - lastBootstrap >= 2000))
            {
                // bootstrap if we doesn't know any remote node
                dht_.getBootstrapManager().process();
                lastBootstrap = aTick;
            }
        }

        if(aTick >= nextSearchTime)
        {
            dht_.getSearchManager().processSearches();
            nextSearchTime = aTick + SEARCH_PROCESSTIME;
        }

        if(aTick >= nextSelfLookup)
        {
            // find myself in the network
            dht_.getSearchManager().findNode(dht_.ctx().getClientManager()->getMe()->getCID());
            nextSelfLookup = aTick + SELF_LOOKUP_TIMER;
        }

        if(aTick >= nextFirewallCheck)
        {
            dht_.setRequestFWCheck();
            nextFirewallCheck = aTick + FWCHECK_TIME;
        }
    }

    void TaskManager::on(TimerManagerListener::Minute, uint64_t aTick) throw()
    {
        Utils::cleanFlood();

        // remove dead nodes
        dht_.checkExpiration(aTick);
        dht_.getIndexManager().checkExpiration(aTick);

        dht_.saveData();
    }

}
