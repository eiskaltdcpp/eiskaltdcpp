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
#include "utility.h"
#ifndef _WIN32
#include <sys/stat.h>
#endif
//---------------------------------------------------------------------------

string LOCAL_PATH="", PATH = "", sTitle = "";

bool DirExist(char * sPath) {
#ifdef _WIN32
    DWORD code = GetFileAttributes(sPath);
    if(code != INVALID_FILE_ATTRIBUTES && code == FILE_ATTRIBUTE_DIRECTORY) {
#else
    struct stat st;
    if(stat(sPath, &st) == 0 && S_ISDIR(st.st_mode)) {
#endif
        return true;
    }

    return false;
}

void Log(const string & sData) {
#ifdef _WIN32
    FILE * fw = fopen((PATH + "\\logs\\daemon.log").c_str(), "a");
#else
    FILE * fw = fopen((PATH + "/Logs/daemon.log").c_str(), "a");
#endif

    if(fw == NULL) {
        return;
    }

    time_t acc_time;
    time(&acc_time);

    struct tm * acc_tm;
    acc_tm = localtime(&acc_time);

    char sBuf[64];
    strftime(sBuf, 64, "%d.%m.%Y %H:%M:%S", acc_tm);

    string sTmp = string(sBuf) + " - " + sData + "\n";

    fprintf(fw, "%s", sTmp.c_str());

    fclose(fw);
}

#include "dcpp/Util.h"
#include "extra/magnet.h"
bool splitMagnet(const string &magnet, string &name, int64_t &size, string &tth) {
    name = "Unknown";
    size = 0;
    tth = "Unknown";

    StringMap params;
    if (magnet::parseUri(magnet,params)) {
        tth=params["xt"];
        size = Util::toInt64(params["xl"]);
        name = params["dn"];
        return true;
    }
    return false;
}
