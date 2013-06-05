/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file jsonrpc_httpclient.h
 * \brief JSON-RPC HTTP client.
 * \author Brian Panneton
 */

#ifndef JSONRPC_HTTPCLIENT_H 
#define JSONRPC_HTTPCLIENT_H 

#include <iostream>
#include <curl/curl.h>

#include "jsonrpc_client.h"
#include "system.h" 

namespace Json
{
  namespace Rpc
  {
    /**
     * \class HttpClient
     * \brief JSON-RPC Http client.
     */
    class HttpClient : public Client
    {
      public:
        /**
         * \brief Constructor without address or port.
         * \note Make sure you set them yourself.
         */
        HttpClient();
       
        /**
         * \brief Destructor.
         */
        virtual ~HttpClient();

        /**
         * \brief Constructor.
         * \param address address URL
         * \note Make sure you either set the port or include it
         * in the string. If you do both, the one set by ChangePort()
         * will override.
         */
        HttpClient(const std::string& address);
 
        /**
         * \brief Constructor.
         * \param address remote network address
         * \param port remote local port
         * \note Port will override whatever is set in the address string.
         */
        HttpClient(const std::string& address, uint16_t port);

        /**
         * \brief Provide the option to change the address.
         * \param address string containing the address (include the port here)
         * \note ChangePort() will override the port added in address.
         */
        void ChangeAddress(const std::string& address);
        
        /**
         * \brief Provide the option to change the port, this will override the
         * port set in ChangeAddress.
         * \param port number
         */
        void ChangePort(uint16_t port);

        /**
         * \brief Receive last string received from server.
         *
         * Strings received from server are placed in a linked list (FIFO)
         * which Recv pops from the front. 'data' will be null if there
         * are not strings left.
         * \param data if data is received it will put in this reference
         * \return length of string
         */
        ssize_t Recv(std::string& data);

        /**
         * \brief Receives next string from server, will wait for data.
         * \param data data received is put in this reference
         * \return length of string
         */
        ssize_t WaitRecv(std::string& data);

        /**
         * \brief Receives next string from server, will wait for data for max
         * timeout seconds.
         * \param data data received is put in this reference
         * \param timeout timeout for receive operation
         * \return length of string
         */
        ssize_t WaitRecv(std::string& data, unsigned int timeout);

        /**
         * \brief Sets the size of the receive string list.
         *
         * If the list exceeds this size, a strings will be popped off of the
         * front until it is of correct size; Default size: -1 == unlimited.
         * \param size number of strings to keep in the list
         */
        void SetRecvListSize(int size);

        /**
         * \brief Gets the size of the receive string list. 
         * \return size of the receive string list (-1 == unlimited)
         */
        int GetRecvListSize();

        /**
         * \brief Send data.
         * \param data data to send
         * \return 0 if successful or libcurl error code if error
         */
        ssize_t Send(const std::string& data);

        /**
         * \brief Connect to the remote machine.
         * \return true if success, false otherwise
         * \note This method is not needed in HTTP and always returns false.
         */
        bool Connect();

        /**
         * \brief Close socket.
         * \note This method is useless in HTTP and does nothing.
         */
        void Close();

        /**
         * \brief Get socket descriptor.
         * \return socket descriptor.
         * \note This method is useless in HTTP and always returns -1.
         */
        int GetSocket() const;

      private:   
        /**
         * \brief Receive data from the network.
         *
         * This is a static function that is the callback when receiving data
         * from the network. It calls Recv providing it with 'this'.
         * \param buffer Non-null terminated data that it received back
         * \param size size_t count of how many nmemb make up buffer
         * \param nmemb size_t multiply by size to get full size of buffer
         * \param f pointer to user data (in our case it is out class instance)
         * \return the size of the data received
         */
        static size_t StaticRecv(void *buffer, size_t size, size_t nmemb, 
                                 void*f);

        /**
         * \brief Stores the last received json string from the server
         * This allows the last json string that the server sent
         * to the static callback to be stored within the class.
         * The next json string from the server will overwrite this.
         * \param data the string represention of the received json
         * \note Any server errors will also be stored here.
         */
        void StoreRecv(const std::string& data);

        /**
         * \brief Initializes the class instance
         */
        void Initialize();

        /**
         * \brief CURL handle.
         */
        CURL *m_curlHandle;

        /**
         * \brief List of CURL headers.
         */
        struct curl_slist *m_headers;

        /**
         * \brief List of received JSON-RPC response.
         */
        std::list<std::string> m_lastReceivedList;

        /**
         * \brief Size of the m_lastReceivedList list.
         */
        int m_lastReceivedListSize;

        /**
         * \brief Mutex to protected m_lastReceivedList.
         */
        system_util::Mutex m_mutex;
    };
  } /* namespace Rpc */
} /* namespace Json */

#endif /* JSONRPC_HTTPCLIENT_H */

