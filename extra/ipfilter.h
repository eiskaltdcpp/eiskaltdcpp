/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <string>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"

enum eDIRECTION {
    eDIRECTION_IN = 0,
    eDIRECTION_OUT,
    eDIRECTION_BOTH
};

enum eTableAction {
    etaDROP=0,
    etaACPT
};

typedef struct _IPFilterElem {
    uint32_t ip;
    uint32_t mask;

    eDIRECTION direction;
    eTableAction action;
} IPFilterElem;

typedef std::unordered_map<uint32_t, IPFilterElem*> IPHash;
typedef std::vector<IPFilterElem*> IPList;

class ipfilter :
        public dcpp::Singleton<ipfilter>
{
    friend class dcpp::Singleton<ipfilter>;

public:
    static uint32_t StringToUint32(const std::string&);
    static std::string Uint32ToString(uint32_t);
    static uint32_t MaskToCIDR(uint32_t);
    static uint32_t MaskForBits(uint32_t);
    static bool ParseString(std::string, uint32_t&, uint32_t&, eTableAction&);

    void load();
    void shutdown();
    void loadList();
    void saveList();

    const IPList &getRules();
    const IPHash &getHash ();

    void addToRules(const std::string &exp, eDIRECTION direction);
    void remFromRules(std::string exp, eTableAction);
    void changeRuleDirection(std::string exp, eDIRECTION, eTableAction);
    void clearRules();

    void moveRuleUp(uint32_t, eTableAction);
    void moveRuleDown(uint32_t, eTableAction);

    bool OK(const std::string &exp, eDIRECTION direction);

    void exportTo(std::string path, std::string& error);
    void importFrom(std::string path, std::string& error);

    void step(uint32_t, eTableAction, bool down = true);

private:
    ipfilter();
    virtual ~ipfilter();


    IPHash list_ip;
    IPList rules;
};
