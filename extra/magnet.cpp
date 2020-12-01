/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dcpp/stdinc.h"
#include "magnet.h"

#include "dcpp/StringTokenizer.h"
#include "dcpp/Text.h"
#include "dcpp/Util.h"

namespace dcpp {

bool magnet::parseUri(const string& uri, StringMap& params) {
        if(::strncmp(uri.c_str(), "magnet:?", 8)) {
                return false;
        }

        // Widely used parameters:
        // xt = eXact Topic                — URN containing file hash
        // xs = eXact Source               — P2P link
        // as = Acceptable Source          — web link to file on the Internet
        // dn = Display Name               — file name
        // kt = Keyword Topic              — search keywords
        // xl = eXact Length               — real file size in bytes
        // dl = Display Length             — size in bytes which should be displayed to user
        // mt = Manifest Topic             — link to a metafile that contains a list of magnets (MAGMA)
        // tr = TRacker                    — tracker address for BitTorrent clients

        StringTokenizer<string> mag(uri.substr(8), '&');
        StringMap hashes;
        string type, param;
        for(auto& idx : mag.getTokens()) {
            auto pos = idx.find('=');
            if(pos != string::npos) {
                type = Text::toLower(Util::encodeURI(idx.substr(0, pos), true));
                param = Util::encodeURI(idx.substr(pos + 1), true);
            } else {
                type = Util::encodeURI(idx, true);
                param.clear();
            }

            static const StringSet supportedParams = { "dn", "kt", "xl", "dl", "mt", "tr" };

            // extract what is of value
            if(param.size() == 85 && ::strncmp(param.c_str(), "urn:bitprint:", 13) == 0) {
                    hashes[type] = param.substr(46);
            } else if(param.size() == 54 && ::strncmp(param.c_str(), "urn:tree:tiger:", 15) == 0) {
                    hashes[type] = param.substr(15);
            } else if(param.size() == 55 && ::strncmp(param.c_str(), "urn:tree:tiger/:", 16) == 0) {
                    hashes[type] = param.substr(16);
            } else if(param.size() == 59 && ::strncmp(param.c_str(), "urn:tree:tiger/1024:", 20) == 0) {
                    hashes[type] = param.substr(20);
            } else if(param.size() == 49 && ::strncmp(param.c_str(), "urn:btih:", 9) == 0) {
                    hashes[type] = param.substr(9);
            } else if(param.size() == 77 && ::strncmp(param.c_str(), "urn:btmh:", 9) == 0) {
                    hashes[type] = param.substr(9);
            } else if(type.size() == 2 && supportedParams.find(type) != supportedParams.end()) {
                    params[type] = param;
            }
        }

        static const StringSet specialParams = { "xt", "xs", "as" };

        // add special params
        for(auto& key : specialParams) {
            if(hashes.find(key) != hashes.end()) {
                params[key] = hashes[key];
            }
        }

        if(!params["xt"].empty() || !params["xs"].empty() || !params["as"].empty() || !params["kt"].empty()) {
            return true;
        }
        return false;
}

} // namespace dcpp
