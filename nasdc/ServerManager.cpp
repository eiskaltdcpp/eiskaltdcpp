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
#include "dcpp/Util.h"
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
	fprintf(stdout,"либа остановлена\n");
	bServerRunning = false;
}
