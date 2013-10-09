/*
 * Copyright (C) 2001-2013 Jacek Sieka, arnetheduck on gmail point com
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

#include "Streams.h"
#include "MerkleTree.h"

namespace dcpp {

template<typename TreeType>
class MerkleTreeOutputStream : public OutputStream {
public:
        MerkleTreeOutputStream(TreeType& aTree) : tree(aTree), bufPos(0) { }

        virtual size_t write(const void* xbuf, size_t len) {
                size_t pos = 0;
                uint8_t* b = (uint8_t*)xbuf;
                while(pos < len) {
                        size_t left = len - pos;
                        if(bufPos == 0 && left >= TreeType::BYTES) {
                                tree.getLeaves().emplace_back(b + pos);
                                pos += TreeType::BYTES;
                        } else {
                                size_t bytes = min(TreeType::BYTES - bufPos, left);
                                memcpy(buf + bufPos, b + pos, bytes);
                                bufPos += bytes;
                                pos += bytes;
                                if(bufPos == TreeType::BYTES) {
                                        tree.getLeaves().emplace_back(buf);
                                        bufPos = 0;
                                }
                        }
                }
                return len;
        }

        virtual size_t flush() {
                return 0;
        }
private:
        TreeType& tree;
        uint8_t buf[TreeType::BYTES];
        size_t bufPos;
};

}
