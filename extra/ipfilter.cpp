/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ipfilter.h"

#include <stdlib.h>
#include <sstream>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "dcpp/Util.h"
#include "dcpp/File.h"
#include "dcpp/StringTokenizer.h"

using namespace std;
using namespace dcpp;

const string signature = "$EISKALTDC IPFILTERLIST$";

inline static uint32_t make_ip(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
    return ((a << 24) | (b << 16) | (c << 8) | d);
}

IPFilter::IPFilter() {
}

IPFilter::~IPFilter() {
    clearRules();
}

uint32_t IPFilter::StringToUint32(const string& ip){
    unsigned int ip1=0,ip2=0,ip3=0,ip4=0;
    if (sscanf(ip.c_str(),"%3u.%3u.%3u.%3u",&ip1,&ip2,&ip3,&ip4) != 4 || ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255)
        return 0;
    return make_ip(ip1,ip2,ip3,ip4);
}

string IPFilter::Uint32ToString(uint32_t ip){
    string ret;
    unsigned int ip1,ip2,ip3,ip4;
    ip1 = (ip & 0xFF000000) >> 24;
    ip2 = (ip & 0x00FF0000) >> 16;
    ip3 = (ip & 0x0000FF00) >> 8;
    ip4 = (ip & 0x000000FF);
    std::stringstream ss;
    ss << ip1 << "." << ip2 << "." << ip3 << "." << ip4;
    ss >> ret;
    return ret;
}

uint32_t IPFilter::MaskToCIDR(uint32_t mask){
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"ipfilter::MaskToCIDR(%x)\n", mask);fflush(stdout);
#endif
    if (mask == 0)
        return 0;
    else if (mask == 0xFFFFFFFF)
        return 32;

    uint32_t shift_mask = 0x00000001;

    int j = 0;

    for (; ((mask & shift_mask) == 0) && (j <= 32); j++, shift_mask <<= 1);

    j = 32 - j;

    return j;
}

uint32_t IPFilter::MaskForBits(uint32_t bits){
    bits = 32 - (bits > 32?32:bits);

    uint32_t mask = 0xFFFFFFFF;
    uint32_t bit  = 0xFFFFFFFE;

    for (unsigned i = 0; i < bits; i++, bit <<= 1)
        mask &= bit;

    return mask;
}

bool IPFilter::ParseString(string exp, uint32_t &ip, uint32_t &mask, eTableAction &act){
    if (exp.empty())
        return false;

    if (exp.find("/0") != string::npos) {
        if (exp.compare(0, 1, "!") == 0)
            act = etaDROP;
        else
            act = etaACPT;

        ip = mask = 0x00000000;

        return true;
    }

    unsigned int ip1=0,ip2=0,ip3=0,ip4=0,mask1=0,nip=0;
    nip = exp.find("!") != string::npos;
    const string str_ip = exp.substr(exp.find("!") != string::npos);
    if (str_ip.find("/") != string::npos) {
        #ifdef _DEBUG_IPFILTER
        fprintf(stdout,"%s\n",str_ip.c_str());fflush(stdout);
        #endif
        if (sscanf(str_ip.c_str(),"%3u.%3u.%3u.%3u/%2u",&ip1,&ip2,&ip3,&ip4,&mask1) != 5 || ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255) {
            #ifdef _DEBUG_IPFILTER
            fprintf(stdout,"fail parse with mask\n"); fflush(stdout);
            #endif
            return false;}
    }
    else {
        if (sscanf(str_ip.c_str(),"%3u.%3u.%3u.%3u",&ip1,&ip2,&ip3,&ip4) != 4 || ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255) {
            #ifdef _DEBUG_IPFILTER
            fprintf(stdout,"fail parse without mask\n"); fflush(stdout);
            #endif
            return false;}
    }
    #ifdef _DEBUG_IPFILTER
        fprintf(stdout,"ip::%u %u %u %u mask::%u \n",ip1,ip2,ip3,ip4,mask1); fflush(stdout);
    #endif
    if (nip > 0)
        act = etaDROP;
    else
        act = etaACPT;
    #ifdef _DEBUG_IPFILTER
        fprintf(stdout,"act::%u\n",nip); fflush(stdout);
    #endif
    mask = MaskForBits(mask1 > 32? 32:mask1);

    ip = make_ip(ip1,ip2,ip3,ip4);
    return true;
}

