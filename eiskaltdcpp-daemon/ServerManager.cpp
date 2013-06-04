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
#include "dcpp/format.h"
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
bool bsyslog = false;

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
    ServerThread::newInstance();
    ServersS = ServerThread::getInstance();

    if(ServersS == NULL)
        return false;

    ServersS->Resume();

    bServerRunning = true;

    return true;
}

void ServerStop()
{
    ServersS->Close();
    fprintf(stdout,"server stops\n");
    fprintf(stdout,"waiting\n");
    ServersS->WaitFor();
    fprintf(stdout,"waiting finished\n");
    ServersS->release();

    ServersS = NULL;
    fprintf(stdout,"library stops\n");
    dcpp::shutdown();
    fprintf(stdout,"library was stopped\n");
    bServerRunning = false;
}
