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

//---------------------------------------------------------------------------
#include "stdafx.h"
#include "dcpp/DCPlusPlus.h"
//---------------------------------------------------------------------------
#include "utility.h"
#include "ServerThread.h"
//---------------------------------------------------------------------------
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

#include "dcpp/version.h"
#ifdef XMLRPC_DAEMON
#include "xmlrpcserver.h"
#endif

//#include "../dht/DHT.h"

ServerThread::ClientMap ServerThread::clientsMap;

//----------------------------------------------------------------------------
ServerThread::ServerThread() : bTerminated(false), lastUp(0), lastDown(0), lastUpdate(GET_TICK())
{

}
//---------------------------------------------------------------------------

ServerThread::~ServerThread() {
        join();
}
//---------------------------------------------------------------------------

void ServerThread::Resume() {
    start();
}
//---------------------------------------------------------------------------

int ServerThread::run()
{
    dcpp::TimerManager::getInstance()->start();
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);
    LogManager::getInstance()->addListener(this);
    //WebServerManager::getInstance()->addListener(this);

    try {
        File::ensureDirectory(SETTING(LOG_DIRECTORY));
    } catch (const FileException) {	}

    startSocket(true, 0);
    autoConnect();
#ifdef LUA_SCRIPT
    ScriptManager::getInstance()->load();
    if (BOOLSETTING(USE_LUA)) {
        // Start as late as possible, as we might (formatting.lua) need to examine settings
        string defaultluascript="startup.lua";
        ScriptManager::getInstance()->EvaluateFile(defaultluascript);
    }
#endif
#ifdef XMLRPC_DAEMON
    xmlrpc_c::methodPtr const sampleAddMethodP(new sampleAddMethod);
    xmlrpc_c::methodPtr const magnetAddMethodP(new magnetAddMethod);
    xmlrpc_c::methodPtr const stopDemonMethodP(new stopDemonMethod);
    xmlrpc_c::methodPtr const hubAddMethodP(new hubAddMethod);
    xmlrpc_c::methodPtr const hubDelMethodP(new hubDelMethod);
    xmlrpc_c::methodPtr const hubSayMethodP(new hubSayMethod);
    xmlrpc_c::methodPtr const listHubsMethodP(new listHubsMethod);
    xmlrpc_c::methodPtr const addDirInShareMethodP(new addDirInShareMethod);
    xmlrpc_c::methodPtr const renameDirInShareMethodP(new renameDirInShareMethod);
    xmlrpc_c::methodPtr const delDirFromShareMethodP(new delDirFromShareMethod);
    xmlrpc_c::methodPtr const listShareMethodP(new listShareMethod);
    xmlrpc_c::methodPtr const refreshShareMethodP(new refreshShareMethod);
    xmlrpc_c::methodPtr const getChatPubMethodP(new getChatPubMethod);
    xmlrpcRegistry.addMethod("sample.add", sampleAddMethodP);
    xmlrpcRegistry.addMethod("magnet.add", magnetAddMethodP);
    xmlrpcRegistry.addMethod("demon.stop", stopDemonMethodP);
    //xmlrpcRegistry.addMethod("hub.add", hubAddMethodP);
    xmlrpcRegistry.addMethod("hub.del", hubDelMethodP);
    xmlrpcRegistry.addMethod("hub.say", hubSayMethodP);
    xmlrpcRegistry.addMethod("hubs.list", listHubsMethodP);
    xmlrpcRegistry.addMethod("share.add", addDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.rename", renameDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.del", delDirFromShareMethodP);
    xmlrpcRegistry.addMethod("share.list", listShareMethodP);
    xmlrpcRegistry.addMethod("share.refresh", refreshShareMethodP);
    xmlrpcRegistry.addMethod("hub.retchat", getChatPubMethodP);
    //xmlrpc_c::xmlrpc_server_abyss_set_handlers()
    AbyssServer.run();
#endif

    return 0;
}
bool ServerThread::disconnect_all(){
    for(ClientIter i = clientsMap.begin() ; i != clientsMap.end() ; i++) {
        if (clientsMap[i->first].curclient != NULL)
            disconnectClient(i->first);
    }
    return true;
}
//---------------------------------------------------------------------------
void ServerThread::Close()
{
    //WebServerManager::getInstance()->removeListener(this);
    SearchManager::getInstance()->disconnect();

    LogManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
#ifdef XMLRPC_DAEMON
    AbyssServer.terminate();
#endif

    ConnectionManager::getInstance()->disconnect();
    disconnect_all();

    bTerminated = true;
}
//---------------------------------------------------------------------------

