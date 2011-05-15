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

ServerThread::ClientMap ServerThread::clientsMap;

//----------------------------------------------------------------------------
ServerThread::ServerThread() : lastUp(0), lastDown(0), lastUpdate(GET_TICK())
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

    try {
        File::ensureDirectory(SETTING(LOG_DIRECTORY));
    } catch (const FileException) {	}

    startSocket(false);
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
    xmlrpc_c::methodPtr const magnetAddMethodP(new magnetAddMethod);
    xmlrpc_c::methodPtr const stopDemonMethodP(new stopDemonMethod);
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
    xmlrpc_c::methodPtr const listSearchStringsMethodP(new listSearchStringsMethod);
    xmlrpcRegistry.addMethod("magnet.add", magnetAddMethodP);
    xmlrpcRegistry.addMethod("demon.stop", stopDemonMethodP);
    xmlrpcRegistry.addMethod("hub.add", hubAddMethodP);
    xmlrpcRegistry.addMethod("hub.del", hubDelMethodP);
    xmlrpcRegistry.addMethod("hub.say", hubSayMethodP);
    xmlrpcRegistry.addMethod("hub.pm", hubSayPrivateMethodP);
    xmlrpcRegistry.addMethod("hub.list", listHubsMethodP);
    xmlrpcRegistry.addMethod("hub.retchat", getChatPubMethodP);
    xmlrpcRegistry.addMethod("share.add", addDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.rename", renameDirInShareMethodP);
    xmlrpcRegistry.addMethod("share.del", delDirFromShareMethodP);
    xmlrpcRegistry.addMethod("share.list", listShareMethodP);
    xmlrpcRegistry.addMethod("share.refresh", refreshShareMethodP);
    xmlrpcRegistry.addMethod("list.download", getFileListMethodP);
    xmlrpcRegistry.addMethod("search.send", sendSearchMethodP);
    xmlrpcRegistry.addMethod("search.list", listSearchStringsMethodP);
    //xmlrpcRegistry.addMethod("search.retresults", returnSearchResultsMethodP);
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
    SearchManager::getInstance()->disconnect();

    LogManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
#ifdef XMLRPC_DAEMON
    AbyssServer.terminate();
#endif

    ConnectionManager::getInstance()->disconnect();
    disconnect_all();
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
            string address = entry->getServer();
            string encoding = entry->getEncoding();
            connectClient(address,encoding);
        }
    }
}

void ServerThread::connectClient(const string& address, const string& encoding)
{
    Lock l(shutcs);
    if (ClientManager::getInstance()->isConnected(address))
        printf("Already connected to %s\n",address.c_str());
    string tmp;
    ClientIter i = clientsMap.find(address);
    if(i != clientsMap.end())
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

void ServerThread::disconnectClient(const string& address){
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
    //int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);
    int64_t updiff = Socket::getTotalUp() - lastUp;
    int64_t downdiff = Socket::getTotalDown() - lastDown;

    SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + updiff);
    SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + downdiff);

    lastUpdate = aTick;
    lastUp = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();

}

