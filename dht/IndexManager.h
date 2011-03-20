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

#ifndef _INDEXMANAGER_H
#define _INDEXMANAGER_H

#include "Constants.h"
#include "KBucket.h"

#include "dcpp/ShareManager.h"
#include "dcpp/Singleton.h"

namespace dht
{

	struct File
	{
		File() { };
		File(const TTHValue& _tth, int64_t _size, bool _partial) :
			tth(_tth), size(_size), partial(_partial) { }

		/** File hash */
		TTHValue tth;

		/** File size in bytes */
		int64_t size;

		/** Is it partially downloaded file? */
		bool partial;
	};

	struct Source
	{
		GETSET(CID, cid, CID);
		GETSET(string, ip, Ip);
		GETSET(uint64_t, expires, Expires);
		GETSET(uint64_t, size, Size);
		GETSET(uint16_t, udpPort, UdpPort);
		GETSET(bool, partial, Partial);
	};

	class IndexManager :
		public Singleton<IndexManager>
	{
	public:
		IndexManager(void);
		~IndexManager(void);

		typedef std::deque<Source> SourceList;

		/** Finds TTH in known indexes and returns it */
		bool findResult(const TTHValue& tth, SourceList& sources) const;

		/** Try to publish next file in queue */
		void publishNextFile();

		/** Loads existing indexes from disk */
		void loadIndexes(SimpleXML& xml);

		/** Save all indexes to disk */
		void saveIndexes(SimpleXML& xml);

		/** How many files is currently being published */
		void incPublishing() { ++publishing; } //{ Thread::safeInc(publishing); }
		void decPublishing() { --publishing; } //{ Lock l(cs); Thread::safeDec(publishing); }

		/** Is publishing allowed? */
		void setPublish(bool _publish) { publish = _publish; }
		bool getPublish() const { return publish; }

		/** Processes incoming request to publish file */
		void processPublishSourceRequest(const Node::Ptr& node, const AdcCommand& cmd);

		/** Removes old sources */
		void checkExpiration(uint64_t aTick);

		/** Publishes shared file */
		void publishFile(const TTHValue& tth, int64_t size);

		/** Publishes partially downloaded file */
		void publishPartialFile(const TTHValue& tth);

		/** Set time when our sharelist should be republished */
		void setNextPublishing() { nextRepublishTime = GET_TICK() + REPUBLISH_TIME; }

		/** Is time when we should republish our sharelist? */
		bool isTimeForPublishing() const { return GET_TICK() >= nextRepublishTime; }

	private:

		/** Contains known hashes in the network and their sources */
		typedef unordered_map<TTHValue, SourceList> TTHMap;
		TTHMap tthList;

		/** Queue of files prepared for publishing */
		typedef std::deque<File> FileQueue;
		FileQueue publishQueue;

		/** Is publishing allowed? */
		bool publish;

		/** How many files is currently being published */
		volatile long publishing;

		/** Time when our sharelist should be republished */
		uint64_t nextRepublishTime;

		/** Synchronizes access to tthList */
		mutable CriticalSection cs;

		/** Add new source to tth list */
		void addSource(const TTHValue& tth, const Node::Ptr& node, uint64_t size, bool partial);

	};

} // namespace dht

#endif	// _INDEXMANAGER_H
