#include <string>
#include <iostream>
#include <stdlib.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>

using namespace std;

int
main(int argc, char **argv) {

    if (argc-1 > 0) {
        if (argv) {}
        cerr << "This program has no arguments" << endl;
        exit(1);
    }

    string const serverUrl("http://localhost:8080/RPC2");
    string const methodName("sample.add");
    string const methodNamemagnet("magnet.add");

    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value result;
    xmlrpc_c::value result1;

    myClient.call(serverUrl, methodName, "ii", &result, 5999, 21478);
    myClient.call(serverUrl, methodNamemagnet, "magnet", &result1, "teststring");

    //myClient.call(serverUrl, methodNamemagnet, "ii", &result, 5, 7);
    int const sum((xmlrpc_c::value_int(result)));
    string const magnet((xmlrpc_c::value_string(result1)));
    // Assume the method returned an integer; throws error if not

    cout << "Result of RPC (sum of 5 and 7): " << sum << endl;
    cout << "Result of RPC (status adding magnet): " << magnet << endl;


    return 0;
}
