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
#include "DCPlusPlus.h"

#include "FavoriteManager.h"

#include "ClientManager.h"
#include "CryptoManager.h"
#include "WindowManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "UserCommand.h"
#include "WindowInfo.h"
#include "File.h"
#include "BZUtils.h"
#include "FilteredFile.h"

namespace dcpp {

FavoriteManager::FavoriteManager() : lastId(0), useHttp(false), running(false), c(NULL), lastServer(0), listType(TYPE_NORMAL), dontSave(false) {
    SettingsManager::getInstance()->addListener(this);
    ClientManager::getInstance()->addListener(this);

    File::ensureDirectory(Util::getHubListsPath());
}

FavoriteManager::~FavoriteManager() throw() {
    ClientManager::getInstance()->removeListener(this);
    SettingsManager::getInstance()->removeListener(this);
    if(c) {
        c->removeListener(this);
        delete c;
        c = NULL;
    }

    for_each(favoriteHubs.begin(), favoriteHubs.end(), DeleteFunction());
}

UserCommand FavoriteManager::addUserCommand(int type, int ctx, int flags, const string& name, const string& command, const string& to, const string& hub) {
    // No dupes, add it...
    Lock l(cs);
        userCommands.push_back(UserCommand(lastId++, type, ctx, flags, name, command, to, hub));
    UserCommand& uc = userCommands.back();
    if(!uc.isSet(UserCommand::FLAG_NOSAVE))
        save();
    return userCommands.back();
}

bool FavoriteManager::getUserCommand(int cid, UserCommand& uc) {
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
        if(i->getId() == cid) {
            uc = *i;
            return true;
        }
    }
    return false;
}

bool FavoriteManager::moveUserCommand(int cid, int pos) {
    dcassert(pos == -1 || pos == 1);
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
        if(i->getId() == cid) {
            swap(*i, *(i + pos));
            return true;
        }
    }
    return false;
}

void FavoriteManager::updateUserCommand(const UserCommand& uc) {
    bool nosave = true;
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
        if(i->getId() == uc.getId()) {
            *i = uc;
            nosave = uc.isSet(UserCommand::FLAG_NOSAVE);
            break;
        }
    }
    if(!nosave)
        save();
}

int FavoriteManager::findUserCommand(const string& aName, const string& aUrl) {
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
        if(i->getName() == aName && i->getHub() == aUrl) {
            return i->getId();
        }
    }
    return -1;
}

void FavoriteManager::removeUserCommand(int cid) {
    bool nosave = true;
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
        if(i->getId() == cid) {
            nosave = i->isSet(UserCommand::FLAG_NOSAVE);
            userCommands.erase(i);
            break;
        }
    }
    if(!nosave)
        save();
}
void FavoriteManager::removeUserCommand(const string& srv) {
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ) {
        if((i->getHub() == srv) && i->isSet(UserCommand::FLAG_NOSAVE)) {
            i = userCommands.erase(i);
        } else {
            ++i;
        }
    }
}

void FavoriteManager::removeHubUserCommands(int ctx, const string& hub) {
    Lock l(cs);
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ) {
        if(i->getHub() == hub && i->isSet(UserCommand::FLAG_NOSAVE) && i->getCtx() & ctx) {
            i = userCommands.erase(i);
        } else {
            ++i;
        }
    }
}

void FavoriteManager::addFavoriteUser(const UserPtr& aUser) {
    Lock l(cs);
    if(users.find(aUser->getCID()) == users.end()) {
                StringList urls = ClientManager::getInstance()->getHubs(aUser->getCID(), Util::emptyString);
                StringList nicks = ClientManager::getInstance()->getNicks(aUser->getCID(), Util::emptyString);

        /// @todo make this an error probably...
        if(urls.empty())
            urls.push_back(Util::emptyString);
        if(nicks.empty())
            nicks.push_back(Util::emptyString);

        FavoriteMap::iterator i = users.insert(make_pair(aUser->getCID(), FavoriteUser(aUser, nicks[0], urls[0]))).first;
        fire(FavoriteManagerListener::UserAdded(), i->second);
        save();
    }
}