void ServerThread::WaitFor() {
    join();
}

//----------------------------------------------------------------------------

void ServerThread::autoConnect()
{
    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();
    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;
        if (entry->getConnect()) {
            address = entry->getServer();
            encoding = entry->getEncoding();
            connectClient(address,encoding);
        }
    }
}

void ServerThread::connectClient(string address, string encoding)
{
    ClientIter i = clientsMap.find(address);
    if(i != clientsMap.end())
        return;
    if (address.substr(0, 6) == "adc://" || address.substr(0, 7) == "adcs://")
        encoding = "UTF-8";
    else if (encoding.empty())
        encoding = Text::systemCharset;
    Lock l(shutcs);
    Client* client = ClientManager::getInstance()->getClient(address);
    client->setEncoding(encoding);
    client->addListener(this);
    client->connect();
}

void ServerThread::disconnectClient(string address){
    ClientIter i = clientsMap.find(address);
    if(i != clientsMap.end() && clientsMap[i->first].curclient != NULL) {
        Lock l(shutcs);
        Client* cl = i->second.curclient;
        cl->removeListener(this);
        cl->disconnect(true);
        ClientManager::getInstance()->putClient(cl);
        clientsMap[i->first].curclient=NULL;
    }
}
//----------------------------------------------------------------------------
void ServerThread::on(TimerManagerListener::Second, uint64_t aTick) throw()
{
    int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);
    int64_t updiff = Socket::getTotalUp() - lastUp;
    int64_t downdiff = Socket::getTotalDown() - lastDown;

    SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + updiff);
    SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downdiff);

    lastUpdate = aTick;
    lastUp = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();

    //if(SETTING(DISCONNECT_SPEED) < 1) {
        //SettingsManager::getInstance()->set(SettingsManager::DISCONNECT_SPEED, 1);
    //}
}

void ServerThread::on(Connecting, Client* cur) throw() {
    ClientIter i = clientsMap.find(cur->getHubUrl());
    if(i == clientsMap.end()) {
        CurHub curhub;
        curhub.curclient = cur;
        //curhub.curchat.push_back("test");
        clientsMap[cur->getHubUrl()] = curhub;
    }
    cout << "Connecting to " <<  cur->getHubUrl() << "..."<< "\n";
}

void ServerThread::on(Connected, Client* cur) throw() {
    cout << "Connect success to " <<  cur->getHubUrl() << "\n";
}

void ServerThread::on(UserUpdated, Client*, const OnlineUserPtr& user) throw() {

}

void ServerThread::on(UsersUpdated, Client*, const OnlineUserList& aList) throw() {

}

void ServerThread::on(UserRemoved, Client*, const OnlineUserPtr& user) throw() {

}

void ServerThread::on(Redirect, Client* cur, const string& line) throw() {
    cout <<  "Redirected to" << line << "\n";
}

void ServerThread::on(Failed, Client* cur, const string& line) throw() {
    cout <<  "Connect failed [ " << cur->getHubUrl() << " ] :"<< line << "\n";
}

void ServerThread::on(GetPassword, Client* cur) throw() {
    ClientIter i = clientsMap.find(cur->getHubUrl());
    if (i != clientsMap.end()) {
        string pass = cur->getPassword();
        if (!pass.empty())
            cur->password(pass);
    }
}

void ServerThread::on(HubUpdated, Client*) throw() {

}

void ServerThread::on(ClientListener::Message, Client *cl, const ChatMessage& message) throw()
{
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
        if(BOOLSETTING(LOG_MAIN_CHAT)) {
            params["message"] = Text::fromUtf8(msg);
            cl->getHubIdentity().getParams(params, "hub", false);
            params["hubURL"] = cl->getHubUrl();
            cl->getMyIdentity().getParams(params, "my", true);
            LOG(LogManager::CHAT, params);
        }
    }

    cout << cl->getHubUrl() << priv << ": [" << Util::getTimeString() << "] " << msg << "\n";
}

void ServerThread::on(StatusMessage, Client *cl, const string& line, int statusFlags) throw()
{
    string msg = line;

    if(BOOLSETTING(LOG_STATUS_MESSAGES)) {
        StringMap params;
        cl->getHubIdentity().getParams(params, "hub", false);
        params["hubURL"] = cl->getHubUrl();
        cl->getMyIdentity().getParams(params, "my", true);
        params["message"] = Text::fromUtf8(msg);
        LOG(LogManager::STATUS, params);
    }

    cout << cl->getHubUrl() << " [" << Util::getTimeString() << "] " << "*"<< msg<< "\n";
}

