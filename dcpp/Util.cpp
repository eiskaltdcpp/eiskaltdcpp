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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "Util.h"
#include "File.h"

#include "StringTokenizer.h"
#include "ClientManager.h"
#include "SettingsManager.h"
#include "LogManager.h"
#include "version.h"
#include "File.h"
#include "SimpleXML.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <ctype.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#include <net/if.h>
#endif
#include <locale.h>

#include "CID.h"

#include "FastAlloc.h"

namespace dcpp {

#ifndef _DEBUG
FastCriticalSection FastAllocBase::cs;
#endif
time_t Util::startTime = time(NULL);
string Util::emptyString;
wstring Util::emptyStringW;
tstring Util::emptyStringT;

bool Util::away = false;
bool Util::manualAway = false;
string Util::awayMsg;
time_t Util::awayTime;

Util::CountryList Util::countries;

string Util::paths[Util::PATH_LAST];

bool Util::localMode = true;

static void sgenrand(unsigned long seed);

extern "C" void bz_internal_error(int errcode) {
    dcdebug("bzip2 internal error: %d\n", errcode);
}

#ifdef _WIN32

typedef HRESULT (WINAPI* _SHGetKnownFolderPath)(GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);

static string getDownloadsPath(const string& def) {
    // Try Vista downloads path
    static _SHGetKnownFolderPath getKnownFolderPath = 0;
    static HINSTANCE shell32 = NULL;

    if(!shell32) {
        shell32 = ::LoadLibrary(_T("Shell32.dll"));
        if(shell32)
        {
            getKnownFolderPath = (_SHGetKnownFolderPath)::GetProcAddress(shell32, "SHGetKnownFolderPath");

            if(getKnownFolderPath) {
                 PWSTR path = NULL;
                 // Defined in KnownFolders.h.
                 static GUID downloads = {0x374de290, 0x123f, 0x4565, {0x91, 0x64, 0x39, 0xc4, 0x92, 0x5e, 0x46, 0x7b}};
                 if(getKnownFolderPath(downloads, 0, NULL, &path) == S_OK) {
                     string ret = Text::fromT((const tstring&)path) + "\\";
                     ::CoTaskMemFree(path);
                     return ret;
                 }
            }
        }
    }

    return def + "Downloads\\";
}

#endif

void Util::initialize(PathsMap pathOverrides) {
    Text::initialize();

    sgenrand((unsigned long)time(NULL));

#ifdef _WIN32
    TCHAR buf[MAX_PATH+1] = { 0 };
    ::GetModuleFileName(NULL, buf, MAX_PATH);

    string exePath = Util::getFilePath(Text::fromT(buf));

    // Global config path is DC++ executable path...
    paths[PATH_GLOBAL_CONFIG] = exePath;

    paths[PATH_USER_CONFIG] = paths[PATH_GLOBAL_CONFIG];

    loadBootConfig();

    if(!File::isAbsolute(paths[PATH_USER_CONFIG])) {
        paths[PATH_USER_CONFIG] = paths[PATH_GLOBAL_CONFIG] + paths[PATH_USER_CONFIG];
    }

    paths[PATH_USER_CONFIG] = validateFileName(paths[PATH_USER_CONFIG]);

    if(localMode) {
        paths[PATH_USER_LOCAL] = paths[PATH_USER_CONFIG];
    } else {
        if(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK) {
            paths[PATH_USER_CONFIG] = Text::fromT(buf) + "\\EiskaltDC++\\";
        }

        paths[PATH_USER_LOCAL] = ::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf) == S_OK ? Text::fromT(buf) + "\\EiskaltDC++\\" : paths[PATH_USER_CONFIG];
    }

    paths[PATH_RESOURCES] = exePath;

    // libintl doesn't support wide path names so we use the short (8.3) format.
    // https://sourceforge.net/forum/message.php?msg_id=4882703
    tstring localePath_ = Text::toT(exePath) + _T("locale\\");
    memset(buf, 0, sizeof(buf));
    ::GetShortPathName(localePath_.c_str(), buf, sizeof(buf)/sizeof(TCHAR));

    paths[PATH_LOCALE] = Text::fromT(buf);
    paths[PATH_DOWNLOADS] = getDownloadsPath(paths[PATH_USER_CONFIG]);

#else
    paths[PATH_GLOBAL_CONFIG] = "/etc/";
    const char* home_ = getenv("HOME");
    string home = home_ ? Text::toUtf8(home_) : "/tmp/";

#ifdef FORCE_XDG
    const char *xdg_config_home_ = getenv("XDG_CONFIG_HOME");
    string xdg_config_home = xdg_config_home_? Text::toUtf8(xdg_config_home_) : (home+"/.config");
    paths[PATH_USER_CONFIG] = xdg_config_home + "/eiskaltdc++/";
    printf("$XDG_CONFIG_HOME: %s\n", paths[PATH_USER_CONFIG].c_str());
#else
    paths[PATH_USER_CONFIG] = home + "/.eiskaltdc++/";
#endif

