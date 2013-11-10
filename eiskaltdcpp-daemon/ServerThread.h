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

#pragma once

#include "dcpp/ClientListener.h"
#include "dcpp/DirectoryListing.h"
#include "dcpp/LogManager.h"
#include "dcpp/QueueManagerListener.h"
#include "dcpp/SearchManager.h"
#include "dcpp/SearchManagerListener.h"
#include "dcpp/SearchResult.h"
#include "dcpp/Singleton.h"
#include "dcpp/ShareManager.h"
#include "dcpp/Socket.h"
#include "dcpp/TimerManager.h"
#include "dcpp/Thread.h"

using namespace dcpp;

class ServerThread :
        private TimerManagerListener,
        private QueueManagerListener,
        private LogManagerListener,
        private ClientListener,
        public SearchManagerListener,
        public Thread,
        public Singleton<ServerThread>
{

public:
    void Resume();
    void Close();
    void WaitFor();

    void connectClient(const string& address, const string& encoding);
    void disconnectClient(const string& address);
    void sendMessage(const string& hubUrl, const string& message);
    void listConnectedClients(string& listhubs,const string& separator);
    bool findHubInConnectedClients(const string& hub);
    bool sendPrivateMessage(const string& hub, const string& nick, const string& message);
    bool getFileList(const string& hub, const string& nick, bool match);
    void getChatPubFromClient(string& chat, const string& hub, const string& separator);
    bool sendSearchonHubs(const string& search, const int& mode, const int& sizemode, const int& sizetype, const double& size, const string& huburls);
    void returnSearchResults(vector<StringMap>& resultarray, const string& huburl);
    bool clearSearchResults(const string& huburl);
    void listShare(string& listshare, const string& sseparator);
    bool delDirFromShare(const string& sdirectory);
    bool renameDirInShare(const string& sdirectory, const string& svirtname);
    bool addDirInShare(const string& sdirectory, const string& svirtname);
    bool addInQueue(const string& sddir, const string& name, const int64_t& size, const string& tth);
    bool setPriorityQueueItem(const string& target, const unsigned int& priority);
    void getQueueParams(QueueItem* item, StringMap& params);
    void listQueueTargets(string& listqueue, const string& sseparator);
    void listQueue(unordered_map<string,StringMap>& listqueue);
    bool moveQueueItem(const string& source, const string& target);
    bool removeQueueItem(const string& target);
    void updatelistQueueTargets();
    void getItemSources(QueueItem* item, const string& separator, string& sources, unsigned int& online);
    void getItemSourcesbyTarget(const string& target, const string& separator, string& sources, unsigned int& online);
    void getHashStatus(string& target, int64_t& bytesLeft, size_t& filesLeft, string& status);
    bool pauseHash();
    void getMethodList(string& tmp);
    void matchAllList();
    void listHubsFullDesc(unordered_map<string,StringMap>& listhubs);
    void getHubUserList(string& userlist, const string& huburl, const string& separator);
    void getParamsUser(StringMap& params, Identity& id);
    void updateUser(const StringMap& params, Client* cl);
    void removeUser(const string& cid, Client* cl);
    bool getUserInfo(StringMap& userinfo, const string& nick, const string& huburl);
    void showLocalLists(string& l, const string& separator);
    bool getClientFileList(const string& filelist, string& ret);
    bool openFileList(const string& filelist);
    //void buildList(const string& filelist, const string& nick, DirectoryListing* listing, bool full);
    bool closeFileList(const string& filelist);
    void closeAllFileLists();
    void showOpenedLists(string& l, const string& separator);
    void lsDirInList(const string& directory, const string& filelist, unordered_map<string,StringMap>& ret);
    void lsDirInList(DirectoryListing::Directory *dir, unordered_map<string,StringMap>& ret);

private:
    friend class Singleton<ServerThread>;

    ServerThread();
    virtual ~ServerThread();

    virtual int run();
    void startSocket(bool changed);
    void autoConnect();
    void showPortsError(const std::string& port);
    bool disconnect_all();
    void parseSearchResult(SearchResultPtr result, StringMap &resultMap);
    string revertSeparator(const string &ps);
    typedef struct {
            deque<string> curchat;
            Client* curclient;
            SearchResultList cursearchresult;
            StringMap curuserlist;
    } CurHub;

    typedef unordered_map <unsigned int, string> QueueMap;
    typedef QueueMap::const_iterator QueueIter;
    QueueMap queuesMap;

    typedef unordered_map <string, CurHub> ClientMap;
    static ClientMap clientsMap;
    bool json_run;

    typedef unordered_map <string, DirectoryListing*> FilelistMap;
    FilelistMap listsMap;

    // TimerManagerListener
    virtual void on(TimerManagerListener::Second, uint64_t aTick) noexcept;

    // ClientListener
    virtual void on(Connecting, Client* cur) noexcept;
    virtual void on(Connected, Client* cur) noexcept;
    virtual void on(UserUpdated, Client* cur, const OnlineUser&) noexcept;
    virtual void on(UsersUpdated, Client* cur, const OnlineUserList&) noexcept;
    virtual void on(UserRemoved, Client* cur, const OnlineUser&) noexcept;
    virtual void on(Redirect, Client* cur, const string&) noexcept;
    virtual void on(Failed, Client* cur, const string&) noexcept;
    virtual void on(GetPassword, Client* cur) noexcept;
    virtual void on(HubUpdated, Client* cur) noexcept;
    virtual void on(StatusMessage, Client* cur, const string&, int = ClientListener::FLAG_NORMAL) noexcept;
    virtual void on(ClientListener::Message, Client*, const ChatMessage&) noexcept;
    virtual void on(NickTaken, Client* cur) noexcept;
    virtual void on(SearchFlood, Client* cur, const string&) noexcept;

    //SearchManagerListener
    virtual void on(SearchManagerListener::SR, const SearchResultPtr &result) noexcept;

    //QueueManagerListener
    //virtual void on(Added, QueueItem*) noexcept;
    //virtual void on(Finished, QueueItem*, const string&, int64_t) noexcept;
    //virtual void on(Removed, QueueItem*) noexcept;
    //virtual void on(Moved, QueueItem*, const string&) noexcept;

    int64_t lastUp;
    int64_t lastDown;
    uint64_t lastUpdate;

    std::unique_ptr<dcpp::Socket> sock;
    CriticalSection shutcs;
    static const unsigned int maxLines = 50;
};
