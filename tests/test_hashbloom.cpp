/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::HashBloom
 *
 * HashBloom is a Bloom filter keyed on TTH values. We test basic
 * add/match round-trips, false-negative guarantee, and helper functions
 * get_k() and get_m() for optimal parameter calculation.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "HashBloom.h"
#include "MerkleTree.h"
#include "TigerHash.h"
#include "Encoder.h"

#include <cstring>
#include <string>

using namespace dcpp;

// ── Helper: create a TTHValue from a string by hashing it ───────────────

static TTHValue makeTTH(const std::string& s) {
    TigerHash ctx;
    ctx.update(s.data(), s.size());
    return TTHValue(ctx.finalize());
}

// ── get_k / get_m ───────────────────────────────────────────────────────

TEST_CASE("HashBloom::get_m returns 64-bit aligned value", "[HashBloom]") {
    for (size_t n : {1, 10, 100, 1000}) {
        for (size_t k : {1, 2, 4, 8}) {
            uint64_t m = HashBloom::get_m(n, k);
            REQUIRE(m % 64 == 0);
        }
    }
}

TEST_CASE("HashBloom::get_m grows with n", "[HashBloom]") {
    uint64_t m1 = HashBloom::get_m(100, 4);
    uint64_t m2 = HashBloom::get_m(1000, 4);
    REQUIRE(m2 > m1);
}

TEST_CASE("HashBloom::get_k returns at least 1", "[HashBloom]") {
    REQUIRE(HashBloom::get_k(1, 16) >= 1);
    REQUIRE(HashBloom::get_k(100, 16) >= 1);
    REQUIRE(HashBloom::get_k(1000000, 16) >= 1);
}

TEST_CASE("HashBloom::get_k decreases as n grows large", "[HashBloom]") {
    // With more items, fewer hash functions fit within the key bits
    size_t k_small = HashBloom::get_k(10, 16);
    size_t k_large = HashBloom::get_k(10000000, 16);
    REQUIRE(k_large <= k_small);
}

// ── Empty bloom ─────────────────────────────────────────────────────────

TEST_CASE("Empty bloom matches nothing", "[HashBloom]") {
    HashBloom bloom;
    TTHValue tth = makeTTH("test");
    REQUIRE_FALSE(bloom.match(tth));
}

// ── Add and match ───────────────────────────────────────────────────────

TEST_CASE("HashBloom add then match returns true", "[HashBloom]") {
    size_t n = 100;
    size_t h = 16;
    size_t k = HashBloom::get_k(n, h);
    uint64_t m = HashBloom::get_m(n, k);

    HashBloom bloom;
    bloom.reset(k, static_cast<size_t>(m), h);

    TTHValue tth = makeTTH("hello");
    bloom.add(tth);

    REQUIRE(bloom.match(tth));
}

TEST_CASE("HashBloom add multiple items, all match", "[HashBloom]") {
    size_t n = 50;
    size_t h = 16;
    size_t k = HashBloom::get_k(n, h);
    uint64_t m = HashBloom::get_m(n, k);

    HashBloom bloom;
    bloom.reset(k, static_cast<size_t>(m), h);

    std::vector<TTHValue> items;
    for (int i = 0; i < 50; ++i) {
        TTHValue tth = makeTTH("item_" + std::to_string(i));
        items.push_back(tth);
        bloom.add(tth);
    }

    // No false negatives — all added items must match
    for (const auto& tth : items) {
        REQUIRE(bloom.match(tth));
    }
}

TEST_CASE("HashBloom copy_to produces non-empty byte vector", "[HashBloom]") {
    size_t n = 10;
    size_t h = 16;
    size_t k = HashBloom::get_k(n, h);
    uint64_t m = HashBloom::get_m(n, k);

    HashBloom bloom;
    bloom.reset(k, static_cast<size_t>(m), h);

    bloom.add(makeTTH("data"));

    ByteVector v;
    bloom.copy_to(v);
    REQUIRE_FALSE(v.empty());
    REQUIRE(v.size() == m / 8);
}

TEST_CASE("HashBloom push_back grows bloom", "[HashBloom]") {
    HashBloom bloom;
    bloom.reset(2, 64, 16);

    // push_back individual bits
    for (size_t i = 0; i < 64; ++i) {
        bloom.push_back(i % 2 == 0);
    }

    ByteVector v;
    bloom.copy_to(v);
    // Original 64 bits from reset + 64 pushed = 128 bits = 16 bytes
    REQUIRE(v.size() == 16);
}
