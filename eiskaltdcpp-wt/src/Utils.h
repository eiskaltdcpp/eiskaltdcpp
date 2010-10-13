#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <map>

#include <Wt/WString>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/CID.h"

inline Wt::WString _q(const std::string &str){ return Wt::WString(str.c_str()); }
inline std::string _tq(const Wt::WString &str){ return str.toUTF8(); }

class Utils
{
public:
    static void init();
    static Wt::WString formatBytes(long long aBytes);
    static Wt::WString getFileImage(const Wt::WString &file);

    static Wt::WString getNicks(const Wt::WString &cid){
        return getNicks(dcpp::CID(cid.toUTF8()));
    }

    static Wt::WString getNicks(const dcpp::CID &cid){
        const dcpp::Identity &user = dcpp::ClientManager::getInstance()->getOnlineUserIdentity(dcpp::ClientManager::getInstance()->getUser(cid));
        return _q(user.getNick());
    }

private:
    Utils(){}
    Utils(const Utils&){}
    Utils &operator=(const Utils&){}
    virtual ~Utils(){}


    static std::map< Wt::WString, Wt::WString > fileTypes;
};

#endif // UTILS_H
