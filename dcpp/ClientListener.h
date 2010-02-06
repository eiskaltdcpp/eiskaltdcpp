#ifndef CLIENTLISTENER_H_
#define CLIENTLISTENER_H_

#include "forward.h"

namespace dcpp {

class ClientListener
{
public:
	virtual ~ClientListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Connecting;
	typedef X<1> Connected;
	typedef X<3> UserUpdated;
	typedef X<4> UsersUpdated;
	typedef X<5> UserRemoved;
	typedef X<6> Redirect;
	typedef X<7> Failed;
	typedef X<8> GetPassword;
	typedef X<9> HubUpdated;
	typedef X<11> Message;
	typedef X<12> StatusMessage;
	typedef X<13> PrivateMessage;
	typedef X<14> HubUserCommand;
	typedef X<15> HubFull;
	typedef X<16> NickTaken;
	typedef X<17> SearchFlood;
	typedef X<18> NmdcSearch;
	typedef X<19> AdcSearch;

	enum StatusFlags {
		FLAG_NORMAL = 0x00,
		FLAG_IS_SPAM = 0x01
	};

	virtual void on(Connecting, Client*) throw() { }
	virtual void on(Connected, Client*) throw() { }
	virtual void on(UserUpdated, Client*, const OnlineUser&) throw() { }
	virtual void on(UsersUpdated, Client*, const OnlineUserList&) throw() { }
	virtual void on(UserRemoved, Client*, const OnlineUser&) throw() { }
	virtual void on(Redirect, Client*, const string&) throw() { }
	virtual void on(Failed, Client*, const string&) throw() { }
	virtual void on(GetPassword, Client*) throw() { }
	virtual void on(HubUpdated, Client*) throw() { }
	virtual void on(Message, Client*, const OnlineUser&, const string&, bool = false) throw() { }
	virtual void on(StatusMessage, Client*, const string&, int = FLAG_NORMAL) throw() { }
	virtual void on(PrivateMessage, Client*, const OnlineUser&, const OnlineUser&, const OnlineUser&, const string&, bool = false) throw() { }
	virtual void on(HubUserCommand, Client*, int, int, const string&, const string&) throw() { }
	virtual void on(HubFull, Client*) throw() { }
	virtual void on(NickTaken, Client*) throw() { }
	virtual void on(SearchFlood, Client*, const string&) throw() { }
	virtual void on(NmdcSearch, Client*, const string&, int, int64_t, int, const string&) throw() { }
	virtual void on(AdcSearch, Client*, const AdcCommand&, const CID&) throw() { }
};

} // namespace dcpp

#endif /*CLIENTLISTENER_H_*/