void FavoriteManager::removeFavoriteUser(const UserPtr& aUser) {
    Lock l(cs);
    FavoriteMap::iterator i = users.find(aUser->getCID());
    if(i != users.end()) {
        fire(FavoriteManagerListener::UserRemoved(), i->second);
        users.erase(i);
        save();
    }
}

std::string FavoriteManager::getUserURL(const UserPtr& aUser) const {
    Lock l(cs);
    FavoriteMap::const_iterator i = users.find(aUser->getCID());
    if(i != users.end()) {
        const FavoriteUser& fu = i->second;
        return fu.getUrl();
    }
    return Util::emptyString;
}

void FavoriteManager::addFavorite(const FavoriteHubEntry& aEntry) {
    FavoriteHubEntry* f;

    FavoriteHubEntryList::iterator i = getFavoriteHub(aEntry.getServer());
    if(i != favoriteHubs.end()) {
        return;
    }
    f = new FavoriteHubEntry(aEntry);
    favoriteHubs.push_back(f);
    fire(FavoriteManagerListener::FavoriteAdded(), f);
    save();
}

void FavoriteManager::removeFavorite(FavoriteHubEntry* entry) {
    FavoriteHubEntryList::iterator i = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
    if(i == favoriteHubs.end()) {
        return;
    }

    fire(FavoriteManagerListener::FavoriteRemoved(), entry);
    favoriteHubs.erase(i);
    delete entry;
    save();
}

bool FavoriteManager::isFavoriteHub(const std::string& url) {
    FavoriteHubEntryList::iterator i = getFavoriteHub(url);
    if(i != favoriteHubs.end()) {
        return true;
    }
    return false;
}

bool FavoriteManager::addFavoriteDir(const string& aDirectory, const string & aName){
    string path = aDirectory;

    if( path[ path.length() -1 ] != PATH_SEPARATOR )
        path += PATH_SEPARATOR;

    for(StringPairIter i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i) {
        if((Util::strnicmp(path, i->first, i->first.length()) == 0) && (Util::strnicmp(path, i->first, path.length()) == 0)) {
            return false;
        }
        if(Util::stricmp(aName, i->second) == 0) {
            return false;
        }
    }
    favoriteDirs.push_back(make_pair(aDirectory, aName));
    save();
    return true;
}

bool FavoriteManager::removeFavoriteDir(const string& aName) {
    string d(aName);

    if(d[d.length() - 1] != PATH_SEPARATOR)
        d += PATH_SEPARATOR;

    for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
        if(Util::stricmp(j->first.c_str(), d.c_str()) == 0) {
            favoriteDirs.erase(j);
            save();
            return true;
        }
    }
    return false;
}

bool FavoriteManager::renameFavoriteDir(const string& aName, const string& anotherName) {

    for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
        if(Util::stricmp(j->second.c_str(), aName.c_str()) == 0) {
            j->second = anotherName;
            save();
            return true;
        }
    }
    return false;
}

class XmlListLoader : public SimpleXMLReader::CallBack {
public:
    XmlListLoader(HubEntryList& lst) : publicHubs(lst) { }
    virtual ~XmlListLoader() { }
    virtual void startTag(const string& name, StringPairList& attribs, bool) {
        if(name == "Hub") {
            const string& name = getAttrib(attribs, "Name", 0);
            const string& server = getAttrib(attribs, "Address", 1);
            const string& description = getAttrib(attribs, "Description", 2);
            const string& users = getAttrib(attribs, "Users", 3);
            const string& country = getAttrib(attribs, "Country", 4);
            const string& shared = getAttrib(attribs, "Shared", 5);
            const string& minShare = getAttrib(attribs, "Minshare", 5);
            const string& minSlots = getAttrib(attribs, "Minslots", 5);
            const string& maxHubs = getAttrib(attribs, "Maxhubs", 5);
            const string& maxUsers = getAttrib(attribs, "Maxusers", 5);
            const string& reliability = getAttrib(attribs, "Reliability", 5);
            const string& rating = getAttrib(attribs, "Rating", 5);
            publicHubs.push_back(HubEntry(name, server, description, users, country, shared, minShare, minSlots, maxHubs, maxUsers, reliability, rating));
        }
    }
    virtual void endTag(const string&, const string&) {

    }
private:
    HubEntryList& publicHubs;
};

