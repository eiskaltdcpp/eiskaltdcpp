/***************************************************************************
*                                                                         *
*   Copyright 2010 Eugene Petrov <dhamp@ya.ru>                            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef XMLRPCSERVER_H_
#define XMLRPCSERVER_H_

#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/StringTokenizer.h"

#include <cassert>
#include <stdexcept>
#include <iostream>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#ifdef WIN32
  #define SLEEP(seconds) SleepEx(seconds * 1000);
#else
  #define SLEEP(seconds) sleep(seconds);
#endif

#include "ServerManager.h"

using namespace std;

bool splitMagnet(const string &magnet, string &name, int64_t &size, string &tth) {
    name = "Unknown";
    size = 0;
    tth = "Unknown";
    string tmp;

    if (magnet.empty() && magnet.find("urn:tree:tiger") == string::npos)
        return false;
#ifdef _DEBUG
    fprintf(stderr,"split:%s\n",magnet.c_str());
    fflush(stderr);
#endif
    tmp = magnet.substr(8);	//magnet:?
#ifdef _DEBUG
    fprintf(stderr,"split:%s\n",tmp.c_str());
    fflush(stderr);
#endif
    StringTokenizer<string> st(tmp, "&");
    for (StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
        string str;
        str=*i;
#ifdef _DEBUG
        fprintf(stderr,"token: %s\n",str.c_str());fflush(stderr);
#endif
        if (str.compare(0, 3, "xt=") == 0)
            tth=str.substr(3+15);
        else if (str.compare(0, 3, "xl=") == 0)
            size = Util::toInt64(str.substr(3));
        else if (str.compare(0, 3, "dn=") == 0)
            name = Util::encodeURI(str.substr(3), true);
    }
    return true;
}

xmlrpc_c::registry xmlrpcRegistry;

xmlrpc_c::serverAbyss AbyssServer(xmlrpc_c::serverAbyss::constrOpt()
                                      .registryP(&xmlrpcRegistry)
                                      .portNumber(8080)
                                      .logFileName("/tmp/xmlrpc_log")
                                      .serverOwnsSignals(false)
                                      .uriPath("/eiskaltdcpp")
                                  //myRegistry,
                                  //8080,              // TCP port on which to listen
                                  //"/tmp/xmlrpc_log"  // Log file
                                  );

//class myshutdown : public xmlrpc_c::registry::shutdown {
    //public:
        //myshutdown(xmlrpc_c::serverAbyss * const serverHandle) :
            //serverHandle(serverHandle) {}

        //void doit(string const& comment,
                  //void * const) const {

            //cerr << "Shutting down because " << comment <<endl;
            //shutdownMyServer(serverHandle);
        //}

    //private:
        //xmlrpc_c::serverAbyss * const serverHandle;
//};

class sampleAddMethod : public xmlrpc_c::method {
public:
    sampleAddMethod() {
        // signature and help strings are documentation -- the client
        // can query this information with a system.methodSignature and
        // system.methodHelp RPC.
        this->_signature = "i:ii";
        // method's result and two arguments are integers
        this->_help = "This method adds two integers together";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        int const addend(paramList.getInt(0));
        int const adder(paramList.getInt(1));

        paramList.verifyEnd(2);

        *retvalP = xmlrpc_c::value_int(addend + adder);
        // Sometimes, make it look hard (so client can see what it's like
        // to do an RPC that takes a while).
        if (adder == 1)
            SLEEP(2);
    }
};

class magnetAddMethod : public xmlrpc_c::method {
public:
    magnetAddMethod() {
        this->_signature = "i:ss";
        this->_help = "This method add queue for magnet";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const smagnet(paramList.getString(0));
        string const sddir(paramList.getString(1));
        paramList.verifyEnd(2);

        string name,tth;int64_t size;
        bool ok = splitMagnet(smagnet, name, size, tth);
        if (ok){
            if (!sddir.empty())
                name = sddir+name;
            else
                name = SETTING(DOWNLOAD_DIRECTORY) + name;
#ifdef _DEBUG
            fprintf(stderr,"tth: %s\n",tth.c_str());
            fprintf(stderr,"size: %d\n",size);
            fprintf(stderr,"name: %s\n",name.c_str());
            fflush(stderr);
#endif
            QueueManager::getInstance()->add(name, size, TTHValue(tth));
            *retvalP = xmlrpc_c::value_string("Magnet added in queue");
        }
        else
            *retvalP = xmlrpc_c::value_string("Fail add magnet in queue");
        SLEEP(2);
    }
};

class stopDemonMethod : public xmlrpc_c::method {
public:
    stopDemonMethod() {
        this->_signature = "i:i";
        this->_help = "This method can stop demon";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        int  const sstop(paramList.getInt(0));
        paramList.verifyEnd(1);

        if (sstop == 1) {
            *retvalP = xmlrpc_c::value_string("Stopping daemon");
            bServerTerminated=true;
        }
        else
            *retvalP = xmlrpc_c::value_string("Param not equal 1, continue executing....");

        SLEEP(2);
    }
};

class hubAddMethod : public xmlrpc_c::method {
public:
    hubAddMethod() {
        this->_signature = "i:ss";
        this->_help = "This method add connect to new hub";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const senc(paramList.getString(1));
        paramList.verifyEnd(2);

       connectClient(shub, senc);
        *retvalP = xmlrpc_c::value_string("Connected to hub");
        //}
        //else
            //*retvalP = xmlrpc_c::value_string("Fail connect");
        SLEEP(2);
    }
};

class hubDelMethod : public xmlrpc_c::method {
public:
    hubDelMethod() {
        this->_signature = "i:s";
        this->_help = "This method disconnect from hub";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        paramList.verifyEnd(1);

       disconnectClient(shub);
        *retvalP = xmlrpc_c::value_string("Disconnected from hub");
        //}
        //else
            //*retvalP = xmlrpc_c::value_string("Fail disconnect");
        SLEEP(2);
    }
};
#endif
