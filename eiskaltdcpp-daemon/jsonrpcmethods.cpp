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

// ./cli-jsonrpc-curl.pl  '{"jsonrpc": "2.0", "id": "1", "method": "show.version"}'
// ./cli-jsonrpc-curl.pl '{"jsonrpc": "2.0", "id": "sv0t7t2r", "method": "queue.getsources", "params":{"target": "/home/egik/Видео/Shakugan no Shana III - 16 - To Battle, Once More [Zero-Raws].mp4"}}'
// ./cli-jsonrpc-curl.pl '{"jsonrpc": "2.0", "id": "sv0t7t2r", "method": "hub.add", "params":{"huburl": "adc://localhost:1511"}}'

bool JsonRpcMethods::StopDaemon(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "StopDaemon (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = 0;
    bServerTerminated = true;
    if (isDebug) std::cout << "StopDaemon (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MagnetAdd(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "MagnetAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::string name,tth;int64_t size;

    bool ok = splitMagnet(root["params"]["magnet"].asString(), name, size, tth);
    if (isDebug) std::cout << "splitMagnet: \n tth: " << tth << "\n size: " << size << "\n name: " << name << std::endl;
    if (ok && ServerThread::getInstance()->addInQueue(root["params"]["directory"].asString(), name, size, tth))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "MagnetAdd (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubAdd(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "HubAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->connectClient(root["params"]["huburl"].asString(), root["params"]["enc"].asString());
    response["result"] = "Connecting to " + root["params"]["huburl"].asString();
    if (isDebug) std::cout << "HubAdd (response): " << response << std::endl;
    return true;
}
bool JsonRpcMethods::HubDel(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "HubDel (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->disconnectClient(root["params"]["huburl"].asString());
    response["result"] = 0;
    if (isDebug) std::cout << "HubDel (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSay(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "HubSay (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->findHubInConnectedClients(root["params"]["huburl"].asString())) {
        ServerThread::getInstance()->sendMessage(root["params"]["huburl"].asString(),root["params"]["message"].asString());
        response["result"] = 0;
    } else
        response["result"] = 1;
    if (isDebug) std::cout << "HubSay (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSayPM(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "HubSayPM (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp = ServerThread::getInstance()->sendPrivateMessage(root["params"]["huburl"].asString(), root["params"]["nick"].asString(), root["params"]["message"].asString());
    response["result"] = tmp;
    if (isDebug) std::cout << "HubSayPM (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubs(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ListHubs (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string listhubs;
    ServerThread::getInstance()->listConnectedClients(listhubs, root["params"]["separator"].asString());
    response["result"] = listhubs;
    if (isDebug) std::cout << "ListHubs (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddDirInShare(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "AddDirInShare (root): " << root << std::endl;
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
    if (isDebug) std::cout << "AddDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RenameDirInShare(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "RenameDirInShare (root): " << root << std::endl;
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
    if (isDebug) std::cout << "RenameDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DelDirFromShare(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "DelDirFromShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->delDirFromShare(root["params"]["directory"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "DelDirFromShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListShare(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ListShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string listshare;
    ServerThread::getInstance()->listShare(listshare, root["params"]["separator"].asString());
    response["result"] = listshare;
    if (isDebug) std::cout << "ListShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RefreshShare(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "RefreshShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ShareManager::getInstance()->setDirty();
    ShareManager::getInstance()->refresh(true);
    response["result"] = 0;
    if (isDebug) std::cout << "RefreshShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetFileList(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "GetFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->getFileList(root["params"]["huburl"].asString(), root["params"]["nick"].asString(), false))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "GetFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetChatPub(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "GetChatPub (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string retchat;
    ServerThread::getInstance()->getChatPubFromClient(retchat, root["params"]["huburl"].asString(), root["params"]["separator"].asString());
    response["result"] = retchat;
    if (isDebug) std::cout << "GetChatPub (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SendSearch(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "SendSearch (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->sendSearchonHubs(root["params"]["searchstring"].asString(), root["params"]["searchtype"].asInt(), root["params"]["sizemode"].asInt(), root["params"]["sizetype"].asInt(), root["params"]["size"].asDouble(), root["params"]["huburls"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "SendSearch (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ReturnSearchResults(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ReturnSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    vector<StringMap> tmp;
    Json::Value parameters;
    ServerThread::getInstance()->returnSearchResults(tmp, root["params"]["huburl"].asString());
    auto i = tmp.begin();int k = 0;
    while (i != tmp.end()) {
        for (auto kk = (*i).begin(); kk != (*i).end(); ++kk) {
            parameters[k][kk->first] = kk->second;
        }
        ++i; ++k;
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "ReturnSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowVersion(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ShowVersion (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string version(EISKALTDCPP_VERSION);
    version.append(" (");
    version.append(EISKALTDCPP_VERSION_SFX);
    version.append(")");
    response["result"] = version;
    if (isDebug) std::cout << "ShowVersion (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowRatio(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ShowRatio (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    double ratio;
    double up   = static_cast<double>(SETTING(TOTAL_UPLOAD));
    double down = static_cast<double>(SETTING(TOTAL_DOWNLOAD));

    if (down > 0)
        ratio = up / down;
    else
        ratio = 0;
    string upload = Util::formatBytes(up);
    string download = Util::formatBytes(down);
    response["result"]["ratio"] = Util::toString(ratio);
    response["result"]["up"] = upload;
    response["result"]["down"] = download;
    if (isDebug) std::cout << "ShowRatio (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SetPriorityQueueItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "SetPriorityQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->setPriorityQueueItem(root["params"]["target"].asString(), root["params"]["priority"].asInt()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "SetPriorityQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MoveQueueItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "MoveQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->moveQueueItem(root["params"]["source"].asString(), root["params"]["target"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "MoveQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RemoveQueueItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "removeQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->removeQueueItem(root["params"]["target"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "removeQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListQueueTargets(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "ListQueueTargets (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp;
    ServerThread::getInstance()->listQueueTargets(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (isDebug) std::cout << "ListQueueTargets (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListQueue(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "ListQueue (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    Json::Value parameters;
    unordered_map<string,StringMap> listqueue;
    ServerThread::getInstance()->listQueue(listqueue);
    for (auto i = listqueue.begin(); i != listqueue.end(); ++i) {
        for (auto kk = i->second.begin(); kk != i->second.end(); ++kk) {
            parameters[i->first][kk->first] = kk->second;
        }
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "ListQueue (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ClearSearchResults(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "ClearSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->clearSearchResults(root["params"]["huburl"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "ClearSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddQueueItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "AddQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (isDebug) std::cout << "AddQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetSourcesItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetSourcesItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string sources;
    unsigned int online = 0;
    ServerThread::getInstance()->getItemSourcesbyTarget(root["params"]["target"].asString(), root["params"]["separator"].asString(), sources, online);
    response["result"]["sources"] = sources;
    response["result"]["online"] = online;
    if (isDebug) std::cout << "GetSourcesItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetHashStatus(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetHashStatus (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp = " ",status = " "; int64_t bytes = 0; size_t files = 0;
    ServerThread::getInstance()->getHashStatus(tmp, bytes, files, status);
    response["result"]["currentfile"]=tmp;
    response["result"]["status"]=status;
    response["result"]["bytesleft"]=Json::Value::Int64(bytes);
    response["result"]["filesleft"]=Json::Value::UInt(files);
    if (isDebug) std::cout << "GetHashStatus (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetMethodList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetMethodList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp;
    ServerThread::getInstance()->getMethodList(tmp);
    response["result"] = tmp;
    if (isDebug) std::cout << "GetMethodList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::PauseHash(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "PauseHash (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->pauseHash())
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "PauseHash (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MatchAllLists(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "MatchAllLists (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->matchAllList();
        response["result"] = 0;
    if (isDebug) std::cout << "MatchAllLists (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubsFullDesc(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ListHubsFullDesc (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    Json::Value parameters;
    unordered_map<string,StringMap> listhubs;
    ServerThread::getInstance()->listHubsFullDesc(listhubs);
    for (auto i = listhubs.begin(); i != listhubs.end(); ++i) {
        for (auto kk = i->second.begin(); kk != i->second.end(); ++kk) {
            parameters[i->first][kk->first] = kk->second;
        }
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "ListHubsFullDesc (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetHubUserList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetHubUserList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    StringMap tmp; Json::Value parameters;
    ServerThread::getInstance()->getHubUserList(tmp, root["params"]["huburl"].asString());
    for (StringMap::iterator kk = tmp.begin(); kk != tmp.end(); ++kk) {
        parameters[kk->first] = kk->second;
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "GetHubUserList (response): " << response << std::endl;
    return true;
}

Json::Value JsonRpcMethods::GetDescriptionStopDaemon() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value returns;

  root["description"] = "Stop daemon";
  root["parameters"] = Json::Value::null;
  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;

  return root;
}

Json::Value JsonRpcMethods::GetDescriptionMagnetAdd() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  root["description"] = "Add magnet in queue";
  param1["type"] = "string";
  param1["description"] = "Magnet link";
  param2["type"] = "string";
  param2["description"] = "Download directory";

  parameters["magnet"] = param1;
  parameters["directory"] = param2;
  root["parameters"] = parameters;

  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;

  return root;
}

Json::Value JsonRpcMethods::GetDescriptionHubAdd() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  root["description"] = "Connect to huburl";
  param1["type"] = "string";
  param1["description"] = "Hub url";
  param2["type"] = "string";
  param2["description"] = "Encoding";

  parameters["huburl"] = param1;
  parameters["enc"] = param2;
  root["parameters"] = parameters;

  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;

  return root;
}

Json::Value JsonRpcMethods::GetDescriptionHubDel() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1;
  Json::Value returns;
  root["description"] = "Disconnect from huburl";
  param1["type"] = "string";
  param1["description"] = "Hub url";

  parameters["huburl"] = param1;
  root["parameters"] = parameters;

  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionHubSay() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  root["description"] = "Send message on hub url";
  param1["type"] = "string";
  param1["description"] = "Hub url";
  param2["type"] = "string";
  param2["description"] = "Message";

  parameters["huburl"] = param1;
  parameters["message"] = param2;
  root["parameters"] = parameters;

  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionHubSayPM() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2,param3;
  Json::Value returns;

  root["description"] = "Send private message to nick on hub url";
  param1["type"] = "string";
  param1["description"] = "Hub url";
  param2["type"] = "string";
  param2["description"] = "Nick";
  param3["type"] = "string";
  param3["description"] = "Message";

  parameters["huburl"] = param1;
  parameters["nick"] = param2;
  parameters["message"] = param3;
  root["parameters"] = parameters;

  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionListHubs() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1;
  Json::Value returns;

  root["description"] = "Get list of hubs";
  param1["type"] = "string";
  param1["description"] = "Separator";

  parameters["separator"] = param1;
  root["parameters"] = parameters;

  returns["type"] = "integer";
  returns["description"] = "Return 0 on success and 1 on failed";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionAddDirInShare() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  root["description"] = "Add directory in share with virtual name";
  param1["type"] = "string";
  param1["description"] = "Directory";
  param2["type"] = "string";
  param2["description"] = "VirtualName";

  parameters["directory"] = param1;
  parameters["virtname"] = param2;
  root["parameters"] = parameters;

  returns["type"] = "integer or string";
  returns["description"] = "Return 0 on success and 1 on failed or shareexception";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionRenameDirInShare() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  root["description"] = "Rename directory in share";
  param1["type"] = "string";
  param1["description"] = "Directory";
  param2["type"] = "string";
  param2["description"] = "VirtualName";

  parameters["directory"] = param1;
  parameters["virtname"] = param2;
  root["parameters"] = parameters;

  returns["type"] = "integer or string";
  returns["description"] = "Return 0 on success and 1 on failed or shareexception";
  root["returns"] = returns;

  return root;
}
Json::Value JsonRpcMethods::GetDescriptionDelDirFromShare() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1;
  Json::Value returns;

  root["description"] = "Delete directory from share";
  param1["type"] = "string";
  param1["description"] = "Directory";

  parameters["directory"] = param1;
  root["parameters"] = parameters;

  returns["type"] = "integer or string";
  returns["description"] = "Return 0 on success and 1 on failed or shareexception";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionListShare() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1;
  Json::Value returns;

  root["description"] = "Return list share";
  param1["type"] = "string";
  param1["description"] = "Separator";

  parameters["separator"] = param1;
  root["parameters"] = parameters;

  returns["type"] = "string";
  returns["description"] = "Return list share";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionRefreshShare() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value returns;

  root["description"] = "Run refresh share";
  root["parameters"] = Json::Value::null;

  returns["type"] = "integer";
  returns["description"] = "Return 0";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionGetFileList() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  return root;
}
Json::Value JsonRpcMethods::GetDescriptionGetChatPub() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;

  root["description"] = "Return chat from huburl";
  param1["type"] = "string";
  param1["description"] = "Hub url";
  param2["type"] = "string";
  param2["description"] = "Separator";

  parameters["huburl"] = param1;
  parameters["separator"] = param2;
  root["parameters"] = parameters;

  returns["type"] = "string";
  returns["description"] = "Return chat from hub url";
  root["returns"] = returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionSendSearch() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionReturnSearchResults() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionShowVersion() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionShowRatio() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionSetPriorityQueueItem() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionMoveQueueItem() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionRemoveQueueItem() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionListQueueTargets() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionListQueue() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionClearSearchResults() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionAddQueueItem() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionGetSourcesItem() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionGetHashStatus() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
Json::Value JsonRpcMethods::GetDescriptionPauseHash() {
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1,param2;
  Json::Value returns;
  return root;
}
