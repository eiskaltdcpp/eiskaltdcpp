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

#include <string>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

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
typedef std::vector<IPFilterElem*> QIPList;

class ipfilter :
        public dcpp::Singleton<ipfilter>
{
    friend class dcpp::Singleton<ipfilter>;

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

    void load();
    void shutdown();
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
    ipfilter();
    /** */
    virtual ~ipfilter();

    /** */
    void step(uint32_t, eTableAction, bool down = true);
    /** */
    QIPHash list_ip;
    /** */
    QIPList rules;
};

#endif // DCIPFILTER_H
