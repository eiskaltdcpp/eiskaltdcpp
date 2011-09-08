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

#include <cassert>
#include <stdexcept>
#include <iostream>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include "utility.h"
#include "ServerManager.h"

using namespace std;

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

class magnetAddMethod : public xmlrpc_c::method {
public:
    magnetAddMethod() {
        this->_signature = "i:ss";
        this->_help = "This method adds a magnet to queue. Params: magnet, download directory";
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
                name = sddir+PATH_SEPARATOR_STR+name;
            else
                name = SETTING(DOWNLOAD_DIRECTORY) + name;
#ifdef _DEBUG
            fprintf(stderr,"tth: %s\n",tth.c_str());
            fprintf(stderr,"size: %d\n",size);
            fprintf(stderr,"name: %s\n",name.c_str());
            fflush(stderr);
#endif
            QueueManager::getInstance()->add(name, size, TTHValue(tth));
            //*retvalP = xmlrpc_c::value_string("Magnet added in queue");
            *retvalP = xmlrpc_c::value_int(0);
        }
        else
            //*retvalP = xmlrpc_c::value_int("Fail add magnet in queue");
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class stopDaemonMethod : public xmlrpc_c::method {
public:
    stopDaemonMethod() {
        this->_signature = "i:i";
        this->_help = "This method can stop daemon. Params: 1";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        int const istop(paramList.getInt(0));
        paramList.verifyEnd(1);

        if (istop == 1) {
            //*retvalP = xmlrpc_c::value_string("Stopping daemon");
            *retvalP = xmlrpc_c::value_int(0);
            bServerTerminated=true;
        }
        else
            //*retvalP = xmlrpc_c::value_string("Param not equal 1, continue executing....");
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class hubAddMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    hubAddMethod() {
        this->_signature = "i:ss";
        this->_help = "This method add connect to new hub. Params: huburl, encoding";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const senc(paramList.getString(1));
        paramList.verifyEnd(2);
        ServerThread::getInstance()->connectClient(shub, senc);
        *retvalP = xmlrpc_c::value_string("Connecting to " + shub);
    }
};

class hubDelMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    hubDelMethod() {
        this->_signature = "i:s";
        this->_help = "This method disconnect from hub. Params: huburl";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        paramList.verifyEnd(1);
        ServerThread::getInstance()->disconnectClient(shub);
        //*retvalP = xmlrpc_c::value_string("Disconnected from " + shub);
        *retvalP = xmlrpc_c::value_int(0);
    }
};

class hubSayMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    hubSayMethod() {
        this->_signature = "i:ss";
        this->_help = "This method add message on hub. Params: huburl, message";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const smess(paramList.getString(1));
        paramList.verifyEnd(2);
        ServerThread svT;
        if (ServerThread::getInstance()->findHubInConnectedClients(shub)) {
            ServerThread::getInstance()->sendMessage(shub,smess);
            //*retvalP = xmlrpc_c::value_string("Message send on hub: " + shub);
            *retvalP = xmlrpc_c::value_int(0);
        } else
            //*retvalP = xmlrpc_c::value_string(shub + " not connected");
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class hubSayPrivateMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    hubSayPrivateMethod() {
        this->_signature = "i:sss";
        this->_help = "This method add private message on hub. Params: huburl, nick, message";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const snick(paramList.getString(1));
        string const smess(paramList.getString(2));
        paramList.verifyEnd(3);
        string tmp = ServerThread::getInstance()->sendPrivateMessage(shub, snick, smess);
        *retvalP = xmlrpc_c::value_string(tmp);
    }
};

class listHubsMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    listHubsMethod() {
        this->_signature = "i:s";
        this->_help = "This method return list of connected hubs in string. Рarams: separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sseparator(paramList.getString(0));
        paramList.verifyEnd(1); string listhubs;
        ServerThread::getInstance()->listConnectedClients(listhubs, sseparator);
        *retvalP = xmlrpc_c::value_string(listhubs);
    }
};