    loadBootConfig();

    if(!File::isAbsolute(paths[PATH_USER_CONFIG])) {
        paths[PATH_USER_CONFIG] = paths[PATH_GLOBAL_CONFIG] + paths[PATH_USER_CONFIG];
    }

    paths[PATH_USER_CONFIG] = validateFileName(paths[PATH_USER_CONFIG]);

    if(localMode) {
        // @todo implement...
    }

    paths[PATH_USER_LOCAL] = paths[PATH_USER_CONFIG];

    paths[PATH_RESOURCES] = paths[PATH_USER_CONFIG];
    paths[PATH_LOCALE] = LOCALEDIR;

#ifdef FORCE_XDG
    const char *xdg_config_down_ = getenv("XDG_DOWNLOAD_DIR");
    string xdg_config_down = xdg_config_down_? (Text::toUtf8(xdg_config_down_)+"/") : (home+"/Downloads/");
    paths[PATH_DOWNLOADS] = xdg_config_down;
    printf("$XDG_DOWNLOAD_DIR: %s\n", paths[PATH_DOWNLOADS].c_str());
#else
    paths[PATH_DOWNLOADS] = home + "/Downloads/";
#endif

#endif

    paths[PATH_FILE_LISTS] = paths[PATH_USER_LOCAL] + "FileLists" PATH_SEPARATOR_STR;
    paths[PATH_HUB_LISTS] = paths[PATH_USER_LOCAL] + "HubLists" PATH_SEPARATOR_STR;
    paths[PATH_NOTEPAD] = paths[PATH_USER_CONFIG] + "Notepad.txt";

    // Override core generated paths
    for (PathsMap::const_iterator it = pathOverrides.begin(); it != pathOverrides.end(); ++it)
    {
        if (!it->second.empty())
            paths[it->first] = it->second;
    }

    File::ensureDirectory(paths[PATH_USER_CONFIG]);
    File::ensureDirectory(paths[PATH_USER_LOCAL]);

    try {
        // This product includes GeoIP data created by MaxMind, available from http://maxmind.com/
        // Updates at http://www.maxmind.com/app/geoip_country
#ifdef WIN32
        string file = getPath(PATH_RESOURCES) + "GeoIPCountryWhois.csv";
#else //WIN32
        string file_usr = getPath(PATH_RESOURCES) + "GeoIPCountryWhois.csv";
        string file_sys = string(_DATADIR) + PATH_SEPARATOR + "GeoIPCountryWhois.csv";
        string file = "";

        struct stat stFileInfo;
        if (stat(file_usr.c_str(),&stFileInfo) == 0)
            file = file_usr;
        else
            file = file_sys;
#endif //WIN32
        string data = File(file, File::READ, File::OPEN).read();

        const char* start = data.c_str();
        string::size_type linestart = 0;
        string::size_type comma1 = 0;
        string::size_type comma2 = 0;
        string::size_type comma3 = 0;
        string::size_type comma4 = 0;
        string::size_type lineend = 0;
        CountryIter last = countries.end();
        uint32_t startIP = 0;
        uint32_t endIP = 0, endIPprev = 0;

        for(;;) {
            comma1 = data.find(',', linestart);
            if(comma1 == string::npos) break;
            comma2 = data.find(',', comma1 + 1);
            if(comma2 == string::npos) break;
            comma3 = data.find(',', comma2 + 1);
            if(comma3 == string::npos) break;
            comma4 = data.find(',', comma3 + 1);
            if(comma4 == string::npos) break;
            lineend = data.find('\n', comma4);
            if(lineend == string::npos) break;

            startIP = Util::toUInt32(start + comma2 + 2);
            endIP = Util::toUInt32(start + comma3 + 2);
            uint16_t* country = (uint16_t*)(start + comma4 + 2);
            if((startIP-1) != endIPprev)
                last = countries.insert(last, make_pair((startIP-1), (uint16_t)16191));
            last = countries.insert(last, make_pair(endIP, *country));

            endIPprev = endIP;
            linestart = lineend + 1;
        }
    } catch(const FileException&) {
    }
}

