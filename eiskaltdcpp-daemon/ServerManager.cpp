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
#include "utility.h"
//---------------------------------------------------------------------------

ServerThread *ServersS = NULL;
bool bServerRunning = false, bServerTerminated = false, bIsRestart = false, bIsClose = false;
bool bDaemon = false;
#ifdef _WIN32
    #ifdef _SERVICE
        bool bService = false;
    #endif
#endif
bool bsyslog = false;

void callBack(void* x, const string& a)
{
    logging(bDaemon, bsyslog, true, "Loading: " + a);
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

    if(!ServersS)
        return false;

    ServersS->Resume();

    bServerRunning = true;

    return true;
}

void ServerStop()
{
    ServersS->Close();
    logging(bDaemon, bsyslog, true, "server stops");
    logging(bDaemon, bsyslog, true, "waiting");
    ServersS->WaitFor();
    logging(bDaemon, bsyslog, true, "waiting finished");
    ServersS->release();

    ServersS = NULL;
    logging(bDaemon, bsyslog, true, "library stops");
    dcpp::shutdown();
    logging(bDaemon, bsyslog, true, "library was stopped");
    bServerRunning = false;
}
