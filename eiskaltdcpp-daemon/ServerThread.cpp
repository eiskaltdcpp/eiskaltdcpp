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

#include "dcpp/ClientManager.h"
#include "dcpp/Client.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/DownloadManager.h"
#include "dcpp/UploadManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/SearchManager.h"
#include "dcpp/ConnectivityManager.h"
#include "dcpp/ChatMessage.h"
#include "dcpp/Text.h"
#include "dcpp/StringTokenizer.h"
#include "dcpp/version.h"

#ifdef XMLRPC_DAEMON
#include "xmlrpcserver.h"
#endif

#ifdef JSONRPC_DAEMON
#include "json/jsonrpc-cpp/jsonrpc.h"
#include "jsonrpcmethods.h"
#endif

unsigned short int lport = 3121;
string lip = "127.0.0.1";
bool isVerbose = false;
string xmlrpcLog = "/tmp/eiskaltdcpp-daemon.xmlrpc.log";
string xmlrpcUriPath = "/eiskaltdcpp";

ServerThread::ClientMap ServerThread::clientsMap;
#ifdef JSONRPC_DAEMON
Json::Rpc::TcpServer * jsonserver;
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
    xmlrpc_c::methodPtr const showVersionMethodP(new showVersionMethod);
    xmlrpc_c::methodPtr const showRatioMethodP(new showRatioMethod);
    xmlrpc_c::methodPtr const setPriorityQueueItemMethodP(new setPriorityQueueItemMethod);
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
    xmlrpcRegistry.addMethod("show.version", showVersionMethodP);
    xmlrpcRegistry.addMethod("show.ratio", showRatioMethodP);
    xmlrpcRegistry.addMethod("queue.setpriority", setPriorityQueueItemMethodP);
    xmlrpcRegistry.setShutdown(new systemShutdownMethod);
    sock.create();
    sock.setSocketOpt(SO_REUSEADDR, 1);
    sock.bind(lport, lip);
#if defined(USE_XMLRPC_ABYSS)
    server = new xmlrpc_c::serverAbyss(xmlrpc_c::serverAbyss::constrOpt()
                                      .registryP(&xmlrpcRegistry)
                                      .socketFd(sock.sock)
                                      .logFileName(xmlrpcLog)
                                      .serverOwnsSignals(false)
                                      .uriPath(xmlrpcUriPath)
                                      );
    server->run();
#elif defined(USE_XMLRPC_PSTREAM)
    sock.listen();
    server = new xmlrpc_c::serverPstream(xmlrpc_c::serverPstream::constrOpt()
                                   .registryP(&xmlrpcRegistry)
                                   .socketFd(sock.sock)
                                  );
    server->runSerial();
#endif

#endif

#ifdef JSONRPC_DAEMON
    jsonserver = new Json::Rpc::TcpServer(lip, lport);
    JsonRpcMethods a;
    if (!networking::init())
        std::cerr << "JSONRPC: Networking initialization failed" << std::endl;
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::Print, std::string("print")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::MagnetAdd, std::string("magnet.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::StopDaemon, std::string("daemon.stop")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubAdd, std::string("hub.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubDel, std::string("hub.del")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubSay, std::string("hub.say")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::HubSayPM, std::string("hub.pm")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListHubs, std::string("hub.list")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::AddDirInShare, std::string("share.add")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::RenameDirInShare, std::string("share.rename")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::DelDirFromShare, std::string("share.del")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListShare, std::string("share.list")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::RefreshShare, std::string("share.refresh")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetFileList, std::string("list.download")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::GetChatPub, std::string("hub.getchat")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::SendSearch, std::string("search.send")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ReturnSearchResults, std::string("search.getresults")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ClearSearchResults, std::string("search.clear")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ShowVersion, std::string("show.version")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ShowRatio, std::string("show.ratio")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::SetPriorityQueueItem, std::string("queue.setpriority")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::MoveQueueItem, std::string("queue.move")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::RemoveQueueItem, std::string("queue.remove")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListQueueTargets, std::string("queue.listtargets")));
    jsonserver->AddMethod(new Json::Rpc::RpcMethod<JsonRpcMethods>(a, &JsonRpcMethods::ListQueue, std::string("queue.list")));

    if (!jsonserver->Bind())
        std::cout << "JSONRPC: Bind failed" << std::endl;
    if (!jsonserver->Listen())
        std::cout << "JSONRPC: Listen failed" << std::endl;
    jsonserver->SetEncapsulatedFormat(Json::Rpc::HTTP_POST);
    std::cout << "JSONRPC: Start JSON-RPC TCP server" << std::endl;
    json_run = true;
    while (json_run) {
        jsonserver->WaitMessage(1000);
    }