class addDirInShareMethod : public xmlrpc_c::method {
public:
    addDirInShareMethod() {
        this->_signature = "i:ss";
        this->_help = "This method add dir in share. Рarams: directory,virtual name";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sdirectory(paramList.getString(0));
        string const svirtname(paramList.getString(1));
        paramList.verifyEnd(2);
        try {
            if (Util::fileExists(sdirectory.c_str())) {
                ShareManager::getInstance()->addDirectory(sdirectory,svirtname);
                ShareManager::getInstance()->refresh(true);
                //*retvalP = xmlrpc_c::value_string("Adding dir in share sucess");
                *retvalP = xmlrpc_c::value_int(0);
            } else
                //*retvalP = xmlrpc_c::value_string("Dir not exist in filesystem");
                *retvalP = xmlrpc_c::value_int(1);
        } catch (const ShareException& e) {
            *retvalP = xmlrpc_c::value_string(e.getError());
        }
    }
};

class renameDirInShareMethod : public xmlrpc_c::method {
public:
    renameDirInShareMethod() {
        this->_signature = "i:ss";
        this->_help = "This method rename dir in share. Рarams: directory,virtual name";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sdirectory(paramList.getString(0));
        string const svirtname(paramList.getString(1));
        paramList.verifyEnd(2);
        try {
            StringPairList directories = ShareManager::getInstance()->getDirectories();
            string tmp;
            for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it) {
                if (it->second.compare(sdirectory) == 0) {
                    tmp = it->first;
                    ShareManager::getInstance()->renameDirectory(sdirectory,svirtname);
                    ShareManager::getInstance()->refresh(true);
                    //*retvalP = xmlrpc_c::value_string("Rename dir " + tmp + "->" + svirtname +" in share success");
                    *retvalP = xmlrpc_c::value_int(0);
                    return;
                }
            }
            //*retvalP = xmlrpc_c::value_string("Rename dir failed");
            *retvalP = xmlrpc_c::value_int(1);
        } catch (const ShareException& e) {
            *retvalP = xmlrpc_c::value_string(e.getError());
        }
    }
};

class delDirFromShareMethod : public xmlrpc_c::method {
public:
    delDirFromShareMethod() {
        this->_signature = "i:ss";
        this->_help = "This method delete dir from share. Рarams: virt name of directory";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sdirectory(paramList.getString(0));
        paramList.verifyEnd(1);
        StringPairList directories = ShareManager::getInstance()->getDirectories();
        for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it) {
            if (it->first.compare(sdirectory) == 0) {
                ShareManager::getInstance()->removeDirectory(it->second);
                ShareManager::getInstance()->refresh(true);
                //*retvalP = xmlrpc_c::value_string("Delete dir from share success");
                *retvalP = xmlrpc_c::value_int(0);
                return;
            }
        }
        //*retvalP = xmlrpc_c::value_string("Delete dir from share failed, this virt name not exist");
        *retvalP = xmlrpc_c::value_int(1);
    }
};