bool FavoriteManager::onHttpFinished(bool fromHttp) throw() {
        MemoryInputStream mis(downloadBuf);
        bool success = true;

        Lock l(cs);
        HubEntryList& list = publicListMatrix[publicListServer];
        list.clear();

    try {
                XmlListLoader loader(list);

                if((listType == TYPE_BZIP2) && (!downloadBuf.empty())) {
                        FilteredInputStream<UnBZFilter, false> f(&mis);
                        SimpleXMLReader(&loader).parse(f);
                } else {
                        SimpleXMLReader(&loader).parse(mis);
                }
        } catch(const Exception&) {
                success = false;
                fire(FavoriteManagerListener::Corrupted(), fromHttp ? publicListServer : Util::emptyString);
        }

    if(fromHttp) {
                try {
                        File f(Util::getHubListsPath() + Util::validateFileName(publicListServer), File::WRITE, File::CREATE | File::TRUNCATE);
                        f.write(downloadBuf);
                        f.close();
                } catch(const FileException&) { }
    }

        downloadBuf = Util::emptyString;

        return success;
}

void FavoriteManager::save() {
    if(dontSave)
        return;

    Lock l(cs);
    try {
        SimpleXML xml;

        xml.addTag("Favorites");
        xml.stepIn();

        xml.addTag("Hubs");
        xml.stepIn();

        for(FavHubGroups::const_iterator i = favHubGroups.begin(), iend = favHubGroups.end(); i != iend; ++i) {
                xml.addTag("Group");
                xml.addChildAttrib("Name", i->first);
                xml.addChildAttrib("Private", i->second.priv);
                xml.addChildAttrib("Connect", i->second.connect);
        }

        for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i) {
            xml.addTag("Hub");
            xml.addChildAttrib("Name", (*i)->getName());
            xml.addChildAttrib("Connect", (*i)->getConnect());
            xml.addChildAttrib("Description", (*i)->getDescription());
            xml.addChildAttrib("Nick", (*i)->getNick(false));
            xml.addChildAttrib("Password", (*i)->getPassword());
            xml.addChildAttrib("Server", (*i)->getServer());
            xml.addChildAttrib("UserDescription", (*i)->getUserDescription());
            xml.addChildAttrib("Encoding", (*i)->getEncoding());
            xml.addChildAttrib("ClientId", (*i)->getClientId());
            xml.addChildAttrib("ExternalIP", (*i)->getExternalIP());
            xml.addChildAttrib("OverrideId", Util::toString((*i)->getOverrideId()));
            xml.addChildAttrib("UseInternetIp",(*i)->getUseInternetIP());
            xml.addChildAttrib("DisableChat", (*i)->getDisableChat());
            xml.addChildAttrib("Mode", Util::toString((*i)->getMode()));
            xml.addChildAttrib("Group", (*i)->getGroup());
        }
        xml.stepOut();
        xml.addTag("Users");
        xml.stepIn();
        for(FavoriteMap::const_iterator i = users.begin(), iend = users.end(); i != iend; ++i) {
            xml.addTag("User");
            xml.addChildAttrib("LastSeen", i->second.getLastSeen());
            xml.addChildAttrib("GrantSlot", i->second.isSet(FavoriteUser::FLAG_GRANTSLOT));
            xml.addChildAttrib("UserDescription", i->second.getDescription());
            xml.addChildAttrib("Nick", i->second.getNick());
            xml.addChildAttrib("URL", i->second.getUrl());
            xml.addChildAttrib("CID", i->first.toBase32());
        }
        xml.stepOut();
        xml.addTag("UserCommands");
        xml.stepIn();
        for(UserCommand::List::const_iterator i = userCommands.begin(), iend = userCommands.end(); i != iend; ++i) {
            if(!i->isSet(UserCommand::FLAG_NOSAVE)) {
                xml.addTag("UserCommand");
                xml.addChildAttrib("Type", i->getType());
                xml.addChildAttrib("Context", i->getCtx());
                xml.addChildAttrib("Name", i->getName());
                xml.addChildAttrib("Command", i->getCommand());
                xml.addChildAttrib("To", i->getTo());
                xml.addChildAttrib("Hub", i->getHub());
            }
        }
        xml.stepOut();
        //Favorite download to dirs
        xml.addTag("FavoriteDirs");
        xml.stepIn();
        StringPairList spl = getFavoriteDirs();
        for(StringPairIter i = spl.begin(), iend = spl.end(); i != iend; ++i) {
            xml.addTag("Directory", i->first);
            xml.addChildAttrib("Name", i->second);
        }
        xml.stepOut();

        xml.stepOut();

        string fname = getConfigFile();

        File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
        f.write(SimpleXML::utf8Header);
        f.write(xml.toXML());
        f.close();
        File::deleteFile(fname);
        File::renameFile(fname + ".tmp", fname);

    } catch(const Exception& e) {
        dcdebug("FavoriteManager::save: %s\n", e.getError().c_str());
    }
}

