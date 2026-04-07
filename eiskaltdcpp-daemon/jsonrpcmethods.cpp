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
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "stdafx.h"
#include "jsonrpcmethods.h"
#include "ServerManager.h"
#include "ServerThread.h"
#include "utility.h"
#include "VersionGlobal.h"
#include "dcpp/format.h"
#include "dcpp/DCPlusPlus.h"
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

bool JsonRpcMethods::debug() const { return server_.config().debug; }

bool JsonRpcMethods::StopDaemon(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "StopDaemon (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = 0;
    server_.mgr().requestTermination();
    if (debug()) std::cout << "StopDaemon (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MagnetAdd(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "MagnetAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::string name,tth;int64_t size;

    if (root["params"].isMember("magnet") && !root["params"]["magnet"].isString()
        && !root["params"]["magnet"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    const bool ok = splitMagnet(root["params"]["magnet"].asString(), name, size, tth);
    if (debug()) {
        std::cout << "splitMagnet: \n tth: " << tth << "\n size: " << size << "\n name: " << name << std::endl;
    }
    if (ok && server_.addInQueue(root["params"]["directory"].asString(), name, size, tth))
        response["result"] = 0;
    else
        response["result"] = 1;

    if (debug()) {
        std::cout << "MagnetAdd (response): " << response << std::endl;
    }
    return true;
}

bool JsonRpcMethods::HubAdd(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "HubAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("enc") && !root["params"]["enc"].isString()
        && !root["params"]["enc"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    server_.connectClient(root["params"]["huburl"].asString(),
                                               root["params"]["enc"].asString());
    response["result"] = "Connecting to " + root["params"]["huburl"].asString();
    if (debug()) std::cout << "HubAdd (response): " << response << std::endl;
    return true;
}
bool JsonRpcMethods::HubDel(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "HubDel (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    server_.disconnectClient(root["params"]["huburl"].asString());
    response["result"] = 0;
    if (debug()) std::cout << "HubDel (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSay(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "HubSay (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("message") && !root["params"]["message"].isString()
        && !root["params"]["message"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.findHubInConnectedClients(root["params"]["huburl"].asString())) {
        server_.sendMessage(root["params"]["huburl"].asString(),
                                                 root["params"]["message"].asString());
        response["result"] = 0;
    } else
        response["result"] = 1;
    if (debug()) std::cout << "HubSay (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSayPM(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "HubSayPM (root): " << root << std::endl;
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

    if (server_.sendPrivateMessage(root["params"]["huburl"].asString(),
                                                        root["params"]["nick"].asString(),
                                                        root["params"]["message"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "HubSayPM (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubs(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "ListHubs (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string listhubs;
    server_.listConnectedClients(listhubs,
                                                      root["params"]["separator"].asString());
    response["result"] = listhubs;
    if (debug()) std::cout << "ListHubs (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddDirInShare(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "AddDirInShare (root): " << root << std::endl;
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
        if (server_.addDirInShare(root["params"]["directory"].asString(),
                                                       root["params"]["virtname"].asString()))
            response["result"] = 0;
        else
            response["result"] = 1;
    } catch (const ShareException& e) {
        response["result"] = e.getError();
    }
    if (debug()) std::cout << "AddDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RenameDirInShare(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "RenameDirInShare (root): " << root << std::endl;
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
        if (server_.renameDirInShare(root["params"]["directory"].asString(),
                                                          root["params"]["virtname"].asString()))
            response["result"] = 0;
        else
            response["result"] = 1;
    } catch (const ShareException& e) {
        response["result"] = e.getError();
    }
    if (debug()) std::cout << "RenameDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DelDirFromShare(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "DelDirFromShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("directory") && !root["params"]["directory"].isString()
        && !root["params"]["directory"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.delDirFromShare(root["params"]["directory"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "DelDirFromShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListShare(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "ListShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string listshare;
    server_.listShare(listshare, root["params"]["separator"].asString());
    response["result"] = listshare;
    if (debug()) std::cout << "ListShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RefreshShare(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "RefreshShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    server_.dcCtx().getShareManager()->setDirty();
    server_.dcCtx().getShareManager()->refresh(true);
    response["result"] = 0;
    if (debug()) std::cout << "RefreshShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetFileList(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "GetFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("nick") && !root["params"]["nick"].isString()
        && !root["params"]["nick"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.getFileList(root["params"]["huburl"].asString(),
                                                 root["params"]["nick"].asString(),
                                                 false))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "GetFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetChatPub(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "GetChatPub (root): " << root << std::endl;
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
    server_.getChatPubFromClient(retchat,
                                                      root["params"]["huburl"].asString(),
                                                      root["params"]["separator"].asString());
    response["result"] = retchat;
    if (debug()) std::cout << "GetChatPub (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SendSearch(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "SendSearch (root): " << root << std::endl;
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

    if (server_.sendSearchOnHubs(root["params"]["searchstring"].asString(),
                                                      root["params"]["searchtype"].asInt(),
                                                      root["params"]["sizemode"].asInt(),
                                                      root["params"]["sizetype"].asInt(),
                                                      root["params"]["size"].asDouble(),
                                                      root["params"]["huburls"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "SendSearch (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ReturnSearchResults(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "ReturnSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    vector<StringMap> hublist;
    Json::Value parameters;
    server_.returnSearchResults(hublist, root["params"]["huburl"].asString());
    int k = 0;
    for (const auto& hub : hublist) {
        for (const auto& rearchresult : hub) {
            parameters[k][rearchresult.first] = rearchresult.second;
        }
        ++k;
    }
    response["result"] = parameters;
    if (debug()) std::cout << "ReturnSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowVersion(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "ShowVersion (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = eiskaltdcppVersionString;
    if (debug()) std::cout << "ShowVersion (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowRatio(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "ShowRatio (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    auto* sm = server_.dcCtx().getSettingsManager();
    auto up    = sm->get(SettingsManager::TOTAL_UPLOAD, true);
    auto down  = sm->get(SettingsManager::TOTAL_DOWNLOAD, true);
    auto ratio = (down > 0) ? up / down : 0;

    string upload = Util::formatBytes(up);
    string download = Util::formatBytes(down);
    response["result"]["ratio"] = Util::toString(ratio);
    response["result"]["up"] = upload;
    response["result"]["down"] = download;
    response["result"]["up_bytes"] = Util::toString(up);
    response["result"]["down_bytes"] = Util::toString(down);
    if (debug()) std::cout << "ShowRatio (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SetPriorityQueueItem(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "SetPriorityQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("priority") && !root["params"]["priority"].isInt()
        && !root["params"]["priority"].isConvertibleTo(Json::intValue))) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.setPriorityQueueItem(root["params"]["target"].asString(),
                                                          root["params"]["priority"].asInt()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "SetPriorityQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MoveQueueItem(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "MoveQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("source") && !root["params"]["source"].isString()
        && !root["params"]["source"].isConvertibleTo(Json::stringValue))
        || (root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.moveQueueItem(root["params"]["source"].asString(),
                                                   root["params"]["target"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "MoveQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RemoveQueueItem(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "removeQueueItem (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.removeQueueItem(root["params"]["target"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "removeQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListQueueTargets(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "ListQueueTargets (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string tmp;
    server_.listQueueTargets(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (debug()) std::cout << "ListQueueTargets (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListQueue(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "ListQueue (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    Json::Value parameters;
    unordered_map<string,StringMap> listqueue;
    server_.listQueue(listqueue);
    for (const auto& item : listqueue) {
        for (const auto& parameter : item.second) {
            parameters[item.first][parameter.first] = parameter.second;
        }
    }
    response["result"] = parameters;
    if (debug()) std::cout << "ListQueue (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ClearSearchResults(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "ClearSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("huburl") && !root["params"]["huburl"].isString()
        && !root["params"]["huburl"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.clearSearchResults(root["params"]["huburl"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "ClearSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddQueueItem(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "AddQueueItem (root): " << root << std::endl;
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

    if (server_.addInQueue(directory, name, size, tth))
        response["result"] = 0;
    else
        response["result"] = 1;

    if (debug()) std::cout << "AddQueueItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetSourcesItem(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "GetSourcesItem (root): " << root << std::endl;
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
    server_.getItemSourcesbyTarget(root["params"]["target"].asString(),
                                                        root["params"]["separator"].asString(),
                                                        sources, online);
    response["result"]["sources"] = sources;
    response["result"]["online"] = online;
    if (debug()) std::cout << "GetSourcesItem (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetHashStatus(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "GetHashStatus (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp = " ",status = " "; uint64_t bytes = 0; size_t files = 0;
    server_.getHashStatus(tmp, bytes, files, status);
    response["result"]["currentfile"]=tmp;
    response["result"]["status"]=status;
    response["result"]["bytesleft"]=Json::Value::Int64(bytes);
    response["result"]["filesleft"]=Json::Value::UInt(files);
    if (debug()) std::cout << "GetHashStatus (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::PauseHash(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "PauseHash (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (server_.pauseHash())
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "PauseHash (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MatchAllLists(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "MatchAllLists (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    server_.matchAllList();
    response["result"] = 0;
    if (debug()) std::cout << "MatchAllLists (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubsFullDesc(const Json::Value& root, Json::Value& response)
{
    if (debug()) std::cout << "ListHubsFullDesc (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    Json::Value parameters;
    unordered_map<string,StringMap> listhubs;
    server_.listHubsFullDesc(listhubs);
    for (const auto& hub : listhubs) {
        for (const auto& parameter : hub.second) {
            parameters[hub.first][parameter.first] = parameter.second;
        }
    }
    response["result"] = parameters;
    if (debug()) std::cout << "ListHubsFullDesc (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetHubUserList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "GetHubUserList (root): " << root << std::endl;
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
    server_.getHubUserList(tmp,
                                                root["params"]["huburl"].asString(),
                                                root["params"]["separator"].asString());
    response["result"] = tmp;
    if (debug()) std::cout << "GetHubUserList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetUserInfo(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "GetUserInfo (root): " << root << std::endl;
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
    if (server_.getUserInfo(params,
                                                 root["params"]["nick"].asString(),
                                                 root["params"]["huburl"].asString())) {
        for (const auto& parameter : params) {
            parameters[parameter.first] = parameter.second;
        }
    }
    response["result"] = parameters;
    if (debug()) std::cout << "GetUserInfo (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowLocalLists(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "ShowLocalLists (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string tmp;
    server_.showLocalLists(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (debug()) std::cout << "ShowLocalLists (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetClientFileList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "GetClientFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string ret;
    if (server_.getClientFileList(root["params"]["filelist"].asString(), ret))
        response["result"] = ret;
    else
        response["result"] = 1;
    if (debug()) std::cout << "GetClientFileList (response): " << response << std::endl;
    return true;
}
bool JsonRpcMethods::OpenFileList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "OpenFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.openFileList(root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "OpenFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::CloseFileList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "CloseFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("filelist") && !root["params"]["filelist"].isString()
        && !root["params"]["filelist"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    if (server_.closeFileList(root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "CloseFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::CloseAllFileLists(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "CloseAllFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    server_.closeAllFileLists();
    response["result"] = 0;
    if (debug()) std::cout << "CloseAllFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowOpenedLists(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "ShowOpenedLists (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string tmp;
    server_.showOpenedLists(tmp, root["params"]["separator"].asString());
    response["result"] = tmp;
    if (debug()) std::cout << "ShowOpenedLists (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::LsDirInList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "LsDirInList (root): " << root << std::endl;
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
    server_.lsDirInList(root["params"]["directory"].asString(),
                                             root["params"]["filelist"].asString(),
                                             map);
    for (const auto& item : map) {
        for (const auto& parameter : item.second) {
            parameters[item.first][parameter.first] = parameter.second;
        }
    }
    response["result"] = parameters;
    if (debug()) std::cout << "LsDirInList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DownloadDirFromList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "DownloadDirFromList (root): " << root << std::endl;
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

    if (server_.downloadDirFromList(root["params"]["target"].asString(),
                                                         root["params"]["downloadto"].asString(),
                                                         root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "DownloadDirFromList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DownloadFileFromList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "DownloadFileFromList (root): " << root << std::endl;
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

    if (server_.downloadFileFromList(root["params"]["target"].asString(),
                                                          root["params"]["downloadto"].asString(),
                                                          root["params"]["filelist"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    if (debug()) std::cout << "DownloadFileFromList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetItemDescbyTarget(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "GetItemDescbyTarget (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("target") && !root["params"]["target"].isString()
        && !root["params"]["target"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    Json::Value parameters; StringMap map;
    server_.getItemDescbyTarget(root["params"]["target"].asString(), map);
    for (const auto& parameter : map) {
        parameters[parameter.first] = parameter.second;
    }
    response["result"] = parameters;
    if (debug()) std::cout << "GetItemDescbyTarget (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::QueueClear(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "QueueClear (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    server_.queueClear();
    response["result"] = 0;
    if (debug()) std::cout << "QueueClear (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SettingsGetSet(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "SettingsGetSet (root): " << root << std::endl;
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
    bool b = server_.settingsGetSet(out,
                                                         root["params"]["key"].asString(),
                                                         root["params"]["value"].asString());
    if (b) {
        if (root["params"]["value"].asString().empty()) {
            response["result"]["value"] = out;
        } else {
            response["result"] = 0;
        }
    } else {
        response["result"] = 1;
    }
    if (debug()) std::cout << "SettingsGetSet (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterOnOff(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "IpFilterOnOff (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("on") && !root["params"]["on"].isInt()
        && !root["params"]["on"].isConvertibleTo(Json::intValue)) {
        FailedValidateRequest(response);
        return false;
    }

    server_.ipFilterOnOff(root["params"]["on"].asInt());
    response["result"] = 0;
    if (debug()) std::cout << "IpFilterOnOff (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterList(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "IpFilterList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    string out;
    server_.ipFilterList(out, root["params"]["separator"].asString());
    response["result"] = out;
    if (debug()) std::cout << "IpFilterList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterAddRules(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "IpFilterAddRules (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("rules") && !root["params"]["rules"].isString()
        && !root["params"]["rules"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    server_.ipFilterAddRules(root["params"]["rules"].asString());
    response["result"] = 0;
    if (debug()) std::cout << "IpFilterAddRules (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterPurgeRules(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "IpFilterPurgeRules (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if (root["params"].isMember("separator") && !root["params"]["separator"].isString()
        && !root["params"]["separator"].isConvertibleTo(Json::stringValue)) {
        FailedValidateRequest(response);
        return false;
    }

    server_.ipFilterPurgeRules(root["params"]["rules"].asString());
    response["result"] = 0;
    if (debug()) std::cout << "IpFilterPurgeRules (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::IpFilterUpDownRule(const Json::Value& root, Json::Value& response) {
    if (debug()) std::cout << "IpFilterUpDownRule (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

    if ((root["params"].isMember("up") && !root["params"]["up"].isInt()
         && !root["params"]["up"].isConvertibleTo(Json::intValue))
         || (root["params"].isMember("rule") && !root["params"]["rule"].isString()
         && !root["params"]["rule"].isConvertibleTo(Json::stringValue))) {
        FailedValidateRequest(response);
        return false;
    }

    server_.ipFilterUpDownRule(root["params"]["up"].asInt(),
                                                    root["params"]["rule"].asString());
    response["result"] = 0;
    if (debug()) std::cout << "IpFilterUpDownRule (response): " << response << std::endl;
    return true;
}
