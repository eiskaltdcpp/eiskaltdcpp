/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_DCPLUSPLUS_H
#define DCPLUSPLUS_DCPP_DCPLUSPLUS_H

#ifdef _WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#ifdef _DEBUG

inline void CDECL debugTrace(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#if defined _WIN32 && defined _MSC_VER
    char buf[512];

    _vsnprintf(buf, sizeof(buf), format, args);
    OutputDebugStringA(buf);
#else // _WIN32
    vprintf(format, args);
#endif // _WIN32
    va_end(args);
}

#define dcdebug debugTrace
#ifdef _MSC_VER
#define dcassert(exp) \
do { if (!(exp)) { \
    dcdebug("Assertion hit in %s(%d): " #exp "\n", __FILE__, __LINE__); \
    if(1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #exp)) \
_CrtDbgBreak(); } } while(false)
#define dcasserta(exp) dcassert(0)
#else
#define dcasserta(exp) assert(exp)
#define dcassert(exp) assert(exp)
#endif
#define dcdrun(exp) exp
#else //_DEBUG
#ifdef _MSC_VER
#define dcasserta(exp) __assume(exp)
#else
#define dcasserta(exp)
#endif // _WIN32
#define dcdebug if (false) printf
#define dcassert(exp)
#define dcdrun(exp)
#endif //_DEBUG

// Make sure we're using the templates from algorithm...
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace dcpp {

typedef std::vector<string> StringList;
typedef StringList::iterator StringIter;
typedef StringList::const_iterator StringIterC;

typedef std::pair<string, string> StringPair;
typedef std::vector<StringPair> StringPairList;
typedef StringPairList::iterator StringPairIter;

typedef std::tr1::unordered_map<string, string> StringMap;
typedef StringMap::iterator StringMapIter;

typedef std::tr1::unordered_set<string> StringSet;
typedef StringSet::iterator StringSetIter;

typedef std::vector<wstring> WStringList;
typedef WStringList::iterator WStringIter;
typedef WStringList::const_iterator WStringIterC;

typedef std::pair<wstring, wstring> WStringPair;
typedef std::vector<WStringPair> WStringPairList;
typedef WStringPairList::iterator WStringPairIter;

typedef std::vector<uint8_t> ByteVector;

template<typename T>
boost::basic_format<T> dcpp_fmt(const T* t) {
    boost::basic_format<T> fmt;
    fmt.exceptions(boost::io::no_error_bits);
    fmt.parse(t);
    return fmt;
}

template<typename T>
boost::basic_format<T> dcpp_fmt(const std::basic_string<T>& t) {
    return dcpp_fmt(t.c_str());
}

#if defined(_MSC_VER) || defined(__MINGW32__)
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%I64d"
#define U64_FMT "%I64d"

#elif defined(SIZEOF_LONG) && SIZEOF_LONG == 8
#define _LL(x) x##l
#define _ULL(x) x##ul
#define I64_FMT "%ld"
#define U64_FMT "%ld"
#else
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%lld"
#define U64_FMT "%lld"
#endif

#ifdef _WIN32

# define PATH_SEPARATOR '\\'
# define PATH_SEPARATOR_STR "\\"

#else

# define PATH_SEPARATOR '/'
# define PATH_SEPARATOR_STR "/"

#endif


typedef unordered_map<wstring, wstring> WStringMap;
typedef WStringMap::iterator WStringMapIter;

#ifdef UNICODE

typedef wstring tstring;
typedef WStringList TStringList;
typedef WStringIter TStringIter;
typedef WStringIterC TStringIterC;

typedef WStringPair TStringPair;
typedef WStringPairIter TStringPairIter;
typedef WStringPairList TStringPairList;

typedef WStringMap TStringMap;
typedef WStringMapIter TStringMapIter;

#else

typedef string tstring;
typedef StringList TStringList;
typedef StringIter TStringIter;
typedef StringIterC TStringIterC;

typedef StringPair TStringPair;
typedef StringPairIter TStringPairIter;
typedef StringPairList TStringPairList;

typedef StringMap TStringMap;
typedef StringMapIter TStringMapIter;

#endif

extern void startup(void (*f)(void*, const string&), void* p);
extern void shutdown();

#ifdef BUILDING_DCPP
#define PACKAGE "libeiskaltdcpp"
#define LOCALEDIR LOCALE_DIR
#define _(String) dgettext(PACKAGE, String)
#define F_(String) dcpp_fmt(dgettext(PACKAGE, String))
#define FN_(String1,String2, N) dcpp_fmt(dngettext(PACKAGE, String1, String2, N))

#endif

} // namespace dcpp

#endif // !defined(DC_PLUS_PLUS_H)
