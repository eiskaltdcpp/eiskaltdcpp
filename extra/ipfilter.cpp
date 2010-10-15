/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ipfilter.h"

//#include <QFile>
//#include <QDir>
//#include <QTextStream>
//#include <QMessageBox>

#define _DEBUG_IPFILTER_

//#ifdef _DEBUG_IPFILTER_
//#include <QtDebug>
//#endif
//#include <sys/stat.h>
//#include <unistd.h>
#include <stdlib.h>
//#include <stdio.h>
#include <iostream>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/File.h"
#include "dcpp/StringTokenizer.h"

//using namespace dcpp;

const string signature = "$EISKALTDC IPFILTERLIST$";

IPFilter::IPFilter() {
}

IPFilter::~IPFilter() {
    clearRules(false);
}

uint32_t IPFilter::StringToUint32(const string& ip){
    string exp_ip(ip);
    int ip1,ip2,ip3,ip4,mask1,nip,nmask;
    char * eta;
    char * char_ip;
    char * char_mask;
    sscanf(ip.c_str(),"%s%x.%x.%x.%x/%x",&eta,&ip1,&ip2,&ip3,&ip4,&mask1);
    //nip = sprintf(char_ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
    //nmask = sprintf(char_mask,"%u",mask1);
    fprintf(stderr,"%i.%i.%i.%i",ip1,ip2,ip3,ip4);
    uint32_t ret = ip1;
    ret <<= 8;
    ret |= ip2;
    ret <<= 8;
    ret |= ip3;
    ret <<= 8;
    ret |= ip4;
    fprintf(stderr,"%x",ret);
    return ret;
}

string IPFilter::Uint32ToString(uint32_t ip){
    string ret;
    char * char_ip;
    long int ip1,ip2,ip3,ip4;
    int n;
    ip1 = (ip & 0xFF000000) >> 24;
    ip2 = (ip & 0x00FF0000) >> 16;
    ip3 = (ip & 0x0000FF00) >> 8;
    ip4 = (ip & 0x000000FF);
    //n=sprintf(char_ip,"%u%u")
    std::stringstream ss;
    ss << ip1 << "." << ip2 << "." << ip3 << "." << ip4;
    ss >> ret;
    return ret;
}

uint32_t IPFilter::MaskToCIDR(uint32_t mask){
#ifdef _DEBUG_IPFILTER_
    fprintf(stderr,"IPFilter::MaskToCIDR(%x)\n", mask);
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

    for (int i = 0; i < bits; i++, bit <<= 1)
        mask &= bit;

    return mask;
}

