#ifndef HASHMANAGERLISTENER_H_
#define HASHMANAGERLISTENER_H_

#include "MerkleTree.h"

namespace dcpp {

class HashManagerListener {
public:
	virtual ~HashManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> TTHDone;

	virtual void on(TTHDone, const string& /* fileName */, const TTHValue& /* root */) throw() = 0;
};

}
#endif /*HASHMANAGERLISTENER_H_*/