void FavoriteManager::load() {

    // Add NMDC standard op commands
    static const char kickstr[] =
        "$To: %[userNI] From: %[myNI] $<%[myNI]> You are being kicked because: %[line:Reason]|<%[myNI]> %[myNI] is kicking %[userNI] because: %[line:Reason]|$Kick %[userNI]|";
    addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_USER | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
        _("Kick user(s)"), kickstr, "", "op");
    static const char redirstr[] =
        "$OpForceMove $Who:%[userNI]$Where:%[line:Target Server]$Msg:%[line:Message]|";
    addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_USER | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
        _("Redirect user(s)"), redirstr, "", "op");

    try {
        SimpleXML xml;
        Util::migrate(getConfigFile());
        xml.fromXML(File(getConfigFile(), File::READ, File::OPEN).read());

        if(xml.findChild("Favorites")) {
            xml.stepIn();
            load(xml);
            xml.stepOut();
        }
    } catch(const Exception& e) {
        dcdebug("FavoriteManager::load: %s\n", e.getError().c_str());
    }
}

void FavoriteManager::load(SimpleXML& aXml) {
    dontSave = true;
    bool needSave = false;

    aXml.resetCurrentChild();
    if(aXml.findChild("Hubs")) {
        aXml.stepIn();

    while(aXml.findChild("Group")) {
        string name = aXml.getChildAttrib("Name");
        if(name.empty())
            continue;
        FavHubGroupProperties props = { aXml.getBoolChildAttrib("Private"), aXml.getBoolChildAttrib("Connect") };
        favHubGroups[name] = props;
    }

    aXml.resetCurrentChild();
        while(aXml.findChild("Hub")) {
            FavoriteHubEntry* e = new FavoriteHubEntry();
            e->setName(aXml.getChildAttrib("Name"));
            e->setConnect(aXml.getBoolChildAttrib("Connect"));
            e->setDescription(aXml.getChildAttrib("Description"));
            e->setNick(aXml.getChildAttrib("Nick"));
            e->setPassword(aXml.getChildAttrib("Password"));
            e->setServer(aXml.getChildAttrib("Server"));
            e->setUserDescription(aXml.getChildAttrib("UserDescription"));
            e->setEncoding(aXml.getChildAttrib("Encoding"));
            e->setExternalIP(aXml.getChildAttrib("ExternalIP"));
            e->setClientId(aXml.getChildAttrib("ClientId"));
            e->setOverrideId(Util::toInt(aXml.getChildAttrib("OverrideId")) != 0);
            e->setUseInternetIP(aXml.getBoolChildAttrib("UseInternetIp"));
            e->setDisableChat(aXml.getBoolChildAttrib("DisableChat"));
            e->setMode(Util::toInt(aXml.getChildAttrib("Mode")));
            e->setGroup(aXml.getChildAttrib("Group"));
            favoriteHubs.push_back(e);

            if(aXml.getBoolChildAttrib("Connect")) {
            // this entry dates from before the window manager & fav hub groups; convert it.
            const string name = _("Auto-connect group (converted)");
            if(favHubGroups.find(name) == favHubGroups.end()) {
                FavHubGroupProperties props = { false, true };
                favHubGroups[name] = props;
            }
            e->setGroup(name);
            needSave = true;
            }
        }
        aXml.stepOut();
    }
    // parse groups that have the "Connect" param and send their hubs to WindowManager
    // бесполезная для нас хрень, только засирает конфиг
    // предназначение: подобие менеджера сессий для подключённых хабов(к автоконнекту не относится).
    //for(FavHubGroups::const_iterator i = favHubGroups.begin(), iend = favHubGroups.end(); i != iend; ++i) {
        //if(i->second.connect) {
            //FavoriteHubEntryList hubs = getFavoriteHubs(i->first);
            //for(FavoriteHubEntryList::const_iterator hub = hubs.begin(), hub_end = hubs.end(); hub != hub_end; ++hub) {
                //StringMap map;
                //map[WindowInfo::address] = (*hub)->getServer();
                //WindowManager::getInstance()->add(WindowManager::hub(), map);
            //}
        //}
    //}

    aXml.resetCurrentChild();
    if(aXml.findChild("Users")) {
        aXml.stepIn();
        while(aXml.findChild("User")) {
            UserPtr u;
            const string& cid = aXml.getChildAttrib("CID");
            const string& nick = aXml.getChildAttrib("Nick");
            const string& hubUrl = aXml.getChildAttrib("URL");

            if(cid.length() != 39) {
                if(nick.empty() || hubUrl.empty())
                    continue;
                u = ClientManager::getInstance()->getUser(nick, hubUrl);
            } else {
                u = ClientManager::getInstance()->getUser(CID(cid));
            }
            FavoriteMap::iterator i = users.insert(make_pair(u->getCID(), FavoriteUser(u, nick, hubUrl))).first;

            if(aXml.getBoolChildAttrib("GrantSlot"))
                i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);

            i->second.setLastSeen((uint32_t)aXml.getIntChildAttrib("LastSeen"));
            i->second.setDescription(aXml.getChildAttrib("UserDescription"));

        }
        aXml.stepOut();
    }
    aXml.resetCurrentChild();
    if(aXml.findChild("UserCommands")) {
        aXml.stepIn();
        while(aXml.findChild("UserCommand")) {
            addUserCommand(aXml.getIntChildAttrib("Type"), aXml.getIntChildAttrib("Context"), 0, aXml.getChildAttrib("Name"),
            aXml.getChildAttrib("Command"), aXml.getChildAttrib("To"), aXml.getChildAttrib("Hub"));
        }
        aXml.stepOut();
    }
    //Favorite download to dirs
    aXml.resetCurrentChild();
    if(aXml.findChild("FavoriteDirs")) {
        aXml.stepIn();
        while(aXml.findChild("Directory")) {
            string virt = aXml.getChildAttrib("Name");
            string d(aXml.getChildData());
            FavoriteManager::getInstance()->addFavoriteDir(d, virt);
        }
        aXml.stepOut();
    }

    dontSave = false;
    if(needSave)
        save();
}