#endif

    return 0;
}

bool ServerThread::disconnect_all() {
    for (ClientIter i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        if (clientsMap[i->first].curclient != NULL)
            disconnectClient(i->first);
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
    json_run = false;
    std::cout << "JSONRPC: Stop JSON-RPC TCP server" << std::endl;
    jsonserver->Close();
    networking::cleanup();
    delete jsonserver;
#endif

    ConnectionManager::getInstance()->disconnect();
    disconnect_all();
}

void ServerThread::WaitFor() {
    join();
}

void ServerThread::autoConnect() {
    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();
    for (FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;
        if (entry->getConnect()) {
            string address = entry->getServer();
            string encoding = entry->getEncoding();
            connectClient(address, encoding);
        }
    }
}

void ServerThread::connectClient(const string& address, const string& encoding) {
    if (ClientManager::getInstance()->isConnected(address))
        printf("Already connected to %s\n", address.c_str());
    string tmp;
    ClientIter i = clientsMap.find(address);
    if (i != clientsMap.end())
        return;
    if (address.substr(0, 6) == "adc://" || address.substr(0, 7) == "adcs://")
        tmp = "UTF-8";
    else if (encoding.empty())
        tmp = Text::systemCharset;
    Client* client = ClientManager::getInstance()->getClient(address);
    client->setEncoding(tmp);
    client->addListener(this);
    client->connect();
}

void ServerThread::disconnectClient(const string& address) {
    ClientIter i = clientsMap.find(address);
    if (i != clientsMap.end() && clientsMap[i->first].curclient != NULL) {
        Client* cl = i->second.curclient;
        cl->removeListener(this);
        cl->disconnect(true);
        ClientManager::getInstance()->putClient(cl);
        clientsMap[i->first].curclient = NULL;
    }
}

void ServerThread::on(TimerManagerListener::Second, uint64_t aTick) noexcept {
    //int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);
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
}

void ServerThread::on(Connected, Client* cur) noexcept {
    ClientIter i = clientsMap.find(cur->getHubUrl());
    if (i == clientsMap.end()) {
        CurHub curhub;
        curhub.curclient = cur;
        clientsMap[cur->getHubUrl()] = curhub;
    } else if (i != clientsMap.end() && clientsMap[cur->getHubUrl()].curclient == NULL)
        clientsMap[cur->getHubUrl()].curclient = cur;

    if (isVerbose)
        cout << "Connecting to " << cur->getHubUrl() << "..." << endl;
}

void ServerThread::on(UserUpdated, Client*, const OnlineUserPtr& user) noexcept {
}

void ServerThread::on(UsersUpdated, Client*, const OnlineUserList& aList) noexcept {
}

void ServerThread::on(UserRemoved, Client*, const OnlineUserPtr& user) noexcept {
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
    ClientIter i = clientsMap.find(cur->getHubUrl());
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
            bool priv = FavoriteManager::getInstance()->isPrivate(hint);
            params["message"] = Text::fromUtf8(msg);
            params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(cid, hint, priv));
            params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(cid, hint, priv));
            params["userCID"] = cid.toBase32();
            params["userNI"] = ClientManager::getInstance()->getNicks(cid, hint, priv)[0];
            params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
            LOG(LogManager::PM, params);
        }
    } else {
        ClientIter it = clientsMap.find(cl->getHubUrl());
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
    if (result == NULL) {
        return;
    }
    for (ClientIter i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        if (clientsMap[i->first].curclient != NULL && i->first == result->getHubURL()) {
            clientsMap[i->first].cursearchresult.push_back(result);
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
    fprintf(stdout,
            "\n\t\tConnectivity Manager: Warning\n\n Unable to open port %s. "
            "Searching or file transfers will\n not work correctly "
            "until you change settings or turn off\n any application "
            "that might be using that port.\n\n",
            port.c_str());
    fflush(stdout);
}

void ServerThread::sendMessage(const string& hubUrl, const string& message) {
    ClientIter i = clientsMap.find(hubUrl);
    if (i != clientsMap.end() && clientsMap[i->first].curclient != NULL) {
        Client* client = i->second.curclient;
        if (client && !message.empty()) {
            bool thirdPerson = !message.compare(0, 3, "/me");
            //printf("%s\t%s\n'", message.c_str(), message.substr(4).c_str());
            client->hubMessage(thirdPerson ? message.substr(4) : message , thirdPerson);
        }
    }
}

void ServerThread::listConnectedClients(string& listhubs, const string& separator) {
    for (ClientIter i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        if (clientsMap[i->first].curclient != NULL) {
            listhubs.append(i->first);
            listhubs.append(separator);
        }
    }
}

bool ServerThread::findHubInConnectedClients(const string& hub) {
    ClientIter i = clientsMap.find(hub);
    return i != clientsMap.end();
}

string ServerThread::sendPrivateMessage(const string& hub, const string& nick, const string& message) {
    ClientIter i = clientsMap.find(hub);
    if (i != clientsMap.end() && clientsMap[i->first].curclient != NULL) {
        Client* client = i->second.curclient;
        if (client && !message.empty()) {
            bool thirdPerson = !message.compare(0, 3, "/me");
            UserPtr user = ClientManager::getInstance()->getUser(nick, hub);
            if (user && user->isOnline())
            {
                ClientManager::getInstance()->privateMessage(HintedUser(user, hub), thirdPerson ? message.substr(4) : message, thirdPerson);
                return "Private message sent to " + nick + " at " + hub;
            }
            else
            {
                return "User went offline at " + hub;
            }
        }
    }

    return "Huburl is invalid";
}

string ServerThread::getFileList_client(const string& hub, const string& nick, bool match) {
    string message = "";
    ClientIter i = clientsMap.find(hub);
    if (i != clientsMap.end() && clientsMap[i->first].curclient != NULL) {
        if (!nick.empty()) {
            try {
                //UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
                UserPtr user = ClientManager::getInstance()->getUser(nick, hub);
                if (user) {
                    const HintedUser hintedUser(user, i->first);
                    if (user == ClientManager::getInstance()->getMe()) {
                        // Don't download file list, open locally instead
                        //WulforManager::get()->getMainWindow()->openOwnList_client(TRUE);
                    } else if (match) {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);
                    } else {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
                    }
                } else {
                    message = _("User not found");
                }
            } catch (const Exception &e) {
                message = e.getError();
                LogManager::getInstance()->message(message);
            }
        }
    }

    return message;
}

void ServerThread::getChatPubFromClient(string& chat, const string& hub, const string& separator) {
    ClientIter it = clientsMap.find(hub);
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
        resultMap["Filename"] = revertSeparator(result->getFileName());
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
    for (string::iterator it = str.begin(); it != str.end(); ++it) {
#ifdef _WIN32
        if ((*it) == '/')
            (*it) = '\\';
#else
        if ((*it) == '\\')
            (*it) = '/';
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
        for (StringIter i = sl.getTokens().begin(); i != sl.getTokens().end(); ++i) {
            clients.push_back((*i));
        }
        if (clients.empty())
            return false;
    } else {
        ClientIter i = clientsMap.begin();
        while (i != clientsMap.end()) {
            clients.push_back(i->first); ++i;
        }
    }
    string ssearch;
    dcpp::TStringList searchlist = StringTokenizer<string>(search, ' ').getTokens();
    for (StringList::const_iterator si = searchlist.begin(); si != searchlist.end(); ++si)
        if ((*si)[0] != '-') ssearch += *si + ' ';
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
    if (llsize == 0)
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
    for (ClientIter i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        if (!huburl.empty() && i->first != huburl)
            continue;
        SearchResultList::const_iterator kk;
        for (kk = clientsMap[i->first].cursearchresult.begin(); kk != clientsMap[i->first].cursearchresult.end(); ++kk) {
            StringMap resultMap;
            parseSearchResult(*kk, resultMap);
            resultarray.push_back(resultMap);
        }
    }
}

bool ServerThread::clearSearchResults(const string& huburl) {
    for (ClientIter i = clientsMap.begin(); i != clientsMap.end(); ++i) {
        if (!huburl.empty() && i->first != huburl)
            continue;
        clientsMap[i->first].cursearchresult.clear();
        return true;
    }
    return false;
}

void ServerThread::listShare(string& listshare, const string& sseparator) {
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it) {
        listshare.append("\n");
        listshare.append(it->second + sseparator);
        listshare.append(it->first + sseparator);
        listshare.append(Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)) + sseparator);
        listshare.append("\n");
    }
}

