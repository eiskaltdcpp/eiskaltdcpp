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

#include "stdafx.h"
#include "UDPSocket.h"

#include "Constants.h"
#include "DHT.h"

#include "../dcpp/AdcCommand.h"
#include "../dcpp/ClientManager.h"
#include "../dcpp/LogManager.h"
#include "../dcpp/SettingsManager.h"

#include <zlib.h>
#ifdef _WIN32
#include <mswsock.h>
#endif
namespace dht
{

    const uint32_t POLL_TIMEOUT = 250;
    #define BUFSIZE 16384

    UDPSocket::UDPSocket(void) : stop(false), port(0)
#ifdef _DEBUG
        , sentBytes(0), receivedBytes(0)
#endif
    {
    }

    UDPSocket::~UDPSocket(void)
    {
        disconnect();

#ifdef _DEBUG
        dcdebug("DHT stats, received: %d bytes, sent: %d bytes\n", receivedBytes, sentBytes);
#endif
    }

    /*
     * Disconnects UDP socket
     */
    void UDPSocket::disconnect() throw()
    {
        if(socket.get())
        {
            stop = true;
            socket->disconnect();
            port = 0;

            join();

            socket.reset();

            stop = false;
        }
    }

    /*
     * Starts listening to UDP socket
     */
    void UDPSocket::listen() throw(SocketException)
    {
        disconnect();

        try
        {
            socket.reset(new Socket);
            socket->create(Socket::TYPE_UDP);
            socket->setSocketOpt(SO_REUSEADDR, 1);
            socket->setSocketOpt(SO_RCVBUF, SETTING(SOCKET_IN_BUFFER));
            port = socket->bind(static_cast<uint16_t>(SETTING(DHT_PORT)), SETTING(BIND_ADDRESS));

            start();
        }
        catch(...)
        {
            socket.reset();
            throw;
        }
    }

    void UDPSocket::checkIncoming() throw(SocketException)
    {
        if(socket->wait(POLL_TIMEOUT, Socket::WAIT_READ) == Socket::WAIT_READ)
        {
            sockaddr_in remoteAddr = { 0 };
            boost::scoped_array<uint8_t> buf(new uint8_t[BUFSIZE]);
            int len = socket->read(&buf[0], BUFSIZE, remoteAddr);
            dcdrun(receivedBytes += len);

            if(len > 1)
            {
                uLongf destLen = BUFSIZE; // what size should be reserved?
                boost::scoped_array<uint8_t> destBuf;

                if(buf[0] == ADC_PACKED_PACKET_HEADER) // is this compressed packet?
                {
                    destBuf.reset(new uint8_t[destLen]);

                    // decompress incoming packet
                    int result = uncompress(destBuf.get(), &destLen, &buf[0] + 1, len - 1);
                    if(result != Z_OK)
                    {
                        // decompression error!!!
                        return;
                    }
                }
                else
                {
                    destBuf.swap(buf);
                    destLen = len;
                }

                // process decompressed packet
                string s((char*)destBuf.get(), destLen);
                if(s[0] == ADC_PACKET_HEADER && s[s.length() - 1] == ADC_PACKET_FOOTER) // is it valid ADC command?
                {
                    string ip = inet_ntoa(remoteAddr.sin_addr);
                    uint16_t port = ntohs(remoteAddr.sin_port);
                    COMMAND_DEBUG(s.substr(0, s.length() - 1), DebugManager::HUB_IN,  ip + ":" + Util::toString(port));
                    DHT::getInstance()->dispatch(s.substr(0, s.length() - 1), ip, port);
                }
            }
        }
    }

    void UDPSocket::checkOutgoing(uint64_t& timer) throw(SocketException)
    {
        std::auto_ptr<Packet> packet;

        {
            Lock l(cs);
            uint64_t now = GET_TICK();
            if(!sendQueue.empty() && (now - timer > 150))
            {
                // take the first packet in queue
                packet.reset(sendQueue.front());
                sendQueue.pop_front();

                //dcdebug("Sending DHT packet: %d bytes, %d ms\n", packet->length, (uint32_t)(now - timer));

                timer = now;
            }
        }

        if(packet.get())
        {
            try
            {
                dcdrun(sentBytes += packet->length);
                socket->writeTo(packet->ip, packet->port, packet->data, packet->length);
            }
            catch(SocketException& e)
            {
                dcdebug("DHT::run Write error: %s\n", e.getError().c_str());
            }

            delete[] packet->data;
        }
    }

    /*
     * Thread for receiving UDP packets
     */
    int UDPSocket::run()
    {
        // fix receiving when sending fails
        DWORD value = FALSE;
        ioctlsocket(socket->sock, SIO_UDP_CONNRESET, &value);

        // antiflood variable
        uint64_t timer = GET_TICK();

        while(!stop)
        {
            try
            {
                // check outgoing queue
                checkOutgoing(timer);

                // check for incoming data
                checkIncoming();
            }
            catch(const SocketException& e)
            {
                dcdebug("DHT::run Error: %s\n", e.getError().c_str());

                bool failed = false;
                while(!stop)
                {
                    try
                    {
                        socket->disconnect();
                        socket->create(Socket::TYPE_UDP);
                        socket->setSocketOpt(SO_RCVBUF, SETTING(SOCKET_IN_BUFFER));
                        socket->bind(port, SETTING(BIND_ADDRESS));
                        if(failed)
                        {
                            LogManager::getInstance()->message("DHT enabled again"); // TODO: translate
                            failed = false;
                        }
                        break;
                    }
                    catch(const SocketException& e)
                    {
                        dcdebug("DHT::run Stopped listening: %s\n", e.getError().c_str());

                        if(!failed)
                        {
                            LogManager::getInstance()->message("DHT disabled: " + e.getError()); // TODO: translate
                            failed = true;
                        }

                        // Spin for 60 seconds
                        for(int i = 0; i < 60 && !stop; ++i)
                        {
                            Thread::sleep(1000);
                        }
                    }
                }
            }
        }

        return 0;
    }

    /*
     * Sends command to ip and port
     */
    void UDPSocket::send(const AdcCommand& cmd, const string& ip, uint16_t port)
    {
        string command = cmd.toString(ClientManager::getInstance()->getMe()->getCID());
        COMMAND_DEBUG(command, DebugManager::HUB_OUT, ip + ":" + Util::toString(port));

        // compress data to have at least some kind of "encryption"
        uLongf destSize = compressBound(command.length()) + 1;

        uint8_t* srcBuf = (uint8_t*)command.data();
        uint8_t* destBuf = new uint8_t[destSize];

        int result = compress2(destBuf + 1, &destSize, srcBuf, command.length(), 9);
        if(result == Z_OK && destSize <= command.length())
        {
            destBuf[0] = ADC_PACKED_PACKET_HEADER;
            destSize += 1;
        }
        else
        {
            // compression failed, send uncompressed packet
            destSize = command.length();
            memcpy(destBuf, srcBuf, destSize);
        }

        Lock l(cs);
        sendQueue.push_back(new Packet(ip, port, destBuf, destSize));
    }

}
