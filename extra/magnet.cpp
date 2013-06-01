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
        if(Util::strnicmp(uri.c_str(), "magnet:?", 8)) {
                return false;
        }

        // official types that are of interest to us
        //  xt = exact topic
        //  xs = exact substitute
        //  as = acceptable substitute
        //  dn = display name
        //  kt = keyword topic
        //  xl = exact length
        //  dl = display length
        //  mt (Manifest Topic) — Ссылка на метафайл, который содержит список магнетов (MAGMA)

        StringTokenizer<string> mag(uri.substr(8), '&');
        StringMap hashes;
        string type, param;
        for(auto& idx: mag.getTokens()) {
            auto pos = idx.find('=');
            if(pos != string::npos) {
                type = Text::toLower(Util::encodeURI(idx.substr(0, pos), true));
                param = Util::encodeURI(idx.substr(pos + 1), true);
            } else {
                type = Util::encodeURI(idx, true);
                param.clear();
            }

            // extract what is of value
            if(param.size() == 85 && ::strncmp(param.c_str(), "urn:bitprint:", 13) == 0) {
                    hashes[type] = param.substr(46);
            } else if(param.size() == 54 && ::strncmp(param.c_str(), "urn:tree:tiger:", 15) == 0) {
                    hashes[type] = param.substr(15);
            } else if(param.size() == 55 && ::strncmp(param.c_str(), "urn:tree:tiger/:", 16) == 0) {
                    hashes[type] = param.substr(16);
            } else if(param.size() == 59 && ::strncmp(param.c_str(), "urn:tree:tiger/1024:", 20) == 0) {
                    hashes[type] = param.substr(20);
            } else if(type.size() == 2 && ::strncmp(type.c_str(), "dn", 2) == 0) {
                    params["dn"] = param;
            } else if(type.size() == 2 && ::strncmp(type.c_str(), "kt", 2) == 0) {
                    params["kt"] = param;
            } else if(type.size() == 2 && ::strncmp(type.c_str(), "xl", 2) == 0) {
                params["xl"] = param;
            }
        }

        if(hashes.find("xt") != hashes.end()) {
            params["xt"] = hashes["xt"];
        } else if(hashes.find("xs") != hashes.end()) {
            params["xs"] = hashes["xs"];
        } else if(hashes.find("as") != hashes.end()) {
            params["as"]= hashes["as"];
        }

        if(!params["xt"].empty() || !params["xs"].empty() || !params["as"].empty() || !params["kt"].empty()) {
            return true;
        }
        return false;
}

} // namespace dcpp
