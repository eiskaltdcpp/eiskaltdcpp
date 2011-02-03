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

#include "Encoder.h"

namespace dcpp {

const int8_t Encoder::base32Table[] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,26,27,28,29,30,31,-1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

const char Encoder::base32Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

string& Encoder::toBase32(const uint8_t* src, size_t len, string& dst) {
	// Code snagged from the bitzi bitcollider
	size_t i, index;
	uint8_t word;
	dst.reserve(((len * 8) / 5) + 1);

	for(i = 0, index = 0; i < len;) {
		/* Is the current word going to span a byte boundary? */
		if (index > 3) {
			word = (uint8_t)(src[i] & (0xFF >> index));
			index = (index + 5) % 8;
			word <<= index;
			if ((i + 1) < len)
				word |= src[i + 1] >> (8 - index);

			i++;
		} else {
			word = (uint8_t)(src[i] >> (8 - (index + 5))) & 0x1F;
			index = (index + 5) % 8;
			if (index == 0)
				i++;
		}

		dcassert(word < 32);
		dst += base32Alphabet[word];
	}
	return dst;
}

void Encoder::fromBase32(const char* src, uint8_t* dst, size_t len) {
	size_t i, index, offset;

	memset(dst, 0, len);
	for(i = 0, index = 0, offset = 0; src[i]; i++) {
		// Skip what we don't recognise
		int8_t tmp = base32Table[(unsigned char)src[i]];

		if(tmp == -1)
			continue;

		if (index <= 3) {
			index = (index + 5) % 8;
			if (index == 0) {
				dst[offset] |= tmp;
				offset++;
				if(offset == len)
					break;
			} else {
				dst[offset] |= tmp << (8 - index);
			}
		} else {
			index = (index + 5) % 8;
			dst[offset] |= (tmp >> index);
			offset++;
			if(offset == len)
				break;
			dst[offset] |= tmp << (8 - index);
		}
	}
}

} // namespace dcpp