void Util::migrate(const string& file) {
    if(localMode) {
        return;
    }

    if(File::getSize(file) != -1) {
        return;
    }

    string fname = getFileName(file);
    string old = paths[PATH_GLOBAL_CONFIG] + fname;
    if(File::getSize(old) == -1) {
        return;
    }

    File::renameFile(old, file);
}

void Util::loadBootConfig() {
    // Load boot settings
    try {
        SimpleXML boot;
        boot.fromXML(File(getPath(PATH_GLOBAL_CONFIG) + "dcppboot.xml", File::READ, File::OPEN).read());
        boot.stepIn();

        if(boot.findChild("LocalMode")) {
            localMode = boot.getChildData() != "0";
        }

        if(boot.findChild("ConfigPath")) {
            StringMap params;
#ifdef _WIN32
            // @todo load environment variables instead? would make it more useful on *nix
            TCHAR path[MAX_PATH];

            params["APPDATA"] = Text::fromT((::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path), path));
            params["PERSONAL"] = Text::fromT((::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path), path));
#endif
            paths[PATH_USER_CONFIG] = Util::formatParams(boot.getChildData(), params, false);
        }
    } catch(const Exception& ) {
        // Unable to load boot settings...
    }
}

#ifdef _WIN32
static const char badChars[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, '<', '>', '/', '"', '|', '?', '*', 0
};
#else

static const char badChars[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, '<', '>', '\\', '"', '|', '?', '*', 0
};
#endif

/**
 * Replaces all strange characters in a file with '_'
 * @todo Check for invalid names such as nul and aux...
 */
string Util::validateFileName(string tmp) {
    string::size_type i = 0;

    // First, eliminate forbidden chars
    while( (i = tmp.find_first_of(badChars, i)) != string::npos) {
        tmp[i] = '_';
        i++;
    }

    // Then, eliminate all ':' that are not the second letter ("c:\...")
    i = 0;
    while( (i = tmp.find(':', i)) != string::npos) {
        if(i == 1) {
            i++;
            continue;
        }
        tmp[i] = '_';
        i++;
    }

    // Remove the .\ that doesn't serve any purpose
    i = 0;
    while( (i = tmp.find("\\.\\", i)) != string::npos) {
        tmp.erase(i+1, 2);
    }
    i = 0;
    while( (i = tmp.find("/./", i)) != string::npos) {
        tmp.erase(i+1, 2);
    }

    // Remove any double \\ that are not at the beginning of the path...
    i = 1;
    while( (i = tmp.find("\\\\", i)) != string::npos) {
        tmp.erase(i+1, 1);
    }
    i = 1;
    while( (i = tmp.find("//", i)) != string::npos) {
        tmp.erase(i+1, 1);
    }

    // And last, but not least, the infamous ..\! ...
    i = 0;
    while( ((i = tmp.find("\\..\\", i)) != string::npos) ) {
        tmp[i + 1] = '_';
        tmp[i + 2] = '_';
        tmp[i + 3] = '_';
        i += 2;
    }
    i = 0;
    while( ((i = tmp.find("/../", i)) != string::npos) ) {
        tmp[i + 1] = '_';
        tmp[i + 2] = '_';
        tmp[i + 3] = '_';
        i += 2;
    }

    // Dots at the end of path names aren't popular
    i = 0;
    while( ((i = tmp.find(".\\", i)) != string::npos) ) {
        tmp[i] = '_';
        i += 1;
    }
    i = 0;
    while( ((i = tmp.find("./", i)) != string::npos) ) {
        tmp[i] = '_';
        i += 1;
    }


    return tmp;
}

bool Util::checkExtension(const string& tmp) {
        for(int i = 0; i < tmp.length(); i++) {
                if (tmp[i] < 0 || tmp[i] == 32 || tmp[i] == ':') {
                        return false;
                }
        }
        if(tmp.find_first_of(badChars, 0) != string::npos) {
                return false;
        }
        return true;
}

string Util::cleanPathChars(string aNick) {
    string::size_type i = 0;

    while( (i = aNick.find_first_of("/.\\", i)) != string::npos) {
        aNick[i] = '_';
    }
    return aNick;
}

string Util::addBrackets(const string& s) {
    return '<' + s + '>';
}

string Util::getShortTimeString(time_t t) {
    char buf[255];
    tm* _tm = localtime(&t);
    if(_tm == NULL) {
        strcpy(buf, "xx:xx");
    } else {
        strftime(buf, 254, SETTING(TIME_STAMPS_FORMAT).c_str(), _tm);
    }
    return Text::toUtf8(buf);
}

/**
 * Decodes a URL the best it can...
 * Default ports:
 * http:// -> port 80
 * dchub:// -> port 411
 */