bool IPFilter::addToRules(const std::string &exp, eDIRECTION direction) {
    uint32_t exp_ip, exp_mask;
    eTableAction act;

    if (!ParseString(exp, exp_ip, exp_mask, act))
        return false;

    IPFilterElem *el = nullptr;

    if (list_ip.find(exp_ip) != list_ip.end()) {
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tIP already in list\n");
#endif
        IPHash::const_iterator it = list_ip.find(exp_ip);

        while (it != list_ip.end() && it->first == exp_ip) {
            el = it->second;

            if ((el->direction != direction) && (el->action == act)) {
#ifdef _DEBUG_IPFILTER
                fprintf(stdout,"\tChange direction of IP\n");fflush(stdout);
#endif
                el->direction = eDIRECTION_BOTH;

                return false;
            }
            else if (el->direction == direction && el->action == act)
                return false;

            ++it;
        }
    }

    el = new IPFilterElem;

    el->mask      = exp_mask;
    el->ip        = exp_ip;
    el->direction = direction;
    el->action    = act;

#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tCreated new element:\n");
    fprintf(stdout,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %i\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           el->mask, el->ip,
           (int)el->direction, (int)el->action
          );fflush(stdout);
#endif

    list_ip.insert(pair<uint32_t, IPFilterElem*>(el->ip,el));
    rules.push_back(el);

    return true;
}

void IPFilter::remFromRules(string exp, eTableAction act) {

    string str_ip;
    uint32_t exp_ip;
#ifdef _DEBUG_IPFILTER
    printf("remove: exp - %s\n", exp.c_str());
#endif
    size_t pos = exp.find("/");
#ifdef _DEBUG_IPFILTER
    printf("pos / - %u\n", (uint32_t)pos);
#endif
    if (pos != string::npos)
        str_ip = exp.erase(pos);
    else
        str_ip = exp;
#ifdef _DEBUG_IPFILTER
    printf("remove: str_ip - %s\n", str_ip.c_str());
#endif
    exp_ip = IPFilter::StringToUint32(str_ip);
    IPHash::iterator it = list_ip.find(exp_ip);

    if (it != list_ip.end() && it->first == exp_ip){
        IPFilterElem *el = it->second;
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tThis element will be deleted:\n");
    fprintf(stdout,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %u\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           el->mask, el->ip,
           (int)el->direction, (int)el->action
          );fflush(stdout);
#endif
        if (el->action == act) {
#ifdef _DEBUG_IPFILTER
        printf("ok, action match. delete element.\n");
#endif
            list_ip.erase(it);
            rules.erase( remove( rules.begin(), rules.end(), el ), rules.end());
#ifdef _DEBUG_IPFILTER
        printf("element is deleted.\n");
#endif
        }
#ifdef _DEBUG_IPFILTER
        printf("delete *el\n");fflush(stdout);
#endif
        delete el;
    }
}

void IPFilter::changeRuleDirection(string exp, eDIRECTION direction, eTableAction act) {
    string str_ip;
    size_t pos = exp.find("/");
#ifdef _DEBUG_IPFILTER
    printf("pos / - %u\n", (uint32_t)pos);fflush(stdout);
#endif
    if (pos != string::npos)
        str_ip = exp.erase(pos);
    else
        str_ip = exp;

    uint32_t exp_ip = IPFilter::StringToUint32(str_ip);
    IPHash::const_iterator it = list_ip.find(exp_ip);

    if (it != list_ip.end() && it->first == exp_ip) {
        IPFilterElem *el = it->second;

        if (el->action == act){
            el->direction = direction;
        }
    }
}

