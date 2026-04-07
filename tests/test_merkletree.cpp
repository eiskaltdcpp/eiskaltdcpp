/*
 * Tests for MerkleTree<TigerHash> (TigerTree) — hash tree construction,
 * calcBlockSize, calcBlocks, incremental update, finalize, leaf data.
 * No DCContext required — header-only template + TigerHash.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "MerkleTree.h"
#include "TigerHash.h"
#include "Encoder.h"

#include <cstring>
#include <vector>

using namespace dcpp;

// ─── Static helpers ─────────────────────────────────────────────────────

TEST_CASE("TigerTree: calcBlocks basic", "[merkletree]") {
    // 1024-byte block size: 1-byte file → 1 block
    REQUIRE(TigerTree::calcBlocks(1, 1024) == 1);
    // Exactly 1 block
    REQUIRE(TigerTree::calcBlocks(1024, 1024) == 1);
    // Just over 1 block
    REQUIRE(TigerTree::calcBlocks(1025, 1024) == 2);
    // Large file
    REQUIRE(TigerTree::calcBlocks(1024 * 1024, 1024) == 1024);
}

TEST_CASE("TigerTree: calcBlocks zero file", "[merkletree]") {
    // Zero-byte file: always at least 1 block
    REQUIRE(TigerTree::calcBlocks(0, 1024) == 1);
}

TEST_CASE("TigerTree: calcBlockSize", "[merkletree]") {
    // For a small file with 10 levels, block size should be base (1024)
    int64_t bs = TigerTree::calcBlockSize(1024, 10);
    REQUIRE(bs == 1024);

    // For a larger file, block size increases
    int64_t bs2 = TigerTree::calcBlockSize(10 * 1024 * 1024, 10);  // 10MB
    REQUIRE(bs2 >= 1024);
    // The max leaves for 10 levels is 2^9 = 512 leaves
    // 10MB / 512 = 20480, so block size should be at least 20480
    // but it must be a power of 2 * 1024
    REQUIRE(bs2 * 512 >= 10 * 1024 * 1024);
}

TEST_CASE("TigerTree: calcBlockSize doubling", "[merkletree]") {
    // Very large file forces block size to double
    int64_t huge = int64_t(1024) * 1024 * 1024 * 10; // 10GB
    int64_t bs = TigerTree::calcBlockSize(huge, 10);
    REQUIRE(bs > 1024);
    REQUIRE(bs % 1024 == 0);  // always a multiple of base
    // Power of 2 multiple
    REQUIRE((bs / 1024 & (bs / 1024 - 1)) == 0);
}

// ─── Hashing ────────────────────────────────────────────────────────────

TEST_CASE("TigerTree: hash empty data", "[merkletree]") {
    TigerTree tt;
    tt.finalize();

    // Should produce a valid root for 0-byte data
    auto& root = tt.getRoot();
    std::string b32 = Encoder::toBase32(root.data, TigerTree::BYTES);
    REQUIRE(b32.length() == 39); // base32(24 bytes) = 39 chars
    REQUIRE(tt.getFileSize() == 0);
    REQUIRE(tt.getLeaves().size() == 1);
}

TEST_CASE("TigerTree: hash small data", "[merkletree]") {
    const char* data = "Hello, World!";
    size_t len = strlen(data);

    TigerTree tt;
    tt.update(data, len);
    tt.finalize();

    REQUIRE(tt.getFileSize() == (int64_t)len);
    REQUIRE(tt.getLeaves().size() == 1); // < 1024 bytes = 1 leaf
    REQUIRE(tt.getBlockSize() == 1024);

    // Root should be deterministic
    std::string b32 = Encoder::toBase32(tt.getRoot().data, TigerTree::BYTES);
    REQUIRE(b32.length() == 39);
}

TEST_CASE("TigerTree: same data produces same root", "[merkletree]") {
    const char* data = "test data for merkle tree";
    size_t len = strlen(data);

    TigerTree t1, t2;
    t1.update(data, len);
    t1.finalize();

    t2.update(data, len);
    t2.finalize();

    REQUIRE(memcmp(t1.getRoot().data, t2.getRoot().data, TigerTree::BYTES) == 0);
}

TEST_CASE("TigerTree: different data produces different root", "[merkletree]") {
    TigerTree t1, t2;
    const char* d1 = "data1";
    const char* d2 = "data2";

    t1.update(d1, strlen(d1));
    t1.finalize();

    t2.update(d2, strlen(d2));
    t2.finalize();

    REQUIRE(memcmp(t1.getRoot().data, t2.getRoot().data, TigerTree::BYTES) != 0);
}

TEST_CASE("TigerTree: multi-block data", "[merkletree]") {
    // Create data that spans multiple blocks (> 1024 bytes)
    std::vector<uint8_t> data(3000, 0xAA);

    TigerTree tt;
    tt.update(data.data(), data.size());
    tt.finalize();

    REQUIRE(tt.getFileSize() == 3000);
    REQUIRE(tt.getLeaves().size() == 3); // ceil(3000/1024) = 3
}

TEST_CASE("TigerTree: incremental update matches single-shot", "[merkletree]") {
    std::vector<uint8_t> data(2048, 0x42);

    // Single-shot
    TigerTree single;
    single.update(data.data(), data.size());
    single.finalize();

    // Incremental in 1024-byte chunks (must be multiple of baseBlockSize)
    TigerTree inc;
    inc.update(data.data(), 1024);
    inc.update(data.data() + 1024, 1024);
    inc.finalize();

    REQUIRE(memcmp(single.getRoot().data, inc.getRoot().data, TigerTree::BYTES) == 0);
}

TEST_CASE("TigerTree: verifyRoot", "[merkletree]") {
    const char* data = "verify me";
    TigerTree tt;
    tt.update(data, strlen(data));
    tt.finalize();

    // Copy root
    uint8_t root[TigerTree::BYTES];
    memcpy(root, tt.getRoot().data, TigerTree::BYTES);

    REQUIRE(tt.verifyRoot(root) == true);

    // Corrupt one byte
    root[0] ^= 0xFF;
    REQUIRE(tt.verifyRoot(root) == false);
}

TEST_CASE("TigerTree: getLeafData", "[merkletree]") {
    std::vector<uint8_t> data(2048, 0x55);

    TigerTree tt;
    tt.update(data.data(), data.size());
    tt.finalize();

    auto leafData = tt.getLeafData();
    REQUIRE(leafData.size() == tt.getLeaves().size() * TigerTree::BYTES);

    // First leaf hash should match
    REQUIRE(memcmp(leafData.data(), tt.getLeaves()[0].data, TigerTree::BYTES) == 0);
}

TEST_CASE("TigerTree: custom block size", "[merkletree]") {
    TigerTree tt(2048); // 2048-byte blocks
    REQUIRE(tt.getBlockSize() == 2048);

    std::vector<uint8_t> data(4096, 0x33);
    tt.update(data.data(), data.size());
    tt.finalize();

    REQUIRE(tt.getFileSize() == 4096);
    // With 2048-byte blocks, 4096 bytes = 2 leaves
    REQUIRE(tt.getLeaves().size() == 2);
}

TEST_CASE("TigerTree: single-root constructor", "[merkletree]") {
    // Create a tree, get its root, then create a new tree from that root
    const char* data = "single root test";
    TigerTree orig;
    orig.update(data, strlen(data));
    orig.finalize();

    TigerTree fromRoot(orig.getFileSize(), orig.getBlockSize(), orig.getRoot());
    REQUIRE(fromRoot.getFileSize() == orig.getFileSize());
    REQUIRE(fromRoot.getBlockSize() == orig.getBlockSize());
    REQUIRE(memcmp(fromRoot.getRoot().data, orig.getRoot().data, TigerTree::BYTES) == 0);
    REQUIRE(fromRoot.getLeaves().size() == 1);
}
