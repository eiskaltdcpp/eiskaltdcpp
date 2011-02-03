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

#if !defined(MERKLE_TREE_H)
#define MERKLE_TREE_H

#include "TigerHash.h"
#include "Encoder.h"
#include "HashValue.h"

namespace dcpp {

/**
 * A class that represents a Merkle Tree hash. Storing
 * only the leaves of the tree, it is rather memory efficient,
 * but can still take a significant amount of memory during / after
 * hash generation.
 * The root hash produced can be used like any
 * other hash to verify the integrity of a whole file, while
 * the leaves provide checking of smaller parts of the file.
 */
template<class Hasher, size_t baseBlockSize = 1024>
class MerkleTree {
public:
    static const size_t BITS = Hasher::BITS;
    static const size_t BYTES = Hasher::BYTES;
    static const size_t BASE_BLOCK_SIZE = baseBlockSize;

    typedef HashValue<Hasher> MerkleValue;
    typedef vector<MerkleValue> MerkleList;
    typedef typename MerkleList::iterator MerkleIter;

    MerkleTree() : fileSize(0), blockSize(baseBlockSize) { }
    MerkleTree(int64_t aBlockSize) : fileSize(0), blockSize(aBlockSize) { }

    /**
     * Loads a set of leaf hashes, calculating the root
     * @param data Pointer to (aFileSize + aBlockSize - 1) / aBlockSize) hash values,
     *             stored consecutively left to right
     */
    MerkleTree(int64_t aFileSize, int64_t aBlockSize, uint8_t* aData) :
        fileSize(aFileSize), blockSize(aBlockSize)
    {
        size_t n = calcBlocks(aFileSize, aBlockSize);
        for(size_t i = 0; i < n; i++)
            leaves.push_back(MerkleValue(aData + i * Hasher::BYTES));

        calcRoot();
    }

    /** Initialise a single root tree */
    MerkleTree(int64_t aFileSize, int64_t aBlockSize, const MerkleValue& aRoot) : root(aRoot), fileSize(aFileSize), blockSize(aBlockSize) {
        leaves.push_back(root);
    }

    ~MerkleTree() {
    }

    static int64_t calcBlockSize(int64_t aFileSize, int maxLevels) {
        int64_t tmp = baseBlockSize;
        int64_t maxHashes = ((int64_t)1) << (maxLevels - 1);
        while((maxHashes * tmp) < aFileSize)
            tmp *= 2;
        return tmp;
    }

    static size_t calcBlocks(int64_t aFileSize, int64_t aBlockSize) {
        return max((size_t)((aFileSize + aBlockSize - 1) / aBlockSize), (size_t)1);
    }

    /**
     * Update the merkle tree.
     * @param len Length of data, must be a multiple of baseBlockSize, unless it's
     *            the last block.
     */
    void update(const void* data, size_t len) {
        uint8_t* buf = (uint8_t*)data;
        uint8_t zero = 0;
        size_t i = 0;

        // Skip empty data sets if we already added at least one of them...
        if(len == 0 && !(leaves.empty() && blocks.empty()))
            return;

        do {
            size_t n = min(baseBlockSize, len-i);
            Hasher h;
            h.update(&zero, 1);
            h.update(buf + i, n);
            if((int64_t)baseBlockSize < blockSize) {
                blocks.push_back(make_pair(MerkleValue(h.finalize()), baseBlockSize));
                reduceBlocks();
            } else {
                leaves.push_back(MerkleValue(h.finalize()));
            }
            i += n;
        } while(i < len);
        fileSize += len;
    }

    uint8_t* finalize() {
        // No updates yet, make sure we have at least one leaf for 0-length files...
        if(leaves.empty() && blocks.empty()) {
            update(0, 0);
        }
        while(blocks.size() > 1) {
            MerkleBlock& a = blocks[blocks.size()-2];
            MerkleBlock& b = blocks[blocks.size()-1];
            a.first = combine(a.first, b.first);
            blocks.pop_back();
        }
        dcassert(blocks.size() == 0 || blocks.size() == 1);
        if(!blocks.empty()) {
            leaves.push_back(blocks[0].first);
        }
        calcRoot();
        return root.data;
    }

    MerkleValue& getRoot() { return root; }
    const MerkleValue& getRoot() const { return root; }
    MerkleList& getLeaves() { return leaves; }
    const MerkleList& getLeaves() const { return leaves; }

    int64_t getBlockSize() const { return blockSize; }
    void setBlockSize(int64_t aSize) { blockSize = aSize; }

    int64_t getFileSize() const { return fileSize; }
    void setFileSize(int64_t aSize) { fileSize = aSize; }

    bool verifyRoot(const uint8_t* aRoot) {
        return memcmp(aRoot, getRoot().data(), BYTES) == 0;
    }

    void calcRoot() {
        root = getHash(0, fileSize);
    }

    ByteVector getLeafData() {
        ByteVector buf(getLeaves().size() * BYTES);
        uint8_t* p = &buf[0];
        for(size_t i = 0; i < getLeaves().size(); ++i) {
            memcpy(p + i * BYTES, &getLeaves()[i], BYTES);
        }
        return buf;
    }

private:
    typedef pair<MerkleValue, int64_t> MerkleBlock;
    typedef vector<MerkleBlock> MBList;

    MBList blocks;

    MerkleList leaves;

    MerkleValue root;
    /** Total size of hashed data */
    int64_t fileSize;
    /** Final block size */
    int64_t blockSize;

    MerkleValue getHash(int64_t start, int64_t length) {
        dcassert((start % blockSize) == 0);
        if(length <= blockSize) {
            dcassert((start / blockSize) < (int64_t)leaves.size());
            return leaves[(uint32_t)(start / blockSize)];
        } else {
            int64_t l = blockSize;
            while(l * 2 < length)
                l *= 2;
            return combine(getHash(start, l), getHash(start+l, length - l));
        }
    }

    MerkleValue combine(const MerkleValue& a, const MerkleValue& b) {
        uint8_t one = 1;
        Hasher h;
        h.update(&one, 1);
        h.update(a.data, MerkleValue::BYTES);
        h.update(b.data, MerkleValue::BYTES);
        return MerkleValue(h.finalize());
    }

    void reduceBlocks() {
        while(blocks.size() > 1) {
            MerkleBlock& a = blocks[blocks.size()-2];
            MerkleBlock& b = blocks[blocks.size()-1];
            if(a.second == b.second) {
                if(a.second*2 == blockSize) {
                    leaves.push_back(combine(a.first, b.first));
                    blocks.pop_back();
                    blocks.pop_back();
                } else {
                    a.second *= 2;
                    a.first = combine(a.first, b.first);
                    blocks.pop_back();
                }
            } else {
                break;
            }
        }
    }
};

typedef MerkleTree<TigerHash> TigerTree;
typedef TigerTree::MerkleValue TTHValue;

template<int64_t aBlockSize>
struct TTFilter {
    TTFilter() : tt(aBlockSize) { }
    void operator()(const void* data, size_t len) { tt.update(data, len); }
    TigerTree& getTree() { return tt; }
private:
    TigerTree tt;
};

} // namespace dcpp

#endif // !defined(MERKLE_TREE_H)
