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

};

#endif
