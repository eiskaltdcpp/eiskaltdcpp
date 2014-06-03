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
#include "json/jsonrpc-cpp/jsonrpc_common.h"

using namespace std;

// ./cli-jsonrpc-curl.pl  '{"jsonrpc": "2.0", "id": "1", "method": "show.version"}'
// ./cli-jsonrpc-curl.pl '{"jsonrpc": "2.0", "id": "sv0t7t2r", "method": "queue.getsources", "params":{"target": "/home/egik/Видео/Shakugan no Shana III - 16 - To Battle, Once More [Zero-Raws].mp4"}}'
// ./cli-jsonrpc-curl.pl '{"jsonrpc": "2.0", "id": "sv0t7t2r", "method": "hub.add", "params":{"huburl": "adc://localhost:1511"}}'
// ./cli-jsonrpc-curl.pl '{"jsonrpc": "2.0", "id": "1", "method": "hub.pm", "params":{"huburl": "adc://localhost:1511", "nick" : "test", "message" : "test"}}'

void JsonRpcMethods::FailedValidateRequest(Json::Value& error) {
    Json::Value err;
    error["id"] = Json::Value::null;
    error["jsonrpc"] = "2.0";

    err["code"] = Json::Rpc::INVALID_PARAMS;
    err["message"] = "Invalid params in JSON-RPC request.";
    error["error"] = err;
}

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

    if (root["params"].isMember("magnet") && !root["params"]["magnet"].isString()
        && !root["params"]["magnet"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("enc") && !root["params"]["enc"].isString()
        && !root["params"]["enc"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("message") && !root["params"]["message"].isString()
        && !root["params"]["message"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("nick") && !root["params"]["nick"].isString()
        && !root["params"]["nick"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("message") && !root["params"]["message"].isString()
        && !root["params"]["message"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    if (ServerThread::getInstance()->sendPrivateMessage(root["params"]["huburl"].asString(), root["params"]["nick"].asString(), root["params"]["message"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "HubSayPM (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubs(const Json::Value& root, Json::Value& response)
{
    if (isDebug) std::cout << "ListHubs (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("directory") && !root["params"]["directory"].isString()
        && !root["params"]["directory"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("virtname") && !root["params"]["virtname"].isString()
        && !root["params"]["virtname"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("directory") && !root["params"]["directory"].isString()
        && !root["params"]["directory"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("virtname") && !root["params"]["virtname"].isString()
        && !root["params"]["virtname"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if (root["params"].isMember("directory") && !root["params"]["directory"].isString()
        && !root["params"]["directory"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("nick") && !root["params"]["nick"].isString()
        && !root["params"]["nick"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("searchstring") && !root["params"]["searchstring"].isString()
        && !root["params"]["searchstring"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("searchtype") && !root["params"]["searchtype"].isInt()
        && !root["params"]["searchtype"].isConvertibleTo(Json::intValue))
        || (root["params"].isMember("sizemode") && !root["params"]["sizemode"].isInt()
        && !root["params"]["sizemode"].isConvertibleTo(Json::intValue))
        || (root["params"].isMember("sizetype") && !root["params"]["sizetype"].isInt()
         && !root["params"]["sizetype"].isConvertibleTo(Json::intValue))
        || (root["params"].isMember("size") && !root["params"]["size"].isDouble()
         && !root["params"]["size"].isConvertibleTo(Json::realValue))
        || (root["params"].isMember("huburls") && !root["params"]["huburls"].isString()
         && !root["params"]["huburls"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

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

    if (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    vector<StringMap> hublist;
    Json::Value parameters;
    ServerThread::getInstance()->returnSearchResults(hublist, root["params"]["huburl"].asString());
    int k = 0;
    for (const auto& hub : hublist) {
        for (const auto& rearchresult : hub) {
            parameters[k][rearchresult.first] = rearchresult.second;
        }
        ++k;
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

    auto up    = SETTING(TOTAL_UPLOAD);
    auto down  = SETTING(TOTAL_DOWNLOAD);
    auto ratio = (down > 0) ? up / down : 0;

    string upload = Util::formatBytes(up);
    string download = Util::formatBytes(down);
    response["result"]["ratio"] = Util::toString(ratio);
    response["result"]["up"] = upload;
    response["result"]["down"] = download;
    response["result"]["up_bytes"] = Util::toString(up);
    response["result"]["down_bytes"] = Util::toString(down);
    if (isDebug) std::cout << "ShowRatio (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SetPriorityQueueItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "SetPriorityQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("priority") && !root["params"]["priority"].isInt()
        && !root["params"]["priority"].isConvertibleTo(Json::intValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("source") && !root["params"]["source"].isString()
        && !root["params"]["source"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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

    if (root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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
    for (const auto& item : listqueue) {
        for (const auto& parameter : item.second) {
            parameters[item.first][parameter.first] = parameter.second;
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

    if (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

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

    if ((root["params"].isMember("directory") && !root["params"]["directory"].isString()
        && !root["params"]["directory"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("tth") && !root["params"]["tth"].isString()
        && !root["params"]["tth"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("filename") && !root["params"]["filename"].isString()
        && !root["params"]["filename"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("size") && !root["params"]["size"].isInt64()
        && !root["params"]["size"].isConvertibleTo(Json::intValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    std::string directory = root["params"]["directory"].asString();
    std::string tth = root["params"]["tth"].asString();
    std::string name = root["params"]["filename"].asString();
    int64_t size = root["params"]["size"].asInt64();

    if (ServerThread::getInstance()->addInQueue(directory, name, size, tth))
        response["result"] = 0;
    else
        response["result"] = 1;

    if (isDebug) std::cout << "AddQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetSourcesItem(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetSourcesItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

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
    for (const auto& hub : listhubs) {
        for (const auto& parameter : hub.second) {
            parameters[hub.first][parameter.first] = parameter.second;
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

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    string tmp;
    ServerThread::getInstance()->getHubUserList(tmp, root["params"]["huburl"].asString(), root["params"]["separator"].asString());
    response["result"] = tmp;
    if (isDebug) std::cout << "GetHubUserList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetUserInfo(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetUserInfo (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("nick") && !root["params"]["nick"].isString()
        && !root["params"]["nick"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    StringMap params; Json::Value parameters;
    if (ServerThread::getInstance()->getUserInfo(params, root["params"]["nick"].asString(),root["params"]["huburl"].asString())) {
        for (const auto& parameter : params) {
            parameters[parameter.first] = parameter.second;
        }
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "GetUserInfo (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowLocalLists(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "ShowLocalLists (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string tmp;
    ServerThread::getInstance()->showLocalLists(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (isDebug) std::cout << "ShowLocalLists (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetClientFileList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetClientFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string ret;
    if (ServerThread::getInstance()->getClientFileList(root["params"]["filelist"].asString(), ret))
        response["result"] = ret;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "GetClientFileList (response): " << response << std::endl;
    return true;
}
bool JsonRpcMethods::OpenFileList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "OpenFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (ServerThread::getInstance()->openFileList(root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "OpenFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::CloseFileList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "CloseFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (ServerThread::getInstance()->closeFileList(root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "CloseFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::CloseAllFileLists(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "CloseAllFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->closeAllFileLists();
    response["result"] = 0;
    if (isDebug) std::cout << "CloseAllFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowOpenedLists(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "ShowOpenedLists (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string tmp;
    ServerThread::getInstance()->showOpenedLists(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (isDebug) std::cout << "ShowOpenedLists (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::LsDirInList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "LsDirInList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("directory") && !root["params"]["directory"].isString()
        && !root["params"]["directory"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    Json::Value parameters;
    unordered_map<string,StringMap> map;
    ServerThread::getInstance()->lsDirInList(root["params"]["directory"].asString(), root["params"]["filelist"].asString(), map);
    for (const auto& item : map) {
        for (const auto& parameter : item.second) {
            parameters[item.first][parameter.first] = parameter.second;
        }
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "LsDirInList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DownloadDirFromList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "DownloadDirFromList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("downloadto") && !root["params"]["downloadto"].isString()
        && !root["params"]["downloadto"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    if (ServerThread::getInstance()->downloadDirFromList(root["params"]["target"].asString(), root["params"]["downloadto"].asString(), root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "DownloadDirFromList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DownloadFileFromList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "DownloadFileFromList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("downloadto") && !root["params"]["downloadto"].isString()
        && !root["params"]["downloadto"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    if (ServerThread::getInstance()->downloadFileFromList(root["params"]["target"].asString(), root["params"]["downloadto"].asString(), root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (isDebug) std::cout << "DownloadFileFromList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetItemDescbyTarget(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "GetItemDescbyTarget (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    Json::Value parameters; StringMap map;
    ServerThread::getInstance()->getItemDescbyTarget(root["params"]["target"].asString(), map);
    for (const auto& parameter : map) {
        parameters[parameter.first] = parameter.second;
    }
    response["result"] = parameters;
    if (isDebug) std::cout << "GetItemDescbyTarget (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::QueueClear(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "QueueClear (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->queueClear();
    response["result"] = 0;
    if (isDebug) std::cout << "QueueClear (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SettingsGetSet(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "SettingsGetSet (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("key") && !root["params"]["key"].isString()
        && !root["params"]["key"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("value") && !root["params"]["value"].isString()
        && !root["params"]["value"].isConvertibleTo(Json::stringValue))
        ) {
        FailedValidateRequest(response);
        return false;
    }

    string out;
    bool b = ServerThread::getInstance()->settingsGetSet(out, root["params"]["key"].asString(), root["params"]["value"].asString());
    if (b) {
        if (root["params"]["value"].asString().empty()) {
            response["result"]["curr_value"] = out;
        } else {
            response["result"] = 0;
        }
    } else {
        response["result"] = 1;
    }
    if (isDebug) std::cout << "SettingsGetSet (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterOnOff(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "IpFilterOnOff (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("on") && !root["params"]["on"].isBool()
        && !root["params"]["on"].isConvertibleTo(Json::booleanValue)) {
        FailedValidateRequest(response);
        return false;
    }

    ServerThread::getInstance()->ipfilterOnOff(root["params"]["on"].asBool());
    response["result"] = 0;
    if (isDebug) std::cout << "IpFilterOnOff (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterList(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "IpFilterList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string out;
    ServerThread::getInstance()->ipfilterList(out, root["params"]["separator"].asString());
    response["result"] = out;
    if (isDebug) std::cout << "IpFilterList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterAddRules(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "IpFilterAddRules (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("rules") && !root["params"]["rules"].isString()
        && !root["params"]["rules"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    ServerThread::getInstance()->ipfilterAddRules(root["params"]["rules"].asString());
    response["result"] = 0;
    if (isDebug) std::cout << "IpFilterAddRules (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterPurgeRules(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "IpFilterPurgeRules (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    ServerThread::getInstance()->ipfilterPurgeRules(root["params"]["rules"].asString());
    response["result"] = 0;
    if (isDebug) std::cout << "IpFilterPurgeRules (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterUpDownRule(const Json::Value& root, Json::Value& response) {
    if (isDebug) std::cout << "IpFilterUpDownRule (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("up") && !root["params"]["up"].isBool()
         && !root["params"]["up"].isConvertibleTo(Json::booleanValue))
         || (root["params"].isMember("rule") && !root["params"]["rule"].isString()
         && !root["params"]["rule"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    ServerThread::getInstance()->ipfilterUpDownRule(root["params"]["up"].asBool(), root["params"]["rule"].asString());
    response["result"] = 0;
    if (isDebug) std::cout << "IpFilterUpDownRule (response): " << response << std::endl;
    return true;
}