void Util::decodeUrl(const string& url, string& protocol, string& host, uint16_t& port, string& path, string& query, string& fragment) {
        size_t fragmentEnd = url.size();
        size_t fragmentStart = url.rfind('#');

        size_t queryEnd;
        if(fragmentStart == string::npos) {
                queryEnd = fragmentStart = fragmentEnd;
        } else {
                dcdebug("f");
                queryEnd = fragmentStart;
                fragmentStart++;
        }

        size_t queryStart = url.rfind('?', queryEnd);
        size_t fileEnd;

        if(queryStart == string::npos) {
                fileEnd = queryStart = queryEnd;
        } else {
                dcdebug("q");
                fileEnd = queryStart;
                queryStart++;
        }

        size_t protoStart = 0;
        size_t protoEnd = url.find("://", protoStart);

        size_t authorityStart = protoEnd == string::npos ? protoStart : protoEnd + 3;
        size_t authorityEnd = url.find_first_of("/#?", authorityStart);

        size_t fileStart;
        if(authorityEnd == string::npos) {
                authorityEnd = fileStart = fileEnd;
        } else {
                dcdebug("a");
                fileStart = authorityEnd;
        }

        protocol = url.substr(protoStart, protoEnd - protoStart);

        if(authorityEnd > authorityStart) {
                dcdebug("x");
                size_t portStart = string::npos;
                if(url[authorityStart] == '[') {
                        // IPv6?
                        size_t hostEnd = url.find(']');
                        if(hostEnd == string::npos) {
                                return;
                        }

                        host = url.substr(authorityStart, hostEnd - authorityStart);
                        if(hostEnd + 1 < url.size() && url[hostEnd + 1] == ':') {
                                portStart = hostEnd + 1;
                        }
                } else {
                        size_t hostEnd;
                        portStart = url.find(':', authorityStart);
                        if(portStart != string::npos && portStart > authorityEnd) {
                                portStart = string::npos;
                        }

                        if(portStart == string::npos) {
                                hostEnd = authorityEnd;
                        } else {
                                hostEnd = portStart;
                                portStart++;
                        }

                        dcdebug("h");
                        host = url.substr(authorityStart, hostEnd - authorityStart);
                }

                if(portStart == string::npos) {
                        if(protocol == "http") {
                                port = 80;
                        } else if(protocol == "https") {
                                port = 443;
                        } else if(protocol == "dchub") {
                                port = 411;
                        }
                } else {
                        dcdebug("p");
                        port = static_cast<uint16_t>(Util::toInt(url.substr(portStart, authorityEnd - portStart)));
                }
        }

        dcdebug("\n");
        path = url.substr(fileStart, fileEnd - fileStart);
        query = url.substr(queryStart, queryEnd - queryStart);
        fragment = url.substr(fragmentStart, fragmentStart);
}

map<string, string> Util::decodeQuery(const string& query) {
        map<string, string> ret;
        size_t start = 0;
        while(start < query.size()) {
                size_t eq = query.find('=', start);
                if(eq == string::npos) {
                        break;
                }

                size_t param = eq + 1;
                size_t end = query.find('&', param);

                if(end == string::npos) {
                        end = query.size();
                }

                if(eq > start && end > param) {
                        ret[query.substr(start, eq-start)] = query.substr(param, end - param);
                }

                start = end + 1;
        }

        return ret;
}

string Util::getAwayMessage() {
    return (formatTime(awayMsg.empty() ? SETTING(DEFAULT_AWAY_MESSAGE) : awayMsg, awayTime)) + " <" APPNAME " v" VERSIONSTRING ">";
}

string Util::formatBytes(int64_t aBytes) {
    char buf[128];
    if(aBytes < 1024) {
        snprintf(buf, sizeof(buf), _("%d B"), (int)(aBytes&0xffffffff));
    } else if(aBytes < 1024*1024) {
        snprintf(buf, sizeof(buf), _("%.02f KiB"), (double)aBytes/(1024.0));
    } else if(aBytes < 1024*1024*1024) {
        snprintf(buf, sizeof(buf), _("%.02f MiB"), (double)aBytes/(1024.0*1024.0));
    } else if(aBytes < (int64_t)1024*1024*1024*1024) {
        snprintf(buf, sizeof(buf), _("%.02f GiB"), (double)aBytes/(1024.0*1024.0*1024.0));
    } else if(aBytes < (int64_t)1024*1024*1024*1024*1024) {
        snprintf(buf, sizeof(buf), _("%.02f TiB"), (double)aBytes/(1024.0*1024.0*1024.0*1024.0));
    } else {
        snprintf(buf, sizeof(buf), _("%.02f PiB"), (double)aBytes/(1024.0*1024.0*1024.0*1024.0*1024.0));
    }

    return buf;
}

