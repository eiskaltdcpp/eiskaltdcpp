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
#include <syslog.h>
#include <sys/stat.h>
#endif
#if defined(__MINGW32__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#include <boost/lexical_cast.hpp>
#define USE_BOOST_LEXICAL_CAST 1
#endif
//---------------------------------------------------------------------------

string LOCAL_PATH="", PATH = "", sTitle = "", LOG_FILE= "";

void logging(bool d, bool s, bool b, const string& msg) {
#ifndef _WIN32
    if (d) {
        if (s) {
            if (b) syslog(LOG_USER | LOG_INFO, "%s", msg.c_str());
            else  syslog(LOG_USER | LOG_ERR, "%s", msg.c_str());
        } else {
            Log(msg);
        }
    } else {
        printf("%s\n",msg.c_str());
    }
#else
    Log(msg);
#endif
}

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
    string tmp;
    if (LOG_FILE.empty())
#ifdef _WIN32
        tmp = PATH + "\\logs\\daemon.log";
#else
        tmp = PATH + "/Logs/daemon.log";
#endif
    else
        tmp = LOG_FILE;

    FILE * fw = fopen(tmp.c_str(), "a");
    if(!fw) return;

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

#include "extra/magnet.h"
bool splitMagnet(const string &magnet, string &name, int64_t &size, string &tth) {
    name = "Unknown";
    size = 0;
    tth = "Unknown";

    StringMap params;
    if (magnet::parseUri(magnet,params)) {
        tth=params["xt"];
#if defined(USE_BOOST_LEXICAL_CAST)
        size = boost::lexical_cast<long long>(params["xl"]);
#else
        size = stoll(params["xl"]);
#endif
        name = params["dn"];
        return true;
    }
    return false;
}