void FavoriteManager::userUpdated(const OnlineUser& info) {
    Lock l(cs);
    FavoriteMap::iterator i = users.find(info.getUser()->getCID());
    if(i != users.end()) {
        FavoriteUser& fu = i->second;
        fu.update(info);
        save();
    }
}

FavoriteHubEntryPtr FavoriteManager::getFavoriteHubEntry(const string& aServer) const {
        for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i) {
        FavoriteHubEntry* hub = *i;
        if(Util::stricmp(hub->getServer(), aServer) == 0) {
            return hub;
        }
    }
    return NULL;
}

FavoriteHubEntryList FavoriteManager::getFavoriteHubs(const string& group) const {
        FavoriteHubEntryList ret;
        for(FavoriteHubEntryList::const_iterator i = favoriteHubs.begin(), iend = favoriteHubs.end(); i != iend; ++i)
                if((*i)->getGroup() == group)
                        ret.push_back(*i);
        return ret;
}

bool FavoriteManager::isPrivate(const string& url) const {
        if(url.empty())
                return false;

        FavoriteHubEntry* fav = getFavoriteHubEntry(url);
        if(fav) {
                const string& name = fav->getGroup();
                if(!name.empty()) {
                        FavHubGroups::const_iterator group = favHubGroups.find(name);
                        if(group != favHubGroups.end())
                                return group->second.priv;
                }
        }
        return false;
}

