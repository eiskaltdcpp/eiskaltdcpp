/*
 * ServerThread.h
 *
 *  Created on: 17.08.2009
 *      Author: alex
 */

#ifndef SERVERTHREAD_H_
#define SERVERTHREAD_H_

#include "dcpp/QueueManagerListener.h"
#include "dcpp/TimerManager.h"
#include "dcpp/LogManager.h"
#include "dcpp/ClientListener.h"
#include "dcpp/ShareManager.h"
//#include "dcpp/WebServerManager.h"

class ServerThread : private TimerManagerListener,
		private QueueManagerListener,
		private LogManagerListener,
		private ClientListener/*,
		private WebServerListener*/
{

public:
	ServerThread();
	~ServerThread();

	void startSocket(bool onstart, int oldmode);
	void autoConnect();
	void showPortsError(const std::string& port);

	void Resume();
	void Run();
	void Close();
	void WaitFor();

private:
    int server;
    unsigned int iSuspendTime;
    pthread_t threadId, threadIdxml;
    pthread_mutex_t mtxServerThread;

	bool bTerminated;

	socket_t webSock;
	Client* client;

	typedef unordered_map<string, Client*> ClientMap;
	typedef ClientMap::const_iterator ClientIter;
	static ClientMap clients;

	int64_t lastUp;
	int64_t lastDown;
	uint64_t lastUpdate;
/*
	int64_t diff;
	int64_t updiff;
	int64_t downdiff;
*/
	// TimerManagerListener
	void on(TimerManagerListener::Second, uint64_t aTick) throw();

	// ClientListener
	void on(Connecting, const Client* cur) throw();
	void on(Connected, const Client* cur) throw();
	void on(UserUpdated, const Client* cur, const OnlineUserPtr&) throw();
	void on(UsersUpdated, const Client* cur, const OnlineUserList&) throw();
	void on(UserRemoved, const Client* cur, const OnlineUserPtr&) throw();
	void on(Redirect, const Client* cur, const string&) throw();
	void on(Failed, const Client* cur, const string&) throw();
	void on(GetPassword, const Client* cur) throw();
	void on(HubUpdated, const Client* cur) throw();
	void on(StatusMessage, const Client* cur, const string&, int = ClientListener::FLAG_NORMAL) throw();
	void on(ClientListener::Message, const Client*, const ChatMessage&) throw();
	void on(NickTaken, const Client* cur) throw();
	void on(SearchFlood, const Client* cur, const string&) throw();

	// WebServerListener
	//void on(WebServerListener::Setup) throw();
	//void on(WebServerListener::ShutdownPC, int) throw();

};

#endif /* SERVERTHREAD_H_ */
