/*
 * Copyright (C) 2009 Big Muscle, http://strongdc.sf.net
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

#pragma once

#include "../dcpp/CID.h"
#include "../dcpp/Pointer.h"
#include "../dcpp/SimpleXML.h"
#include "../dcpp/TimerManager.h"
#include "../dcpp/User.h"

namespace dht
{

    struct Node :
        public OnlineUser
    {
        typedef boost::intrusive_ptr<Node> Ptr;
        typedef std::map<CID, Node::Ptr> Map;

        Node();
        Node(const UserPtr& u);
        ~Node() { }

        uint8_t getType() const { return type; }
        void setAlive();

    private:

        friend class KBucket;

        uint64_t    created;
        uint64_t    expires;
        uint8_t     type;
    };

    class KBucket
    {
    public:
        KBucket(void);
        ~KBucket(void);

        typedef std::deque<Node::Ptr> NodeList;

        /** Inserts node to bucket */
        Node::Ptr insert(const UserPtr& u);

        /** Finds "max" closest nodes and stores them to the list */
        void getClosestNodes(const CID& cid, Node::Map& closest, unsigned int max, uint8_t maxType) const;

        /** Return list of all nodes in this bucket */
        const NodeList& getNodes() const { return nodes; }

        /** Removes dead nodes */
        bool checkExpiration(uint64_t currentTime);

        /** Loads existing nodes from disk */
        void loadNodes(SimpleXML& xml);

        /** Save bootstrap nodes to disk */
        void saveNodes(SimpleXML& xml);

    private:

        /** List of nodes in this bucket */
        NodeList nodes;

    };

}
