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

#include "stdinc.h"
#include "User.h"
#include "File.h"
#include "PerFolderLimit.h"
#include "ClientManager.h"
#include "LogManager.h"
#include "FavoriteManager.h"

namespace dcpp {

CPerfolderLimit::CPerfolderLimit(string const *config_name): m_limits()
{
  RenewList(config_name);
}

CPerfolderLimit::~CPerfolderLimit()
{
  while (!m_limits.empty())
  {
    delete m_limits.back();
    m_limits.pop_back();
  }
}

bool CPerfolderLimit::IsUserAllowed(string const& request, const UserPtr user, string *message)
{
  bool found=false;
  FavoriteManager *FM = FavoriteManager::getInstance();
  Identity id=ClientManager::getInstance()->getOnlineUserIdentity(user);
  int64_t user_share=id.getBytesShared();

  if ( NULL != message )
  {
    *message="";
    //*message=string("Limits check: user '")+id.getNick()+"' "+id.getIp()+" req: "+request+" : ";
  }

  if ( m_limits.empty() || id.isOp() || FM->isFavoriteUser(user) || FM->hasSlot(user))
  {
    return true;
  }

  TFolderSetting *pos = *m_limits.begin();
  int max_path_len = 0;
  for (TFolderSetting::Iter i=m_limits.begin(); i!=m_limits.end(); ++i)
  {
    TFolderSetting *s = *i;
    if ( pos->m_minshare<=s->m_minshare && 0==request.find(s->m_folder) )
    {
        if (s->m_folder.length() > max_path_len){
            max_path_len = s->m_folder.length();
            pos=s;
            found=true;
        }
    }
  }
  if (found)
  {
    if ( user_share>= (static_cast<int64_t>(pos->m_minshare))*1024*1024*1024 )
    {
      return true;
    }

    if ( NULL != message )
    {
      char buf_need[100], buf_user[100];
      sprintf(buf_need, "%i", pos->m_minshare);
      sprintf(buf_user, "%i", (int)(user_share/(1024*1024*1024)));
      *message=string("Too small share to download from ") + pos->m_folder + ": " + buf_user + "/" + buf_need + " GiB";
    }

    LogManager::getInstance()->message(string("Denied to send file '")+request+"' to "+id.getNick()+" ("+id.getIp()+"): "+*message);

    return false;
  }

  return true;
}

void CPerfolderLimit::RenewList(string const *config_name)
{
  string config_n;
  if ( NULL==config_name )
  {
    config_n=Util::getPath(Util::PATH_USER_CONFIG) + "PerFolderLimit.conf";
    config_name=&config_n;
  }

  while (!m_limits.empty())
  {
    delete m_limits.back();
    m_limits.pop_back();
  }

  string config;
  try
  {
    config = File(*config_name, File::READ, File::OPEN).read();
  }
  catch (...)
  {
    return;
  }

  config.push_back(0);
  config.push_back(0);

  for ( int i=0; 0!=config[i]; i++ )
  {
    string n;
    string f;

    while ( ' '==config[i] || 0x09==config[i] || 0x0D==config[i] || 0x0A==config[i] )
    {
      i++;
    }

    if ( '#' == config[i] )
    {
      while ( 0!=config[i] && 0x0D!=config[i] && 0x0A!=config[i])
      {
        i++;
      }
      continue;
    }

    while ( config[i]>='0' && config[i]<='9' )
    {
      n.push_back(config[i]);
      i++;
    }
    if ( ' '!=config[i] && 0x09!=config[i] )
    {
      while ( 0!=config[i] && 0x0D!=config[i] && 0x0A!=config[i])
      {
        i++;
      }
      continue;
    }

    while ( ' '==config[i] || 0x09==config[i] )
    {
      i++;
    }

    while ( 0!=config[i] && 0x0D!=config[i] && 0x0A!=config[i] )
    {
      f.push_back(config[i]);
      i++;
    }
    if (f.length()>0)
    {
      TFolderSetting::Ptr t=new TFolderSetting;
      t->m_folder=f;
      t->m_minshare=atoi(n.c_str());
      m_limits.push_back(t);
    }
  }
}
} // namespace dcpp
