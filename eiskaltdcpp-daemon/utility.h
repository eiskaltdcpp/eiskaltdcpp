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

#ifndef UTILITY_H_
#define UTILITY_H_
//---------------------------------------------------------------------------
#include <string>
extern std::string PATH, sTitle;

bool DirExist(char * sPath);
void Log(const std::string & sData);
std::string toString(short val);
bool splitMagnet(const std::string &, std::string &, int64_t &, std::string &);
#endif /* UTILITY_H_ */
