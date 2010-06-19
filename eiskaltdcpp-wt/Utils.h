#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <map>

#include <Wt/WString>

class Utils
{
public:
    Utils();

    static Wt::WString formatBytes(long long aBytes);

private:
    static std::map< Wt::WString, Wt::WString > fileTypes;
};

#endif // UTILS_H