bool FavoriteManager::hasSlot(const UserPtr& aUser) const {
    Lock l(cs);
    FavoriteMap::const_iterator i = users.find(aUser->getCID());
    if(i == users.end())
        return false;
    return i->second.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

time_t FavoriteManager::getLastSeen(const UserPtr& aUser) const {
    Lock l(cs);
    FavoriteMap::const_iterator i = users.find(aUser->getCID());
    if(i == users.end())
        return 0;
    return i->second.getLastSeen();
}

void FavoriteManager::setAutoGrant(const UserPtr& aUser, bool grant) {
    Lock l(cs);
    FavoriteMap::iterator i = users.find(aUser->getCID());
    if(i == users.end())
        return;
    if(grant)
        i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);
    else
        i->second.unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
    save();
}
void FavoriteManager::setUserDescription(const UserPtr& aUser, const string& description) {
    Lock l(cs);
    FavoriteMap::iterator i = users.find(aUser->getCID());
    if(i == users.end())
        return;
    i->second.setDescription(description);
    save();
}

StringList FavoriteManager::getHubLists() {
    StringTokenizer<string> lists(SETTING(HUBLIST_SERVERS), ';');
    return lists.getTokens();
}

FavoriteHubEntryList::iterator FavoriteManager::getFavoriteHub(const string& aServer) {
    for(FavoriteHubEntryList::iterator i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
        if(Util::stricmp((*i)->getServer(), aServer) == 0) {
            return i;
        }
    }
    return favoriteHubs.end();
}


void FavoriteManager::setHubList(int aHubList) {
    lastServer = aHubList;
    refresh();
}

void FavoriteManager::refresh(bool forceDownload /* = false */) {
    StringList sl = getHubLists();
    if(sl.empty())
        return;
    publicListServer = sl[(lastServer) % sl.size()];
    if(Util::strnicmp(publicListServer.c_str(), "http://", 7) != 0) {
        lastServer++;
        return;
    }

    if(!forceDownload) {
        string path = Util::getHubListsPath() + Util::validateFileName(publicListServer);
        if(File::getSize(path) > 0) {
            useHttp = false;
                        string fileDate;
            {
                Lock l(cs);
                publicListMatrix[publicListServer].clear();
            }
            listType = (Util::stricmp(path.substr(path.size() - 4), ".bz2") == 0) ? TYPE_BZIP2 : TYPE_NORMAL;
            try {
                                File cached(path, File::READ, File::OPEN);
                                downloadBuf = cached.read();
                                char buf[20];
                                time_t fd = cached.getLastModified();
                                if (strftime(buf, 20, "%x", localtime(&fd))) {
                                        fileDate = string(buf);
                                }
            } catch(const FileException&) {
                downloadBuf = Util::emptyString;
            }
            if(!downloadBuf.empty()) {
                                if (onHttpFinished(false)) {
                                        fire(FavoriteManagerListener::LoadedFromCache(), publicListServer, fileDate);
                                }
                return;
            }
        }
    }

    if(!running) {
        useHttp = true;
        {
            Lock l(cs);
            publicListMatrix[publicListServer].clear();
        }
        fire(FavoriteManagerListener::DownloadStarting(), publicListServer);
        if(c == NULL)
            c = new HttpConnection();
        c->addListener(this);
        c->downloadFile(publicListServer);
        running = true;
    }
}

