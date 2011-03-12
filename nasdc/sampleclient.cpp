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

    string const serverUrl("http://localhost:8080/eiskaltdcpp");
    string const methodName("sample.add");
    string const methodmagnet("magnet.add");
    string const methodhub_add("hub.add");
    string const methodhub_del("hub.del");
    string const methodhub_say("hub.say");
    string const methodhubs_list("hubs.list");
    string const methodstopDemon("demon.stop");
    string const methodadd_share("share.add");
    string const methoddel_share("share.del");
    string const methodrename_share("share.del");
    string const methodshare_list("share.list");
    string const testmagnet("teststring");
    string const testddir("testddir");


    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value result;
    xmlrpc_c::value result_magnet_add;
    xmlrpc_c::value result_stop_demon;
    xmlrpc_c::value result_hub_add;
    xmlrpc_c::value result_hub_del;
    xmlrpc_c::value result_hub_say;
    xmlrpc_c::value result_hub_list;
    xmlrpc_c::value result_add_share;
    xmlrpc_c::value result_del_share;
    xmlrpc_c::value result_rename_share;
    xmlrpc_c::value result_share_list;


    //myClient.call(serverUrl, methodName, "ii", &result, 5, 7);
    //myClient.call(serverUrl, methodhub_add, "ss", &result_hub_add, "adc://adc3.san.ru:10000", "utf8");
    //myClient.call(serverUrl, methodhub_add, "ss", &result_hub_add, "adc://217.197.6.21:4111","utf8");
    //myClient.call(serverUrl, methodhub_del, "s", &result_hub_del, "adc://217.197.6.21:4111");
    myClient.call(serverUrl, methodhubs_list, "s", &result_hub_list, "|");
    myClient.call(serverUrl, methodadd_share, "ss", &result_add_share,"/home/egik/Загрузки/苏打绿 - 春.日光", "testing xml-rpc");
    //myClient.call(serverUrl, methodrename_share, "ss", &result_rename_share, "/home/egik/Загрузки/苏打绿 - 春.日光", "new name for testing");
    //myClient.call(serverUrl, methoddel_share, "s", &result_del_share, "/home/egik/Загрузки/苏打绿 - 春.日光");
    myClient.call(serverUrl, methodshare_list, "s", &result_share_list, "|");
    //myClient.call(serverUrl, methodhub_say, "ss", &result_hub_say, "adc://217.197.6.21:4111","/me доволен тем, что этот xml-rpc вызов работает :D");
    //myClient.call(serverUrl, methodmagnet, "ss", &result_magnet_add, "magnet:?xt=urn:tree:tiger:S4ZGSTLMQYYMSNZSJKQBJYC3SEEOARIAOKF2APA&xl=716695&dn=%D0%A0%D0%B0%D0%B7%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%BA%D0%B0+%D0%B8+%D0%BF%D1%80%D0%B8%D0%BC%D0%B5%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5+%D0%B2%D1%8B%D1%81%D0%BE%D0%BA%D0%BE%D1%81%D0%BA%D0%BE%D1%80%D0%BE%D1%81%D1%82%D0%BD%D1%8B%D1%85+%D1%81%D1%85%D0%B5%D0%BC+%D1%83%D0%BF%D1%80%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D1%8F+%D1%81%D0%B8%D0%BB%D0%BE%D0%B2%D1%8B%D0%BC%D0%B8+%D0%BF%D0%BE%D0%BB%D0%B5%D0%B2%D1%8B%D0%BC%D0%B8+%D1%82%D1%80%D0%B0%D0%BD%D0%B7%D0%B8%D1%81%D1%82%D0%BE%D1%80%D0%B0%D0%BC%D0%B8.pdf", "/home/egik/test/");
    //myClient.call(serverUrl, methodstopDemon, "i", &result_stop_demon, 1);

    //myClient.call(serverUrl, methodNamemagnet, "ii", &result, 5, 7);
    //int const sum((xmlrpc_c::value_int(result)));
    //string const magnet((xmlrpc_c::value_string(result_magnet_add)));
    //string const hub_add((xmlrpc_c::value_string(result_hub_add)));
    //string const hub_del((xmlrpc_c::value_string(result_hub_del)));
    //string const hub_say((xmlrpc_c::value_string(result_hub_say)));
    string const hub_list((xmlrpc_c::value_string(result_hub_list)));
    string const add_share((xmlrpc_c::value_string(result_add_share)));
    //string const del_share((xmlrpc_c::value_string(result_del_share)));
    //string const rename_share((xmlrpc_c::value_string(result_rename_share)));
    string const share_list((xmlrpc_c::value_string(result_share_list)));
    //string const stop_demon((xmlrpc_c::value_string(result_stop_demon)));
    // Assume the method returned an integer; throws error if not

    //cout << "Result of RPC (sum of 5 and 7): " << sum << endl;
    //cout << "Result of RPC (status adding magnet): " << magnet << endl;
    //cout << "Result of RPC (status add hub): " << hub_add << endl;
    //cout << "Result of RPC (status del hub): " << hub_del << endl;
    //cout << "Result of RPC (status say hub): " << hub_say << endl;
    cout << "Result of RPC (status list hubs): " << hub_list << endl;
    cout << "Result of RPC (status add share): " << add_share << endl;
    //cout << "Result of RPC (status rename share): " << rename_share << endl;
    cout << "Result of RPC (status list share): " << share_list << endl;
    //cout << "Result of RPC (status del share): " << del_share << endl;
    //cout << "Result of RPC (status demon): " << stop_demon << endl;


    return 0;
}
