/*
 * Copyright (C) 2008-2010 Big Muscle, http://strongdc.sf.net
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

#include "stdafx.h"

#include "DHT.h"
#include "IndexManager.h"
#include "SearchManager.h"

#include "dcpp/CID.h"
#include "dcpp/LogManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/TimerManager.h"

namespace dht
{

	IndexManager::IndexManager(void) :
		nextRepublishTime(GET_TICK()), publishing(0), publish(false)
	{
	}

	IndexManager::~IndexManager(void)
	{
	}

	/*
	 * Add new source to tth list
	 */
	void IndexManager::addSource(const TTHValue& tth, const Node::Ptr& node, uint64_t size, bool partial)
	{
		Source source;
		source.setCID(node->getUser()->getCID());
		source.setIp(node->getIdentity().getIp());
		source.setUdpPort(static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())));
		source.setSize(size);
		source.setExpires(GET_TICK() + (partial ? PFS_REPUBLISH_TIME : REPUBLISH_TIME));
		source.setPartial(partial);

		Lock l(cs);

		TTHMap::iterator i = tthList.find(tth);
		if(i != tthList.end())
		{
			// no user duplicites
			SourceList& sources = i->second;
			for(SourceList::iterator s = sources.begin(); s != sources.end(); s++)
			{
				if(node->getUser()->getCID() == (*s).getCID())
				{
					// delete old item
					sources.erase(s);
					break;
				}
			}

			// old items in front, new items in back
			sources.push_back(source);

			// if maximum sources reached, remove the oldest one
			if(sources.size() > MAX_SEARCH_RESULTS)
				sources.pop_front();
		}
		else
		{
			// new file
			tthList.insert(std::make_pair(tth, SourceList(1, source)));
		}

		DHT::getInstance()->setDirty();
	}

	/*
	 * Finds TTH in known indexes and returns it
	 */
	bool IndexManager::findResult(const TTHValue& tth, SourceList& sources) const
	{
		// TODO: does file exist in my own sharelist?
		Lock l(cs);
		TTHMap::const_iterator i = tthList.find(tth);
		if(i != tthList.end())
		{
			sources = i->second;
			return true;
		}

		return false;
	}

	/*
	 * Try to publish next file in queue
	 */
	void IndexManager::publishNextFile()
	{
		File f;
		{
			Lock l(cs);

			if(publishQueue.empty() || publishing >= MAX_PUBLISHES_AT_TIME)
				return;

			incPublishing();

			f = publishQueue.front(); // get the first file in queue
			publishQueue.pop_front(); // and remove it from queue
		}
		SearchManager::getInstance()->findStore(f.tth.toBase32(), f.size, f.partial);
	}

	/*
	 * Loads existing indexes from disk
	 */
	void IndexManager::loadIndexes(SimpleXML& xml)
	{
		xml.resetCurrentChild();
		if(xml.findChild("Indexes"))
		{
			xml.stepIn();
			while(xml.findChild("Index"))
			{
				const TTHValue tth = TTHValue(xml.getChildAttrib("TTH"));
				SourceList sources;

				xml.stepIn();
				while(xml.findChild("Source"))
				{
					Source source;
					source.setCID(CID(xml.getChildAttrib("CID")));
					source.setIp(xml.getChildAttrib("I4"));
					source.setUdpPort(static_cast<uint16_t>(xml.getIntChildAttrib("U4")));
					source.setSize(xml.getLongLongChildAttrib("SI"));
					source.setExpires(xml.getLongLongChildAttrib("EX"));
					source.setPartial(false);

					sources.push_back(source);
				}

				tthList.insert(std::make_pair(tth, sources));
				xml.stepOut();
			}
			xml.stepOut();
		}
	}

	/*
	 * Save all indexes to disk
	 */
	void IndexManager::saveIndexes(SimpleXML& xml)
	{
		xml.addTag("Files");
		xml.stepIn();

		Lock l(cs);
		for(TTHMap::const_iterator i = tthList.begin(); i != tthList.end(); i++)
		{
			xml.addTag("File");
			xml.addChildAttrib("TTH", i->first.toBase32());

			xml.stepIn();
			for(SourceList::const_iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				const Source& source = *j;

				if(source.getPartial())
					continue;	// don't store partial sources

				xml.addTag("Source");
				xml.addChildAttrib("CID", source.getCID().toBase32());
				xml.addChildAttrib("I4", source.getIp());
				xml.addChildAttrib("U4", source.getUdpPort());
				xml.addChildAttrib("SI", source.getSize());
				xml.addChildAttrib("EX", source.getExpires());
			}
			xml.stepOut();
		}

		xml.stepOut();
	}

	/*
	 * Processes incoming request to publish file
	 */
	void IndexManager::processPublishSourceRequest(const Node::Ptr& node, const AdcCommand& cmd)
	{
		string tth;
		if(!cmd.getParam("TR", 1, tth))
			return;	// nothing to identify a file?

		string size;
		if(!cmd.getParam("SI", 1, size))
			return;	// no file size?

		string partial;
		cmd.getParam("PF", 1, partial);

		addSource(TTHValue(tth), node, Util::toInt64(size), partial == "1");

		// send response
		AdcCommand res(AdcCommand::SEV_SUCCESS, AdcCommand::SUCCESS, "File published", AdcCommand::TYPE_UDP);
		res.addParam("FC", "PUB");
		res.addParam("TR", tth);
		DHT::getInstance()->send(res, node->getIdentity().getIp(), static_cast<uint16_t>(Util::toInt(node->getIdentity().getUdpPort())), node->getUser()->getCID(), node->getUdpKey());
	}

	/*
	 * Removes old sources
	 */
	void IndexManager::checkExpiration(uint64_t aTick)
	{
		Lock l(cs);

		bool dirty = false;

		TTHMap::iterator i = tthList.begin();
		while(i != tthList.end())
		{
			SourceList::iterator j = i->second.begin();
			while(j != i->second.end())
			{
				const Source& source = *j;
				if(source.getExpires() <= aTick)
				{
					dirty = true;
					j = i->second.erase(j);
				}
				else
					break;	// list is sorted, so finding first non-expired can stop iteration

			}

			if(i->second.empty())
				tthList.erase(i++);
			else
				++i;
		}

		if(dirty)
			DHT::getInstance()->setDirty();
	}

	/** Publishes shared file */
	void IndexManager::publishFile(const TTHValue& tth, int64_t size)
	{
		if(size > MIN_PUBLISH_FILESIZE)
		{
			Lock l(cs);
			publishQueue.push_back(File(tth, size, false));
		}
	}

	/*
	 * Publishes partially downloaded file
	 */
	void IndexManager::publishPartialFile(const TTHValue& tth)
	{
		Lock l(cs);
		publishQueue.push_front(File(tth, 0, true));
	}


} // namespace dht
