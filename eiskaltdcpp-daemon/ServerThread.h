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

    void connectClient(const string&, const string&);
    void disconnectClient(const string&);
    bool sendMessage(const string&, const string&);
    void listConnectedClients(string&, const string&);
    bool findHubInConnectedClients(const string&);
    bool sendPrivateMessage(const string&, const string&, const string&);
    bool getFileList(const string&, const string&, bool match);
    void getChatPubFromClient(string&, const string&, const string&);
    bool sendSearchonHubs(const string&, const int&, const int&, const int&, const double&, const string&);
    void returnSearchResults(vector<StringMap>&, const string&);
    bool clearSearchResults(const string&);
    void listShare(string&, const string&);
    bool delDirFromShare(const string&);
    bool renameDirInShare(const string&, const string&);
    bool addDirInShare(const string&, const string&);
    bool addInQueue(const string&, const string&, const int64_t&, const string&);
    bool setPriorityQueueItem(const string&, const unsigned int&);
    void listQueueTargets(string&, const string&);
    void listQueue(unordered_map<string,StringMap>&);
    bool moveQueueItem(const string&, const string&);
    bool removeQueueItem(const string&);
    void getItemSourcesbyTarget(const string&, const string&, string&, unsigned int&);
    void getHashStatus(string&, int64_t&, size_t&, string&);
    bool pauseHash();
    void matchAllList();
    void listHubsFullDesc(unordered_map<string,StringMap>&);
    void getHubUserList(string&, const string&, const string&);
    bool getUserInfo(StringMap&, const string&, const string&);
    void showLocalLists(string&, const string&);
    bool getClientFileList(const string&, string&);
    bool openFileList(const string&);
    bool closeFileList(const string&);
    void closeAllFileLists();
    void showOpenedLists(string&, const string&);
    void lsDirInList(const string&, const string&, unordered_map<string,StringMap>&);
    bool downloadDirFromList(const string&, const string&, const string&);
    bool downloadFileFromList(const string&, const string&, const string&);
    void getItemDescbyTarget(const string&, StringMap&);
    void queueClear();
    bool settingsGetSet(string&, const string&, const string&);
    void ipfilterList(string&, const string&);
    void ipfilterOnOff(bool);
    void ipfilterPurgeRules(const string&);
    void ipfilterAddRules(const string&);
    void ipfilterUpDownRule(bool, const string&);
    bool configReload();

private:
    friend class Singleton<ServerThread>;

    ServerThread();
    virtual ~ServerThread();

    virtual int run();
    void startSocket(bool);
    void autoConnect();
    void showPortsError(const std::string&);
    bool disconnect_all();
    void parseSearchResult(SearchResultPtr, StringMap&);
    string revertSeparator(const string&);
    typedef struct {
            deque<string> chat;
            Client* client;
            SearchResultList searchresult;
            StringMap userlist;
    } HubDescribe;

    typedef unordered_map <unsigned int, string> QueueMap;
    typedef QueueMap::const_iterator QueueIter;
    QueueMap queuesMap;

    typedef unordered_map <string, HubDescribe> ClientMap;
    static ClientMap clientsMap;
    bool json_run;

    typedef unordered_map <string, DirectoryListing*> FilelistMap;
    FilelistMap listsMap;

    // TimerManagerListener
    virtual void on(TimerManagerListener::Second, uint64_t aTick) noexcept;

    // ClientListener
    virtual void on(Connecting, Client*) noexcept;
    virtual void on(Connected, Client*) noexcept;
    virtual void on(UserUpdated, Client*, const OnlineUser&) noexcept;
    virtual void on(UsersUpdated, Client*, const OnlineUserList&) noexcept;
    virtual void on(UserRemoved, Client*, const OnlineUser&) noexcept;
    virtual void on(Redirect, Client*, const string&) noexcept;
    virtual void on(Failed, Client*, const string&) noexcept;
    virtual void on(GetPassword, Client*) noexcept;
    virtual void on(HubUpdated, Client*) noexcept;
    virtual void on(StatusMessage, Client*, const string&, int = ClientListener::FLAG_NORMAL) noexcept;
    virtual void on(ClientListener::Message, Client*, const ChatMessage&) noexcept;
    virtual void on(NickTaken, Client*) noexcept;
    virtual void on(SearchFlood, Client*, const string&) noexcept;

    //SearchManagerListener
    virtual void on(SearchManagerListener::SR, const SearchResultPtr&) noexcept;

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

    void getQueueParams(QueueItem*, StringMap&);
    void getItemSources(QueueItem*, const string&, string&, unsigned int&);
    void getParamsUser(StringMap&, Identity&);
    void updateUser(const StringMap&, Client*);
    void removeUser(const string&, Client*);
    void lsDirInList(DirectoryListing::Directory*, unordered_map<string,StringMap>&);
    bool downloadDirFromList(DirectoryListing::Directory*, DirectoryListing*, const string&);
    bool downloadFileFromList(DirectoryListing::File*, DirectoryListing*, const string&);
};
