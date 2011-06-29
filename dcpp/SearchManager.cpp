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

#include "stdinc.h"

#include "SearchManager.h"
#include "UploadManager.h"
#include "format.h"
#include "ClientManager.h"
#include "ShareManager.h"
#include "SearchResult.h"
#include "LogManager.h"
#include "QueueManager.h"
#include "QueueItem.h"
#include "StringTokenizer.h"
#include "FinishedManager.h"

namespace dcpp {

const char* SearchManager::types[TYPE_LAST] = {
        N_("Any"),
        N_("Audio"),
        N_("Compressed"),
        N_("Document"),
        N_("Executable"),
        N_("Picture"),
        N_("Video"),
        N_("Directory"),
        N_("TTH"),
        N_("CD Image")
};
const char* SearchManager::getTypeStr(int type) {
        return _(types[type]);
}

SearchManager::SearchManager() :
    port(0),
    stop(false)
{
    queue.start();
}

SearchManager::~SearchManager() throw() {
    if(socket.get()) {
        stop = true;
        socket->disconnect();
#ifdef _WIN32
        join();
#endif
    }
}

string SearchManager::normalizeWhitespace(const string& aString){
    string::size_type found = 0;
    string normalized = aString;
    while((found = normalized.find_first_of("\t\n\r", found)) != string::npos) {
        normalized[found] = ' ';
        found++;
    }
    return normalized;
}

void SearchManager::search(const string& aName, int64_t aSize, TypeModes aTypeMode /* = TYPE_ANY */, SizeModes aSizeMode /* = SIZE_ATLEAST */, const string& aToken /* = Util::emptyString */, void* aOwner /* = NULL */) {
        ClientManager::getInstance()->search(aSizeMode, aSize, aTypeMode, normalizeWhitespace(aName), aToken, aOwner);
}

uint64_t SearchManager::search(StringList& who, const string& aName, int64_t aSize /* = 0 */, TypeModes aTypeMode /* = TYPE_ANY */, SizeModes aSizeMode /* = SIZE_ATLEAST */, const string& aToken /* = Util::emptyString */, const StringList& aExtList, void* aOwner /* = NULL */) {
        return ClientManager::getInstance()->search(who, aSizeMode, aSize, aTypeMode, normalizeWhitespace(aName), aToken, aExtList, aOwner);
}

void SearchManager::listen() throw(SocketException) {

    disconnect();

    try {
        socket.reset(new Socket);
        socket->create(Socket::TYPE_UDP);
        socket->setBlocking(true);
        socket->setSocketOpt(SO_REUSEADDR, 1);
        port = socket->bind(static_cast<uint16_t>(SETTING(UDP_PORT)), SETTING(BIND_ADDRESS));
        start();
    } catch(...) {
        socket.reset();
        throw;
    }
}

void SearchManager::disconnect() throw() {
    if(socket.get()) {
        stop = true;
        queue.shutdown();
        socket->disconnect();
        port = 0;

        join();

        socket.reset();

        stop = false;
    }
}

#define BUFSIZE 8192
int SearchManager::run() {
    boost::scoped_array<uint8_t> buf(new uint8_t[BUFSIZE]);
    int len;
    sockaddr_in remoteAddr = { 0 };

    while(!stop) {
        try {
            while( true ) {
                // Patch from FlylinkDC++, see:
                // http://code.google.com/p/flylinkdc/source/detail?r=6132
                // http://bazaar.launchpad.net/~dcplusplus-team/dcplusplus/trunk/revision/2414
                // .
                // @todo: remove this workaround for http://bugs.winehq.org/show_bug.cgi?id=22291
                // if that's fixed by reverting to simpler while (read(...) > 0) {...} code.
                while (socket->wait(400, Socket::WAIT_READ) != Socket::WAIT_READ);
                if (stop || (len = socket->read(&buf[0], BUFSIZE, remoteAddr)) <= 0)
                    break;
                onData(&buf[0], len, inet_ntoa(remoteAddr.sin_addr));
            }
        } catch(const SocketException& e) {
            dcdebug("SearchManager::run Error: %s\n", e.getError().c_str());
        }

        bool failed = false;
        while(!stop) {
            try {
                socket->disconnect();
                socket->create(Socket::TYPE_UDP);
                socket->setBlocking(true);
                socket->bind(port, SETTING(BIND_ADDRESS));
                if(failed) {
                    LogManager::getInstance()->message(_("Search enabled again"));
                    failed = false;
                }
                break;
            } catch(const SocketException& e) {
                dcdebug("SearchManager::run Stopped listening: %s\n", e.getError().c_str());

                if(!failed) {
                    LogManager::getInstance()->message(str(F_("Search disabled: %1%") % e.getError()));
                    failed = true;
                }

                // Spin for 60 seconds
                for(int i = 0; i < 60 && !stop; ++i) {
                    Thread::sleep(1000);
                }
            }
        }
    }
    return 0;
}

int SearchManager::UdpQueue::run() {
    string x = Util::emptyString;
    string remoteIp = Util::emptyString;
    stop = false;
;
    while(true) {
        if (resultList.empty())
            s.wait();

        if(stop)
            break;

        {
            Lock l(csudp);
            if(resultList.empty()) continue;

            x = resultList.front().first;
            remoteIp = resultList.front().second;
            resultList.pop_front();
        }

    if(x.compare(0, 4, "$SR ") == 0) {
        string::size_type i, j;
        // Directories: $SR <nick><0x20><directory><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
        // Files:       $SR <nick><0x20><filename><0x05><filesize><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
        i = 4;
        if( (j = x.find(' ', i)) == string::npos) {
            continue;
        }
        string nick = x.substr(i, j-i);
        i = j + 1;

        // A file has 2 0x05, a directory only one
        size_t cnt = count(x.begin() + j, x.end(), 0x05);

        SearchResult::Types type = SearchResult::TYPE_FILE;
        string file;
        int64_t size = 0;

        if(cnt == 1) {
            // We have a directory...find the first space beyond the first 0x05 from the back
            // (dirs might contain spaces as well...clever protocol, eh?)
            type = SearchResult::TYPE_DIRECTORY;
            // Get past the hubname that might contain spaces
            if((j = x.rfind(0x05)) == string::npos) {
                continue;
            }
            // Find the end of the directory info
            if((j = x.rfind(' ', j-1)) == string::npos) {
                continue;
            }
            if(j < i + 1) {
                continue;
            }
            file = x.substr(i, j-i) + '\\';
        } else if(cnt == 2) {
            if( (j = x.find((char)5, i)) == string::npos) {
                continue;
            }
            file = x.substr(i, j-i);
            i = j + 1;
            if( (j = x.find(' ', i)) == string::npos) {
                continue;
            }
            size = Util::toInt64(x.substr(i, j-i));
        }
        i = j + 1;

        if( (j = x.find('/', i)) == string::npos) {
            continue;
        }
        uint8_t freeSlots = (uint8_t)Util::toInt(x.substr(i, j-i));
        i = j + 1;
        if( (j = x.find((char)5, i)) == string::npos) {
            continue;
        }
        uint8_t slots = (uint8_t)Util::toInt(x.substr(i, j-i));
        i = j + 1;
        if( (j = x.rfind(" (")) == string::npos) {
            continue;
        }
        string hubName = x.substr(i, j-i);
        i = j + 2;
        if( (j = x.rfind(')')) == string::npos) {
            continue;
        }

        string hubIpPort = x.substr(i, j-i);
        string url = ClientManager::getInstance()->findHub(hubIpPort);

        string encoding = ClientManager::getInstance()->findHubEncoding(url);
        nick = Text::toUtf8(nick, encoding);
        file = Text::toUtf8(file, encoding);
        hubName = Text::toUtf8(hubName, encoding);

        UserPtr user = ClientManager::getInstance()->findUser(nick, url);
        if(!user) {
            // Could happen if hub has multiple URLs / IPs
            user = ClientManager::getInstance()->findLegacyUser(nick);
            if(!user)
                continue;
        }

        ClientManager::getInstance()->setIPUser(user, remoteIp);

        string tth;
        if(hubName.compare(0, 4, "TTH:") == 0) {
            tth = hubName.substr(4);
                        StringList names = ClientManager::getInstance()->getHubNames(user->getCID(), Util::emptyString);
            hubName = names.empty() ? _("Offline") : Util::toString(names);
        }

        if(tth.empty() && type == SearchResult::TYPE_FILE) {
            continue;
        }

        SearchResultPtr sr(new SearchResult(user, type, slots, freeSlots, size,
                        file, hubName, url, remoteIp, TTHValue(tth), Util::emptyString));
        SearchManager::getInstance()->fire(SearchManagerListener::SR(), sr);

    } else if(x.compare(1, 4, "RES ") == 0 && x[x.length() - 1] == 0x0a) {
        AdcCommand c(x.substr(0, x.length()-1));
        if(c.getParameters().empty())
            continue;
        string cid = c.getParam(0);
        if(cid.size() != 39)
            continue;

        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
        if(!user)
            continue;

        // This should be handled by AdcCommand really...
        c.getParameters().erase(c.getParameters().begin());

        SearchManager::getInstance()->onRES(c, user, remoteIp);

    } if(x.compare(1, 4, "PSR ") == 0 && x[x.length() - 1] == 0x0a) {
            AdcCommand c(x.substr(0, x.length()-1));
            if(c.getParameters().empty())
                    continue;
            string cid = c.getParam(0);
            if(cid.size() != 39)
                    continue;

            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
            // when user == NULL then it is probably NMDC user, check it later

            c.getParameters().erase(c.getParameters().begin());

            SearchManager::getInstance()->onPSR(c, user, remoteIp);

    } /*else if(x.compare(1, 4, "SCH ") == 0 && x[x.length() - 1] == 0x0a) {
        try {
            respond(AdcCommand(x.substr(0, x.length()-1)));
        } catch(ParseException& ) {
        }
    }*/ // Needs further DoS investigation


                Thread::sleep(10);
        }
        return 0;
}

void SearchManager::onData(const uint8_t* buf, size_t aLen, const string& remoteIp) {
    string x((char*)buf, aLen);
    queue.addResult(x, remoteIp);
}

void SearchManager::onRES(const AdcCommand& cmd, const UserPtr& from, const string& remoteIp) {
    int freeSlots = -1;
    int64_t size = -1;
    string file;
    string tth;
    string token;

    for(StringIterC i = cmd.getParameters().begin(); i != cmd.getParameters().end(); ++i) {
        const string& str = *i;
        if(str.compare(0, 2, "FN") == 0) {
            file = Util::toNmdcFile(str.substr(2));
        } else if(str.compare(0, 2, "SL") == 0) {
            freeSlots = Util::toInt(str.substr(2));
        } else if(str.compare(0, 2, "SI") == 0) {
            size = Util::toInt64(str.substr(2));
        } else if(str.compare(0, 2, "TR") == 0) {
            tth = str.substr(2);
        } else if(str.compare(0, 2, "TO") == 0) {
            token = str.substr(2);
        }
    }

    if(!file.empty() && freeSlots != -1 && size != -1) {

        /// @todo get the hub this was sent from, to be passed as a hint? (eg by using the token?)
        StringList names = ClientManager::getInstance()->getHubNames(from->getCID(), Util::emptyString);
        string hubName = names.empty() ? _("Offline") : Util::toString(names);
        StringList hubs = ClientManager::getInstance()->getHubs(from->getCID(), Util::emptyString);
        string hub = hubs.empty() ? _("Offline") : Util::toString(hubs);

        SearchResult::Types type = (file[file.length() - 1] == '\\' ? SearchResult::TYPE_DIRECTORY : SearchResult::TYPE_FILE);
        if(type == SearchResult::TYPE_FILE && tth.empty())
                return;

        uint8_t slots = ClientManager::getInstance()->getSlots(from->getCID());
        SearchResultPtr sr(new SearchResult(from, type, slots, (uint8_t)freeSlots, size,
                file, hubName, hub, remoteIp, TTHValue(tth), token));
        fire(SearchManagerListener::SR(), sr);
    }
}

void SearchManager::onPSR(const AdcCommand& cmd, UserPtr from, const string& remoteIp) {

    uint16_t udpPort = 0;
    uint32_t partialCount = 0;
    string tth;
    string hubIpPort;
    string nick;
    PartsInfo partialInfo;

    for(StringIterC i = cmd.getParameters().begin(); i != cmd.getParameters().end(); ++i) {
            const string& str = *i;
            if(str.compare(0, 2, "U4") == 0) {
                    udpPort = static_cast<uint16_t>(Util::toInt(str.substr(2)));
            } else if(str.compare(0, 2, "NI") == 0) {
                    nick = str.substr(2);
            } else if(str.compare(0, 2, "HI") == 0) {
                    hubIpPort = str.substr(2);
            } else if(str.compare(0, 2, "TR") == 0) {
                    tth = str.substr(2);
            } else if(str.compare(0, 2, "PC") == 0) {
                    partialCount = Util::toUInt32(str.substr(2))*2;
            } else if(str.compare(0, 2, "PI") == 0) {
                    StringTokenizer<string> tok(str.substr(2), ',');
                    for(StringIter i = tok.getTokens().begin(); i != tok.getTokens().end(); ++i) {
                            partialInfo.push_back((uint16_t)Util::toInt(*i));
                    }
            }
    }

    string url = ClientManager::getInstance()->findHub(hubIpPort);
    if(!from || from == ClientManager::getInstance()->getMe()) {
            // for NMDC support

            if(nick.empty() || hubIpPort.empty()) {
                    return;
            }

            from = ClientManager::getInstance()->findUser(nick, url);
            if(!from) {
                    // Could happen if hub has multiple URLs / IPs
                    from = ClientManager::getInstance()->findLegacyUser(nick);
                    if(!from) {
                            dcdebug("Search result from unknown user");
                            return;
                    }
            }
    }

    ClientManager::getInstance()->setIPUser(from, remoteIp, udpPort);

    if(partialInfo.size() != partialCount) {
            // what to do now ? just ignore partial search result :-/
            return;
    }

    PartsInfo outPartialInfo;
    QueueItem::PartialSource ps(from->isNMDC() ? ClientManager::getInstance()->getClient(url)->getMyIdentity().getNick() : Util::emptyString, hubIpPort, remoteIp, udpPort);
    ps.setPartialInfo(partialInfo);

    QueueManager::getInstance()->handlePartialResult(from, url, TTHValue(tth), ps, outPartialInfo);

    if((udpPort > 0) && !outPartialInfo.empty()) {
        try {
            AdcCommand cmd = SearchManager::getInstance()->toPSR(false, ps.getMyNick(), hubIpPort, tth, outPartialInfo);
            ClientManager::getInstance()->send(cmd, from->getCID());
        } catch(...) {
            dcdebug("Partial search caught error\n");
        }
    }

}

void SearchManager::respond(const AdcCommand& adc, const CID& from,  bool isUdpActive, const string& hubIpPort) {
    // Filter own searches
    if(from == ClientManager::getInstance()->getMe()->getCID())
        return;

    UserPtr p = ClientManager::getInstance()->findUser(from);
    if(!p)
        return;

    SearchResultList results;
    ShareManager::getInstance()->search(results, adc.getParameters(), isUdpActive ? 10 : 5);

    string token;

    adc.getParam("TO", 0, token);

    // TODO: don't send replies to passive users
    if(results.empty()) {
        string tth;
        if(!adc.getParam("TR", 0, tth))
                return;

        PartsInfo partialInfo;
        if(!QueueManager::getInstance()->handlePartialSearch(TTHValue(tth), partialInfo)) {
                // if not found, try to find in finished list
                if(!FinishedManager::getInstance()->handlePartialRequest(TTHValue(tth), partialInfo)) {
                        return;
                }
        }

        AdcCommand cmd = toPSR(true, Util::emptyString, hubIpPort, tth, partialInfo);
        ClientManager::getInstance()->send(cmd, from);
        return;
    }

    for(SearchResultList::const_iterator i = results.begin(); i != results.end(); ++i) {
        AdcCommand cmd = (*i)->toRES(AdcCommand::TYPE_UDP);
        if(!token.empty())
            cmd.addParam("TO", token);
        ClientManager::getInstance()->send(cmd, from);
    }
}

string SearchManager::getPartsString(const PartsInfo& partsInfo) const {
        string ret;

        for(PartsInfo::const_iterator i = partsInfo.begin(); i < partsInfo.end(); i+=2){
                ret += Util::toString(*i) + "," + Util::toString(*(i+1)) + ",";
        }

        return ret.substr(0, ret.size()-1);
}

AdcCommand SearchManager::toPSR(bool wantResponse, const string& myNick, const string& hubIpPort, const string& tth, const vector<uint16_t>& partialInfo) const {
        AdcCommand cmd(AdcCommand::CMD_PSR, AdcCommand::TYPE_UDP);

        if(!myNick.empty())
                cmd.addParam("NI", Text::utf8ToAcp(myNick));

        cmd.addParam("HI", hubIpPort);
        cmd.addParam("U4", Util::toString(wantResponse && ClientManager::getInstance()->isActive(hubIpPort) ? SearchManager::getInstance()->getPort() : 0));
        cmd.addParam("TR", tth);
        cmd.addParam("PC", Util::toString(partialInfo.size() / 2));
        cmd.addParam("PI", getPartsString(partialInfo));

        return cmd;
}

} // namespace dcpp
