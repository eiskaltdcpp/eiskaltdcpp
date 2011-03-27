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

#ifndef DCPLUSPLUS_DCPP_UTIL_H
#define DCPLUSPLUS_DCPP_UTIL_H

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#include "Text.h"

extern "C" int  _nl_msg_cat_cntr;

namespace dcpp {

template<typename T, bool flag> struct ReferenceSelector {
    typedef T ResultType;
};
template<typename T> struct ReferenceSelector<T,true> {
    typedef const T& ResultType;
};

template<typename T> class IsOfClassType {
public:
    template<typename U> static char check(int U::*);
    template<typename U> static float check(...);
public:
    enum { Result = sizeof(check<T>(0)) };
};

template<typename T> struct TypeTraits {
    typedef IsOfClassType<T> ClassType;
    typedef ReferenceSelector<T, ((ClassType::Result == 1) || (sizeof(T) > sizeof(char*)) ) > Selector;
    typedef typename Selector::ResultType ParameterType;
};

#define GETSET(type, name, name2) \
private: type name; \
public: TypeTraits<type>::ParameterType get##name2() const { return name; } \
    void set##name2(TypeTraits<type>::ParameterType a##name2) { name = a##name2; }

#define LIT(x) x, (sizeof(x)-1)

/** Evaluates op(pair<T1, T2>.first, compareTo) */
template<class T1, class T2, class op = equal_to<T1> >
class CompareFirst {
public:
    CompareFirst(const T1& compareTo) : a(compareTo) { }
    bool operator()(const pair<T1, T2>& p) { return op()(p.first, a); }
private:
    CompareFirst& operator=(const CompareFirst&);
    const T1& a;
};

/** Evaluates op(pair<T1, T2>.second, compareTo) */
template<class T1, class T2, class op = equal_to<T2> >
class CompareSecond {
public:
    CompareSecond(const T2& compareTo) : a(compareTo) { }
    bool operator()(const pair<T1, T2>& p) { return op()(p.second, a); }
private:
    CompareSecond& operator=(const CompareSecond&);
    const T2& a;
};

/**
 * Compares two values
 * @return -1 if v1 < v2, 0 if v1 == v2 and 1 if v1 > v2
 */
template<typename T1>
inline int compare(const T1& v1, const T1& v2) { return (v1 < v2) ? -1 : ((v1 == v2) ? 0 : 1); }

template<typename T1> inline double fraction(T1 a, T1 b) { return static_cast<double>(a) / b; }

class Util
{
public:
    static tstring emptyStringT;
    static string emptyString;
    static wstring emptyStringW;

    enum Paths {
        /** Global configuration */
        PATH_GLOBAL_CONFIG,
        /** Per-user configuration (queue, favorites, ...) */
        PATH_USER_CONFIG,
        /** Per-user local data (cache, temp files, ...) */
        PATH_USER_LOCAL,
        /** Various resources (help files etc) */
        PATH_RESOURCES,
        /** Translations */
        PATH_LOCALE,
        /** Default download location */
        PATH_DOWNLOADS,
        /** Default file list location */
        PATH_FILE_LISTS,
        /** Default hub list cache */
        PATH_HUB_LISTS,
        /** Where the notepad file is stored */
        PATH_NOTEPAD,
        PATH_LAST
    };

    typedef std::map<Util::Paths, std::string> PathsMap;
    static void initialize(PathsMap pathOverrides = PathsMap());

    /** Path of temporary storage */
    static string getTempPath() {
#ifdef _WIN32
        TCHAR buf[MAX_PATH + 1];
        DWORD x = GetTempPath(MAX_PATH, buf);
        return Text::fromT(tstring(buf, x));
#else
        return "/tmp/";
#endif
    }

    /** Path of configuration files */
    static const string& getPath(Paths path) { return paths[path]; }

    /** Migrate from pre-localmode config location */
    static void migrate(const string& file);

    /** Path of file lists */
    static string getListPath() { return getPath(PATH_FILE_LISTS); }
    /** Path of hub lists */
    static string getHubListsPath() { return getPath(PATH_HUB_LISTS); }
    /** Notepad filename */
    static string getNotepadFile() { return getPath(PATH_NOTEPAD); }

    static string translateError(int aError);

