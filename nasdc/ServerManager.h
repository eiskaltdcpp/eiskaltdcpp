/*
 * ServerManager.h
 *
 *  Created on: 17.08.2009
 *      Author: alex
 */

#ifndef SERVERMANAGER_H_
#define SERVERMANAGER_H_

class ServerThread;

void callBack(void* x, const string& a);
void ServerInitialize();
bool ServerStart();
void ServerStop();

extern bool bServerRunning, bServerTerminated, bIsRestart, bIsClose;
#ifdef _WIN32
	#ifdef _SERVICE
	    extern bool bService;
	#endif
#else
	extern bool bDaemon;
#endif

extern ServerThread *ServersS;

#endif /* SERVERMANAGER_H_ */
