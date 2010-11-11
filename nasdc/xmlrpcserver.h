#ifndef XMLRPCSERVER_H_
#define XMLRPCSERVER_H_

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
#include "utility.h"

using namespace std;

xmlrpc_c::registry xmlrpcRegistry;

xmlrpc_c::serverAbyss AbyssServer(xmlrpc_c::serverAbyss::constrOpt()
                                      .registryP(&xmlrpcRegistry)
                                      .portNumber(8080)
                                      .logFileName("/tmp/xmlrpc_log")
                                      .serverOwnsSignals(false)
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

        //string name="",tth="";int64_t size;
        //string smagnet="magnet:?xt=urn:tree:tiger:S4ZGSTLMQYYMSNZSJKQBJYC3SEEOARIAOKF2APA&xl=716695&dn=%D0%A0%D0%B0%D0%B7%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%BA%D0%B0+%D0%B8+%D0%BF%D1%80%D0%B8%D0%BC%D0%B5%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5+%D0%B2%D1%8B%D1%81%D0%BE%D0%BA%D0%BE%D1%81%D0%BA%D0%BE%D1%80%D0%BE%D1%81%D1%82%D0%BD%D1%8B%D1%85+%D1%81%D1%85%D0%B5%D0%BC+%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D1%8F+%D1%81%D0%B8%D0%BB%D0%BE%D0%B2%D1%8B%D0%BC%D0%B8+%D0%BF%D0%BE%D0%BB%D0%B5%D0%B2%D1%8B%D0%BC%D0%B8+%D1%82%D1%80%D0%B0%D0%BD%D0%B7%D0%B8%D1%81%D1%82%D0%BE%D1%80%D0%B0%D0%BC%D0%B8.pdf";
        //string sddir="/home/egik/Загрузки";
        //fprintf(stderr,"%s\n",smagnet.c_str());
        //fprintf(stderr,"%s\n",sddir.c_str());
        ////if (splitMagnet(smagnet, name, size, tth)){
            //splitMagnet(smagnet, name, size, tth);
            //if (!sddir.empty())
                //name = sddir+name;
            //else
                //name = SETTING(DOWNLOAD_DIRECTORY) + name;
        //fprintf(stderr,"%s\n",name.c_str());
        //fflush(stderr);
        ////}

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
        // signature and help strings are documentation -- the client
        // can query this information with a system.methodSignature and
        // system.methodHelp RPC.
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
            
           QueueManager::getInstance()->add(name, size, TTHValue(tth));
            *retvalP = xmlrpc_c::value_string("Magnet added in queue");
        }
        else
            *retvalP = xmlrpc_c::value_string("Fail add magnet in queue");
        // Sometimes, make it look hard (so client can see what it's like
        // to do an RPC that takes a while).
        //SLEEP(2);
    }
};
#endif
