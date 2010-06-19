#include "Utils.h"

using namespace Wt;

std::map<WString,WString> Utils::fileTypes = std::map<WString,WString>();

Utils::Utils()
{
}

WString Utils::formatBytes(long long aBytes){
    WString s;

    if(aBytes < 1024)
        s = WString("{1} B").arg((int)(aBytes & 0xffffffff));
    else if(aBytes < 1024*1024)
        s = WString("{1} KiB").arg(static_cast<double>(aBytes)/1024.0);
    else if(aBytes < 1024*1024*1024)
        s = WString("{1} MiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0));
    else if(aBytes < static_cast<int64_t>(1024)*1024*1024*1024)
        s = WString("{1} GiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0*1024.0));
    else if(aBytes < static_cast<int64_t>(1024)*1024*1024*1024*1024)
        s = WString("{1} TiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0*1024.0*1024.0));
    else
        s = WString("{1} PiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0*1024.0*1024.0*1024.0));

    return s;
}