bool IPFilter::ParseString(string exp, uint32_t &ip, uint32_t &mask, eTableAction &act){
    if (exp.length() == 0)
        return false;

    if (exp.find("/0") >= 0){
        if (exp.find("!") == 0)
            act = etaDROP;
        else
            act = etaACPT;

        ip = mask = 0x00000000;

        return true;
    }

    string str_ip = "", str_mask = "";
    //size_t pos = exp.find("/");
    int ip1,ip2,ip3,ip4,mask1,nip,nmask;
    char * eta;
    char * char_ip;
    char * char_mask;
    int pos = strspn(exp.c_str(),"/");
    fprintf(stderr,"%d\n",pos);
    cerr << pos << "\n";
    if (pos > 0 ) {
        //str_ip = exp.erase(pos);
        //str_mask = exp.substr(pos+1);
        sscanf(exp.c_str(),"%s%u.%u.%u.%u/%u",&eta,&ip1,&ip2,&ip3,&ip4,&mask1);
        nip = sprintf(char_ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
        nmask = sprintf(char_mask,"%u",mask1);
    } else {
        //str_ip = exp;
        sscanf(exp.c_str(),"%s%u.%u.%u.%u",&eta,&ip1,&ip2,&ip3,&ip4);
        nip = sprintf(char_ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
    }
    str_ip.append(nip,(char)*char_ip);
    str_mask.append(nmask,(char)*char_mask);
    fprintf(stderr,"%s %s",str_ip.c_str(),str_mask.c_str());
    //pos = str_ip.find("!");
    if (/*pos == 0*/eta == "!")
        act = etaDROP;
    else
        act = etaACPT;

    if (str_mask.length() > 0)
        mask = MaskForBits(atoi(str_mask.c_str())/* > 32?32:strtoul(str_mask.c_str(),NULL,0)*/);
    else
        mask = MaskForBits(32);
    cout << str_ip.substr(pos+1);
    ip = StringToUint32(str_ip.substr(pos+1));

    return true;
}

void IPFilter::addToRules(string exp, eDIRECTION direction) {
    uint32_t exp_ip, exp_mask;
    eTableAction act;

    if (!ParseString(exp, exp_ip, exp_mask, act))
        return;

    IPFilterElem *el = NULL;

    //if (list_ip.contains(exp_ip)) {
#ifdef _DEBUG_IPFILTER_
    //fprintf(stderr,"\tIP already in list\n");
#endif
        QIPHash::const_iterator it = list_ip.find(exp_ip);

        while (it != list_ip.end() && it->first == exp_ip){
            el = it->second;

            if ((el->direction != direction) && (el->action == act)){
#ifdef _DEBUG_IPFILTER_
                fprintf(stderr,"\tChange direction of IP\n");
#endif
                //emit ruleChanged(exp, el->direction, eDIRECTION_BOTH, act);

                el->direction = eDIRECTION_BOTH;

                return;
            }
            else if (el->direction == direction && el->action == act)
                return;

            ++it;
        }
    //}

    el = new IPFilterElem;

    el->mask      = exp_mask;
    el->ip        = exp_ip;
    el->direction = direction;
    el->action    = act;

#ifdef _DEBUG_IPFILTER_
    fprintf(stderr,"\tCreated new element:\n");
    fprintf(stderr,"\t\tMASK: 0x%x\n"
           "\t\tIP  : %s\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           el->mask, IPFilter::Uint32ToString(el->ip).c_str(),
           (int)el->direction, (int)el->action
          );
#endif

    list_ip.insert(pair<uint32_t, IPFilterElem*>(el->ip,el));
    rules.push_back(el);

    //emit ruleAdded(exp, direction);
}

void IPFilter::remFromRules(string exp, eTableAction act) {

    string str_ip;
    uint32_t exp_ip;
    //int c_pos = strspn(exp,"/");
    //int ip1,ip2,ip3,ip4,nip;
    //char * char_ip;
    //if (c_pos > 0) {
        //sscanf(exp.c_str(),"%u.%u.%u.%u/%u",&ip1,&ip2,&ip3,&ip4,&mask1)
        //nip = sprintf(char_ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
        //str_ip.append(nip,char_ip);
    //}
    size_t pos = exp.find("/");
    if (pos > 0)
        str_ip = exp.erase(pos);

    exp_ip = IPFilter::StringToUint32(str_ip);

    //if (!list_ip.contains(exp_ip))
        //return;

    QIPHash::/*const_*/iterator it = list_ip.find(exp_ip);
    IPFilterElem *el;

    while (it != list_ip.end() && it->first == exp_ip){
        el = it->second;

        if (!el)//i know that it's impossible..
            return;

        if (el->action == act){
            list_ip.erase(it);

            //if (rules.)
            QIPList::iterator itt = rules.begin();
            while (itt != rules.end() && *itt == el){
                    rules.erase(itt);
            }


            //emit ruleRemoved(exp, el->direction, el->action);

            delete el;

            return;
        }

        ++ it;
    }
}

void IPFilter::changeRuleDirection(string exp, eDIRECTION direction, eTableAction act) {
    //int c_pos = strspn(exp,"/");
    //int ip1,ip2,ip3,ip4,nip;
    //char * char_ip;
    //if (c_pos > 0) {
        //sscanf(exp.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4)
        //nip = sprintf(char_ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
        //str_ip.append(nip,char_ip);
    //}
    size_t pos = exp.find("/");
    if (pos > 0)
        exp = exp.erase(pos);

    uint32_t exp_ip = IPFilter::StringToUint32(exp);
    QIPHash::const_iterator it = list_ip.find(exp_ip);

    while (it != list_ip.end() && it->first == exp_ip){
        IPFilterElem *el = it->second;

        if (!el)
            return;

        if (el->action == act){
            //emit ruleChanged(exp, el->direction, direction, act);

            el->direction = direction;

            return;
        }

        ++it;
    }
}

bool IPFilter::OK(const string &exp, eDIRECTION direction){
#ifdef _DEBUG_IPFILTER_
    fprintf(stderr,"IPFilter::OK(%s,%i)\n",exp.c_str(),(int)direction);
#endif
    string str_src(exp);
    int c_pos = strspn(str_src.c_str(),":");
    int ip1,ip2,ip3,ip4,nip;
    char * char_ip;
    if (c_pos > 0) {
        sscanf(str_src.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
        nip = sprintf(char_ip,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);
        str_src.append(nip,(char )*char_ip);
    //size_t pos = str_src.find(":");
    ////if (pos > 0) {
        //str_src = str_src.erase(pos);
        ////XXX.XXX.XXX.XXX:PORT -> XXX.XXX.XXX.XXX
    }

    uint32_t src = IPFilter::StringToUint32(str_src);
    IPFilterElem *el;

    for (int i = 0; i < rules.size(); i++){
        el = rules.at(i);

#ifdef _DEBUG_IPFILTER_
    fprintf(stderr,"\tel->ip & el->mask == %x\n"
           "\tsrc    & el->mask == %x\n",
           (el->ip & el->mask),
           (src    & el->mask)
          );
#endif

        if ((el->ip & el->mask) == (src & el->mask)){//Exact match
            bool exact_direction = ((el->direction == direction) || (el->direction == eDIRECTION_BOTH));
#ifdef _DEBUG_IPFILTER_
            fprintf(stderr,"\tFound match... ");
#endif
            if      ((el->action == etaDROP) && exact_direction){
#ifdef _DEBUG_IPFILTER_
                fprintf(stderr,"DROP.\n");
#endif
                return false;
            }
            else if ((el->action == etaACPT) && exact_direction){
#ifdef _DEBUG_IPFILTER_
                fprintf(stderr,"ACCEPT.\n");
#endif
                return true;
            }
#ifdef _DEBUG_IPFILTER_
            else
                fprintf(stderr,"IGNORE.\n");
#endif
        }
    }

    return true;
}

void IPFilter::step(uint32_t ip, eTableAction act, bool down){
    IPFilterElem *el = NULL;

    QIPHash::const_iterator it = list_ip.find(ip);

    while (it != list_ip.end() && it->first == ip){
        if (it->second->action == act){
            el = it->second;

            break;
        }

        ++it;
    }

    if (!el)
        return;

    int index;
    //QIPList::iterator itt = rules.begin();
    int itt=0;
    while (itt < rules.size() && rules.at(itt) == el){
        index=itt;itt++;
    }
    int control = (down?(rules.size()-1):0);
    int inc = (down?1:-1);

    if ((index == control) || (index < 0))
        return;

    int new_index = index+inc;
    IPFilterElem *old_el = rules.at(new_index);

    rules[index]    = old_el;
    rules[new_index]= el;
}

void IPFilter::moveRuleUp(uint32_t ip, eTableAction act){
    step(ip, act, false);
}

void IPFilter::moveRuleDown(uint32_t ip, eTableAction act){
    step(ip, act, true);
}

bool IPFilter::isIP(string &exp) {
    //return (QRegExp("^(\\d{1,3}.){3,3}\\d{1,3}$").exactMatch(exp));
}

void IPFilter::loadList() {
    File file(Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter", File::READ, File::OPEN);
    string f = file.read();
    file.close();

    if (!list_ip.empty())
        clearRules();

    StringTokenizer<string> st(f, "\n");
    //bool bMatched = false;
    for (StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
        //bMatched = patternMatch(text, *i, useSet);
        //if (bMatched)
            //return true;
    //}
    //return bMatched;

    //while (!in.atEnd()) {
        //string pattern = in.readLine();
        string str_ip, str_mask = "";
        eDIRECTION direction = eDIRECTION_IN;

        //pattern.replace("\n", "");
        //pattern.replace(" ", "");
        str_ip = *i;
#ifdef _DEBUG_IPFILTER_
        fprintf(stderr,"%s\n",str_ip.c_str());
#endif
        //str_ip = *i;
        if (str_ip.find("|D_IN|:") == 0){
            str_ip = str_ip.erase(0,7);//delete "|D_IN|:"
        }
        else if (str_ip.find("|D_OUT|:") == 0) {//delete "|D_OUT|:"
            str_ip = str_ip.erase(0,8);
            direction = eDIRECTION_OUT;
        }
        else if (str_ip.find("|D_BOTH|:") == 0) {//delete "|D_BOTH|:"
            str_ip = str_ip.erase(0, 9);
            direction = eDIRECTION_BOTH;
        } else
            continue;

#ifdef _DEBUG_IPFILTER_
        fprintf(stderr,"%s\n",str_ip.c_str());
#endif

        addToRules(str_ip, direction);
    }

#ifdef _DEBUG_IPFILTER_
    //fprintf(stderr,""rules;
#endif

}

void IPFilter::saveList(){
    string file= Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter";
    File f(file, File::WRITE, File::CREATE | File::TRUNCATE);
    f.write(signature+"\n");

    for (int i = 0; i < rules.size(); i++) {
        string prefix;
        IPFilterElem *el = rules.at(i);

        eDIRECTION direction = el->direction;
        eTableAction act = el->action;

        prefix = (direction == eDIRECTION_IN?"|D_IN|:":(direction == eDIRECTION_OUT?"|D_OUT|:":"|D_BOTH|:"));
        prefix += string(act == etaACPT?"":"!");
        char * buffer;int n;string prefix1;
        //n = sprintf(buffer,"%lld",IPFilter::MaskToCIDR(el->mask));
        std::stringstream ss;
        ss << IPFilter::MaskToCIDR(el->mask);
        ss >> prefix1;
        f.write(prefix + IPFilter::Uint32ToString(el->ip).c_str() + "/" + prefix1.c_str() + "\n");
    }
    f.close();
}

void IPFilter::exportTo(string path) {
    string file = Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter";
    //File f(file, File::READ, File::OPEN);
    //QMessageBox msgBox;

    //msgBox.setStandardButtons(QMessageBox::Ok);
    //msgBox.setDefaultButton(QMessageBox::Ok);
    //msgBox.setIcon(QMessageBox::Warning);

    saveList();
    //if (stat(file.c_str(),&stFileInfo) != 0){
        //fprintf(stderr,"Nothing to export.");
        ////msgBox.setText(tr("Nothing to export."));
        ////msgBox.exec();

        //return;
    //}

    //File f(path, File::WRITE, File::CREATE | File::TRUNCATE);

    //if (stat(path.c_str(),&stFileInfo) == 0){
        File::deleteFile(path);

        try{
            File::copyFile(file,path);
        } catch (...) {
        //msgBox.setText(tr("Unable to export settings."));
        //msgBox.exec();
        fprintf(stderr,"Unable to export settings.");
        return;
        }
    //}
    //return;
}

void IPFilter::importFrom(string path) {
    //QFile file(path);
    //QMessageBox msgBox;

    //msgBox.setStandardButtons(QMessageBox::Ok);
    //msgBox.setDefaultButton(QMessageBox::Ok);
    //msgBox.setIcon(QMessageBox::Warning);

    //if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //if (stat(path.c_str(),&stFileInfo) == 0) {
        ////msgBox.setText(tr("Nothing to import."));
        ////msgBox.exec();
        ////file.close();
        //fprintf(stderr,"Nothing to export.");
        //return;
    //}
    File f(path, File::READ, File::OPEN);
    //QTextStream in(&file);
    string sign = f.read();//in.readLine();
    StringTokenizer<string> st(sign, "\n");
    //sign.replace("\n", "");

    f.close();

    if (*st.getTokens().begin() == signature) {
        string old_file = Util::getPath(Util::PATH_USER_CONFIG)+ "ipfilter";
        //old_file.remove();
        File::deleteFile(old_file);
        try{
            File::copyFile(path,Util::getPath(Util::PATH_USER_CONFIG) + "ipfilter");
        } catch (...) {}

        clearRules();

        loadList();
    } else {
        fprintf(stderr,"Invalid signature.");
        //msgBox.setText(tr("Invalid signature."));
        //msgBox.exec();
    }
}

const QIPList &IPFilter::getRules() {
    return rules;
}

const QIPHash &IPFilter::getHash() {
    return list_ip;
}

void IPFilter::clearRules(bool emit_signal) {
    //QIPHash::const_iterator it = list_ip.begin();

    //while (!list_ip.empty() && it != list_ip.end()){
        //if (it.value())
            //delete it.value();

        //++it;
    //}

    list_ip.clear();
    rules.clear();

    //if (emit_signal)
        //emit ruleRemoved("*", eDIRECTION_BOTH, etaACPT);
}
