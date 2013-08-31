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

#ifndef DCPLUSPLUS_DCPP_SHA1HASH_H_
#define DCPLUSPLUS_DCPP_SHA1HASH_H_

#include <openssl/sha.h>

#include "HashValue.h"

namespace dcpp {

class SHA1Hash {
public:
	/** Hash size in bytes */
	static const size_t BITS = 160;
	static const size_t BYTES = BITS / 8;

	SHA1Hash() { SHA1_Init(&ctx); }

	~SHA1Hash() { }

	/** Calculates the Tiger hash of the data. */
	void update(const void* data, size_t len) { SHA1_Update(&ctx, data, len); }
	/** Call once all data has been processed. */
	uint8_t* finalize() { SHA1_Final(reinterpret_cast<unsigned char*>(&res), &ctx); return res; }

	uint8_t* getResult() { return res; }
private:
	SHA_CTX ctx;
	uint8_t res[BYTES];
};

typedef HashValue<SHA1Hash> SHA1Value;

} // namespace dcpp

#endif /* DCPLUSPLUS_DCPP_SHA1HASH_H_ */
