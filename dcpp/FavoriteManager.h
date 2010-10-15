/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_FAVORITE_MANAGER_H
#define DCPLUSPLUS_DCPP_FAVORITE_MANAGER_H

#include "SettingsManager.h"

#include "CriticalSection.h"
#include "HttpConnection.h"
#include "User.h"
#include "UserCommand.h"
#include "FavoriteUser.h"
#include "Singleton.h"
#include "ClientManagerListener.h"
#include "FavoriteManagerListener.h"
#include "HubEntry.h"
#include "FavHubGroup.h"

namespace dcpp {

class SimpleXML;

/**
 * Public hub list, favorites (hub&user). Assumed to be called only by UI thread.
 */
class FavoriteManager : public Speaker<FavoriteManagerListener>, private HttpConnectionListener, public Singleton<FavoriteManager>,
    private SettingsManagerListener, private ClientManagerListener
{
public:
// Public Hubs
    enum HubTypes {
        TYPE_NORMAL,
        TYPE_BZIP2
    };
    StringList getHubLists();
    void setHubList(int aHubList);
    int getSelectedHubList() { return lastServer; }
    void refresh(bool forceDownload = false);
    HubTypes getHubListType() { return listType; }
    HubEntryList getPublicHubs() {
        Lock l(cs);
        return publicListMatrix[publicListServer];
    }
    bool isDownloading() { return (useHttp && running); }

// Favorite Users
    typedef unordered_map<CID, FavoriteUser> FavoriteMap;
    FavoriteMap getFavoriteUsers() { Lock l(cs); return users; }

    void addFavoriteUser(const UserPtr& aUser);
    bool isFavoriteUser(const UserPtr& aUser) const { Lock l(cs); return users.find(aUser->getCID()) != users.end(); }
    void removeFavoriteUser(const UserPtr& aUser);

    bool hasSlot(const UserPtr& aUser) const;
    void setUserDescription(const UserPtr& aUser, const string& description);
    void setAutoGrant(const UserPtr& aUser, bool grant);
    void userUpdated(const OnlineUser& info);
    time_t getLastSeen(const UserPtr& aUser) const;
    std::string getUserURL(const UserPtr& aUser) const;

// Favorite Hubs
        const FavoriteHubEntryList& getFavoriteHubs() const { return favoriteHubs; }
    FavoriteHubEntryList& getFavoriteHubs() { return favoriteHubs; }

    void addFavorite(const FavoriteHubEntry& aEntry);
    void removeFavorite(FavoriteHubEntry* entry);
    bool isFavoriteHub(const std::string& aUrl);
        FavoriteHubEntryPtr getFavoriteHubEntry(const string& aServer) const;

// Favorite hub groups
        const FavHubGroups& getFavHubGroups() const { return favHubGroups; }
        void setFavHubGroups(const FavHubGroups& favHubGroups_) { favHubGroups = favHubGroups_; }

        FavoriteHubEntryList getFavoriteHubs(const string& group) const;
        bool isPrivate(const string& url) const;

// Favorite Directories
    bool addFavoriteDir(const string& aDirectory, const string& aName);
    bool removeFavoriteDir(const string& aName);
    bool renameFavoriteDir(const string& aName, const string& anotherName);
    StringPairList getFavoriteDirs() { return favoriteDirs; }

// User Commands
        UserCommand addUserCommand(int type, int ctx, int flags, const string& name, const string& command, const string& to, const string& hub);
    bool getUserCommand(int cid, UserCommand& uc);
    int findUserCommand(const string& aName, const string& aUrl);
    bool moveUserCommand(int cid, int pos);
    void updateUserCommand(const UserCommand& uc);
    void removeUserCommand(int cid);
    void removeUserCommand(const string& srv);
    void removeHubUserCommands(int ctx, const string& hub);

    UserCommand::List getUserCommands() { Lock l(cs); return userCommands; }
    UserCommand::List getUserCommands(int ctx, const StringList& hub);

    void load();
    void save();

private:
    FavoriteHubEntryList favoriteHubs;
        FavHubGroups favHubGroups;
    StringPairList favoriteDirs;
    UserCommand::List userCommands;
    int lastId;

    FavoriteMap users;

    mutable CriticalSection cs;

    // Public Hubs
    typedef unordered_map<string, HubEntryList> PubListMap;
    PubListMap publicListMatrix;
    string publicListServer;
    bool useHttp, running;
    HttpConnection* c;
    int lastServer;
    HubTypes listType;
    string downloadBuf;

    /** Used during loading to prevent saving. */
    bool dontSave;

    friend class Singleton<FavoriteManager>;

    FavoriteManager();
    virtual ~FavoriteManager() throw();

    FavoriteHubEntryList::iterator getFavoriteHub(const string& aServer);

    // ClientManagerListener
    virtual void on(UserUpdated, const OnlineUser& user) throw();
    virtual void on(UserConnected, const UserPtr& user) throw();
    virtual void on(UserDisconnected, const UserPtr& user) throw();

    // HttpConnectionListener
    virtual void on(Data, HttpConnection*, const uint8_t*, size_t) throw();
    virtual void on(Failed, HttpConnection*, const string&) throw();
        virtual void on(Complete, HttpConnection*, const string&, bool) throw();
    virtual void on(Redirected, HttpConnection*, const string&) throw();
    virtual void on(TypeNormal, HttpConnection*) throw();
    virtual void on(TypeBZ2, HttpConnection*) throw();

        bool onHttpFinished(bool fromHttp) throw();

    // SettingsManagerListener
    virtual void on(SettingsManagerListener::Load, SimpleXML& xml) throw() {
        load(xml);
    }

    void load(SimpleXML& aXml);

    string getConfigFile() { return Util::getPath(Util::PATH_USER_CONFIG) + "Favorites.xml"; }
};

} // namespace dcpp

#endif // !defined(FAVORITE_MANAGER_H)
