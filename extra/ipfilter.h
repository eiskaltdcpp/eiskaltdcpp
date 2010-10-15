/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DCIPFILTER_H
#define DCIPFILTER_H

//#include <QObject>
//#include <QRegExp>
//#include <QHash>
//#include <QFile>
//#include <hash_map>
#include <string>
#include <sstream>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

//#ifdef _DEBUG_
//#include <QtDebug>
//#endif

using namespace dcpp;

enum eDIRECTION {
    eDIRECTION_IN = 0,
    eDIRECTION_OUT,
    eDIRECTION_BOTH
};

enum eTableAction{
    etaDROP=0,
    etaACPT
};

typedef struct _IPFilterElem{
    uint32_t ip;
    uint32_t mask;

    eDIRECTION direction;
    eTableAction action;
} IPFilterElem;

typedef std::map<uint32_t, IPFilterElem*> QIPHash;
//typedef std::vector<string> StringList;
typedef std::vector<IPFilterElem*> QIPList;

class IPFilter :
        public dcpp::Singleton<IPFilter>
{
    friend class dcpp::Singleton<IPFilter>;

public:
    /** */
    static uint32_t StringToUint32(const std::string&);
    /** */
    static std::string Uint32ToString(uint32_t);
    /** */
    static uint32_t MaskToCIDR(uint32_t);
    /** */
    static uint32_t MaskForBits(uint32_t);
    /** */
    static bool ParseString(std::string, uint32_t&, uint32_t&, eTableAction&);
    /** */
    static bool isIP(std::string &exp);

    /** */
    void loadList();
    /** */
    void saveList();

    /** */
    const QIPList &getRules();
    /** */
    const QIPHash &getHash ();

    /** */
    void addToRules(std::string exp, eDIRECTION direction);
    /** */
    void remFromRules(std::string exp, eTableAction);
    /** */
    void changeRuleDirection(std::string exp, eDIRECTION, eTableAction);
    /** */
    void clearRules(bool emit_signal = true);

    /** */
    void moveRuleUp(uint32_t, eTableAction);
    /** */
    void moveRuleDown(uint32_t, eTableAction);

    /** */
    bool OK(const std::string &exp, eDIRECTION direction);

    /** */
    void exportTo(std::string path);
    /** */
    void importFrom(std::string path);

#ifdef _DEBUG_
    void printHash();
#endif

private:
    /** */
    IPFilter();
    /** */
    virtual ~IPFilter();

    /** */
    void step(uint32_t, eTableAction, bool down = true);
    /** */
    QIPHash list_ip;
    /** */
    QIPList rules;

//signals:
    //void ruleAdded(std::string, eDIRECTION);
    //void ruleRemoved(std::string, eDIRECTION, eTableAction);
    //void ruleChanged(std::string, eDIRECTION, eDIRECTION, eTableAction);
};

#endif // DCIPFILTER_H
