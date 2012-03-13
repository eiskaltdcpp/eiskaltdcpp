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
        this->_help = "Adds a magnet link to the queue. Params: magnet link, download directory; returns: 0 on success, non-zero otherwise";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const smagnet(paramList.getString(0));
        string const sddir(paramList.getString(1));
        paramList.verifyEnd(2);

        string name, tth;
        int64_t size;
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
        this->_signature = "s:";
        this->_help = "(DEPRECATED) Stops the daemon. Params: none; returns: deprecation warning";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        *retvalP = xmlrpc_c::value_string("This method is deprecated. Please use system.shutdown instead.");
        bServerTerminated = true;
    }
};

class hubAddMethod : public xmlrpc_c::method {
public:
    hubAddMethod() {
        this->_signature = "i:ss";
        this->_help = "Adds a new hub and connects to it. Params: hub URL, encoding; returns: 0";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const senc(paramList.getString(1));
        paramList.verifyEnd(2);
        ServerThread::getInstance()->connectClient(shub, senc);
        *retvalP = xmlrpc_c::value_int(0);
    }
};

class hubDelMethod : public xmlrpc_c::method {
public:
    hubDelMethod() {
        this->_signature = "i:s";
        this->_help = "Disconnects from a hub. Params: hub URL; returns: 0";
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
        this->_help = "Sends a message on a hub. Params: hub URL, message; returns: 0 on success, non-zero otherwise";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const smess(paramList.getString(1));
        paramList.verifyEnd(2);
        if (ServerThread::getInstance()->findHubInConnectedClients(shub)) {
            ServerThread::getInstance()->sendMessage(shub, smess);
            *retvalP = xmlrpc_c::value_int(0);
        } else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class hubSayPrivateMethod : public xmlrpc_c::method {
public:
    hubSayPrivateMethod() {
        this->_signature = "s:sss";
        this->_help = "Sends a private message on a hub. Params: hub URL, nick, message; returns: error string";
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
        this->_signature = "s:s";
        this->_help = "Returns a list of connected hubs. Params: separator";
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
        this->_signature = "s:";
        this->_help = "Returns full client version. Params: none";
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
        this->_signature = "s:";
        this->_help = "Returns upload/download ratio and absolute upload and download. "
                      "Params: none; returns: \"ratio: X (uploads: Y, downloads: Z)\"";
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
        map<string, xmlrpc_c::value> tmp_struct_in;
        tmp_struct_in["ratio"] = xmlrpc_c::value_double(ratio);
        tmp_struct_in["up"] = xmlrpc_c::value_double(up);
        tmp_struct_in["down"] = xmlrpc_c::value_double(down);
        xmlrpc_c::value_struct const tmp_struct_out(tmp_struct_in);
        *retvalP = tmp_struct_out;
    }
};

class addDirInShareMethod : public xmlrpc_c::method {
public:
    addDirInShareMethod() {
        this->_signature = "s:ss";
        this->_help = "Adds a directory to the share. Params: directory, virtual name; returns: error string";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sdirectory(paramList.getString(0));
        string const svirtname(paramList.getString(1));
        paramList.verifyEnd(2);
        try {
            if (ServerThread::getInstance()->addDirInShare(sdirectory, svirtname))
                *retvalP = xmlrpc_c::value_string("Directory added to share");
            else
                *retvalP = xmlrpc_c::value_string("Directory doesn't exist");
        } catch (const ShareException& e) {
            *retvalP = xmlrpc_c::value_string(e.getError());
        }
    }
};

class renameDirInShareMethod : public xmlrpc_c::method {
public:
    renameDirInShareMethod() {
        this->_signature = "s:ss";
        this->_help = "Renames a directory in the share. Params: directory, new virtual name; returns: error string";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sdirectory(paramList.getString(0));
        string const svirtname(paramList.getString(1));
        paramList.verifyEnd(2);
        try {
            if (ServerThread::getInstance()->renameDirInShare(sdirectory, svirtname))
                *retvalP = xmlrpc_c::value_string("Directory has been renamed");
            else
                *retvalP = xmlrpc_c::value_string("Directory not found");
        } catch (const ShareException& e) {
            *retvalP = xmlrpc_c::value_string(e.getError());
        }
    }
};

class delDirFromShareMethod : public xmlrpc_c::method {
public:
    delDirFromShareMethod() {
        this->_signature = "i:s";
        this->_help = "Deletes a directory from the share. Params: virtual name of directory; returns: 0 on success, non-zero otherwise";
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
        this->_signature = "s:s";
        this->_help = "Returns a list of shared directories. Params: separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sseparator(paramList.getString(0));
        paramList.verifyEnd(1);
        string listshare;
        ServerThread::getInstance()->listShare(listshare, sseparator);
        *retvalP = xmlrpc_c::value_string(listshare);
    }
};

class refreshShareMethod : public xmlrpc_c::method {
public:
    refreshShareMethod() {
        this->_signature = "i:s";
        this->_help = "Refreshes the share. Params: none; returns: 0";
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
        this->_signature = "s:ss";
        this->_help = "Adds the file list of a user to the download queue. Params: hub URL, nick; returns: error string";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const snick(paramList.getString(1));
        paramList.verifyEnd(2);
        if (ServerThread::getInstance()->getFileList(shub, snick, false))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class getChatPubMethod : public xmlrpc_c::method {
public:
    getChatPubMethod() {
        this->_signature = "s:ss";
        this->_help = "Returns a list of unread public chat messages on a hub. Params: hub URL, separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shub(paramList.getString(0));
        string const sseparator(paramList.getString(1));
        paramList.verifyEnd(2);
        string retchat;
        ServerThread::getInstance()->getChatPubFromClient(retchat, shub, sseparator);
        *retvalP = xmlrpc_c::value_string(retchat);
    }
};

class sendSearchMethod : public xmlrpc_c::method {
public:
    sendSearchMethod() {
        this->_signature = "i:s";
        this->_help = "Starts a search on all hubs. Params: search string; returns: 0 on success, non-zero otherwise";
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
        this->_signature = "A:s";
        this->_help = "Returns a list of search results on a certain hub. Params: hub URL; returns: array of search results";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shuburl(paramList.getString(0));
        vector<StringMap> tmp;
        ServerThread::getInstance()->returnSearchResults(tmp, shuburl);
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
        this->_help = "Sets the priority of a target. Params: target, priority in range from 0 (paused) to 5 (highest); returns: 0 on success; non-zero otherwise";
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

class systemShutdownMethod : public xmlrpc_c::registry::shutdown {
public:
    ~systemShutdownMethod() {}

    void
    doit(std::string const& comment, void * const callInfo) const {
        if (isVerbose) std::cout << "Shutting down, comment: " << comment << std::endl;
        bServerTerminated = true;
    }
};
#endif