UserCommand::List FavoriteManager::getUserCommands(int ctx, const StringList& hubs) {
    vector<bool> isOp(hubs.size());

    for(size_t i = 0; i < hubs.size(); ++i) {
        if(ClientManager::getInstance()->isOp(ClientManager::getInstance()->getMe(), hubs[i])) {
            isOp[i] = true;
        }
    }

    Lock l(cs);
    UserCommand::List lst;
    for(UserCommand::List::iterator i = userCommands.begin(); i != userCommands.end(); ++i) {
        UserCommand& uc = *i;
        if(!(uc.getCtx() & ctx)) {
            //printf("%s\n", uc.getName().c_str());
            //printf("false\n");
            continue;
        }
        for(size_t j = 0; j < hubs.size(); ++j) {
            const string& hub = hubs[j];
            bool hubAdc = hub.compare(0, 6, "adc://") == 0 || hub.compare(0, 7, "adcs://") == 0;
            bool commandAdc = uc.getHub().compare(0, 6, "adc://") == 0 || uc.getHub().compare(0, 7, "adcs://") == 0;
            if(hubAdc && commandAdc) {
                if((uc.getHub() == "adc://" || uc.getHub() == "adcs://") ||
                   ((uc.getHub() == "adc://op" || uc.getHub() == "adcs://op") && isOp[j]) ||
                   (uc.getHub() == hub) )
                {
                    //printf("Found ADC command for ADC hub.\n");
                    lst.push_back(*i);
                    break;
                }
            } else if((!hubAdc && !commandAdc) || uc.isChat()) {
                if((uc.getHub().length() == 0) ||
                   (uc.getHub() == "op" && isOp[j]) ||
                   (uc.getHub() == hub) )
                {
                    //printf("Found non-ADC command for non-ADC hub.\n");
                    lst.push_back(*i);
                    break;
                }
            }
        }
    }
    return lst;
}

// HttpConnectionListener
void FavoriteManager::on(Data, HttpConnection*, const uint8_t* buf, size_t len) throw() {
    if(useHttp)
        downloadBuf.append((const char*)buf, len);
}

void FavoriteManager::on(Failed, HttpConnection*, const string& aLine) throw() {
    c->removeListener(this);
    lastServer++;
    running = false;
    if(useHttp){
        downloadBuf = Util::emptyString;
        fire(FavoriteManagerListener::DownloadFailed(), aLine);
    }
}
void FavoriteManager::on(Complete, HttpConnection*, const string& aLine, bool fromCoral) throw() {
        bool parseSuccess = false;

    c->removeListener(this);
        if(useHttp) {
                parseSuccess = onHttpFinished(true);
        }
    running = false;
        if(parseSuccess) {
                fire(FavoriteManagerListener::DownloadFinished(), aLine, fromCoral);
        }
}

void FavoriteManager::on(Retried, HttpConnection*, const bool Connected) throw() {
        if (Connected)
                downloadBuf = Util::emptyString;
}

void FavoriteManager::on(Redirected, HttpConnection*, const string& aLine) throw() {
    if(useHttp)
        fire(FavoriteManagerListener::DownloadStarting(), aLine);
}
void FavoriteManager::on(TypeNormal, HttpConnection*) throw() {
    if(useHttp)
        listType = TYPE_NORMAL;
}
void FavoriteManager::on(TypeBZ2, HttpConnection*) throw() {
    if(useHttp)
        listType = TYPE_BZIP2;
}

void FavoriteManager::on(UserUpdated, const OnlineUser& user) throw() {
    userUpdated(user);
}

//NOTE: freedcpp
void FavoriteManager::on(UserDisconnected, const UserPtr& user) throw()
    {
        Lock l(cs);

        FavoriteMap::iterator i = users.find(user->getCID());
        if (i != users.end())
        {
            i->second.setLastSeen(GET_TIME());
                fire(FavoriteManagerListener::StatusChanged(), i->second);
            save();
        }
    }

void FavoriteManager::on(UserConnected, const UserPtr& user) throw()
    {
        Lock l(cs);

        FavoriteMap::iterator i = users.find(user->getCID());
        if (i != users.end())
        {
                fire(FavoriteManagerListener::StatusChanged(), i->second);
    }
}
//NOTE: freedcpp
} // namespace dcpp