        static string getFilePath(const string& path, char separator = PATH_SEPARATOR) {
                string::size_type i = path.rfind(separator);
        return (i != string::npos) ? path.substr(0, i + 1) : path;
    }
        static string getFileName(const string& path, char separator = PATH_SEPARATOR) {
                string::size_type i = path.rfind(separator);
        return (i != string::npos) ? path.substr(i + 1) : path;
    }
    static string getFileExt(const string& path) {
        string::size_type i = path.rfind('.');
        return (i != string::npos) ? path.substr(i) : Util::emptyString;
    }
    static string getLastDir(const string& path, char separator = PATH_SEPARATOR) {
        string::size_type i = path.rfind(separator);
        if(i == string::npos)
            return Util::emptyString;
        string::size_type j = path.rfind(separator, i-1);
        return (j != string::npos) ? path.substr(j+1, i-j-1) : path;
    }

    static wstring getFilePath(const wstring& path) {
        wstring::size_type i = path.rfind(PATH_SEPARATOR);
        return (i != wstring::npos) ? path.substr(0, i + 1) : path;
    }
    static wstring getFileName(const wstring& path) {
        wstring::size_type i = path.rfind(PATH_SEPARATOR);
        return (i != wstring::npos) ? path.substr(i + 1) : path;
    }
    static wstring getFileExt(const wstring& path) {
        wstring::size_type i = path.rfind('.');
        return (i != wstring::npos) ? path.substr(i) : Util::emptyStringW;
    }
    static wstring getLastDir(const wstring& path) {
        wstring::size_type i = path.rfind(PATH_SEPARATOR);
        if(i == wstring::npos)
            return Util::emptyStringW;
        wstring::size_type j = path.rfind(PATH_SEPARATOR, i-1);
        return (j != wstring::npos) ? path.substr(j+1, i-j-1) : path;
    }

    template<typename string_t>
    static void replace(const string_t& search, const string_t& replacement, string_t& str) {
        typename string_t::size_type i = 0;
        while((i = str.find(search, i)) != string_t::npos) {
                str.replace(i, search.size(), replacement);
                i += replacement.size();
        }
    }
    template<typename string_t>
    static inline void replace(const typename string_t::value_type* search, const typename string_t::value_type* replacement, string_t& str) {
            replace(string_t(search), string_t(replacement), str);
    }

    static void decodeUrl(const string& aUrl, string& protocol, string& host, uint16_t& port, string& path, string& query, string& fragment);
    static map<string, string> decodeQuery(const string& query);
    static string validateFileName(string aFile);
    static bool checkExtension(const string& tmp);
    static string cleanPathChars(string aNick);
    static string addBrackets(const string& s);

    static string formatBytes(const string& aString) { return formatBytes(toInt64(aString)); }

    static string getShortTimeString(time_t t = time(NULL) );

    static string getTimeString();
    static string toAdcFile(const string& file);
    static string toNmdcFile(const string& file);

    static string formatBytes(int64_t aBytes);

    static string formatExactSize(int64_t aBytes);
    static time_t getStartTime() { return startTime; }
    static time_t getUpTime() { return time(NULL) - Util::getStartTime(); }
    static string formatSeconds(int64_t aSec) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%01lu:%02d:%02d", (unsigned long)(aSec / (60*60)), (int)((aSec / 60) % 60), (int)(aSec % 60));
        return buf;
    }

    static string formatParams(const string& msg, const StringMap& params, bool filter);
    static string formatTime(const string &msg, const time_t t);

    static inline int64_t roundDown(int64_t size, int64_t blockSize) {
        return ((size + blockSize / 2) / blockSize) * blockSize;
    }

    static inline int64_t roundUp(int64_t size, int64_t blockSize) {
        return ((size + blockSize - 1) / blockSize) * blockSize;
    }

    static inline int roundDown(int size, int blockSize) {
        return ((size + blockSize / 2) / blockSize) * blockSize;
    }

    static inline int roundUp(int size, int blockSize) {
        return ((size + blockSize - 1) / blockSize) * blockSize;
    }


    static int64_t toInt64(const string& aString) {
#ifdef _WIN32
        return _atoi64(aString.c_str());
#else
        return strtoll(aString.c_str(), (char **)NULL, 10);
#endif
    }

    static int toInt(const string& aString) {
        return atoi(aString.c_str());
    }
    static uint32_t toUInt32(const string& str) {
        return toUInt32(str.c_str());
    }
    static uint32_t toUInt32(const char* c) {
#ifdef _MSC_VER
        /*
        * MSVC's atoi returns INT_MIN/INT_MAX if out-of-range; hence, a number
        * between INT_MAX and UINT_MAX can't be converted back to uint32_t.
        */
        uint32_t ret = atoi(c);
        if(errno == ERANGE)
            return (uint32_t)_atoi64(c);
        return ret;
#else
        return (uint32_t)atoi(c);
#endif
    }

