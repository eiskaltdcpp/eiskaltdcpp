/*
* Copyright (C) 2010 ggrundik (http://code.google.com/u/ggrundik/)
* for EiskaltDC++ Project (http://code.google.com/p/eiskaltdc/)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#if !defined(PERFOLDER_LIMIT_H)
#define PERFOLDER_LIMIT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
#include <sys/time.h>
#endif

namespace dcpp {

class Identity;

struct TFolderSetting
{
  typedef TFolderSetting* Ptr;
  typedef std::list<Ptr> List;
  typedef List::iterator Iter;

  string m_folder;
  int m_minshare;
};

class CPerfolderLimit
{
  TFolderSetting::List m_limits;
public:
  CPerfolderLimit(string const *config_name=NULL);
  ~CPerfolderLimit();
  bool IsUserAllowed(string const& request, const UserPtr user, string *message=NULL);
  void RenewList(string const *config_name=NULL);
};

}

#endif
