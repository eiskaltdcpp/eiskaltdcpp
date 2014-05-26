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

#pragma once

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
xmlrpc_c::serverAbyss * server;

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
        if (ServerThread::getInstance()->sendPrivateMessage(shub, snick, smess))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
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
        string upload = Util::formatBytes(up);
        string download = Util::formatBytes(down);
        map<string, xmlrpc_c::value> tmp_struct_in;
        tmp_struct_in["ratio"] = xmlrpc_c::value_string(Util::toString(ratio));
        tmp_struct_in["up"] = xmlrpc_c::value_string(upload);
        tmp_struct_in["down"] = xmlrpc_c::value_string(download);
        tmp_struct_in["up_bytes"] = xmlrpc_c::value_double(up);
        tmp_struct_in["down_bytes"] = xmlrpc_c::value_double(down);
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
        this->_signature = "i:";
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

class clearSearchResultsMethod : public xmlrpc_c::method {
public:
    clearSearchResultsMethod() {
        this->_signature = "i:s";
        this->_help = "Clear search result on target hub/all hubs. Params: hub url; returns: array of search results";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shuburl(paramList.getString(0));
        if (ServerThread::getInstance()->clearSearchResults(shuburl))
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
        vector<StringMap> hublist;
        ServerThread::getInstance()->returnSearchResults(hublist, shuburl);
        vector<xmlrpc_c::value> tmp_array_in;
        for (const auto& hub : hublist) {
            map<string, xmlrpc_c::value> tmp_struct_in;
            for (const auto& rearchresult : hub) {
                pair<string, xmlrpc_c::value> member(rearchresult.first, xmlrpc_c::value_string(rearchresult.second));
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

class moveQueueItemMethod : public xmlrpc_c::method {
public:
    moveQueueItemMethod() {
        this->_signature = "i:ss";
        this->_help = "Move target param of queue item to new location. Params: target, new target; returns: 0 on success; non-zero otherwise";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const starget(paramList.getString(0));
        string const snewtarget(paramList.getString(1));
        paramList.verifyEnd(2);
        if (ServerThread::getInstance()->moveQueueItem(starget, snewtarget))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class removeQueueItemMethod : public xmlrpc_c::method {
public:
    removeQueueItemMethod() {
        this->_signature = "i:ss";
        this->_help = "Remove queue item by target. Params: target; returns: 0 on success; non-zero otherwise";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const starget(paramList.getString(0));;
        paramList.verifyEnd(1);
        if (ServerThread::getInstance()->removeQueueItem(starget))
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class listQueueTargetsMethod : public xmlrpc_c::method {
public:
    listQueueTargetsMethod() {
        this->_signature = "i:s";
        this->_help = "List queue item targets. Params: separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const sseparator(paramList.getString(0));;
        paramList.verifyEnd(1);
        string tmp;
        ServerThread::getInstance()->listQueueTargets(tmp, sseparator);
        *retvalP = xmlrpc_c::value_string(tmp);
    }
};

class listQueueMethod : public xmlrpc_c::method {
public:
    listQueueMethod() {
        this->_signature = "i:s";
        this->_help = "List queue items. Params: separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        unordered_map<string,StringMap> listqueue;
        ServerThread::getInstance()->listQueue(listqueue);
        map<string, xmlrpc_c::value> tmp_struct1_in;
        for (const auto& item : listqueue) {
            map<string, xmlrpc_c::value> tmp_struct2_in;
            for (const auto& parameter : item.second) {
                pair<string, xmlrpc_c::value> member2(parameter.first, xmlrpc_c::value_string(parameter.second));
                tmp_struct2_in.insert(member2);
            }
            xmlrpc_c::value_struct const tmp_struct2_out(tmp_struct2_in);
            pair<string, xmlrpc_c::value> member1(i->first, xmlrpc_c::value_struct(tmp_struct2_out));
            tmp_struct1_in.insert(member1);
        }
        xmlrpc_c::value_struct tmp_struct1_out(tmp_struct1_in);
        *retvalP = tmp_struct1_out;
    }
};

class getSourcesItemMethod : public xmlrpc_c::method {
public:
    getSourcesItemMethod() {
        this->_signature = "i:s";
        this->_help = "Return sources on target. Params: target, separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const starget(paramList.getString(0));;
        string const sseparator(paramList.getString(1));;
        paramList.verifyEnd(2);
        string sources;
        unsigned int online = 0;
        ServerThread::getInstance()->getItemSourcesbyTarget(starget, sseparator, sources, online);
        map<string, xmlrpc_c::value> tmp_struct_in;
        tmp_struct_in["sources"] = xmlrpc_c::value_string(sources);
        tmp_struct_in["online"] = xmlrpc_c::value_int(online);
        xmlrpc_c::value_struct const tmp_struct_out(tmp_struct_in);
        *retvalP = tmp_struct_out;
    }
};

class getHashStatusMethod : public xmlrpc_c::method {
public:
    getHashStatusMethod() {
        this->_signature = "i:s";
        this->_help = "Return hashing status. Params: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string tmp = " ",status = " "; int64_t bytes = 0; size_t files = 0;
        ServerThread::getInstance()->getHashStatus(tmp, bytes, files, status);
        map<string, xmlrpc_c::value> tmp_struct_in;
        tmp_struct_in["currentfile"] = xmlrpc_c::value_string(tmp);
        tmp_struct_in["status"] = xmlrpc_c::value_string(status);
        tmp_struct_in["bytesleft"] = xmlrpc_c::value_i8(bytes);
        tmp_struct_in["filesleft"] = xmlrpc_c::value_i8(files);
        xmlrpc_c::value_struct const tmp_struct_out(tmp_struct_in);
        *retvalP = tmp_struct_out;
    }
};

class pauseHashMethod : public xmlrpc_c::method {
public:
    pauseHashMethod() {
        this->_signature = "i:s";
        this->_help = "Switch pause hashing. Params: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        if (ServerThread::getInstance()->pauseHash())
            *retvalP = xmlrpc_c::value_int(0);
        else
            *retvalP = xmlrpc_c::value_int(1);
    }
};

class getMethodListMethod : public xmlrpc_c::method {
public:
    getMethodListMethod() {
        this->_signature = "i:s";
        this->_help = "Return method list separated by \"|\". Params: none";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string tmp;
        ServerThread::getInstance()->getMethodList(tmp);
        *retvalP = xmlrpc_c::value_string(tmp);
    }
};

class getHubUserListMethod: public xmlrpc_c::method {
public:
    getHubUserListMethod() {
        this->_signature = "i:ss";
        this->_help = "Return user list on huburl. Params: huburl, separator";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const shuburl(paramList.getString(0));;
        string const sseparator(paramList.getString(1));;
        paramList.verifyEnd(2);
        string tmp;
        ServerThread::getInstance()->getHubUserList(tmp, shuburl, sseparator);
        *retvalP = xmlrpc_c::value_string(tmp);
    }
};

class getUserInfoMethod : public xmlrpc_c::method {
public:
    getUserInfoMethod() {
        this->_signature = "i:ss";
        this->_help = "Return info about user on huburl. Params: nick, huburl";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        string const snick(paramList.getString(0));;
        string const shuburl(paramList.getString(1));;
        paramList.verifyEnd(2);
        StringMap params;
        if (ServerThread::getInstance()->getUserInfo(params, snick, shuburl)) {
            map<string, xmlrpc_c::value> tmp_struct_in;
            for (const auto& parameter : params) {
                pair<string, xmlrpc_c::value> member(parameter.first, xmlrpc_c::value_string(parameter.second));
                tmp_struct_in.insert(member);
            }
            xmlrpc_c::value_struct const tmp_struct_out(tmp_struct_in);
            *retvalP = tmp_struct_out;
        } else
            *retvalP = xmlrpc_c::value_nil();
    }
};

class matchAllListMethod : public xmlrpc_c::method {
public:
    matchAllListMethod() {
        this->_signature = "s:";
        this->_help = "Match all local file lists. Params: none; returns: 0";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        ServerThread::getInstance()->matchAllList();
        *retvalP = xmlrpc_c::value_int(0);
    }
};

class listHubsFullDescMethod : public xmlrpc_c::method {
public:
    listHubsFullDescMethod() {
        this->_signature = "s:";
        this->_help = "Return list all hubs with params. Params: none; returns: array of hub with params";
    }

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {

        unordered_map<string,StringMap> listhubs;
        ServerThread::getInstance()->listHubsFullDesc(listhubs);
        map<string, xmlrpc_c::value> tmp_struct1_in;
        for (const auto& hub : listhubs) {
            map<string, xmlrpc_c::value> tmp_struct2_in;
            for (const auto& parameter : hub.second) {
                pair<string, xmlrpc_c::value> member2(parameter.first, xmlrpc_c::value_string(parameter.second));
                tmp_struct2_in.insert(member2);
            }
            xmlrpc_c::value_struct const tmp_struct2_out(tmp_struct2_in);
            pair<string, xmlrpc_c::value> member1(hub.first, xmlrpc_c::value_struct(tmp_struct2_out));
            tmp_struct1_in.insert(member1);
        }
        xmlrpc_c::value_struct tmp_struct1_out(tmp_struct1_in);
        *retvalP = tmp_struct1_out;
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
