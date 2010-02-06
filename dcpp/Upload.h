#ifndef UPLOAD_H_
#define UPLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "Flags.h"

namespace dcpp {

class Upload : public Transfer, public Flags {
public:
	enum Flags {
		FLAG_ZUPLOAD = 1 << 0,
		FLAG_PENDING_KICK = 1 << 1
	};

	Upload(UserConnection& conn, const string& path, const TTHValue& tth);
	virtual ~Upload();

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	GETSET(InputStream*, stream, Stream);
};

} // namespace dcpp

#endif /*UPLOAD_H_*/