void ServerThread::on(Connecting, Client* cur) throw() {
    ClientIter i = clientsMap.find(cur->getHubUrl());
    if(i == clientsMap.end()) {
        CurHub curhub;
        curhub.curclient = cur;
        clientsMap[cur->getHubUrl()] = curhub;
    }
    else if (i != clientsMap.end() && clientsMap[cur->getHubUrl()].curclient == NULL)
        clientsMap[cur->getHubUrl()].curclient = cur;
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

void ServerThread::on(SearchManagerListener::SR, const SearchResultPtr &result) throw() {
    // Без пол-литра не разберёшься :D
    // Варианты как всю эту херню реализовать по-проще принимаются к рассмотрению...
    if (result == NULL) return;
    ClientIter i = clientsMap.begin();
    for ( ; i != clientsMap.end(); i++)
        if (result->getHubURL() == i->first && clientsMap[i->first].curclient !=NULL) break;
    if (i == clientsMap.end() && result->getHubURL() != i->first) return;

    unordered_map<string, SearchResultList>::const_iterator it;
    for (it = clientsMap[i->first].cursearchresult.begin(); it != clientsMap[i->first].cursearchresult.end(); ++it)
    {
        dcpp::TStringList searchlist = StringTokenizer<string>(it->first, ' ').getTokens();
        for (TStringIter itt = searchlist.begin(); itt != searchlist.end(); ++itt)
        {
            if ((*itt->begin() != '-' && Util::findSubString(result->getFile(), *itt) == (string::size_type)-1) ||
                (*itt->begin() == '-' && itt->size() != 1 && Util::findSubString(result->getFile(), itt->substr(1)) != (string::size_type)-1))
            {
                return;
            }
            else
                continue;
        }
        clientsMap[i->first].cursearchresult[it->first].push_back(result);
    }
}

void ServerThread::startSocket(bool changed){
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

string ServerThread::sendPrivateMessage(const string& hub,const string& nick, const string& message) {
    ClientIter i = clientsMap.find(hub);
    if(i != clientsMap.end() && clientsMap[i->first].curclient !=NULL) {
        Client* client = i->second.curclient;
        if (client && !message.empty()) {
            bool thirdPerson = !message.compare(0,3,"/me");
            UserPtr user = ClientManager::getInstance()->getUser(nick, hub);
            if (user && user->isOnline())
            {
                ClientManager::getInstance()->privateMessage(HintedUser(user, hub), thirdPerson ? message.substr(4) : message, thirdPerson);
                return "Private message sent to "+ nick + " at " + hub;
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
    if(i != clientsMap.end() && clientsMap[i->first].curclient !=NULL) {
        if (!nick.empty()) {
            try {
                //UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
                UserPtr user = ClientManager::getInstance()->getUser(nick, hub);
                if (user) {
                    const HintedUser hintedUser(user, i->first);
                    if (user == ClientManager::getInstance()->getMe()) {
                        // Don't download file list, open locally instead
                        //WulforManager::get()->getMainWindow()->openOwnList_client(TRUE);
                    }
                    else if (match) {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);
                    }
                    else {
                        QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
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

    return message;
}

void ServerThread::getChatPubFromClient(string& chat, const string& hub, const string& separator) {
    Lock l(shutcs);
    ClientIter it = clientsMap.find(hub);
    if (it != clientsMap.end()) {
        for (int i =0; i < it->second.curchat.size(); ++i) {
            chat += it->second.curchat.at(i);
            chat.append(separator);
        }
        clientsMap[hub].curchat.clear();
    } else
        chat = "Huburl is invalid";
}

void ServerThread::parseSearchResult_gui(SearchResultPtr result, StringMap &resultMap) {
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

string ServerThread::revertSeparator(const string &ps) {
    string str = ps;
    for (string::iterator it = str.begin(); it != str.end(); ++it) {
#ifdef _WIN32
        if ((*it) == '/')
            (*it) = '\\'
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
        if (clients.size() < 1)
            return false;
    } else {
        ClientIter i = clientsMap.begin();
        while (i != clientsMap.end()) {
            clients.push_back(i->first);++i;
        }
    }
    string ssearch;
    dcpp::TStringList searchlist = StringTokenizer<string>(search, ' ').getTokens();
    for (StringList::const_iterator si = searchlist.begin(); si != searchlist.end(); ++si)
        if ((*si)[0] != '-') ssearch += *si + ' ';
    ssearch = ssearch.substr(0, std::max(ssearch.size(), static_cast<string::size_type>(1)) - 1);

    double llsize = lsize;
    switch (sizetype)
    {
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
    if (ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_LAST)
    {
        ftypeStr = SearchManager::getInstance()->getTypeStr(ftype);
    }
    else
    {
        ftype = SearchManager::TYPE_ANY;
    }
    // Get ADC searchtype extensions if any is selected
    StringList exts;
    try
    {
        if (ftype == SearchManager::TYPE_ANY)
        {
            // Custom searchtype
            exts = SettingsManager::getInstance()->getExtensions(ftypeStr);
        }
        else if ((ftype > SearchManager::TYPE_ANY && ftype < SearchManager::TYPE_DIRECTORY) || ftype == SearchManager::TYPE_CD_IMAGE)
        {
            // Predefined searchtype
            exts = SettingsManager::getInstance()->getExtensions(string(1, '0' + ftype));
        }
    }
    catch (const SearchTypeException&)
    {
        ftype = SearchManager::TYPE_ANY;
    }
    for (StringIter it = clients.begin(); it != clients.end(); ++it){
        clientsMap[(*it)].cursearchlist.push_back(ssearch);
        clientsMap[(*it)].cursearchresult[ssearch].push_back(NULL);
    }

    SearchManager::getInstance()->search(clients, ssearch, lllsize, (SearchManager::TypeModes)ftype, mode, "manual", exts);

    return true;
}

void ServerThread::listSearchStrings(string& listsearchstrings, const string& search, const string& separator) {
    for(ClientIter i = clientsMap.begin() ; i != clientsMap.end() ; i++) {
        unordered_map<string, SearchResultList>::const_iterator it;
        for (it = clientsMap[i->first].cursearchresult.begin(); it != clientsMap[i->first].cursearchresult.end(); ++it)
        {
            listsearchstrings.append(it->first);
            listsearchstrings.append(separator);
        }
    }
}