bool ServerThread::delDirFromShare(const string& sdirectory) {
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it) {
        if (it->first.compare(sdirectory) == 0) {
            ShareManager::getInstance()->removeDirectory(it->second);
            ShareManager::getInstance()->refresh(true);
            return true;
        }
    }
    return false;
}

bool ServerThread::renameDirInShare(const string& sdirectory, const string& svirtname) {
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it) {
        if (it->second.compare(sdirectory) == 0) {
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

    if (sddir.empty())
        QueueManager::getInstance()->add(name, size, TTHValue(tth));
    else
        QueueManager::getInstance()->add(sddir + PATH_SEPARATOR_STR + name, size, TTHValue(tth));

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
        string *file;
        const QueueItem::StringMap& ll = QueueManager::getInstance()->lockQueue();

        for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it) {
            file = it->first;
            if (file->length() >= target.length() && file->substr(0, target.length()) == target)
                QueueManager::getInstance()->setPriority(*file, p);
        }
        QueueManager::getInstance()->unlockQueue();
    } else {
        QueueManager::getInstance()->setPriority(target, p);
    }
    return true;
}

void ServerThread::getQueueParams(QueueItem* item, StringMap& params) {
	string nick;
	int online = 0;

	params["Filename"] = item->getTargetFileName();
	params["Path"] = Util::getFilePath(item->getTarget());
	params["Target"] = item->getTarget();

	params["Users"] = "";
	for (QueueItem::SourceConstIter it = item->getSources().begin(); it != item->getSources().end(); ++it) {
		if (it->getUser().user->isOnline())
			++online;

		if (params["Users"].size() > 0)
			params["Users"] += ", ";

		nick = Util::toString(ClientManager::getInstance()->getNicks(it->getUser().user->getCID(), it->getUser().hint));
		params["Users"] += nick;
	}
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
	for (QueueItem::SourceConstIter it = item->getBadSources().begin(); it != item->getBadSources().end(); ++it) {
		nick = Util::toString(ClientManager::getInstance()->getNicks(it->getUser().user->getCID(), it->getUser().hint));

		if (!it->isSet(QueueItem::Source::FLAG_REMOVED)) {
			if (params["Errors"].size() > 0)
				params["Errors"] += ", ";
			params["Errors"] += nick + " (";

			if (it->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
				params["Errors"] += _("File not available");
			else if (it->isSet(QueueItem::Source::FLAG_PASSIVE))
				params["Errors"] += _("Passive user");
			else if (it->isSet(QueueItem::Source::FLAG_CRC_FAILED))
				params["Errors"] += _("CRC32 inconsistency (SFV-Check)");
			else if (it->isSet(QueueItem::Source::FLAG_BAD_TREE))
				params["Errors"] += _("Full tree does not match TTH root");
			else if (it->isSet(QueueItem::Source::FLAG_SLOW_SOURCE))
				params["Errors"] += _("Source too slow");
			else if (it->isSet(QueueItem::Source::FLAG_NO_TTHF))
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
    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

    for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it) {
        listqueue.append(*it->first);
        listqueue.append(sseparator);
    }
    QueueManager::getInstance()->unlockQueue();
}

void ServerThread::updatelistQueueTargets() {
    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();
    queuesMap.clear();
    QueueItem::StringMap::const_iterator it = ll.begin(); unsigned int i = 0;
    while (it != ll.end()) {
        queuesMap[i] = *it->first;
         ++it, ++i;
    }
    QueueManager::getInstance()->unlockQueue();
}

void ServerThread::on(Added, QueueItem* item) noexcept {
    queuesMap[queuesMap.size()+1] = item->getTarget();

}

void ServerThread::on(Finished, QueueItem*, const string&, int64_t) noexcept {

}

void ServerThread::on(Removed, QueueItem*) noexcept {

}

void ServerThread::on(Moved, QueueItem*, const string&) noexcept {

}

void ServerThread::listQueue(unordered_map<string,StringMap>& listqueue) {
    const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

    for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it) {
        StringMap sm;
        getQueueParams(it->second,sm);
        listqueue[*it->first] = sm;
    }
    QueueManager::getInstance()->unlockQueue();
}

bool ServerThread::moveQueueItem(const string& source, const string& target) {
    if (!source.empty() && !target.empty()) {
        if (target[target.length() - 1] == PATH_SEPARATOR) {
            // Can't modify QueueItem::StringMap in the loop, so we have to queue them.
            vector<string> targets;
            string *file;
            const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

            for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
            {
                file = it->first;
                if (file->length() >= source.length() && file->substr(0, source.length()) == source)
                    targets.push_back(*file);
            }
            QueueManager::getInstance()->unlockQueue();

            for (vector<string>::const_iterator it = targets.begin(); it != targets.end(); ++it)
                QueueManager::getInstance()->move(*it, target + it->substr(source.length()));
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

            for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it) {
                file = it->first;
                if (file->length() >= target.length() && file->substr(0, target.length()) == target)
                    targets.push_back(*file);
            }
            QueueManager::getInstance()->unlockQueue();

            for (vector<string>::const_iterator it = targets.begin(); it != targets.end(); ++it)
                QueueManager::getInstance()->remove(*it);
        } else {
            QueueManager::getInstance()->remove(target);
        }
        return true;
    }
    return false;
}

