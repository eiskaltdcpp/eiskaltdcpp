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
        this->_signature = "i:magnet";
        this->_help = "This method add queue for magnet";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const smagnet(paramList.getString(0));

        paramList.verifyEnd(1);

        *retvalP = xmlrpc_c::value_string("Magnet added in queue");
        // Sometimes, make it look hard (so client can see what it's like
        // to do an RPC that takes a while).
    }
};
#endif
