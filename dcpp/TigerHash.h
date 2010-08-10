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

#if !defined(TIGER_HASH_H)
#define TIGER_HASH_H

namespace dcpp {

class TigerHash {
public:
    /** Hash size in bytes */
    static const size_t BITS = 192;
    static const size_t BYTES = BITS / 8;

    TigerHash() : pos(0) {
        res[0]=_ULL(0x0123456789ABCDEF);
        res[1]=_ULL(0xFEDCBA9876543210);
        res[2]=_ULL(0xF096A5B4C3B2E187);
    }

    ~TigerHash() {
    }

    /** Calculates the Tiger hash of the data. */
    void update(const void* data, size_t len);
    /** Call once all data has been processed. */
    uint8_t* finalize();

    uint8_t* getResult() { return (uint8_t*) res; }
private:
    enum { BLOCK_SIZE = 512/8 };
    /** 512 bit blocks for the compress function */
    uint8_t tmp[512/8];
    /** State / final hash value */
    uint64_t res[3];
    /** Total number of bytes compressed */
    uint64_t pos;
    /** S boxes */
    static uint64_t table[];

    void tigerCompress(const uint64_t* data, uint64_t state[3]);
};

} // namespace dcpp

#endif // !defined(TIGER_HASH_H)
