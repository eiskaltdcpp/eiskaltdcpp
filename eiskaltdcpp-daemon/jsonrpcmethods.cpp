/***************************************************************************
*                                                                         *
*   Copyright 2011 Eugene Petrov <dhamp@ya.ru>                            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "stdafx.h"
#include "jsonrpcmethods.h"
#include "ServerManager.h"
#include "utility.h"
#include "ServerThread.h"
#include "VersionGlobal.h"
#include "dcpp/format.h"

using namespace std;

bool JsonRpcMethods::Print(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "Receive query: " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = "success";
    return true;
}

bool JsonRpcMethods::StopDaemon(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "StopDaemon (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = 0;
    bServerTerminated = true;
    if (isVerbose) std::cout << "StopDaemon (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MagnetAdd(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "MagnetAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::string name,tth;int64_t size;

    bool ok = splitMagnet(root["params"]["magnet"].asString(), name, size, tth);
    if (isVerbose) std::cout << "splitMagnet: \n tth: " << tth << "\n size: " << size << "\n name: " << name << std::endl;
    if (ok && ServerThread::getInstance()->addInQueue(root["params"]["directory"].asString(), name, size, tth))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "MagnetAdd (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubAdd(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "HubAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->connectClient(root["params"]["huburl"].asString(), root["params"]["enc"].asString());
    response["result"] = "Connecting to " + root["huburl"].asString();
    if (isVerbose) std::cout << "HubAdd (response): " << response << std::endl;
    return true;
}
bool JsonRpcMethods::HubDel(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "HubDel (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->disconnectClient(root["params"]["huburl"].asString());
    response["result"] = 0;
    if (isVerbose) std::cout << "HubDel (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSay(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "HubSay (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->findHubInConnectedClients(root["params"]["huburl"].asString())) {
        ServerThread::getInstance()->sendMessage(root["params"]["huburl"].asString(),root["params"]["message"].asString());
        response["result"] = 0;
    } else
        response["result"] = 1;
    if (isVerbose) std::cout << "HubSay (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSayPM(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "HubSayPM (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp = ServerThread::getInstance()->sendPrivateMessage(root["params"]["huburl"].asString(), root["params"]["nick"].asString(), root["params"]["message"].asString());
    response["result"] = tmp;
    if (isVerbose) std::cout << "HubSayPM (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubs(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "ListHubs (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string listhubs;
    ServerThread::getInstance()->listConnectedClients(listhubs, root["params"]["separator"].asString());
    response["result"] = listhubs;
    if (isVerbose) std::cout << "ListHubs (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddDirInShare(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "AddDirInShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    try {
        if (ServerThread::getInstance()->addDirInShare(root["params"]["directory"].asString(), root["params"]["virtname"].asString()))
            response["result"] = 0;
        else
            response["result"] = 1;
    } catch (const ShareException& e) {
        response["result"] = e.getError();
    }
    if (isVerbose) std::cout << "AddDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RenameDirInShare(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "RenameDirInShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    try {
        if (ServerThread::getInstance()->renameDirInShare(root["params"]["directory"].asString(), root["params"]["virtname"].asString()))
            response["result"] = 0;
        else
            response["result"] = 1;
    } catch (const ShareException& e) {
        response["result"] = e.getError();
    }
    if (isVerbose) std::cout << "RenameDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DelDirFromShare(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "DelDirFromShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->delDirFromShare(root["params"]["directory"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "DelDirFromShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListShare(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "ListShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string listshare;
    ServerThread::getInstance()->listShare(listshare, root["params"]["separator"].asString());
    response["result"] = listshare;
    if (isVerbose) std::cout << "ListShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RefreshShare(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "RefreshShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ShareManager::getInstance()->setDirty();
    ShareManager::getInstance()->refresh(true);
    response["result"] = 0;
    if (isVerbose) std::cout << "RefreshShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetFileList(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "GetFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp;
    tmp = ServerThread::getInstance()->getFileList_client(root["params"]["huburl"].asString(), root["params"]["nick"].asString(), false);
    response["result"] = tmp;
    if (isVerbose) std::cout << "GetFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetChatPub(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "GetChatPub (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string retchat;
    ServerThread::getInstance()->getChatPubFromClient(retchat, root["params"]["huburl"].asString(), root["params"]["separator"].asString());
    response["result"] = retchat;
    if (isVerbose) std::cout << "GetChatPub (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SendSearch(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "SendSearch (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->sendSearchonHubs(root["params"]["searchstring"].asString(), root["params"]["searchtype"].asInt(), root["params"]["sizemode"].asInt(), root["params"]["sizetype"].asInt(), root["params"]["size"].asDouble(), root["params"]["huburls"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "SendSearch (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ReturnSearchResults(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "ReturnSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    vector<StringMap> tmp;
    Json::Value parameters;
    ServerThread::getInstance()->returnSearchResults(tmp, root["params"]["huburl"].asString());
    vector<StringMap>::iterator i = tmp.begin();int k = 0;
    while (i != tmp.end()) {
        for (StringMap::iterator kk = (*i).begin(); kk != (*i).end(); ++kk) {
            parameters[k][kk->first] = kk->second;
        }
        ++i; ++k;
    }
    response["result"] = parameters;
    if (isVerbose) std::cout << "ReturnSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowVersion(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "ShowVersion (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string version(EISKALTDCPP_VERSION);
    version.append(" (");
    version.append(EISKALTDCPP_VERSION_SFX);
    version.append(")");
    response["result"] = version;
    if (isVerbose) std::cout << "ShowVersion (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowRatio(const Json::Value& root, Json::Value& response)
{
    if (isVerbose) std::cout << "ShowRatio (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
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
    response["result"] = line;
    if (isVerbose) std::cout << "ShowRatio (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SetPriorityQueueItem(const Json::Value& root, Json::Value& response) {
    if (isVerbose) std::cout << "SetPriorityQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->setPriorityQueueItem(root["params"]["target"].asString(), root["params"]["priority"].asInt()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "SetPriorityQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MoveQueueItem(const Json::Value& root, Json::Value& response) {
    if (isVerbose) std::cout << "MoveQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->moveQueueItem(root["params"]["source"].asString(), root["params"]["target"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "MoveQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RemoveQueueItem(const Json::Value& root, Json::Value& response) {
    if (isVerbose) std::cout << "removeQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->removeQueueItem(root["params"]["target"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "removeQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListQueueTargets(const Json::Value& root, Json::Value& response) {
    if (isVerbose) std::cout << "ListQueueTargets (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp;
    ServerThread::getInstance()->listQueueTargets(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (isVerbose) std::cout << "ListQueueTargets (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListQueue(const Json::Value& root, Json::Value& response) {
    if (isVerbose) std::cout << "ListQueue (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    Json::Value parameters;
    unordered_map<string,StringMap> listqueue;
    ServerThread::getInstance()->listQueue(listqueue);
    unordered_map<string,StringMap>::iterator i = listqueue.begin();
    while (i != listqueue.end()) {
        for (StringMap::iterator kk = i->second.begin(); kk != i->second.end(); ++kk) {
            parameters[i->first][kk->first] = kk->second;
        }
    }
    response["result"] = parameters;
    if (isVerbose) std::cout << "ListQueue (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ClearSearchResults(const Json::Value& root, Json::Value& response) {
    if (isVerbose) std::cout << "ClearSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->clearSearchResults(root["params"]["huburl"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isVerbose) std::cout << "ClearSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddQueueItem(const Json::Value& root, Json::Value& response){
    if (isVerbose) std::cout << "AddQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    
    if (isVerbose) std::cout << "AddQueueItem (response): " << response << std::endl;
    return true;
}