bool IPFilter::OK(const string &exp, eDIRECTION direction){
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"ipfilter::OK(%s,%i)\n",exp.c_str(),(int)direction);fflush(stdout);
#endif
    string str_src(exp);
    size_t pos = str_src.find(":") != string::npos;
    if (pos > 0) {
        str_src = str_src.erase(pos);
        //XXX.XXX.XXX.XXX:PORT -> XXX.XXX.XXX.XXX
    }

    uint32_t src = IPFilter::StringToUint32(str_src);
    IPFilterElem *el;

    for (unsigned i = 0; i < rules.size(); i++){
        el = rules.at(i);

#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tel->ip & el->mask == %x\n"
           "\tsrc    & el->mask == %x\n",
           (el->ip & el->mask),
           (src    & el->mask)
          );fflush(stdout);
#endif

        if ((el->ip & el->mask) == (src & el->mask)){//Exact match
            bool exact_direction = ((el->direction == direction) || (el->direction == eDIRECTION_BOTH));
#ifdef _DEBUG_IPFILTER
            fprintf(stdout,"\tFound match... ");fflush(stdout);
#endif
            if      ((el->action == etaDROP) && exact_direction){
#ifdef _DEBUG_IPFILTER
                fprintf(stdout,"DROP.\n");fflush(stdout);
#endif
                return false;
            }
            else if ((el->action == etaACPT) && exact_direction){
#ifdef _DEBUG_IPFILTER
                fprintf(stdout,"ACCEPT.\n");fflush(stdout);
#endif
                return true;
            }
#ifdef _DEBUG_IPFILTER
            else
                fprintf(stdout,"IGNORE.\n");fflush(stdout);
#endif
        }
    }

    return true;
}

void IPFilter::step(uint32_t ip, eTableAction act, bool down){
    IPFilterElem *el = nullptr;

    IPHash::const_iterator it = list_ip.find(ip);

    if (it != list_ip.end() && it->first == ip && it->second->action == act) {
        el = it->second;
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tThis element will be moved:\n");
    fprintf(stdout,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %u\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           el->mask, el->ip,
           (int)el->direction, (int)el->action
          );fflush(stdout);
#endif
    }

    if (!el)
        return;

    int index = -1;

    for (size_t itt = 0; itt < rules.size(); ++itt) {
        if (rules.at(itt) == el) {
            index = itt;
            break;
        }
    }

    if (index < 0)
        return;

    const int control = (down?(rules.size()-1):0);
    const int inc = (down?1:-1);

#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tat place this element:\n");
    fprintf(stdout,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %u\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           rules.at(index+inc)->mask, rules.at(index+inc)->ip,
           (int)rules.at(index+inc)->direction, (int)rules.at(index+inc)->action
          );fflush(stdout);
#endif

    if (index == control)
        return;

    const int new_index = index + inc;
    IPFilterElem *old_el = rules.at(new_index);

    rules[index]= old_el;
    rules[new_index]= el;
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"\tElement has been moved at new_index:\n");
    fprintf(stdout,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %u\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           rules.at(new_index)->mask, rules.at(new_index)->ip,
           (int)rules.at(new_index)->direction, (int)rules.at(new_index)->action
          );fflush(stdout);
    fprintf(stdout,"\tElement at index moved from new_index:\n");
    fprintf(stdout,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %x\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           rules.at(index)->mask, rules.at(index)->ip,
           (int)rules.at(index)->direction, (int)rules.at(index)->action
          );fflush(stdout);
#endif


}

void IPFilter::moveRuleUp(uint32_t ip, eTableAction act){
    step(ip, act, false);
}

void IPFilter::moveRuleDown(uint32_t ip, eTableAction act){
    step(ip, act, true);
}

