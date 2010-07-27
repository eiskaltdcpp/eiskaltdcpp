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

#include "../dcpp/AdcCommand.h"
#include "../dcpp/CriticalSection.h"
#include "../dcpp/FastAlloc.h"
#include "../dcpp/Socket.h"
#include "../dcpp/Thread.h"

namespace dht
{

    struct Packet :
        FastAlloc<Packet>
    {
        /** Public constructor */
        Packet(const string& ip_, uint16_t port_, const uint8_t* data_, size_t length_) :
            ip(ip_), port(port_), data(data_), length(length_)
        {
        }

        /** IP where send this packet to */
        string ip;

        /** To which port this packet should be sent */
        uint16_t port;

        /** Data to sent */
        const uint8_t* data;

        /** Data's length */
        size_t length;
    };

    class UDPSocket :
        private Thread
    {
    public:
        UDPSocket(void);
        ~UDPSocket(void);

        /** Disconnects UDP socket */
        void disconnect() throw();

        /** Starts listening to UDP socket */
        void listen() throw(SocketException);

        /** Returns port used to listening to UDP socket */
        uint16_t getPort() const { return port; }

        /** Sends command to ip and port */
        void send(const AdcCommand& cmd, const string& ip, uint16_t port);

    private:

        std::auto_ptr<Socket> socket;

        /** Indicates to stop socket thread */
        bool stop;

        /** Port for communicating in this network */
        uint16_t port;

        /** Queue for sending packets through UDP socket */
        std::deque<Packet*> sendQueue;

        /** Locks access to sending queue */
        // TODO:
        // Use Fast critical section, because we don't need locking so often.
        // Almost all shared access is done within one thread.
        CriticalSection cs;

#ifdef _DEBUG
        // debug constants to optimize bandwidth
        size_t sentBytes;
        size_t receivedBytes;
#endif

        /** Thread for receiving UDP packets */
        int run();

        void checkIncoming() throw(SocketException);
        void checkOutgoing(uint64_t& timer) throw(SocketException);
    };

}
