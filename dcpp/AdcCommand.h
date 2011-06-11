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

#ifndef DCPLUSPLUS_DCPP_ADC_COMMAND_H
#define DCPLUSPLUS_DCPP_ADC_COMMAND_H

#include "typedefs.h"

#include "Exception.h"
#include "Util.h"

namespace dcpp {

STANDARD_EXCEPTION(ParseException);

class CID;

class AdcCommand {
public:
    template<uint32_t T>
    struct Type {
        enum { CMD = T };
    };

    enum Error {
        SUCCESS = 0,
        ERROR_GENERIC = 0,
        ERROR_HUB_GENERIC = 10,
        ERROR_HUB_FULL = 11,
        ERROR_HUB_DISABLED = 12,
        ERROR_LOGIN_GENERIC = 20,
        ERROR_NICK_INVALID = 21,
        ERROR_NICK_TAKEN = 22,
        ERROR_BAD_PASSWORD = 23,
        ERROR_CID_TAKEN = 24,
        ERROR_COMMAND_ACCESS = 25,
        ERROR_REGGED_ONLY = 26,
        ERROR_INVALID_PID = 27,
        ERROR_BANNED_GENERIC = 30,
        ERROR_PERM_BANNED = 31,
        ERROR_TEMP_BANNED = 32,
        ERROR_PROTOCOL_GENERIC = 40,
        ERROR_PROTOCOL_UNSUPPORTED = 41,
        ERROR_CONNECT_FAILED = 42,
        ERROR_INF_MISSING = 43,
        ERROR_BAD_STATE = 44,
        ERROR_FEATURE_MISSING = 45,
        ERROR_BAD_IP = 46,
        ERROR_NO_HUB_HASH = 47,
        ERROR_TRANSFER_GENERIC = 50,
        ERROR_FILE_NOT_AVAILABLE = 51,
        ERROR_FILE_PART_NOT_AVAILABLE = 52,
        ERROR_SLOTS_FULL = 53,
        ERROR_NO_CLIENT_HASH = 54
    };

    enum Severity {
        SEV_SUCCESS = 0,
        SEV_RECOVERABLE = 1,
        SEV_FATAL = 2
    };

    static const char TYPE_BROADCAST = 'B';
    static const char TYPE_CLIENT = 'C';
    static const char TYPE_DIRECT = 'D';
    static const char TYPE_ECHO = 'E';
    static const char TYPE_FEATURE = 'F';
    static const char TYPE_INFO = 'I';
    static const char TYPE_HUB = 'H';
    static const char TYPE_UDP = 'U';

#if defined(_WIN32) || defined(__i386__) || defined(__x86_64__) || defined(__alpha) || defined(__arm__)
#define C(n, a, b, c) static const uint32_t CMD_##n = (((uint32_t)a) | (((uint32_t)b)<<8) | (((uint32_t)c)<<16)); typedef Type<CMD_##n> n
#else
#define C(n, a, b, c) static const uint32_t CMD_##n = ((((uint32_t)a)<<24) | (((uint32_t)b)<<16) | (((uint32_t)c)<<8)); typedef Type<CMD_##n> n
#endif
    // Base commands
    C(SUP, 'S','U','P');
    C(STA, 'S','T','A');
    C(INF, 'I','N','F');
    C(MSG, 'M','S','G');
    C(SCH, 'S','C','H');
    C(RES, 'R','E','S');
    C(CTM, 'C','T','M');
    C(RCM, 'R','C','M');
    C(GPA, 'G','P','A');
    C(PAS, 'P','A','S');
    C(QUI, 'Q','U','I');
    C(GET, 'G','E','T');
    C(GFI, 'G','F','I');
    C(SND, 'S','N','D');
    C(SID, 'S','I','D');
    // Extensions
    C(CMD, 'C','M','D');
    C(PSR, 'P','S','R');
    C(PUB, 'P','U','B');
    C(NAT, 'N','A','T');
    C(RNT, 'R','N','T');
#undef C