    static unsigned toUInt(const string& s) {
        if(s.empty())
                return 0;
        int ret = toInt(s);
        if(ret < 0)
                return 0;
        return ret;
    }

    static double toDouble(const string& aString) {
        // Work-around for atof and locales...
        lconv* lv = localeconv();
        string::size_type i = aString.find_last_of(".,");
        if(i != string::npos && aString[i] != lv->decimal_point[0]) {
            string tmp(aString);
            tmp[i] = lv->decimal_point[0];
            return atof(tmp.c_str());
        }
        return atof(aString.c_str());
    }

    static float toFloat(const string& aString) {
        return (float)toDouble(aString.c_str());
    }

    static string toString(short val) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", (int)val);
        return buf;
    }
    static string toString(unsigned short val) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%u", (unsigned int)val);
        return buf;
    }
    static string toString(int val) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", val);
        return buf;
    }
    static string toString(unsigned int val) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%u", val);
        return buf;
    }
    static string toString(long val) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%ld", val);
        return buf;
    }
    static string toString(unsigned long val) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%lu", val);
        return buf;
    }
    static string toString(long long val) {
        char buf[32];
        snprintf(buf, sizeof(buf), I64_FMT, val);
        return buf;
    }
    static string toString(unsigned long long val) {
        char buf[32];
        snprintf(buf, sizeof(buf), U64_FMT, val);
        return buf;
    }
    static string toString(double val) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%0.2f", val);
        return buf;
    }

    template<typename string_t>
    static string_t toString(const string_t& sep, const vector<string_t>& lst) {
        string_t ret;
        for(typename vector<string_t>::const_iterator i = lst.begin(), iend = lst.end(); i != iend; ++i) {
            ret += *i;
            if(i + 1 != iend)
                ret += sep;
    }
            return ret;
    }
    template<typename string_t>
    static inline string_t toString(const typename string_t::value_type* sep, const vector<string_t>& lst) {
        return toString(string_t(sep), lst);
    }
    static string toString(const string& sep, const StringList& lst);
    static string toString(const StringList& lst);

    static string toHexEscape(char val) {
        char buf[sizeof(int)*2+1+1];
        snprintf(buf, sizeof(buf), "%%%X", val&0x0FF);
        return buf;
    }
    static char fromHexEscape(const string aString) {
        unsigned int res = 0;
        sscanf(aString.c_str(), "%X", &res);
        return static_cast<char>(res);
    }

    template<typename T>
    static T& intersect(T& t1, const T& t2) {
        for(typename T::iterator i = t1.begin(); i != t1.end();) {
            if(find_if(t2.begin(), t2.end(), bind1st(equal_to<typename T::value_type>(), *i)) == t2.end())
                i = t1.erase(i);
            else
                ++i;
        }
        return t1;
    }

    static string encodeURI(const string& /*aString*/, bool reverse = false);
    static string getLocalIp();
    static bool isPrivateIp(string const& ip);
    static string formatAdditionalInfo(const std::string& aIp, bool sIp, bool sCC);
    /**
     * Case insensitive substring search.
     * @return First position found or string::npos
     */
    static string::size_type findSubString(const string& aString, const string& aSubString, string::size_type start = 0) throw();
    static wstring::size_type findSubString(const wstring& aString, const wstring& aSubString, wstring::size_type start = 0) throw();

    /* Utf-8 versions of strnicmp and stricmp, unicode char code order (!) */
    static int stricmp(const char* a, const char* b);
    static int strnicmp(const char* a, const char* b, size_t n);

    static int stricmp(const wchar_t* a, const wchar_t* b) {
        while(*a && Text::toLower(*a) == Text::toLower(*b))
            ++a, ++b;
        return ((int)Text::toLower(*a)) - ((int)Text::toLower(*b));
    }
    static int strnicmp(const wchar_t* a, const wchar_t* b, size_t n) {
        while(n && *a && Text::toLower(*a) == Text::toLower(*b))
            --n, ++a, ++b;

        return n == 0 ? 0 : ((int)Text::toLower(*a)) - ((int)Text::toLower(*b));
    }

    static int stricmp(const string& a, const string& b) { return stricmp(a.c_str(), b.c_str()); }
    static int strnicmp(const string& a, const string& b, size_t n) { return strnicmp(a.c_str(), b.c_str(), n); }
    static int stricmp(const wstring& a, const wstring& b) { return stricmp(a.c_str(), b.c_str()); }
    static int strnicmp(const wstring& a, const wstring& b, size_t n) { return strnicmp(a.c_str(), b.c_str(), n); }

    static string getIpCountry (string IP);

    static void setLang(const string lang) {
        if(!lang.empty())
#ifdef _WIN32
            putenv((char *)string("LANGUAGE=" + lang).c_str());
#else
            setenv ("LANGUAGE", lang.c_str(), 1);
#endif
        /* Make change known.  */
        {
        ++_nl_msg_cat_cntr;
        }
    }

    static bool getAway();
    static void setAway(bool aAway);
    static void switchAway();

    static bool getManualAway() { return manualAway; }
    static void setManualAway(bool aManualAway) { manualAway = aManualAway; }

    static string getAwayMessage();
    static void setAwayMessage(const string& aMsg) { awayMsg = aMsg; }
    static bool fileExists(const string &aFile);
    static uint32_t rand();
    static uint32_t rand(uint32_t high) { return rand() % high; }
    static uint32_t rand(uint32_t low, uint32_t high) { return rand(high-low) + low; }
    static double randd() { return ((double)rand()) / ((double)0xffffffff); }