class listShareMethod : public xmlrpc_c::method {
public:
    listShareMethod() {
        this->_signature = "i:s";
        this->_help = "This method return list of shared directories in string. Рarams: separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sseparator(paramList.getString(0));
        paramList.verifyEnd(1);
        string listshare;
        StringPairList directories = ShareManager::getInstance()->getDirectories();
        for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it) {
            listshare.append("\n");
            listshare.append(it->second+sseparator);
            listshare.append(it->first+sseparator);
            listshare.append(Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second))+sseparator);
            listshare.append("\n");
        }
        *retvalP = xmlrpc_c::value_string(listshare);
    }
};
class refreshShareMethod : public xmlrpc_c::method {
public:
    refreshShareMethod() {
        this->_signature = "i:i";
        this->_help = "This method run refresh. Рarams: 1";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        int const irefresh(paramList.getInt(0));
        paramList.verifyEnd(1);
        if (irefresh == 1) {
            //*retvalP = xmlrpc_c::value_string("Refresh share started");
            *retvalP = xmlrpc_c::value_int(0);
            ShareManager::getInstance()->setDirty();
            ShareManager::getInstance()->refresh(true);
        }
        else
            //*retvalP = xmlrpc_c::value_string("Param not equal 1, ignoring....");
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class getFileListMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    getFileListMethod() {
        this->_signature = "i:ss";
        this->_help = "This method get file list from user by nick and huburl. Рarams: huburl, nick";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const snick(paramList.getString(1));
        paramList.verifyEnd(2); string tmp;
        tmp = ServerThread::getInstance()->getFileList_client(shub, snick, false);
        *retvalP = xmlrpc_c::value_string(tmp);
    }
};

class getChatPubMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    getChatPubMethod() {
        this->_signature = "i:ss";
        this->_help = "This method return last message in chat on target hub. Рarams: huburl, separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const sseparator(paramList.getString(1));
        paramList.verifyEnd(2); string retchat;
        ServerThread::getInstance()->getChatPubFromClient(retchat, shub, sseparator);
        *retvalP = xmlrpc_c::value_string(retchat);
    }
};

class sendSearchMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    sendSearchMethod() {
        //this->_signature = "i:siiids";
        this->_signature = "i:s";
        this->_help = "This method send search. Рarams: search string, type, sizemode, sizetype, size, huburls";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const ssearch(paramList.getString(0));
        //int const itype(paramList.getInt(1));
        //int const isizemode(paramList.getInt(2));
        //int const isizetype(paramList.getInt(3));
        //int const isize(paramList.getDouble(4));
        //string const shuburls(paramList.getString(5));
        paramList.verifyEnd(1);
        if (ServerThread::getInstance()->sendSearchonHubs(ssearch, 0, 0, 0, 0, ""))
            //*retvalP = xmlrpc_c::value_string("Start search " + ssearch + " on " + shuburls);
            *retvalP = xmlrpc_c::value_int(0);
        else
            //*retvalP = xmlrpc_c::value_string("Start search " + ssearch + " on " + shuburls + " was been failed");
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class listSearchStringsMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    listSearchStringsMethod() {
        this->_signature = "i:s";
        this->_help = "This method return list of search strings. Рarams: separator.";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sseparator(paramList.getString(0));
        paramList.verifyEnd(1); string listsearchstrings;
        ServerThread::getInstance()->listSearchStrings(listsearchstrings, sseparator);
        *retvalP = xmlrpc_c::value_string(listsearchstrings);
    }
};

class returnSearchResultsMethod : public xmlrpc_c::method {
    friend class ServerThread;
public:
    returnSearchResultsMethod() {
        this->_signature = "i:is";
        this->_help = "This method return results list by search string from huburls. Рarams: index, huburls";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        int const iindex(paramList.getInt(0));
        string const shuburls(paramList.getString(1));
        paramList.verifyEnd(2); vector<StringMap> tmp;
        ServerThread::getInstance()->returnSearchResults(tmp, iindex, shuburls);
        vector<xmlrpc_c::value> tmp_array_in;
        for (vector<StringMap>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
            map<string, xmlrpc_c::value> tmp_struct_in;
            for (StringMap::iterator kk = (*i).begin(); kk != (*i).end(); ++kk) {
                pair<string, xmlrpc_c::value> member(kk->first, xmlrpc_c::value_string(kk->second));
                tmp_struct_in.insert(member);
            }
            xmlrpc_c::value_struct const tmp_struct_out(tmp_struct_in);
            tmp_array_in.push_back(xmlrpc_c::value_struct(tmp_struct_out));
        }
        xmlrpc_c::value_array tmp_array_out(tmp_array_in);
        *retvalP = tmp_array_out;
    }
};
#endif
