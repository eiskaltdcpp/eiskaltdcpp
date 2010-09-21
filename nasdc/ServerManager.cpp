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
#include "../upnp/upnpc.h"
#include "dcpp/UPnPManager.h"
#endif
#include "dcpp/ConnectivityManager.h"
#include "dcpp/ClientManager.h"
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
	startSocket(true, 0);
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
	ServersS->WaitFor();

	delete ServersS;

	ServersS = NULL;

	dcpp::shutdown();

	bServerRunning = false;
}

void startSocket(bool onstart, int oldmode){
    if (onstart) {
        try {
            ConnectivityManager::getInstance()->setup(true, SettingsManager::INCOMING_DIRECT);
        } catch (const Exception& e) {
            showPortsError(e.getError());
        }
    } else {
        try {
            ConnectivityManager::getInstance()->setup(true, oldmode);
        } catch (const Exception& e) {
            showPortsError(e.getError());
        }
    }
    ClientManager::getInstance()->infoUpdated();
}

void showPortsError(const string& port) {
    printf("Connectivity Manager: Warning\n\n Unable to open %d port. Searching or file transfers will\n not work correctly until you change settings or turn off\n any application that might be using that port.", port.c_str());
}
