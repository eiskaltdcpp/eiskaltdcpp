/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "SettingsManager.h"
#include "CriticalSection.h"
#include "HttpManagerListener.h"
#include "UserCommand.h"
#include "FavoriteUser.h"
#include "Singleton.h"
#include "ClientManagerListener.h"
#include "FavoriteManagerListener.h"
#include "HubEntry.h"
#include "FavHubGroup.h"
#include "User.h"

namespace dcpp {

class SimpleXML;

/**
 * Public hub list, favorites (hub&user). Assumed to be called only by UI thread.
 */
class FavoriteManager : public Speaker<FavoriteManagerListener>, public Singleton<FavoriteManager>,
private ClientManagerListener, private HttpManagerListener, private SettingsManagerListener
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
    const string& getCurrentHubList() const { return publicListServer; }

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
    void shutdown();
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

    /** Used during loading to prevent saving. */
    bool dontSave;

    friend class Singleton<FavoriteManager>;

    FavoriteManager();
    virtual ~FavoriteManager();

    FavoriteHubEntryPtr getFavoriteHub(const string& aServer);

    // ClientManagerListener
    void on(UserUpdated, const OnlineUser& user) noexcept;
    void on(UserConnected, const UserPtr& user) noexcept;
    void on(UserDisconnected, const UserPtr& user) noexcept;

    // HttpManagerListener
    void on(HttpManagerListener::Added, HttpConnection*) noexcept;
    void on(HttpManagerListener::Failed, HttpConnection*, const string&) noexcept;
    void on(HttpManagerListener::Complete, HttpConnection*, OutputStream*) noexcept;

    bool onHttpFinished(const string& buf) noexcept;

    // SettingsManagerListener
    void on(SettingsManagerListener::Load, SimpleXML& xml) noexcept {
        load(xml);
    }

    void load(SimpleXML& aXml);

    string getConfigFile();
};

} // namespace dcpp
