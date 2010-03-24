/*
 * Copyright (C) 2001-2009 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CLIENT_MANAGER_H
#define DCPLUSPLUS_DCPP_CLIENT_MANAGER_H

#include "TimerManager.h"

#include "Client.h"
#include "Singleton.h"
#include "SettingsManager.h"
#include "User.h"
#include "Socket.h"

#include "ClientManagerListener.h"

namespace dcpp {

class UserCommand;

class ClientManager : public Speaker<ClientManagerListener>,
	private ClientListener, public Singleton<ClientManager>,
	private TimerManagerListener
{
public:
	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	size_t getUserCount() const;
	int64_t getAvailable() const;
	StringList getHubs(const CID& cid) const;
	StringList getHubNames(const CID& cid) const;
	StringList getNicks(const CID& cid) const;
	string getConnection(const CID& cid) const;

	bool isConnected(const string& aUrl) const;

	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void infoUpdated();

	UserPtr getUser(const string& aNick, const string& aHubUrl) throw();
	UserPtr getUser(const CID& cid) throw();

	string findHub(const string& ipPort) const;
	string findHubEncoding(const string& aUrl) const;

	UserPtr findUser(const string& aNick, const string& aHubUrl) const throw() { return findUser(makeCid(aNick, aHubUrl)); }
	UserPtr findUser(const CID& cid) const throw();
	UserPtr findLegacyUser(const string& aNick) const throw();

	bool isOnline(const UserPtr& aUser) const {
		Lock l(cs);
		return onlineUsers.find(aUser->getCID()) != onlineUsers.end();
	}

  Identity getOnlineUserIdentity(const UserPtr& aUser) const {
    Lock l(cs);
    OnlineMap::const_iterator i;
    i=onlineUsers.find(aUser->getCID());
    if ( i != onlineUsers.end() )
    {
      return i->second->getIdentity();
    }
    return Identity();
  }

	bool isOp(const UserPtr& aUser, const string& aHubUrl) const;

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) const throw();

	void putOnline(OnlineUser* ou) throw();
	void putOffline(OnlineUser* ou, bool disconnect = false) throw();

	UserPtr& getMe();

	void connect(const UserPtr& p, const string& token, const string& hintUrl);
	void send(AdcCommand& c, const CID& to);
	void privateMessage(const UserPtr& p, const string& msg, bool thirdPerson, const string& hintUrl);

	void userCommand(const UserPtr& p, const UserCommand& uc, StringMap& params, bool compatibility);

	bool isActive() { return SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_PASSIVE; }

	void lock() throw() { cs.enter(); }
	void unlock() throw() { cs.leave(); }

	Client::List& getClients() { return clients; }

	CID getMyCID();
	const CID& getMyPID();

private:
	typedef unordered_map<string, UserPtr> LegacyMap;
	typedef LegacyMap::iterator LegacyIter;

	typedef unordered_map<CID, UserPtr> UserMap;
	typedef UserMap::iterator UserIter;

	typedef unordered_map<CID, std::string> NickMap;

	typedef unordered_multimap<CID, OnlineUser*> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;
	typedef pair<OnlineIter, OnlineIter> OnlinePair;
	typedef pair<OnlineIterC, OnlineIterC> OnlinePairC;

	Client::List clients;
	mutable CriticalSection cs;

	UserMap users;
	OnlineMap onlineUsers;
	NickMap nicks;

	UserPtr me;

	Socket udp;

	CID pid;

	friend class Singleton<ClientManager>;

	ClientManager() {
		TimerManager::getInstance()->addListener(this);
	}

	virtual ~ClientManager() throw() {
		TimerManager::getInstance()->removeListener(this);
	}

	void updateNick(const OnlineUser& user) throw();

	OnlineUser* findOnlineUser(const CID& cid, const string& hintUrl) throw();

	// ClientListener
	virtual void on(Connected, Client* c) throw();
	virtual void on(UserUpdated, Client*, const OnlineUser& user) throw();
	virtual void on(UsersUpdated, Client* c, const OnlineUserList&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(HubUpdated, Client* c) throw();
	virtual void on(HubUserCommand, Client*, int, int, const string&, const string&) throw();
	virtual void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
		int aFileType, const string& aString) throw();
	virtual void on(AdcSearch, Client* c, const AdcCommand& adc, const CID& from) throw();
	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, uint32_t aTick) throw();
};

} // namespace dcpp

#endif // !defined(CLIENT_MANAGER_H)