void IPFilter::loadList() {
    const string &&fileNameFull = Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter";

    if (!Util::fileExists(fileNameFull))
        return;

    File file(fileNameFull, File::READ, File::OPEN);
    const string fileData = file.read();
    file.close();
#ifdef _DEBUG_IPFILTER
    fprintf(stdout,"full string: %s\n",fileData.c_str());fflush(stdout);
#endif

    clearRules();

    if (fileData.empty())
        return;

    StringTokenizer<string> st(fileData, "\n");
    for (string str_ip : st.getTokens()) {
        eDIRECTION direction = eDIRECTION_IN;
#ifdef _DEBUG_IPFILTER
        fprintf(stdout,"pointer on part string: %s\n",str_ip.c_str());fflush(stdout);
#endif
        if (str_ip.compare(0, 7, "|D_IN|:") == 0){
            str_ip = str_ip.erase(0, 7);//delete "|D_IN|:"
        }
        else if (str_ip.compare(0, 8, "|D_OUT|:") == 0) {//delete "|D_OUT|:"
            str_ip = str_ip.erase(0, 8);
            direction = eDIRECTION_OUT;
        }
        else if (str_ip.compare(0, 9, "|D_BOTH|:") == 0) {//delete "|D_BOTH|:"
            str_ip = str_ip.erase(0, 9);
            direction = eDIRECTION_BOTH;
        } else
            continue;

#ifdef _DEBUG_IPFILTER
        fprintf(stdout,"string without direction: %s\n",str_ip.c_str());fflush(stdout);
#endif

        addToRules(str_ip, direction);
    }
}

void IPFilter::saveList(){
    string file= Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter";
    File f(file, File::WRITE, File::CREATE | File::TRUNCATE);
    f.write(signature+"\n");

    for (size_t i = 0; i < rules.size(); ++i) {
        string prefix;
        IPFilterElem *el = rules.at(i);

        eDIRECTION direction = el->direction;
        eTableAction act = el->action;

        prefix = (direction == eDIRECTION_IN?"|D_IN|:":(direction == eDIRECTION_OUT?"|D_OUT|:":"|D_BOTH|:"));
        prefix += string(act == etaACPT?"":"!");
        string prefix1; stringstream ss;
        ss << MaskToCIDR(el->mask); ss >> prefix1;
        f.write(prefix + Uint32ToString(el->ip).c_str() + "/" + prefix1.c_str() + "\n");
    }
    f.close();
}

void IPFilter::exportTo(string path, string& error) {
    string file = Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter";
    saveList();
    if (!Util::fileExists(path)) {
        error = "Nothing to export.";
        fprintf(stdout, "Nothing to export.");fflush(stdout);
        return;
    }
    File::deleteFile(path);
    try{
        File::copyFile(file,path);
    } catch (...) {
        error = "Unable to export settings.";
        fprintf(stdout, "Unable to export settings.");fflush(stdout);
    return;
    }
    return;
}

void IPFilter::importFrom(string path, string& error) {
    if (!Util::fileExists(path)) {
        error = "Nothing to export.";
        fprintf(stdout,"Nothing to export.");fflush(stdout);
        return;
    }
    File f(path, File::READ, File::OPEN);
    string sign = f.read();//in.readLine();
    StringTokenizer<string> st(sign, "\n");
    f.close();

    if (*st.getTokens().begin() == signature) {
        string old_file = Util::getPath(Util::PATH_USER_CONFIG)+ "ipfilter";
        File::deleteFile(old_file);
        try{
            File::copyFile(path,old_file);
        } catch (...) {}

        clearRules();
        loadList();
    } else {
        error = "Invalid signature.";
        fprintf(stdout, "Invalid signature.");fflush(stdout);
    }
}

const IPList &IPFilter::getRules() {
    return rules;
}

const IPHash &IPFilter::getHash() {
    return list_ip;
}

void IPFilter::clearRules() {
    list_ip.clear();
    rules.clear();
}

void IPFilter::load() {
    if (IPFilter::getInstance())
        IPFilter::getInstance()->loadList();
    else {
        IPFilter::newInstance();
        IPFilter::getInstance()->loadList();
    }
}

void IPFilter::shutdown() {
    if (IPFilter::getInstance()) {
        IPFilter::getInstance()->saveList();
        IPFilter::deleteInstance();
    }
}
