#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <map>

#include <Wt/WString>

class Utils
{
public:
    static void init();
    static Wt::WString formatBytes(long long aBytes);
    static Wt::WString getFileImage(const Wt::WString &file);

private:
    Utils(){}
    Utils(const Utils&){}
    Utils &operator=(const Utils&){}
    virtual ~Utils(){}


    static std::map< Wt::WString, Wt::WString > fileTypes;
};

#endif // UTILS_H
