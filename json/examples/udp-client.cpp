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
 * \file udp-client.cpp
 * \brief Simple JSON-RPC UDP client.
 * \author Sebastien Vincent
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "../src/jsonrpc.h"

/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char** argv)
{
  Json::Rpc::UdpClient udpClient(std::string("127.0.0.1"), 8086);
  Json::Value query;
  Json::Value query1;
  Json::Value query2;
  Json::Value query3;
  Json::Value query4;
  Json::FastWriter writer;
  std::string queryStr;
  std::string responseStr;

  /* avoid compilation warnings */
  argc = argc;
  argv = argv;

  if(!networking::init())
  {
    std::cerr << "Networking initialization failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  if(!udpClient.Connect())
  {
    std::cerr << "Cannot connect to remote peer!" << std::endl;
    exit(EXIT_FAILURE);
  }

  /* build JSON-RPC query */
  query1["jsonrpc"] = "2.0";
  query1["id"] = 1;
  query1["method"] = "print";

  query2["jsonrpc"] = "2.0";
  query2["method"] = "notify";

  query3["foo"] = "bar";

  query4["jsonrpc"] = "2.0";
  query4["id"] = 4;
  query4["method"] = "method";

  query[(unsigned int)0] = query1;
  query[(unsigned int)1] = query2;
  query[(unsigned int)2] = query3;
  query[(unsigned int)3] = query4;

  queryStr = writer.write(query);
  std::cout << "Query is: " << queryStr << std::endl;

  if(udpClient.Send(queryStr) == -1)
  {
    std::cerr << "Error while sending data!" << std::endl;
    exit(EXIT_FAILURE);
  }
  
  /* wait the response */
  if(udpClient.Recv(responseStr) != -1)
  {
    std::cout << "Received: " << responseStr << std::endl;
  }
  else
  {
    std::cerr << "Error while receiving data!" << std::endl;
  }

  udpClient.Close();
  networking::cleanup();

  return EXIT_SUCCESS;
}

