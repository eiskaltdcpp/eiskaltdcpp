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

#ifndef DCPLUSPLUS_DCPP_Z_UTILS_H
#define DCPLUSPLUS_DCPP_Z_UTILS_H

#include <zlib.h>

namespace dcpp {

class ZFilter {
public:
	/** Compression will automatically be turned off if below this... */
	static const double MIN_COMPRESSION_LEVEL;

	ZFilter();
	~ZFilter();
	/**
	 * Compress data.
	 * @param in Input data
	 * @param insize Input size (Set to 0 to indicate that no more data will follow)
	 * @param out Output buffer
	 * @param outsize Output size, set to compressed size on return.
	 * @return True if there's more processing to be done
	 */
	bool operator()(const void* in, size_t& insize, void* out, size_t& outsize);
private:
	z_stream zs;
	int64_t totalIn;
	int64_t totalOut;
	bool compressing;
};

class UnZFilter {
public:
	UnZFilter();
	~UnZFilter();
	/**
	 * Decompress data.
	 * @param in Input data
	 * @param insize Input size (Set to 0 to indicate that no more data will follow)
	 * @param out Output buffer
	 * @param outsize Output size, set to decompressed size on return.
	 * @return True if there's more processing to be done
	 */
	bool operator()(const void* in, size_t& insize, void* out, size_t& outsize);
private:
	z_stream zs;
};

class CRC32Filter {
public:
	CRC32Filter() : crc(crc32(0, NULL, 0)) { }
	// TODO 64-bits?
	void operator()(const void* buf, size_t len) { crc = crc32(crc, (const Bytef*)buf, (uInt)len); }
	uint32_t getValue() const { return crc; }
private:
	uint32_t crc;
};

} // namespace dcpp

#endif // !defined(_Z_UTILS_H)
