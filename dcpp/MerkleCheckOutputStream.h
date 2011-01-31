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

#ifndef DCPLUSPLUS_DCPP_MERKLE_CHECK_OUTPUT_STREAM_H
#define DCPLUSPLUS_DCPP_MERKLE_CHECK_OUTPUT_STREAM_H

#include "Streams.h"
#include "MerkleTree.h"

namespace dcpp {

template<class TreeType, bool managed>
class MerkleCheckOutputStream : public OutputStream {
public:
    MerkleCheckOutputStream(const TreeType& aTree, OutputStream* aStream, int64_t start) : s(aStream), real(aTree), cur(aTree.getBlockSize()), verified(0), bufPos(0) {
        // Only start at block boundaries
        dcassert(start % aTree.getBlockSize() == 0);
        cur.setFileSize(start);

        size_t nBlocks = static_cast<size_t>(start / aTree.getBlockSize());
        if(nBlocks > aTree.getLeaves().size()) {
            dcdebug("Invalid tree / parameters");
            return;
        }
        cur.getLeaves().insert(cur.getLeaves().begin(), aTree.getLeaves().begin(), aTree.getLeaves().begin() + nBlocks);
    }

    virtual ~MerkleCheckOutputStream() throw() { if(managed) delete s; }

    virtual size_t flush() throw(FileException) {
        if (bufPos != 0)
            cur.update(buf, bufPos);
        bufPos = 0;

        cur.finalize();
        if(cur.getLeaves().size() == real.getLeaves().size()) {
            if (cur.getRoot() != real.getRoot())
                throw FileException(_("TTH inconsistency"));
        } else {
            checkTrees();
        }
        return s->flush();
    }

    void commitBytes(const void* b, size_t len) throw(FileException) {
        uint8_t* xb = (uint8_t*)b;
        size_t pos = 0;

        if(bufPos != 0) {
            size_t bytes = min(TreeType::BASE_BLOCK_SIZE - bufPos, len);
            memcpy(buf + bufPos, xb, bytes);
            pos = bytes;
            bufPos += bytes;

            if(bufPos == TreeType::BASE_BLOCK_SIZE) {
                cur.update(buf, TreeType::BASE_BLOCK_SIZE);
                bufPos = 0;
            }
        }

        if(pos < len) {
            dcassert(bufPos == 0);
            size_t left = len - pos;
            size_t part = left - (left % TreeType::BASE_BLOCK_SIZE);
            if(part > 0) {
                cur.update(xb + pos, part);
                pos += part;
            }
            left = len - pos;
            memcpy(buf, xb + pos, left);
            bufPos = left;
        }
    }

    virtual size_t write(const void* b, size_t len) throw(FileException) {
        commitBytes(b, len);
        checkTrees();
        return s->write(b, len);
    }

    int64_t verifiedBytes() {
        return min(real.getFileSize(), (int64_t)(cur.getBlockSize() * cur.getLeaves().size()));
    }
private:
    OutputStream* s;
    TreeType real;
    TreeType cur;
    size_t verified;

    uint8_t buf[TreeType::BASE_BLOCK_SIZE];
    size_t bufPos;

    void checkTrees() throw(FileException) {
        while(cur.getLeaves().size() > verified) {
            if(cur.getLeaves().size() > real.getLeaves().size() ||
                !(cur.getLeaves()[verified] == real.getLeaves()[verified]))
            {
                throw FileException(_("TTH inconsistency"));
            }
            verified++;
        }
    }
};

} // namespace dcpp

#endif // !defined(MERKLE_CHECK_OUTPUT_STREAM_H)
