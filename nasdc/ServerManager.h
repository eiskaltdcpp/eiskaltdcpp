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

#ifndef SERVERMANAGER_H_
#define SERVERMANAGER_H_

class ServerThread;
#include <string>
void callBack(void* x, const std::string& a);
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
