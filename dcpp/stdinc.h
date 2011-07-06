/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_STDINC_H
#define DCPLUSPLUS_DCPP_STDINC_H

#include "compiler.h"

#ifndef BZ_NO_STDIO
#define BZ_NO_STDIO 1
#endif

#ifdef _MSC_VER

//disable the deprecated warnings for the CRT functions.
#define _CRT_SECURE_NO_DEPRECATE 1
#define _ATL_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1


typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

# ifndef CDECL
#  define CDECL _cdecl
# endif

#else // _MSC_VER

# ifndef CDECL
#  define CDECL
# endif

#endif // _MSC_VER

#ifdef _WIN32
#include "w.h"
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include <crtdbg.h>
#else
#include <assert.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <sys/types.h>
#include <time.h>
#include <locale.h>
#ifndef _MSC_VER
#include <stdint.h>
#endif

#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <deque>
#include <list>
#include <utility>
#include <functional>
#include <memory>
#include <numeric>
#include <limits>
#include <libintl.h>

#include <boost/format.hpp>
#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

#if defined(_MSC_VER) || defined(_STLPORT_VERSION)

#include <unordered_map>
#include <unordered_set>

#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)  // Using GNU C++ library?

#include <tr1/unordered_set>
#include <tr1/unordered_map>

#else
#error "Unknown STL, please configure accordingly"
#endif

namespace dcpp {
using namespace std;
using namespace std::tr1;
}

#endif // !defined(STDINC_H)
