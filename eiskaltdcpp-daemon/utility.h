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

#pragma once

//---------------------------------------------------------------------------
#include <string>
extern std::string LOCAL_PATH, PATH, sTitle, LOG_FILE;

void logging(bool d, bool s, bool b, const string& msg);
bool DirExist(char * sPath);
void Log(const std::string & sData);
std::string toString(short val);
bool splitMagnet(const std::string &, std::string &, int64_t &, std::string &);