void ServerThread::on(NickTaken, Client*) throw() {

}

void ServerThread::on(SearchFlood, Client*, const string& line) throw() {

}

//void ServerThread::on(WebServerListener::Setup) throw() {
    ////webSock = WebServerManager::getInstance()->getServerSocket().getSock();
//}

//void ServerThread::on(WebServerListener::ShutdownPC, int action) throw() {

//}
void ServerThread::startSocket(bool onstart, int oldmode){
    if (onstart) {
        try {
            ConnectivityManager::getInstance()->setup(true, SettingsManager::INCOMING_DIRECT);
        } catch (const Exception& e) {
            showPortsError(e.getError());
        }
    } else {
        bool b = false;
        if (oldmode != SETTING(INCOMING_CONNECTIONS))
            b = true;
        try {
            ConnectivityManager::getInstance()->setup(b, oldmode);
        } catch (const Exception& e) {
            showPortsError(e.getError());
        }
    }
    ClientManager::getInstance()->infoUpdated();
}
void ServerThread::showPortsError(const string& port) {
    fprintf(stdout,
            "\n\t\tConnectivity Manager: Warning\n\n Unable to open %s port. "
            "Searching or file transfers will\n not work correctly "
            "until you change settings or turn off\n any application "
            "that might be using that port.\n\n",
            port.c_str());
    fflush(stdout);
}

void ServerThread::sendMessage(const string& hubUrl, const string& message) {
    ClientIter i = clientsMap.find(hubUrl);
    if(i != clientsMap.end() && clientsMap[i->first].curclient !=NULL) {
        Client* client = i->second.curclient;
        if (client && !message.empty()) {
            bool thirdPerson = !message.compare(0,3,"/me");
            //printf("%s\t%s\n'",message.c_str(),message.substr(4).c_str());
            client->hubMessage(thirdPerson ? message.substr(4) : message , thirdPerson);
        }
    }
}

void ServerThread::listConnectedClients(string& listhubs,const string& separator) {
    for(ClientIter i = clientsMap.begin() ; i != clientsMap.end() ; i++) {
        if (clientsMap[i->first].curclient !=NULL) {
            listhubs.append(i->first);
            listhubs.append(separator);
        }
    }
}

bool ServerThread::findHubInConnectedClients(const string& hub) {
    ClientIter i = clientsMap.find(hub);
    if(i != clientsMap.end())
        return true;
    return false;
}

bool ServerThread::sendPrivateMessage(const string& hub,const string& cid,const string& message) {
    ClientIter i = clientsMap.find(hub);
    if(i != clientsMap.end() && clientsMap[i->first].curclient !=NULL) {
        Client* client = i->second.curclient;
        if (client && !message.empty()) {
            bool thirdPerson = !message.compare(0,3,"/me");
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
            if (user && user->isOnline())
            {
                ClientManager::getInstance()->privateMessage(HintedUser(user, hub), thirdPerson ? message.substr(4) : message, thirdPerson);
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}

string ServerThread::getFileList_client(const string& hub, const string& cid, bool match) {
    string message;
    ClientIter i = clientsMap.find(hub);
    if(i != clientsMap.end() && clientsMap[i->first].curclient !=NULL) {
        if (!cid.empty()) {
            try {
                UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
                if (user) {
                    const HintedUser hintedUser(user, i->first);//NOTE: core 0.762
                    if (user == ClientManager::getInstance()->getMe()) {
                        // Don't download file list, open locally instead
                        //WulforManager::get()->getMainWindow()->openOwnList_client(TRUE);
                    }
                    else if (match) {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);//NOTE: core 0.762
                    }
                    else {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
                    }
                }
                else {
                    message = _("User not found");
                }
            }
            catch (const Exception &e) {
                message = e.getError();
                LogManager::getInstance()->message(message);
            }
        }
    }
    if (!message.empty()) {
        return message;
    }
}

void ServerThread::getChatPubFromClient(string& chat, const string& hub, const string& separator) {
    Lock l(shutcs);
    ClientIter it = clientsMap.find(hub);
    if (it != clientsMap.end()) {
        for (int i =0; i < it->second.curchat.size(); ++i) {
            chat += it->second.curchat.at(i);
            chat.append(separator);
            //chatsPubMap[hub].pop_front();
        }
        clientsMap[hub].curchat.clear();
    } else
        chat = "Huburl is invalid";
}
