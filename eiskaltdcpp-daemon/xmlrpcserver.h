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

#if defined(USE_XMLRPC_ABYSS)
    #include <xmlrpc-c/server_abyss.hpp>
#elif defined(USE_XMLRPC_PSTREAM)
    #include <xmlrpc-c/server_pstream.hpp>
#endif

#include "utility.h"
#include "ServerManager.h"

using namespace std;

xmlrpc_c::registry xmlrpcRegistry;
#if defined(USE_XMLRPC_ABYSS)
xmlrpc_c::serverAbyss * server;
#elif defined(USE_XMLRPC_PSTREAM)
xmlrpc_c::serverPstream * server;
#endif

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
        if (isVerbose) std::cout << "splitMagnet: \n tth: " << tth << "\n size: " << size << "\n name: " << name << std::endl;
        if (ok && ServerThread::getInstance()->addInQueue(sddir, name, size, tth))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class stopDaemonMethod : public xmlrpc_c::method {
public:
    stopDaemonMethod() {
        this->_signature = "i:s";
        this->_help = "This method can stop daemon. Params: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        *retvalP = xmlrpc_c::value_int(0);
        bServerTerminated=true;
    }
};

class hubAddMethod : public xmlrpc_c::method {
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
        *retvalP = xmlrpc_c::value_int(0);
    }
};

class hubSayMethod : public xmlrpc_c::method {
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
        if (ServerThread::getInstance()->findHubInConnectedClients(shub)) {
            ServerThread::getInstance()->sendMessage(shub,smess);
            *retvalP = xmlrpc_c::value_int(0);
        } else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class hubSayPrivateMethod : public xmlrpc_c::method {
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

class showVersionMethod : public xmlrpc_c::method {
public:
    showVersionMethod() {
        this->_signature = "i:s";
        this->_help = "This method return full client version in string. Рarams: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string version(EISKALTDCPP_VERSION);
        version.append(" (");
        version.append(EISKALTDCPP_VERSION_SFX);
        version.append(")");
        *retvalP = xmlrpc_c::value_string(version);
    }
};

class showRatioMethod : public xmlrpc_c::method {
public:
    showRatioMethod() {
        this->_signature = "i:s";
        this->_help = "This method return client ratio in string. Рarams: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        double ratio;
        double up   = static_cast<double>(SETTING(TOTAL_UPLOAD));
        double down = static_cast<double>(SETTING(TOTAL_DOWNLOAD));

        if (down > 0)
            ratio = up / down;
        else
            ratio = 0;

        char ratio_c[32];
        sprintf(ratio_c,"%.3f", ratio);

        string uploaded = Util::formatBytes(up);
        string downloaded = Util::formatBytes(down);

        string line = str(dcpp_fmt("ratio: %1% (uploads: %2%, downloads: %3% )")
        % string(ratio_c) % uploaded % downloaded);

        *retvalP = xmlrpc_c::value_string(line);
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
            if (ServerThread::getInstance()->addDirInShare(sdirectory, svirtname))
                *retvalP = xmlrpc_c::value_int(0);
            else
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
            if (ServerThread::getInstance()->renameDirInShare(sdirectory, svirtname))
                *retvalP = xmlrpc_c::value_int(0);
            else
                *retvalP = xmlrpc_c::value_int(1);
        } catch (const ShareException& e) {
            *retvalP = xmlrpc_c::value_string(e.getError());
        }
    }
};

class delDirFromShareMethod : public xmlrpc_c::method {
public:
    delDirFromShareMethod() {
        this->_signature = "i:s";
        this->_help = "This method delete dir from share. Рarams: virt name of directory";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sdirectory(paramList.getString(0));
        paramList.verifyEnd(1);
        if (ServerThread::getInstance()->delDirFromShare(sdirectory))
            *retvalP = xmlrpc_c::value_int(0);
        else
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
        paramList.verifyEnd(1); string listshare;
        ServerThread::getInstance()->listShare(listshare, sseparator);
        *retvalP = xmlrpc_c::value_string(listshare);
    }
};

class refreshShareMethod : public xmlrpc_c::method {
public:
    refreshShareMethod() {
        this->_signature = "i:s";
        this->_help = "This method run refresh. Рarams: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        ShareManager::getInstance()->setDirty();
        ShareManager::getInstance()->refresh(true);
        *retvalP = xmlrpc_c::value_int(0);
    }
};

class getFileListMethod : public xmlrpc_c::method {
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
public:
    sendSearchMethod() {
        this->_signature = "i:s";
        this->_help = "This method send search. Рarams: search string";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const ssearch(paramList.getString(0));
        paramList.verifyEnd(1);
        if (ServerThread::getInstance()->sendSearchonHubs(ssearch, 0, 0, 0, 0, ""))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class returnSearchResultsMethod : public xmlrpc_c::method {
public:
    returnSearchResultsMethod() {
        this->_signature = "i:s";
        this->_help = "This method return search results list. Рarams: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shuburl(paramList.getString(0));
        vector<StringMap> tmp;
        ServerThread::getInstance()->returnSearchResults(tmp,shuburl);
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
class setPriorityQueueItemMethod : public xmlrpc_c::method {
public:
    setPriorityQueueItemMethod() {
        this->_signature = "i:si";
        this->_help = "This method set priority on target. Рarams: target, priority";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const starget(paramList.getString(0));
        int const ipriority(paramList.getInt(1));
        paramList.verifyEnd(2);
        if (ServerThread::getInstance()->setPriorityQueueItem(starget, ipriority))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};
#endif
