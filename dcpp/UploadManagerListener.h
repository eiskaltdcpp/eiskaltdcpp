#ifndef UPLOADMANAGERLISTENER_H_
#define UPLOADMANAGERLISTENER_H_

#include "forward.h"

namespace dcpp {

class UploadManagerListener {
public:
	virtual ~UploadManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Complete;
	typedef X<1> Failed;
	typedef X<2> Starting;
	typedef X<3> Tick;
	typedef X<4> WaitingAddFile;
	typedef X<5> WaitingRemoveUser;

	virtual void on(Starting, Upload*) throw() { }
	virtual void on(Tick, const UploadList&) throw() { }
	virtual void on(Complete, Upload*) throw() { }
	virtual void on(Failed, Upload*, const string&) throw() { }
	virtual void on(WaitingAddFile, const UserPtr&, const string&) throw() { }
	virtual void on(WaitingRemoveUser, const UserPtr&) throw() { }

};

} // namespace dcpp

#endif /*UPLOADMANAGERLISTENER_H_*/
