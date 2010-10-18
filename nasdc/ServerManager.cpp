/*
 * ServerManager.cpp
 *
 *  Created on: 17.08.2009
 *      Author: alex
 */

//---------------------------------------------------------------------------
#include "stdafx.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#ifdef USE_MINIUPNP
#include "extra/upnpc.h"
#include "dcpp/UPnPManager.h"
#endif
//---------------------------------------------------------------------------
#include "ServerManager.h"
#include "ServerThread.h"
//---------------------------------------------------------------------------

ServerThread *ServersS = NULL;
bool bServerRunning = false, bServerTerminated = false, bIsRestart = false, bIsClose = false;
#ifdef _WIN32
	#ifdef _SERVICE
	    bool bService = false;
	#endif
#else
	bool bDaemon = false;
#endif

void callBack(void* x, const string& a)
{
	cout << _("Loading: ") << a << endl;
}

void ServerInitialize()
{
	ServersS = NULL;
	bServerRunning = bIsRestart = bIsClose = false;
}

bool ServerStart()
{

	dcpp::startup(callBack, NULL);
#ifdef USE_MINIUPNP
    UPnPManager::getInstance()->addImplementation(new UPnPc());//NOTE: core 0.762
#endif
	ServersS = new ServerThread();

	if(ServersS == NULL)
		return false;

	ServersS->Resume();

	bServerRunning = true;

	return true;
}

void ServerStop()
{
	ServersS->Close();
	fprintf(stdout,"сервер стоп\n");
	fprintf(stdout,"ждём\n");
	ServersS->WaitFor();
	fprintf(stdout,"ожидание закончено\n");
	delete ServersS;

	ServersS = NULL;
	fprintf(stdout,"остановка либы\n");
	dcpp::shutdown();

	bServerRunning = false;
}