string Util::formatExactSize(int64_t aBytes) {
#ifdef _WIN32
        TCHAR tbuf[128];
        TCHAR number[64];
        NUMBERFMT nf;
        _sntprintf(number, 64, _T("%I64d"), aBytes);
        TCHAR Dummy[16];
        TCHAR sep[2] = _T(",");

        /*No need to read these values from the system because they are not
        used to format the exact size*/
        nf.NumDigits = 0;
        nf.LeadingZero = 0;
        nf.NegativeOrder = 0;
        nf.lpDecimalSep = sep;

        GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_SGROUPING, Dummy, 16 );
        nf.Grouping = Util::toInt(Text::fromT(Dummy));
        GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, Dummy, 16 );
        nf.lpThousandSep = Dummy;

        GetNumberFormat(LOCALE_USER_DEFAULT, 0, number, &nf, tbuf, sizeof(tbuf)/sizeof(tbuf[0]));

        char buf[128];
        _snprintf(buf, sizeof(buf), _("%s B"), Text::fromT(tbuf).c_str());
        return buf;
#else
        char buf[128];
        snprintf(buf, sizeof(buf), _("%'lld B"), (long long int)aBytes);
        return string(buf);
#endif
}

string Util::getLocalIp() {
#if defined(HAVE_IFADDRS_H) || defined(HAVE_ADDRS_H)
    vector<string> addresses;
    struct ifaddrs *ifap;

    if (getifaddrs(&ifap) == 0)
    {
        for (struct ifaddrs *i = ifap; i != NULL; i = i->ifa_next)
        {
            struct sockaddr *sa = i->ifa_addr;

            // If the interface is up, is not a loopback and it has an address
            if ((i->ifa_flags & IFF_UP) && !(i->ifa_flags & IFF_LOOPBACK) && sa != NULL)
            {
                void* src = NULL;
                socklen_t len;

                // IPv4 address
                if (sa->sa_family == AF_INET)
                {
                    struct sockaddr_in* sai = (struct sockaddr_in*)sa;
                    src = (void*) &(sai->sin_addr);
                    len = INET_ADDRSTRLEN;
                }
                // IPv6 address
                else if (sa->sa_family == AF_INET6)
                {
                    struct sockaddr_in6* sai6 = (struct sockaddr_in6*)sa;
                    src = (void*) &(sai6->sin6_addr);
                    len = INET6_ADDRSTRLEN;
                }

                // Convert the binary address to a string and add it to the output list
                if (src != NULL)
                {
                    char address[len];
                    inet_ntop(sa->sa_family, src, address, len);
                    addresses.push_back(address);
                }
            }
        }
        freeifaddrs(ifap);
    }
    return addresses[0];
#else
    string tmp;

    char buf[256];
    gethostname(buf, 255);
    hostent* he = gethostbyname(buf);
    if(he == NULL || he->h_addr_list[0] == 0)
        return Util::emptyString;
    sockaddr_in dest;
    int i = 0;

    // We take the first ip as default, but if we can find a better one, use it instead...
    memcpy(&(dest.sin_addr), he->h_addr_list[i++], he->h_length);
    tmp = inet_ntoa(dest.sin_addr);
    if(Util::isPrivateIp(tmp) || strncmp(tmp.c_str(), "169", 3) == 0) {
        while(he->h_addr_list[i]) {
            memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
            string tmp2 = inet_ntoa(dest.sin_addr);
            if(!Util::isPrivateIp(tmp2) && strncmp(tmp2.c_str(), "169", 3) != 0) {
                tmp = tmp2;
            }
            i++;
        }
    }
    return tmp;
#endif
}

