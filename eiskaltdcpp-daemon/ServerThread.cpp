/***************************************************************************
*                                                                         *
*   Copyright (C) 2009-2010  Alexandr Tkachev <tka4ev@gmail.com>          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

// Created on: 17.08.2009

#include "stdafx.h"
#include "utility.h"
#include "ServerThread.h"

#include "dcpp/AdcHub.h"
#include "dcpp/ADLSearch.h"
#include "dcpp/ChatMessage.h"
#include "dcpp/ClientManager.h"
#include "dcpp/Client.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/ConnectivityManager.h"
#include "dcpp/DownloadManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/HashManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/SearchManager.h"
#include "dcpp/StringTokenizer.h"
#include "dcpp/Text.h"
#include "dcpp/UploadManager.h"
#include "dcpp/version.h"

#ifdef XMLRPC_DAEMON
#include "xmlrpcserver.h"
#endif

#ifdef JSONRPC_DAEMON
#include "json/jsonrpc-cpp/jsonrpc.h"
#include "jsonrpcmethods.h"
#endif

using std::cout;
using std::endl;
using namespace dcpp;

unsigned short int lport = 3121;
string lip = "127.0.0.1";
bool isVerbose = false;
bool isDebug = false;
string xmlrpcLog = "/tmp/eiskaltdcpp-daemon.xmlrpc.log";
string xmlrpcUriPath = "/eiskaltdcpp";

ServerThread::ClientMap ServerThread::clientsMap;
#ifdef JSONRPC_DAEMON
Json::Rpc::HTTPServer * jsonserver;
#endif

ServerThread::ServerThread() : lastUp(0), lastDown(0), lastUpdate(GET_TICK()) {
}

ServerThread::~ServerThread() {
    join();
}

void ServerThread::Resume() {
    start();
}

int ServerThread::run() {
    setThreadName("ServerThread");
    dcpp::TimerManager::getInstance()->start();
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);
    LogManager::getInstance()->addListener(this);
    SearchManager::getInstance()->addListener(this);

    try {
        File::ensureDirectory(SETTING(LOG_DIRECTORY));
    } catch (const FileException&) { }

    startSocket(false);
    autoConnect();
#ifdef LUA_SCRIPT
    ScriptManager::getInstance()->load();
    if (BOOLSETTING(USE_LUA)) {
        // Start as late as possible, as we might (formatting.lua) need to examine settings
        string defaultluascript = "startup.lua";
        ScriptManager::getInstance()->EvaluateFile(defaultluascript);
    }
#endif
#ifdef XMLRPC_DAEMON
    xmlrpc_c::methodPtr const magnetAddMethodP(new magnetAddMethod);
    xmlrpc_c::methodPtr const stopDaemonMethodP(new stopDaemonMethod);
    xmlrpc_c::methodPtr const hubAddMethodP(new hubAddMethod);
    xmlrpc_c::methodPtr const hubDelMethodP(new hubDelMethod);
    xmlrpc_c::methodPtr const hubSayMethodP(new hubSayMethod);
    xmlrpc_c::methodPtr const hubSayPrivateMethodP(new hubSayPrivateMethod);
    xmlrpc_c::methodPtr const listHubsMethodP(new listHubsMethod);
    xmlrpc_c::methodPtr const addDirInShareMethodP(new addDirInShareMethod);
    xmlrpc_c::methodPtr const renameDirInShareMethodP(new renameDirInShareMethod);
    xmlrpc_c::methodPtr const delDirFromShareMethodP(new delDirFromShareMethod);
    xmlrpc_c::methodPtr const listShareMethodP(new listShareMethod);
    xmlrpc_c::methodPtr const refreshShareMethodP(new refreshShareMethod);
    xmlrpc_c::methodPtr const getChatPubMethodP(new getChatPubMethod);
    xmlrpc_c::methodPtr const getFileListMethodP(new getFileListMethod);
    xmlrpc_c::methodPtr const sendSearchMethodP(new sendSearchMethod);
    xmlrpc_c::methodPtr const returnSearchResultsMethodP(new returnSearchResultsMethod);
    xmlrpc_c::methodPtr const clearSearchResultsMethodP(new clearSearchResultsMethod);
    xmlrpc_c::methodPtr const showVersionMethodP(new showVersionMethod);
    xmlrpc_c::methodPtr const showRatioMethodP(new showRatioMethod);
    xmlrpc_c::methodPtr const setPriorityQueueItemMethodP(new setPriorityQueueItemMethod);
    xmlrpc_c::methodPtr const moveQueueItemMethodP(new moveQueueItemMethod);
    xmlrpc_c::methodPtr const removeQueueItemMethodP(new removeQueueItemMethod);
    xmlrpc_c::methodPtr const listQueueTargetsMethodP(new listQueueTargetsMethod);
    xmlrpc_c::methodPtr const listQueueMethodP(new listQueueMethod);
    xmlrpc_c::methodPtr const getSourcesItemMethodP(new getSourcesItemMethod);
    xmlrpc_c::methodPtr const getHashStatusMethodP(new getHashStatusMethod);
    xmlrpc_c::methodPtr const pauseHashMethodP(new pauseHashMethod);
    xmlrpc_c::methodPtr const getMethodListMethodP(new getMethodListMethod);
    xmlrpc_c::methodPtr const listHubsFullDescMethodP(new listHubsFullDescMethod);
    xmlrpc_c::methodPtr const getHubUserListMethodP(new getHubUserListMethod);
    xmlrpc_c::methodPtr const getUserInfoMethodP(new getUserInfoMethod);
    xmlrpc_c::methodPtr const matchAllListMethodP(new matchAllListMethod);
    xmlrpcRegistry.addMethod("magnet.add", magnetAddMethodP);
    xmlrpcRegistry.addMethod("daemon.stop", stopDaemonMethodP);
    xmlrpcRegistry.addMethod("hub.add", hubAddMethodP);
    xmlrpcRegistry.addMethod("hub.del", hubDelMethodP);
    xmlrpcRegistry.addMethod("hub.say", hubSayMethodP);
    xmlrpcRegistry.addMethod("hub.pm", hubSayPrivateMethodP);
    xmlrpcRegistry.addMethod("hub.list", listHubsMethodP);
    xmlrpcRegistry.addMethod("hub.getchat", getChatPubMethodP);
    xmlrpcRegistry.addMethod("share.add", addDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.rename", renameDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.del", delDirFromShareMethodP);
    xmlrpcRegistry.addMethod("share.list", listShareMethodP);
    xmlrpcRegistry.addMethod("share.refresh", refreshShareMethodP);
    xmlrpcRegistry.addMethod("list.download", getFileListMethodP);
    xmlrpcRegistry.addMethod("search.send", sendSearchMethodP);
    xmlrpcRegistry.addMethod("search.getresults", returnSearchResultsMethodP);
    xmlrpcRegistry.addMethod("search.clear", clearSearchResultsMethodP);
    xmlrpcRegistry.addMethod("show.version", showVersionMethodP);
    xmlrpcRegistry.addMethod("show.ratio", showRatioMethodP);
    xmlrpcRegistry.addMethod("queue.setpriority", setPriorityQueueItemMethodP);
    xmlrpcRegistry.addMethod("queue.move", moveQueueItemMethodP);
    xmlrpcRegistry.addMethod("queue.remove", removeQueueItemMethodP);
    xmlrpcRegistry.addMethod("queue.listtargets", listQueueTargetsMethodP);
    xmlrpcRegistry.addMethod("queue.list", listQueueMethodP);
    xmlrpcRegistry.addMethod("queue.getsources", getSourcesItemMethodP);
    xmlrpcRegistry.addMethod("hash.status", getHashStatusMethodP);
    xmlrpcRegistry.addMethod("hash.pause", pauseHashMethodP);
    xmlrpcRegistry.addMethod("method.list", getMethodListMethodP);
    xmlrpcRegistry.addMethod("hub.listfulldesc", listHubsFullDescMethodP);
    xmlrpcRegistry.addMethod("hub.getusers", getHubUserListMethodP);
    xmlrpcRegistry.addMethod("hub.getuserinfo", getUserInfoMethodP);
    xmlrpcRegistry.addMethod("queue.matchlists", matchAllListMethodP);
    xmlrpcRegistry.setShutdown(new systemShutdownMethod);
    sock.reset(new Socket(Socket::TYPE_TCP));
    sock.setLocalIp4(lip);
    sock.listen(lport);
    server = new xmlrpc_c::serverAbyss(xmlrpc_c::serverAbyss::constrOpt()
                                      .registryP(&xmlrpcRegistry)
                                      .socketFd(sock.sock)
                                      .logFileName(xmlrpcLog)
                                      .serverOwnsSignals(false)
                                      .uriPath(xmlrpcUriPath)
                                      );
    server->run();
#endif

#ifdef JSONRPC_DAEMON
    jsonserver = new Json::Rpc::HTTPServer(lip, lport);
    JsonRpcMethods a;
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::MagnetAdd, std::string("magnet.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::StopDaemon, std::string("daemon.stop")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubAdd, std::string("hub.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubDel, std::string("hub.del")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubSay, std::string("hub.say")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubSayPM, std::string("hub.pm")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListHubs, std::string("hub.list")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListHubsFullDesc, std::string("hub.listfulldesc")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetChatPub, std::string("hub.getchat")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::AddDirInShare, std::string("share.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::RenameDirInShare, std::string("share.rename")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::DelDirFromShare, std::string("share.del")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListShare, std::string("share.list")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::RefreshShare, std::string("share.refresh")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetFileList, std::string("list.download")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::SendSearch, std::string("search.send")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ReturnSearchResults, std::string("search.getresults")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ClearSearchResults, std::string("search.clear")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ShowVersion, std::string("show.version")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ShowRatio, std::string("show.ratio")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::AddQueueItem, std::string("queue.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::SetPriorityQueueItem, std::string("queue.setpriority")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::MoveQueueItem, std::string("queue.move")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::RemoveQueueItem, std::string("queue.remove")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListQueueTargets, std::string("queue.listtargets")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListQueue, std::string("queue.list")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetSourcesItem, std::string("queue.getsources")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetHashStatus, std::string("hash.status")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::PauseHash, std::string("hash.pause")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetMethodList, std::string("methods.list")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::MatchAllLists, std::string("queue.matchlists")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetHubUserList, std::string("hub.getusers")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetUserInfo, std::string("hub.getuserinfo")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ShowLocalLists, std::string("list.local")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetClientFileList, std::string("list.get")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::OpenFileList, std::string("list.open")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::CloseFileList, std::string("list.close")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::CloseAllFileLists, std::string("list.closeall")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ShowOpenedLists, std::string("list.listopened")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::LsDirInList, std::string("list.lsdir")));

    if (!jsonserver->startPolling())
        std::cout << "JSONRPC: Start mongoose failed" << std::endl;
    else
        std::cout << "JSONRPC: Start mongoose" << std::endl;
#endif

    return 0;
}

bool ServerThread::disconnect_all() {
    for (auto& client : clientsMap) {
        if (clientsMap[client.first].curclient)
            disconnectClient(client.first);
    }
    return true;
}

void ServerThread::Close() {
    SearchManager::getInstance()->disconnect();

    LogManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    SearchManager::getInstance()->removeListener(this);
#ifdef XMLRPC_DAEMON
    server->terminate();
    delete server;
#endif
#ifdef JSONRPC_DAEMON
    jsonserver->stopPolling();
    std::cout << "JSONRPC: Stop mongoose" << std::endl;
    delete jsonserver;
#endif

    ConnectionManager::getInstance()->disconnect();
    disconnect_all();
}

void ServerThread::WaitFor() {
    join();
}

void ServerThread::autoConnect() {
    const FavoriteHubEntryList& favhublist = FavoriteManager::getInstance()->getFavoriteHubs();
    for (auto& hub : favhublist) {
        if (hub->getConnect()) {
            string address = hub->getServer();
            string encoding = hub->getEncoding();
            connectClient(address, encoding);
        }
    }
}

void ServerThread::connectClient(const string& address, const string& encoding) {
    if (ClientManager::getInstance()->isConnected(address)) {
        printf("Already connected to %s\n", address.c_str());
        return;
    }
    string tmp;
    if (address.substr(0, 6) == "adc://" || address.substr(0, 7) == "adcs://")
        tmp = "UTF-8";
    else if (encoding.empty())
        tmp = Text::systemCharset;
    Client* client = ClientManager::getInstance()->getClient(address);
    if (client) {
        client->setEncoding(tmp);
        client->addListener(this);
        client->connect();
    }
}

void ServerThread::disconnectClient(const string& address) {
    Client* cl = clientsMap[address].curclient;
    if (cl) {
        cl->removeListener(this);
        cl->disconnect(true);
        ClientManager::getInstance()->putClient(cl);
        clientsMap[address].curclient = NULL;
    }
}

void ServerThread::on(TimerManagerListener::Second, uint64_t aTick) noexcept {

    int64_t upDiff = Socket::getTotalUp() - lastUp;
    int64_t downDiff = Socket::getTotalDown() - lastDown;

    SettingsManager *SM = SettingsManager::getInstance();
    SM->set(SettingsManager::TOTAL_UPLOAD,   SETTING(TOTAL_UPLOAD)   + upDiff);
    SM->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downDiff);

    lastUpdate = aTick;
    lastUp   = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();
}

void ServerThread::on(Connecting, Client* cur) noexcept {
    if (isVerbose)
        cout << "Connecting to " <<  cur->getHubUrl() << "..."<< "\n";

    if (auto i = clientsMap.find(cur->getHubUrl()) == clientsMap.end()) {
        CurHub curhub;
        curhub.curclient = cur;
        clientsMap[cur->getHubUrl()] = curhub;
    } else if (!clientsMap[cur->getHubUrl()].curclient)
        clientsMap[cur->getHubUrl()].curclient = cur;
}

void ServerThread::on(Connected, Client* cur) noexcept {
    if (isVerbose)
        cout << "Connected to " << cur->getHubUrl() << "..." << endl;
}

void ServerThread::on(UserUpdated, Client* cur, const OnlineUser& user) noexcept {
    Identity id = user.getIdentity();

    if (!id.isHidden())
    {
        StringMap params;
        getParamsUser(params, id);
        if (isDebug) {printf ("HUB: %s == UserUpdated %s\n", cur->getHubUrl().c_str(), params["Nick"].c_str()); fflush (stdout);}
        updateUser(params, cur);
    }
}
void ServerThread::on(UsersUpdated, Client* cur, const OnlineUserList& list) noexcept {
    Identity id;

    for (auto& item : list)
    {
        id = item->getIdentity();
        if (!id.isHidden())
        {
            StringMap params;
            getParamsUser(params, id);
            if (isDebug) {printf ("HUB: %s == UsersUpdated %s\n", cur->getHubUrl().c_str(), params["Nick"].c_str()); fflush (stdout);}
            updateUser(params, cur);
        }
    }

}

void ServerThread::on(UserRemoved, Client* cur, const OnlineUser& user) noexcept {
    removeUser(user.getUser()->getCID().toBase32(), cur);
}

void ServerThread::on(Redirect, Client* cur, const string& line) noexcept {
    if (isVerbose)
        cout << "Redirected to " << line << endl;
}

void ServerThread::on(Failed, Client* cur, const string& line) noexcept {
    if (isVerbose)
        cout <<  "Connection failed [ " << cur->getHubUrl() << " ]: " << line << endl;
}

void ServerThread::on(GetPassword, Client* cur) noexcept {
    auto i = clientsMap.find(cur->getHubUrl());
    if (i != clientsMap.end()) {
        string pass = cur->getPassword();
        if (!pass.empty())
            cur->password(pass);
    }
}

void ServerThread::on(HubUpdated, Client*) noexcept {
}

void ServerThread::on(ClientListener::Message, Client *cl, const ChatMessage& message) noexcept {
    Lock l(shutcs);
    StringMap params;
    string msg = message.format();
    bool privatemsg = message.to && message.replyTo;
    string priv = privatemsg ? " Private from " + message.from->getIdentity().getNick() : " Public";
    if (privatemsg) {
        if (BOOLSETTING(LOG_PRIVATE_CHAT)) {
            const string& hint = cl->getHubUrl();
            const CID& cid = message.replyTo->getUser()->getCID();
            params["message"] = Text::fromUtf8(msg);
            params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(cid, hint));
            params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubUrls(cid, hint));
            params["userCID"] = cid.toBase32();
            params["userNI"] = ClientManager::getInstance()->getNicks(cid, hint)[0];
            params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
            LOG(LogManager::PM, params);
        }
    } else {
        auto it = clientsMap.find(cl->getHubUrl());
        if (it != clientsMap.end()) {
            if (it->second.curchat.size() >= maxLines)
                clientsMap[cl->getHubUrl()].curchat.pop_front();
            string tmp = "[" + Util::getTimeString() + "] " + msg;
            clientsMap[cl->getHubUrl()].curchat.push_back(tmp);
        }
        if (BOOLSETTING(LOG_MAIN_CHAT)) {
            params["message"] = Text::fromUtf8(msg);
            cl->getHubIdentity().getParams(params, "hub", false);
            params["hubURL"] = cl->getHubUrl();
            cl->getMyIdentity().getParams(params, "my", true);
            LOG(LogManager::CHAT, params);
        }
    }

    if (isVerbose)
        cout << cl->getHubUrl() << priv << ": [" << Util::getTimeString() << "] " << msg << endl;
}

void ServerThread::on(StatusMessage, Client *cl, const string& line, int statusFlags) noexcept {
    string msg = line;

    if (BOOLSETTING(LOG_STATUS_MESSAGES)) {
        StringMap params;
        cl->getHubIdentity().getParams(params, "hub", false);
        params["hubURL"] = cl->getHubUrl();
        cl->getMyIdentity().getParams(params, "my", true);
        params["message"] = Text::fromUtf8(msg);
        LOG(LogManager::STATUS, params);
    }

    if (isVerbose)
        cout << cl->getHubUrl() << " [" << Util::getTimeString() << "] *" << msg << endl;
}

void ServerThread::on(NickTaken, Client*) noexcept {
}

void ServerThread::on(SearchFlood, Client*, const string& line) noexcept {
}

void ServerThread::on(SearchManagerListener::SR, const SearchResultPtr& result) noexcept {
    if (!result) {
        return;
    }
    for (auto& client : clientsMap) {
        if (clientsMap[client.first].curclient && client.first == result->getHubURL()) {
            clientsMap[client.first].cursearchresult.push_back(result);
        }
    }
}

void ServerThread::startSocket(bool changed) {
    if (changed)
        ConnectivityManager::getInstance()->updateLast();
    try {
        ConnectivityManager::getInstance()->setup(true);
    } catch (const Exception& e) {
        showPortsError(e.getError());
    }
    ClientManager::getInstance()->infoUpdated();
}

void ServerThread::showPortsError(const string& port) {
    printf("\n\t\tConnectivity Manager: Warning\n\n Unable to open port %s. "
            "Searching or file transfers will\n not work correctly "
            "until you change settings or turn off\n any application "
            "that might be using that port.\n\n",
            port.c_str());
    fflush(stdout);
}

void ServerThread::sendMessage(const string& hubUrl, const string& message) {
    if (clientsMap[hubUrl].curclient) {
        Client* client = clientsMap[hubUrl].curclient;
        if (client && !message.empty()) {
            bool thirdPerson = !message.compare(0, 3, "/me");
            client->hubMessage(thirdPerson ? message.substr(4) : message , thirdPerson);
        }
    }
}

void ServerThread::listConnectedClients(string& listhubs, const string& separator) {
    for (auto& client : clientsMap) {
        if (clientsMap[client.first].curclient) {
            listhubs.append(client.first);
            listhubs.append(separator);
        }
    }
}

bool ServerThread::findHubInConnectedClients(const string& hub) {
    auto i = clientsMap.find(hub);
    return i != clientsMap.end();
}

bool ServerThread::sendPrivateMessage(const string& hub, const string& nick, const string& message) {
    Client* client = clientsMap[hub].curclient;
    if (client) {
        if (!message.empty()) {
            bool thirdPerson = !message.compare(0, 3, "/me");
            auto it = clientsMap[hub].curuserlist.find(nick);
            if (it == clientsMap[hub].curuserlist.end())
                return false;
            UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
            if (user && user->isOnline()) {
                ClientManager::getInstance()->privateMessage(HintedUser(user, hub), thirdPerson ? message.substr(4) : message, thirdPerson);
                return true;
            } else {
                return false;
            }
        }
    }

    return "Huburl is invalid";
}

bool ServerThread::getFileList(const string& hub, const string& nick, bool match) {
    if (clientsMap[hub].curclient) {
        if (!nick.empty()) {
            try {
                auto it = clientsMap[hub].curuserlist.find(nick);
                if (it == clientsMap[hub].curuserlist.end())
                    return false;
                UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
                if (user && user->isOnline()) {
                    const HintedUser hintedUser(user, hub);
                    if (user == ClientManager::getInstance()->getMe()) {
                        // Don't download file list, open locally instead
                        //WulforManager::get()->getMainWindow()->openOwnList_client(TRUE);
                    } else if (match) {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);
                    } else {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
                    }
                    return true;
                } else {
                    return false;
                }
            } catch (const Exception &e) {
                LogManager::getInstance()->message(e.getError());
            }
        }
    }

    return false;
}

void ServerThread::getChatPubFromClient(string& chat, const string& hub, const string& separator) {
    auto it = clientsMap.find(hub);
    if (it != clientsMap.end()) {
        for (unsigned int i = 0; i < it->second.curchat.size(); ++i) {
            chat += it->second.curchat.at(i);
            chat.append(separator);
        }
        clientsMap[hub].curchat.clear();
    } else
        chat = "Hub URL is invalid";
}

void ServerThread::parseSearchResult(SearchResultPtr result, StringMap &resultMap) {
    if (result->getType() == SearchResult::TYPE_FILE) {
        string file = revertSeparator(result->getFile());
        if (file.rfind('/') == tstring::npos) {
            resultMap["Filename"] = file;
        } else {
            resultMap["Filename"] = Util::getFileName(file);
            resultMap["Path"] = Util::getFilePath(file);
        }

        resultMap["File Order"] = "f" + resultMap["Filename"];
        resultMap["Type"] = Util::getFileExt(resultMap["Filename"]);
        if (!resultMap["Type"].empty() && resultMap["Type"][0] == '.')
            resultMap["Type"].erase(0, 1);
        resultMap["Size"] = Util::formatBytes(result->getSize());
        resultMap["Exact Size"] = Util::formatExactSize(result->getSize());
        resultMap["Icon"] = "icon-file";
        resultMap["Shared"] = Util::toString(ShareManager::getInstance()->isTTHShared(result->getTTH()));
    } else {
        string path = revertSeparator(result->getFile());
        resultMap["Filename"] = Util::getLastDir(path) + PATH_SEPARATOR;
        resultMap["Path"] = Util::getFilePath(path.substr(0, path.length() - 1)); // getFilePath just returns path unless we chop the last / off
        if (resultMap["Path"].find("/") == string::npos)
            resultMap["Path"] = "";
        resultMap["File Order"] = "d" + resultMap["Filename"];
        resultMap["Type"] = _("Directory");
        resultMap["Icon"] = "icon-directory";
        resultMap["Shared"] = "0";
        if (result->getSize() > 0) {
            resultMap["Size"] = Util::formatBytes(result->getSize());
            resultMap["Exact Size"] = Util::formatExactSize(result->getSize());
        }
    }

    resultMap["Nick"] = Util::toString(ClientManager::getInstance()->getNicks(result->getUser()->getCID(), result->getHubURL()));
    resultMap["CID"] = result->getUser()->getCID().toBase32();
    resultMap["Slots"] = result->getSlotString();
    resultMap["Connection"] = ClientManager::getInstance()->getConnection(result->getUser()->getCID());
    resultMap["Hub"] = result->getHubName().empty() ? result->getHubURL().c_str() : result->getHubName().c_str();
    resultMap["Hub URL"] = result->getHubURL();
    resultMap["IP"] = result->getIP();
    resultMap["Real Size"] = Util::toString(result->getSize());
    if (result->getType() == SearchResult::TYPE_FILE)
        resultMap["TTH"] = result->getTTH().toBase32();

    // assumption: total slots is never above 999
    resultMap["Slots Order"] = Util::toString(-1000 * result->getFreeSlots() - result->getSlots());
    resultMap["Free Slots"] = Util::toString(result->getFreeSlots());
}

string ServerThread::revertSeparator(const string& ps) {
    string str = ps;
    for (auto& ch : str) {
#ifdef _WIN32
        if (ch == '/')
            ch = '\\';
#else
        if (ch == '\\')
            ch = '/';
#endif //_WIN32
    }
    return str;
}

bool ServerThread::sendSearchonHubs(const string& search, const int& searchtype, const int& sizemode, const int& sizetype, const double& lsize, const string& huburls) {
    if (search.empty())
        return false;
    StringList clients;
    if (!huburls.empty()) {
        StringTokenizer<string> sl(huburls, ";");
        for (auto& client : sl.getTokens()) {
            clients.push_back(client);
        }
        if (clients.empty())
            return false;
    } else {
        for (auto& client : clientsMap) {
            clients.push_back(client.first);
        }
    }
    string ssearch;
    dcpp::TStringList searchlist = StringTokenizer<string>(search, ' ').getTokens();
    for (auto& item : searchlist) {
        if (item[0] != '-')
            ssearch += item + ' ';
    }

    ssearch = ssearch.substr(0, std::max(ssearch.size(), static_cast<string::size_type>(1)) - 1);

    double llsize = lsize;
    switch (sizetype) {
        case 1:
            llsize *= 1024.0;
            break;
        case 2:
            llsize *= 1024.0 * 1024.0;
            break;
        case 3:
            llsize *= 1024.0 * 1024.0 * 1024.0;
            break;
    }
    int64_t lllsize = static_cast<int64_t>(llsize);

    SearchManager::SizeModes mode((SearchManager::SizeModes)sizemode);
    if (!llsize)
        mode = SearchManager::SIZE_DONTCARE;
    int ftype = searchtype;
    string ftypeStr;
    if (ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_LAST) {
        ftypeStr = SearchManager::getInstance()->getTypeStr(ftype);
    } else {
        ftype = SearchManager::TYPE_ANY;
    }
    // Get ADC searchtype extensions if any is selected
    StringList exts;
    try {
        if (ftype == SearchManager::TYPE_ANY) {
            // Custom searchtype
            exts = SettingsManager::getInstance()->getExtensions(ftypeStr);
        } else if ((ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_DIRECTORY) || ftype == SearchManager::TYPE_CD_IMAGE) {
            // Predefined searchtype
            exts = SettingsManager::getInstance()->getExtensions(string(1, '0' + ftype));
        }
    } catch (const SearchTypeException&) {
        ftype = SearchManager::TYPE_ANY;
    }

    SearchManager::getInstance()->search(clients, ssearch, lllsize, (SearchManager::TypeModes)ftype, mode, "manual", exts);

    return true;
}

void ServerThread::returnSearchResults(vector<StringMap>& resultarray, const string& huburl) {
    for (auto& client : clientsMap) {
        if (!huburl.empty() && client.first != huburl)
            continue;
        for (auto& searchresult : clientsMap[client.first].cursearchresult) {
            StringMap resultMap;
            parseSearchResult(searchresult, resultMap);
            resultarray.push_back(resultMap);
        }
    }
}

bool ServerThread::clearSearchResults(const string& huburl) {
    for (auto& client : clientsMap) {
        if (!huburl.empty() && client.first != huburl)
            continue;
        clientsMap[client.first].cursearchresult.clear();
        return true;
    }
    return false;
}

void ServerThread::listShare(string& listshare, const string& sseparator) {
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (auto& dir : directories) {
        listshare.append("\n");
        listshare.append(dir.second + sseparator);
        listshare.append(dir.first + sseparator);
        listshare.append(Util::formatBytes(ShareManager::getInstance()->getShareSize(dir.second)) + sseparator);
        listshare.append("\n");
    }
}

bool ServerThread::delDirFromShare(const string& sdirectory) {
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (auto& dir : directories) {
        if (!dir.first.compare(sdirectory)) {
            ShareManager::getInstance()->removeDirectory(dir.second);
            ShareManager::getInstance()->refresh(true);
            return true;
        }
    }
    return false;
}

bool ServerThread::renameDirInShare(const string& sdirectory, const string& svirtname) {
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (auto& dir : directories) {
        if (!dir.second.compare(sdirectory)) {
            ShareManager::getInstance()->renameDirectory(sdirectory, svirtname);
            ShareManager::getInstance()->refresh(true);
            return true;
        }
    }
    return false;
}

bool ServerThread::addDirInShare(const string& sdirectory, const string& svirtname) {
    if (Util::fileExists(sdirectory.c_str())) {
        ShareManager::getInstance()->addDirectory(sdirectory, svirtname);
        ShareManager::getInstance()->refresh(true);
        return true;
    }
    return false;
}

bool ServerThread::addInQueue(const string& sddir, const string& name, const int64_t& size, const string& tth) {
    if (name.empty() && tth.empty())
        return false;

    try
    {
        if (sddir.empty())
            QueueManager::getInstance()->add(SETTING(DOWNLOAD_DIRECTORY) + PATH_SEPARATOR_STR + name, size, TTHValue(tth));
        else
            QueueManager::getInstance()->add(sddir + PATH_SEPARATOR_STR + name, size, TTHValue(tth));
    }
    catch (const Exception& e)
    {
        if (isDebug) std::cout << "ServerThread::addInQueue->(" << e.getError() << ")"<< std::endl;
        return false;
    }

    return true;
}

bool ServerThread::setPriorityQueueItem(const string& target, const unsigned int& priority) {
    if (target.empty())
        return false;

    QueueItem::Priority p;
    switch (priority) {
        case 0:  p = QueueItem::PAUSED;  break;
        case 1:  p = QueueItem::LOWEST;  break;
        case 2:  p = QueueItem::LOW;     break;
        case 3:  p = QueueItem::NORMAL;  break;
        case 4:  p = QueueItem::HIGH;    break;
        case 5:  p = QueueItem::HIGHEST; break;
        default: p = QueueItem::PAUSED;  break;
    }

    if (target[target.length() - 1] == PATH_SEPARATOR) {
        const QueueItem::StringMap& ll = QueueManager::getInstance()->lockQueue();
        string *file;
        for (auto& item : ll) {
            file = item.first;
            if (file->length() >= target.length() && file->substr(0, target.length()) == target)
                QueueManager::getInstance()->setPriority(*file, p);
        }
        QueueManager::getInstance()->unlockQueue();
    } else {
        QueueManager::getInstance()->setPriority(target, p);
    }
    return true;
}

void ServerThread::getItemSources(QueueItem* item, const string& separator, string& sources, unsigned int& online_tmp) {
    string nick;
    for (auto& s : item->getSources()) {
        if (s.getUser().user->isOnline())
            ++online_tmp;
        if (!sources.empty())
            sources += separator;
        nick = Util::toString(ClientManager::getInstance()->getNicks(s.getUser().user->getCID(), s.getUser().hint));
        sources += nick;
    }
}
void ServerThread::getItemSourcesbyTarget(const string& target, const string& separator, string& sources, unsigned int& online) {
    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();
    for (auto& item : ll) {
        if (target == *(item.first)) {
            getItemSources(item.second, separator, sources, online);
        }
    }
    QueueManager::getInstance()->unlockQueue();
}

void ServerThread::getQueueParams(QueueItem* item, StringMap& params) {
    string nick;
    unsigned int online = 0;

    params["Filename"] = item->getTargetFileName();
    params["Path"] = Util::getFilePath(item->getTarget());
    params["Target"] = item->getTarget();

    params["Users"] = "";

    getItemSources(item, ", ", params["Users"], online);

    if (params["Users"].empty())
        params["Users"] = _("No users");

    // Status
    if (item->isWaiting())
        params["Status"] = Util::toString(online) + _(" of ") + Util::toString(item->getSources().size()) + _(" user(s) online");
    else
        params["Status"] = _("Running...");

    // Size
    params["Size Sort"] = Util::toString(item->getSize());
    if (item->getSize() < 0) {
        params["Size"] = _("Unknown");
        params["Exact Size"] = _("Unknown");
    } else {
        params["Size"] = Util::formatBytes(item->getSize());
        params["Exact Size"] = Util::formatExactSize(item->getSize());
    }

    // Downloaded
    params["Downloaded Sort"] = Util::toString(item->getDownloadedBytes());
    if (item->getSize() > 0) {
        double percent = (double)item->getDownloadedBytes() * 100.0 / (double)item->getSize();
        params["Downloaded"] = Util::formatBytes(item->getDownloadedBytes()) + " (" + Util::toString(percent) + "%)";
    } else {
        params["Downloaded"] = _("0 B (0.00%)");
    }

    // Priority
    switch (item->getPriority()) {
        case QueueItem::PAUSED: params["Priority"] = _("Paused"); break;
        case QueueItem::LOWEST: params["Priority"] = _("Lowest"); break;
        case QueueItem::LOW: params["Priority"] = _("Low"); break;
        case QueueItem::HIGH: params["Priority"] = _("High"); break;
        case QueueItem::HIGHEST: params["Priority"] = _("Highest"); break;
        default: params["Priority"] = _("Normal"); break;
    }

    // Error
    params["Errors"] = "";
    for (auto& s : item->getBadSources()) {
        nick = Util::toString(ClientManager::getInstance()->getNicks(s.getUser().user->getCID(), s.getUser().hint));

        if (!s.isSet(QueueItem::Source::FLAG_REMOVED)) {
            if (params["Errors"].size() > 0)
                params["Errors"] += ", ";
            params["Errors"] += nick + " (";

            if (s.isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
                params["Errors"] += _("File not available");
            else if (s.isSet(QueueItem::Source::FLAG_PASSIVE))
                params["Errors"] += _("Passive user");
            else if (s.isSet(QueueItem::Source::FLAG_CRC_FAILED))
                params["Errors"] += _("CRC32 inconsistency (SFV-Check)");
            else if (s.isSet(QueueItem::Source::FLAG_BAD_TREE))
                params["Errors"] += _("Full tree does not match TTH root");
            else if (s.isSet(QueueItem::Source::FLAG_SLOW_SOURCE))
                params["Errors"] += _("Source too slow");
            else if (s.isSet(QueueItem::Source::FLAG_NO_TTHF))
                params["Errors"] += _("Remote client does not fully support TTH - cannot download");

            params["Errors"] += ")";
        }
    }
    if (params["Errors"].empty())
        params["Errors"] = _("No errors");

    // Added
    params["Added"] = Util::formatTime("%Y-%m-%d %H:%M", item->getAdded());

    // TTH
    params["TTH"] = item->getTTH().toBase32();
}

void ServerThread::listQueueTargets(string& listqueue, const string& sseparator) {
    string separator;
    if (sseparator.empty())
        separator = "\n";
    else
        separator = sseparator;
    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

    for (auto& item : ll) {
        listqueue += *(item.first);
        listqueue += separator;
    }
    QueueManager::getInstance()->unlockQueue();
}

//void ServerThread::updatelistQueueTargets() {
    //const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();
    //queuesMap.clear();
    //unsigned int i = 0;
    //for (auto& item : ll) {
        //queuesMap[i] = *(item.first);
         //++i;
    //}
    //QueueManager::getInstance()->unlockQueue();
//}

//void ServerThread::on(Added, QueueItem* item) noexcept {
    //queuesMap[queuesMap.size()+1] = item->getTarget();
//}

//void ServerThread::on(Finished, QueueItem*, const string&, int64_t) noexcept {

//}

//void ServerThread::on(Removed, QueueItem* item) noexcept {
    //for (auto& queue : queuesMap) {
        //if (queue.second == item->getTarget()) {
            //queue = queuesMap.erase(&queue);
        //} else
            //++it;
    //}
//}

//void ServerThread::on(Moved, QueueItem* item, const string& oldTarget) noexcept {
    //for (auto& t : queuesMap) {
        //if (t.second == oldTarget) {
            //t.second = item->getTarget();
        //}
    //}
//}

void ServerThread::listQueue(unordered_map<string,StringMap>& listqueue) {
    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();
    for (auto& item : ll) {
        StringMap sm;
        getQueueParams(item.second,sm);
        listqueue[*(item.first)] = sm;
    }
    QueueManager::getInstance()->unlockQueue();
}

void ServerThread::listHubsFullDesc(unordered_map<string,StringMap>& listhubs) {
    for (auto& client : clientsMap) {
        Client* cl = client.second.curclient;
        StringMap sm;
        sm["connected"] = cl->isConnected() ? "1"  : "0";
        sm["users"] = Util::toString(cl->getUserCount());
        sm["totalshare"] = Util::toString(cl->getAvailable());
        sm["totalshare preformatted"] = Util::formatBytes(cl->getAvailable());
        sm["hubname"] = cl->getHubName();
        sm["description"] = cl->getHubDescription();
        listhubs[client.first] = sm;
    }
}

bool ServerThread::moveQueueItem(const string& source, const string& target) {
    if (!source.empty() && !target.empty()) {
        if (target[target.length() - 1] == PATH_SEPARATOR) {
            // Can't modify QueueItem::StringMap in the loop, so we have to queue them.
            vector<string> targets;
            string *file;
            const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

            for (auto& item : ll) {
                file = item.first;
                if (file->length() >= source.length() && file->substr(0, source.length()) == source)
                    targets.push_back(*file);
            }
            QueueManager::getInstance()->unlockQueue();

            for (auto& item : targets) {
                QueueManager::getInstance()->move(item, target + item.substr(source.length()));
            }
        } else {
            QueueManager::getInstance()->move(source, target);
        }
        return true;
    }
    return false;
}

bool ServerThread::removeQueueItem(const string& target) {
    if (!target.empty()) {
        if (target[target.length() - 1] == PATH_SEPARATOR) {
            string *file;
            vector<string> targets;
            const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

            for (auto& item : ll) {
                file = item.first;
                if (file->length() >= target.length() && file->substr(0, target.length()) == target)
                    targets.push_back(*file);
            }
            QueueManager::getInstance()->unlockQueue();

            for (auto& item : targets) {
                QueueManager::getInstance()->remove(item);
            }
        } else {
            QueueManager::getInstance()->remove(target);
        }
        return true;
    }
    return false;
}

void ServerThread::getHashStatus(string& target, int64_t& bytesLeft, size_t& filesLeft, string& status) {
    HashManager::getInstance()->getStats(target, bytesLeft, filesLeft);
    status = HashManager::getInstance()->isHashingPaused() ? "pause" : bytesLeft > 0 ? "hashing" : "idle";
}

bool ServerThread::pauseHash() {
    bool paused = HashManager::getInstance()->isHashingPaused();
    if (paused)
        HashManager::getInstance()->resumeHashing();
    else
        HashManager::getInstance()->pauseHashing();
    return !paused;
}

void ServerThread::getMethodList(string& tmp) {
    tmp = "magnet.add|daemon.stop|hub.add|hub.del|hub.say|hub.pm|hub.list|hub.list|share.add|share.rename|share.del|share.list|share.refresh|list.download|hub.getchat|search.send|search.getresults|show.version|show.ratio|queue.setpriority|queue.move|queue.remove|queue.listtargets|queue.list|queue.getsources|hash.status|hash.pause|methods.list|queue.matchlists|hub.getusers|hub.getuserinfo|hub.listfulldesc";
}

void ServerThread::matchAllList() {
    QueueManager::getInstance()->matchAllListings();
}

void ServerThread::getHubUserList(string& userlist, const string& huburl, const string& separator) {
    string sep = separator.empty()? ";" : separator;
    if (clientsMap[huburl].curclient) {
        StringMap& ll = clientsMap[huburl].curuserlist;
        for (auto& user : ll) {
            userlist += user.first;
            userlist += sep;
        }
    }
}

void ServerThread::getParamsUser(StringMap& params, Identity& id)
{
    if(id.supports(AdcHub::ADCS_FEATURE) && id.supports(AdcHub::SEGA_FEATURE) &&
    ((id.supports(AdcHub::TCP4_FEATURE) && id.supports(AdcHub::UDP4_FEATURE)) || id.supports(AdcHub::NAT0_FEATURE)))
        params.insert(StringMap::value_type("Icon", "dc++"));
    else
        params.insert(StringMap::value_type("Icon", "normal"));

    if (id.getUser()->isSet(User::PASSIVE))
        params["Icon"] += "-fw";

    if (id.isOp())
    {
        params["Icon"] += "-op";
        params.insert(StringMap::value_type("Nick Order", "o" + id.getNick()));
    }
    else
    {
        params.insert(StringMap::value_type("Nick Order", "u" + id.getNick()));
    }

    params.insert(StringMap::value_type("Nick", id.getNick()));
    params.insert(StringMap::value_type("Shared", Util::toString(id.getBytesShared())));
    params.insert(StringMap::value_type("Description", id.getDescription()));
    params.insert(StringMap::value_type("Tag", id.getTag()));
    params.insert(StringMap::value_type("Connection", id.getConnection()));
    params.insert(StringMap::value_type("IP", id.getIp()));
    params.insert(StringMap::value_type("eMail", id.getEmail()));
    params.insert(StringMap::value_type("CID", id.getUser()->getCID().toBase32()));
}

void ServerThread::updateUser(const StringMap& params, Client* cl)
{
    const string &cid = params.at("CID");
    const string &Nick = params.at("Nick");
    StringMap & item = clientsMap[cl->getHubUrl()].curuserlist;
    for (auto& parameter : item) {
        if (parameter.second == cid) {
            if (parameter == *(item.end())) {
                item.insert(StringMap::value_type(Nick, cid));
                if (isDebug) {printf ("HUB: %s == Add user: %s\n", cl->getHubUrl().c_str(), Nick.c_str()); fflush (stdout);}
            } else if (parameter.first != Nick) {
                // User has changed nick, update userMap and remove the old Nick tag
                item.erase(parameter.first);
                item.insert(StringMap::value_type(Nick, cid));
                if (isDebug) {printf ("HUB: %s == Update user: %s\n", cl->getHubUrl().c_str(), Nick.c_str()); fflush (stdout);}
            }
            break;
        }

    }
}

void ServerThread::removeUser(const string& cid, Client* cl)
{
    StringMap & userlist = clientsMap[cl->getHubUrl()].curuserlist;
    for (auto& user : userlist) {
        if (user.second == cid) {
            if (user == *(userlist.end())) {
                if (isDebug) {printf ("HUB: %s == ERROR: no user with this cid (%s)\n", cl->getHubUrl().c_str(), cid.c_str()); fflush (stdout);}
            } else {
                userlist.erase(user.first);
                if (isDebug) {printf ("HUB: %s == Remove user: %s\n", cl->getHubUrl().c_str(), (user.first).c_str()); fflush (stdout);}
            }
            break;
        }
    }
}

bool ServerThread::getUserInfo(StringMap& userinfo, const string& nick, const string& huburl) {
    if (clientsMap[huburl].curclient) {
        auto it = clientsMap[huburl].curuserlist.find(nick);
        if (it == clientsMap[huburl].curuserlist.end())
            return false;
        UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
        if (user && user->isOnline()) {
            Identity id = ClientManager::getInstance()->getOnlineUserIdentity(user);
            if (!id.isHidden()) {
                getParamsUser(userinfo, id);
                return true;
            }
        }
    }
    return false;
}

void ServerThread::showLocalLists(string& l, const string& separator) {
    string tmp = separator.empty()? ";" : separator;
    StringList lists = File::findFiles(Util::getListPath(), "*.xml*");
    for (auto i : lists) {
        l += Util::getFileName(i);
        l += tmp;
    }
}

bool ServerThread::getClientFileList(const string& filelist, string& ret) {
    if (!Util::fileExists(Util::getListPath() + filelist))
        return false;
    File f(Util::getListPath() + filelist, File::READ, File::OPEN);
    size_t datalen = static_cast<uint32_t>(f.getSize());
    std::unique_ptr<uint8_t[]> buf(new uint8_t[datalen]);
    f.read((void*)buf.get(), datalen);
    ret = Encoder::toBase32(buf.get(), datalen);
    return true;
}

void ServerThread::buildList(const string& filelist, const string& nick, DirectoryListing* listing, bool full) {
    try
    {
        listing->getRoot()->setName(nick);
        //if (full) {
            listing->loadFile(Util::getListPath() + filelist);
            ADLSearchManager::getInstance()->matchListing(*(listing));
        //}
    }
    catch (const Exception &e)
    {
        printf("Unable to load file list: %s\n", e.getError().c_str());fflush(stdout);
        //ex = "Unable to load file list: " + e.getError();
    }
}
void ServerThread::openFileList(const string& filelist) {
    auto it = listsMap.find(filelist);
    if (it == listsMap.end()) {
        UserPtr u = DirectoryListing::getUserFromFilename(filelist);
        if (!u)
            return;
        //// Use the nick from the file name in case the user is offline and core only returns CID
        //auto nick = ClientManager::getNicks(user, "");
        //if (nick.find(user->getCID().toBase32(), 1) != string::npos)
        //{
            //string name = Util::getFileName(file);
            //string::size_type loc = name.find('.');
            //nick = name.substr(0, loc);
            //setLabel_gui(_("List: ") + nick);
        //}
        HintedUser user(u, Util::emptyString);
        DirectoryListing* dl = new DirectoryListing(user);
        buildList(filelist, "test", dl, false);
        listsMap.insert(FilelistMap::value_type(filelist,dl));
    }
}

bool ServerThread::closeFileList(const string& filelist) {
    return (1 <= listsMap.erase(filelist));
}

void ServerThread::closeAllFileLists() {
    listsMap.clear();
}

void ServerThread::showOpenedLists(string& l, const string& separator) {
    string sep = separator.empty()? ";" : separator;
    for (auto i : listsMap) {
        l += i.first;
        l += sep;
    }
}

void ServerThread::lsDirInList(const string& directory, const string& filelist, unordered_map<string,StringMap>& ret) {
    auto it = listsMap.find(filelist);
    if (it != listsMap.end()) {
        DirectoryListing::Directory *dir;
        if (directory.empty() || directory == "\\") {
            dir = it->second->getRoot();
        } else {
            dir = it->second->find(directory,it->second->getRoot());
        }
        lsDirInList(dir, ret);
    }
}

void ServerThread::lsDirInList(DirectoryListing::Directory *dir, unordered_map<string,StringMap>& ret) {
    if (dir == NULL)
        return;
    for (auto d : dir->directories) {
        StringMap map;
        map["Name"] = "d" + d->getName();
        map["Size"] = Util::toString(d->getSize());
        map["Size preformatted"] = Util::formatBytes(d->getSize());
        ret[d->getName()] = map;
    }
    for (auto file : dir->files) {
        StringMap map;
        map["Name"] = file->getName();
        map["Size"] = Util::toString(file->getSize());
        map["Size preformatted"] = Util::formatBytes(file->getSize());
        map["TTH"] = file->getTTH().toBase32();
        map["Bitrate"] = file->mediaInfo.bitrate ? (Util::toString(file->mediaInfo.bitrate)) : Util::emptyString;
        map["Resolution"] = !file->mediaInfo.video_info.empty() ? file->mediaInfo.resolution : Util::emptyString;
        map["Video"] = file->mediaInfo.video_info;
        map["Audio"] = file->mediaInfo.audio_info;
        ret[file->getName()] = map;
    }
}