private:
    /** In local mode, all config and temp files are kept in the same dir as the executable */
    static bool localMode;

    static string paths[PATH_LAST];

    static bool away;
    static bool manualAway;
    static string awayMsg;
    static time_t awayTime;
    static time_t startTime;
    typedef map<uint32_t, uint16_t> CountryList;
    typedef CountryList::iterator CountryIter;

    static CountryList countries;

    static void loadBootConfig();
};

/** Case insensitive hash function for strings */
struct noCaseStringHash {
    size_t operator()(const string* s) const {
        return operator()(*s);
    }

    size_t operator()(const string& s) const {
        size_t x = 0;
        const char* end = s.data() + s.size();
        for(const char* str = s.data(); str < end; ) {
            wchar_t c = 0;
            int n = Text::utf8ToWc(str, c);
            if(n < 0) {
                x = x*32 - x + '_';
                str += abs(n);
            } else {
                x = x*32 - x + (size_t)Text::toLower(c);
                str += n;
            }
        }
        return x;
    }

    size_t operator()(const wstring* s) const {
        return operator()(*s);
    }
    size_t operator()(const wstring& s) const {
        size_t x = 0;
        const wchar_t* y = s.data();
        wstring::size_type j = s.size();
        for(wstring::size_type i = 0; i < j; ++i) {
            x = x*31 + (size_t)Text::toLower(y[i]);
        }
        return x;
    }

    bool operator()(const string* a, const string* b) const {
        return Util::stricmp(*a, *b) < 0;
    }
    bool operator()(const string& a, const string& b) const {
        return Util::stricmp(a, b) < 0;
    }
    bool operator()(const wstring* a, const wstring* b) const {
        return Util::stricmp(*a, *b) < 0;
    }
    bool operator()(const wstring& a, const wstring& b) const {
        return Util::stricmp(a, b) < 0;
    }
};

/** Case insensitive string comparison */
struct noCaseStringEq {
    bool operator()(const string* a, const string* b) const {
        return a == b || Util::stricmp(*a, *b) == 0;
    }
    bool operator()(const string& a, const string& b) const {
        return Util::stricmp(a, b) == 0;
    }
    bool operator()(const wstring* a, const wstring* b) const {
        return a == b || Util::stricmp(*a, *b) == 0;
    }
    bool operator()(const wstring& a, const wstring& b) const {
        return Util::stricmp(a, b) == 0;
    }
};

/** Case insensitive string ordering */
struct noCaseStringLess {
    bool operator()(const string* a, const string* b) const {
        return Util::stricmp(*a, *b) < 0;
    }
    bool operator()(const string& a, const string& b) const {
        return Util::stricmp(a, b) < 0;
    }
    bool operator()(const wstring* a, const wstring* b) const {
        return Util::stricmp(*a, *b) < 0;
    }
    bool operator()(const wstring& a, const wstring& b) const {
        return Util::stricmp(a, b) < 0;
    }
};

} // namespace dcpp

#endif // !defined(UTIL_H)