    static const uint32_t HUB_SID = 0xffffffff;     // No client will have this sid
    static uint32_t toFourCC(const char* x) { return *reinterpret_cast<const uint32_t*>(x); }
    static std::string fromFourCC(uint32_t x) { return std::string(reinterpret_cast<const char*>(&x), sizeof(x)); }
    explicit AdcCommand(uint32_t aCmd, char aType = TYPE_CLIENT);
    explicit AdcCommand(uint32_t aCmd, const uint32_t aTarget, char aType);
    explicit AdcCommand(Severity sev, Error err, const string& desc, char aType = TYPE_CLIENT);
    explicit AdcCommand(const string& aLine, bool nmdc = false) throw(ParseException);
    void parse(const string& aLine, bool nmdc = false) throw(ParseException);

    uint32_t getCommand() const { return cmdInt; }
    char getType() const { return type; }
    void setType(char t) { type = t; }
    string getFourCC() const { string tmp(4, 0); tmp[0] = type; tmp[1] = cmd[0]; tmp[2] = cmd[1]; tmp[3] = cmd[2]; return tmp; }

    const string& getFeatures() const { return features; }
    AdcCommand& setFeatures(const string& feat) { features = feat; return *this; }

    StringList& getParameters() { return parameters; }
    const StringList& getParameters() const { return parameters; }

    string toString(const CID& aCID) const;
    string toString(uint32_t sid, bool nmdc = false) const;

    AdcCommand& addParam(const string& name, const string& value) {
        parameters.push_back(name);
        parameters.back() += value;
        return *this;
    }
    AdcCommand& addParam(const string& str) {
        parameters.push_back(str);
        return *this;
    }
    const string& getParam(size_t n) const {
        return getParameters().size() > n ? getParameters()[n] : Util::emptyString;
    }
    /** Return a named parameter where the name is a two-letter code */
    bool getParam(const char* name, size_t start, string& ret) const;
    bool hasFlag(const char* name, size_t start) const;
    static uint16_t toCode(const char* x) { return *((uint16_t*)x); }

    bool operator==(uint32_t aCmd) { return cmdInt == aCmd; }

    static string escape(const string& str, bool old);
    uint32_t getTo() const { return to; }
    AdcCommand& setTo(const uint32_t sid) { to = sid; return *this; }
    uint32_t getFrom() const { return from; }

    static uint32_t toSID(const string& aSID) { return *reinterpret_cast<const uint32_t*>(aSID.data()); }
    static string fromSID(const uint32_t aSID) { return string(reinterpret_cast<const char*>(&aSID), sizeof(aSID)); }
private:
    string getHeaderString(const CID& cid) const;
    string getHeaderString(uint32_t sid, bool nmdc) const;
    string getParamString(bool nmdc) const;
    StringList parameters;
    string features;
    union {
        char cmdChar[4];
        uint8_t cmd[4];
        uint32_t cmdInt;
    };
    uint32_t from;
    uint32_t to;
    char type;

};

template<class T>
class CommandHandler {
public:
    void dispatch(const string& aLine, bool nmdc = false) {
        try {
            AdcCommand c(aLine, nmdc);

#define C(n) case AdcCommand::CMD_##n: ((T*)this)->handle(AdcCommand::n(), c); break;
            switch(c.getCommand()) {
                C(SUP);
                C(STA);
                C(INF);
                C(MSG);
                C(SCH);
                C(RES);
                C(CTM);
                C(RCM);
                C(GPA);
                C(PAS);
                C(QUI);
                C(GET);
                C(GFI);
                C(SND);
                C(SID);
                C(CMD);
                C(PSR);
                C(NAT);
                C(RNT);
            default:
                dcdebug("Unknown ADC command: %.50s\n", aLine.c_str());
                break;
#undef C

            }
        } catch(const ParseException&) {
            dcdebug("Invalid ADC command: %.50s\n", aLine.c_str());
            return;
        }
    }
};

} // namespace dcpp

#endif // !defined(ADC_COMMAND_H)