bool Util::isPrivateIp(string const& ip) {
    struct in_addr addr;

    addr.s_addr = inet_addr(ip.c_str());

    if (addr.s_addr != INADDR_NONE) {
        unsigned long haddr = ntohl(addr.s_addr);
        return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
                (haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
                (haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
                (haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
    }
    return false;
}

typedef const uint8_t* ccp;
static wchar_t utf8ToLC(ccp& str) {
    wchar_t c = 0;
    if(str[0] & 0x80) {
        if(str[0] & 0x40) {
            if(str[0] & 0x20) {
                if(str[1] == 0 || str[2] == 0 ||
                    !((((unsigned char)str[1]) & ~0x3f) == 0x80) ||
                    !((((unsigned char)str[2]) & ~0x3f) == 0x80))
                {
                    str++;
                    return 0;
                }
                c = ((wchar_t)(unsigned char)str[0] & 0xf) << 12 |
                    ((wchar_t)(unsigned char)str[1] & 0x3f) << 6 |
                    ((wchar_t)(unsigned char)str[2] & 0x3f);
                str += 3;
            } else {
                if(str[1] == 0 ||
                    !((((unsigned char)str[1]) & ~0x3f) == 0x80))
                {
                    str++;
                    return 0;
                }
                c = ((wchar_t)(unsigned char)str[0] & 0x1f) << 6 |
                    ((wchar_t)(unsigned char)str[1] & 0x3f);
                str += 2;
            }
        } else {
            str++;
            return 0;
        }
    } else {
        wchar_t c = Text::asciiToLower((char)str[0]);
        str++;
        return c;
    }

    return Text::toLower(c);
}

string Util::toString(const string& sep, const StringList& lst) {
        string ret;
        for(StringList::const_iterator i = lst.begin(), iend = lst.end(); i != iend; ++i) {
                ret += *i;
                if(i + 1 != iend)
                        ret += sep;
        }
        return ret;
}

string Util::toString(const StringList& lst) {
        if(lst.empty())
                return emptyString;
        if(lst.size() == 1)
                return lst[0];
        return '[' + toString(",", lst) + ']';
}

string::size_type Util::findSubString(const string& aString, const string& aSubString, string::size_type start) throw() {
    if(aString.length() < start)
        return (string::size_type)string::npos;

    if(aString.length() - start < aSubString.length())
        return (string::size_type)string::npos;

    if(aSubString.empty())
        return 0;

    // Hm, should start measure in characters or in bytes? bytes for now...
    const uint8_t* tx = (const uint8_t*)aString.c_str() + start;
    const uint8_t* px = (const uint8_t*)aSubString.c_str();

    const uint8_t* end = tx + aString.length() - start - aSubString.length() + 1;

    wchar_t wp = utf8ToLC(px);

    while(tx < end) {
        const uint8_t* otx = tx;
        if(wp == utf8ToLC(tx)) {
            const uint8_t* px2 = px;
            const uint8_t* tx2 = tx;

            for(;;) {
                if(*px2 == 0)
                    return otx - (uint8_t*)aString.c_str();

                if(utf8ToLC(px2) != utf8ToLC(tx2))
                    break;
            }
        }
    }
    return (string::size_type)string::npos;
}

wstring::size_type Util::findSubString(const wstring& aString, const wstring& aSubString, wstring::size_type pos) throw() {
    if(aString.length() < pos)
        return static_cast<wstring::size_type>(wstring::npos);

    if(aString.length() - pos < aSubString.length())
        return static_cast<wstring::size_type>(wstring::npos);

    if(aSubString.empty())
        return 0;

    wstring::size_type j = 0;
    wstring::size_type end = aString.length() - aSubString.length() + 1;

    for(; pos < end; ++pos) {
        if(Text::toLower(aString[pos]) == Text::toLower(aSubString[j])) {
            wstring::size_type tmp = pos+1;
            bool found = true;
            for(++j; j < aSubString.length(); ++j, ++tmp) {
                if(Text::toLower(aString[tmp]) != Text::toLower(aSubString[j])) {
                    j = 0;
                    found = false;
                    break;
                }
            }

            if(found)
                return pos;
        }
    }
    return static_cast<wstring::size_type>(wstring::npos);
}

int Util::stricmp(const char* a, const char* b) {
    while(*a) {
        wchar_t ca = 0, cb = 0;
        int na = Text::utf8ToWc(a, ca);
        int nb = Text::utf8ToWc(b, cb);
        ca = Text::toLower(ca);
        cb = Text::toLower(cb);
        if(ca != cb) {
            return (int)ca - (int)cb;
        }
        a += abs(na);
        b += abs(nb);
    }
    wchar_t ca = 0, cb = 0;
    Text::utf8ToWc(a, ca);
    Text::utf8ToWc(b, cb);

    return (int)Text::toLower(ca) - (int)Text::toLower(cb);
}

int Util::strnicmp(const char* a, const char* b, size_t n) {
    const char* end = a + n;
    while(*a && a < end) {
        wchar_t ca = 0, cb = 0;
        int na = Text::utf8ToWc(a, ca);
        int nb = Text::utf8ToWc(b, cb);
        ca = Text::toLower(ca);
        cb = Text::toLower(cb);
        if(ca != cb) {
            return (int)ca - (int)cb;
        }
        a += abs(na);
        b += abs(nb);
    }
    wchar_t ca = 0, cb = 0;
    Text::utf8ToWc(a, ca);
    Text::utf8ToWc(b, cb);
    return (a >= end) ? 0 : ((int)Text::toLower(ca) - (int)Text::toLower(cb));
}

string Util::encodeURI(const string& aString, bool reverse) {
    // reference: rfc2396
    string tmp = aString;
    if(reverse) {
        string::size_type idx;
        for(idx = 0; idx < tmp.length(); ++idx) {
            if(tmp.length() > idx + 2 && tmp[idx] == '%' && isxdigit(tmp[idx+1]) && isxdigit(tmp[idx+2])) {
                tmp[idx] = fromHexEscape(tmp.substr(idx+1,2));
                tmp.erase(idx+1, 2);
            } else { // reference: rfc1630, magnet-uri draft
                if(tmp[idx] == '+')
                    tmp[idx] = ' ';
            }
        }
    } else {
        const string disallowed = ";/?:@&=+$," // reserved
                                  "<>#%\" "    // delimiters
                                  "{}|\\^[]`"; // unwise
        string::size_type idx, loc;
        for(idx = 0; idx < tmp.length(); ++idx) {
            if(tmp[idx] == ' ') {
                tmp[idx] = '+';
            } else {
                if(tmp[idx] <= 0x1F || tmp[idx] >= 0x7f || (loc = disallowed.find_first_of(tmp[idx])) != string::npos) {
                    tmp.replace(idx, 1, toHexEscape(tmp[idx]));
                    idx+=2;
                }
            }
        }
    }
    return tmp;
}

/**
 * This function takes a string and a set of parameters and transforms them according to
 * a simple formatting rule, similar to strftime. In the message, every parameter should be
 * represented by %[name]. It will then be replaced by the corresponding item in
 * the params stringmap. After that, the string is passed through strftime with the current
 * date/time and then finally written to the log file. If the parameter is not present at all,
 * it is removed from the string completely...
 */
string Util::formatParams(const string& msg, const StringMap& params, bool filter) {
    string result = msg;

    string::size_type i, j, k;
    i = 0;
    while (( j = result.find("%[", i)) != string::npos) {
        if( (result.size() < j + 2) || ((k = result.find(']', j + 2)) == string::npos) ) {
            break;
        }
        string name = result.substr(j + 2, k - j - 2);
                StringMap::const_iterator smi = params.find(name);
        if(smi == params.end()) {
            result.erase(j, k-j + 1);
            i = j;
        } else {
            if(smi->second.find_first_of("%\\./") != string::npos) {
                string tmp = smi->second;   // replace all % in params with %% for strftime
                string::size_type m = 0;
                while(( m = tmp.find('%', m)) != string::npos) {
                    tmp.replace(m, 1, "%%");
                    m+=2;
                }
                if(filter) {
                    // Filter chars that produce bad effects on file systems
                    m = 0;
                    while(( m = tmp.find_first_of("\\./", m)) != string::npos) {
                        tmp[m] = '_';
                    }
                }

                result.replace(j, k-j + 1, tmp);
                i = j + tmp.size();
            } else {
                result.replace(j, k-j + 1, smi->second);
                i = j + smi->second.size();
            }
        }
    }

    result = formatTime(result, time(NULL));

    return result;
}

string Util::formatTime(const string &msg, const time_t t) {
    if (!msg.empty()) {
    tm* loc = localtime(&t);

        if(!loc) {
            return Util::emptyString;
        }
        size_t bufsize = msg.size() + 256;
        string buf(bufsize, 0);

        errno = 0;

        buf.resize(strftime(&buf[0], bufsize-1, msg.c_str(), loc));

        while(buf.empty()) {
            if(errno == EINVAL)
                return Util::emptyString;
            bufsize+=64;
            buf.resize(bufsize);
            buf.resize(strftime(&buf[0], bufsize-1, msg.c_str(), loc));
        }

#ifdef _WIN32
                if(!Text::validateUtf8(buf))
#endif
                {
                        buf = Text::toUtf8(buf);
                }
                return buf;
    }
    return Util::emptyString;
}

/* Below is a high-speed random number generator with much
   better granularity than the CRT one in msvc...(no, I didn't
   write it...see copyright) */
/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.
   Any feedback is very welcome. For any question, comments,
   see http://www.math.keio.ac.jp/matumoto/emt.html or email
   matumoto@math.keio.ac.jp */
/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializing the array with a NONZERO seed */
static void sgenrand(unsigned long seed) {
    /* setting initial seeds to mt[N] using         */
    /* the generator Line 25 of Table 1 in          */
    /* [KNUTH 1981, The Art of Computer Programming */
    /*    Vol. 2 (2nd Ed.), pp102]                  */
    mt[0]= seed & 0xffffffff;
    for (mti=1; mti<N; mti++)
        mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

uint32_t Util::rand() {
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if sgenrand() has not been called, */
            sgenrand(4357); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }

    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y;
}

/*  getIpCountry
    This function returns the country(Abbreviation) of an ip
    for exemple: it returns "PT", whitch standards for "Portugal"
    more info: http://www.maxmind.com/app/csv
*/
string Util::getIpCountry (string IP) {
    if (BOOLSETTING(GET_USER_COUNTRY)) {
        dcassert(count(IP.begin(), IP.end(), '.') == 3);

        //e.g IP 23.24.25.26 : w=23, x=24, y=25, z=26
        string::size_type a = IP.find('.');
        string::size_type b = IP.find('.', a+1);
        string::size_type c = IP.find('.', b+2);

        uint32_t ipnum = (Util::toUInt32(IP.c_str()) << 24) |
            (Util::toUInt32(IP.c_str() + a + 1) << 16) |
            (Util::toUInt32(IP.c_str() + b + 1) << 8) |
            (Util::toUInt32(IP.c_str() + c + 1) );

        CountryIter i = countries.lower_bound(ipnum);

        if(i != countries.end()) {
            return string((char*)&(i->second), 2);
        }
    }

    return Util::emptyString; //if doesn't returned anything already, something is wrong...
}

string Util::getTimeString() {
    char buf[64];
    time_t _tt;
    time(&_tt);
    tm* _tm = localtime(&_tt);
    if(_tm == NULL) {
        strcpy(buf, "xx:xx:xx");
    } else {
        strftime(buf, 64, "%X", _tm);
    }
    return buf;
}

string Util::toAdcFile(const string& file) {
    if(file == "files.xml.bz2" || file == "files.xml")
        return file;

    string ret;
    ret.reserve(file.length() + 1);
    ret += '/';
    ret += file;
    for(string::size_type i = 0; i < ret.length(); ++i) {
        if(ret[i] == '\\') {
            ret[i] = '/';
        }
    }
    return ret;
}
string Util::toNmdcFile(const string& file) {
    if(file.empty())
        return Util::emptyString;

    string ret(file.substr(1));
    for(string::size_type i = 0; i < ret.length(); ++i) {
        if(ret[i] == '/') {
            ret[i] = '\\';
        }
    }
    return ret;
}

string Util::translateError(int aError) {
#ifdef _WIN32
    LPTSTR lpMsgBuf;
    DWORD chars = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        aError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );
    if(chars == 0) {
        return string();
    }
    string tmp = Text::fromT(lpMsgBuf);
    // Free the buffer.
    LocalFree( lpMsgBuf );
    string::size_type i = 0;

    while( (i = tmp.find_first_of("\r\n", i)) != string::npos) {
        tmp.erase(i, 1);
    }
    return tmp;
#else // _WIN32
    return Text::toUtf8(strerror(aError));
#endif // _WIN32
}

bool Util::getAway() {
    return away;
}

void Util::setAway(bool aAway) {
    bool changed = aAway != away;
    away = aAway;
    if(away)
        awayTime = time(NULL);

    if(changed)
        ClientManager::getInstance()->infoUpdated();
 }

 void Util::switchAway() {
    setAway(!away);
 }

string Util::formatAdditionalInfo(const string& aIp, bool sIp, bool sCC) {
	string ret = Util::emptyString;

	if(!aIp.empty()) {
		string cc = Util::getIpCountry(aIp);
		bool showIp = BOOLSETTING(USE_IP) || sIp;
		bool showCc = (BOOLSETTING(GET_USER_COUNTRY) || sCC) && !cc.empty();

		if(showIp) {
			ret = "[" + aIp + "] ";
		}
        //printf("%s\n",ret.c_str());
		if(showCc) {
			ret += "[" + cc + "] ";
        //printf("%s\n",ret.c_str());
		}
        //printf("%s\n",ret.c_str());
	}
	return Text::toT(ret);
}

bool Util::fileExists(const string &aFile) {
#ifdef _WIN32
    DWORD attr = GetFileAttributes(Text::toT(aFile).c_str());
    return (attr != 0xFFFFFFFF);
#else
    struct stat stFileInfo;
    return (stat(aFile.c_str(),&stFileInfo) != 0);
#endif
}

} // namespace dcpp
