/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::TigerHash
 *
 * Test vectors come from the official Tiger reference implementation
 * by Eli Biham and Ross Anderson. The Tiger hash produces 192-bit (24-byte)
 * digests. We encode results as Base32 (using dcpp::Encoder) for easy
 * comparison with known TTH values used across the DC++ ecosystem.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "TigerHash.h"
#include "Encoder.h"

#include <cstring>
#include <string>
#include <vector>

using namespace dcpp;

// ── Helper: compute Tiger hash of a byte buffer and return hex string ───

static std::string tigerHex(const void* data, size_t len) {
    TigerHash ctx;
    ctx.update(data, len);
    uint8_t* result = ctx.finalize();

    // Convert 24 bytes to hex
    std::string hex;
    hex.reserve(48);
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < TigerHash::BYTES; ++i) {
        hex += digits[result[i] >> 4];
        hex += digits[result[i] & 0x0f];
    }
    return hex;
}

static std::string tigerHex(const std::string& s) {
    return tigerHex(s.data(), s.size());
}

static std::string tigerBase32(const void* data, size_t len) {
    TigerHash ctx;
    ctx.update(data, len);
    uint8_t* result = ctx.finalize();
    return Encoder::toBase32(result, TigerHash::BYTES);
}

static std::string tigerBase32(const std::string& s) {
    return tigerBase32(s.data(), s.size());
}

// ── Official Tiger test vectors ─────────────────────────────────────────
// Reference: https://www.cs.technion.ac.il/~biham/Reports/Tiger/

TEST_CASE("Tiger hash of empty string", "[TigerHash]") {
    // Tiger("") = 3293AC630C13F0245F92BBB1766E16167A4E58492DDE73F3
    REQUIRE(tigerHex("") == "3293ac630c13f0245f92bbb1766e16167a4e58492dde73f3");
}

TEST_CASE("Tiger hash of 'abc'", "[TigerHash]") {
    // Tiger("abc") = 2AAB1484E8C158F2BFB8C5FF41B57A525129131C957B5F93
    REQUIRE(tigerHex("abc") == "2aab1484e8c158f2bfb8c5ff41b57a525129131c957b5f93");
}

TEST_CASE("Tiger hash of 'Tiger'", "[TigerHash]") {
    // Tiger("Tiger") = DD00230799F5009FEC6DEBC838BB6A27DF2B9D6F110C7937
    REQUIRE(tigerHex("Tiger") == "dd00230799f5009fec6debc838bb6a27df2b9d6f110c7937");
}

TEST_CASE("Tiger hash of 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-'", "[TigerHash]") {
    // Tiger of the full alphanumeric + +-
    REQUIRE(tigerHex("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-") ==
            "f71c8583902afb879edfe610f82c0d4786a3a534504486b5");
}

TEST_CASE("Tiger hash of 'a' repeated 1 million times", "[TigerHash]") {
    // Tiger("aaa...a" × 10^6) = 6DB0E2729CBEAD93D715C6A7D36302E9B3CEE0D2BC314B41
    std::string million_a(1000000, 'a');
    REQUIRE(tigerHex(million_a) == "6db0e2729cbead93d715c6a7d36302e9b3cee0d2bc314b41");
}

TEST_CASE("Tiger hash of 'message digest'", "[TigerHash]") {
    // Tiger("message digest") = D981F8CB78201A950DCF3048751E441C517FCA1AA55A29F6
    REQUIRE(tigerHex("message digest") == "d981f8cb78201a950dcf3048751e441c517fca1aa55a29f6");
}

TEST_CASE("Tiger hash of 'ABCDEFGHIJKLMNOPQRSTUVWXYZ...' (long alphabet)", "[TigerHash]") {
    // Tiger("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
    REQUIRE(tigerHex("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") ==
            "8dcea680a17583ee502ba38a3c368651890ffbccdc49a8cc");
}

// ── Incremental update ──────────────────────────────────────────────────

TEST_CASE("Tiger incremental update matches single-shot", "[TigerHash]") {
    const std::string msg = "Hello, World!";

    // Single-shot
    std::string single = tigerHex(msg);

    // Incremental: feed one byte at a time
    TigerHash ctx;
    for (char c : msg) {
        ctx.update(&c, 1);
    }
    uint8_t* result = ctx.finalize();
    std::string hex;
    hex.reserve(48);
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < TigerHash::BYTES; ++i) {
        hex += digits[result[i] >> 4];
        hex += digits[result[i] & 0x0f];
    }

    REQUIRE(hex == single);
}

TEST_CASE("Tiger incremental split at block boundary", "[TigerHash]") {
    // Tiger block size is 64 bytes. Feed exactly 64, then more.
    std::string data(128, 'X');

    std::string single = tigerHex(data);

    TigerHash ctx;
    ctx.update(data.data(), 64);
    ctx.update(data.data() + 64, 64);
    uint8_t* result = ctx.finalize();

    std::string hex;
    hex.reserve(48);
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < TigerHash::BYTES; ++i) {
        hex += digits[result[i] >> 4];
        hex += digits[result[i] & 0x0f];
    }

    REQUIRE(hex == single);
}

TEST_CASE("Tiger incremental split mid-block", "[TigerHash]") {
    std::string data = "The quick brown fox jumps over the lazy dog";

    std::string single = tigerHex(data);

    // Split at position 10
    TigerHash ctx;
    ctx.update(data.data(), 10);
    ctx.update(data.data() + 10, data.size() - 10);
    uint8_t* result = ctx.finalize();

    std::string hex;
    hex.reserve(48);
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < TigerHash::BYTES; ++i) {
        hex += digits[result[i] >> 4];
        hex += digits[result[i] & 0x0f];
    }

    REQUIRE(hex == single);
}

// ── Base32 output ───────────────────────────────────────────────────────

TEST_CASE("Tiger Base32 of empty string", "[TigerHash][Base32]") {
    std::string b32 = tigerBase32("");
    // Base32 of a 24-byte hash should be ceil(24*8/5) = 39 chars, padded
    REQUIRE_FALSE(b32.empty());
    REQUIRE(b32.size() > 0);
}

TEST_CASE("Tiger Base32 round-trip via Encoder", "[TigerHash][Base32]") {
    std::string b32 = tigerBase32("test data");

    // Decode back to bytes
    uint8_t decoded[TigerHash::BYTES];
    Encoder::fromBase32(b32.c_str(), decoded, TigerHash::BYTES);

    // Compute directly
    TigerHash ctx;
    std::string input = "test data";
    ctx.update(input.data(), input.size());
    uint8_t* direct = ctx.finalize();

    REQUIRE(memcmp(decoded, direct, TigerHash::BYTES) == 0);
}

// ── Result size ─────────────────────────────────────────────────────────

TEST_CASE("TigerHash constants", "[TigerHash]") {
    REQUIRE(TigerHash::BITS == 192);
    REQUIRE(TigerHash::BYTES == 24);
}

// ── Determinism ─────────────────────────────────────────────────────────

TEST_CASE("Tiger hash is deterministic", "[TigerHash]") {
    std::string input = "deterministic test";
    std::string h1 = tigerHex(input);
    std::string h2 = tigerHex(input);
    REQUIRE(h1 == h2);
}

TEST_CASE("Different inputs produce different hashes", "[TigerHash]") {
    REQUIRE(tigerHex("input A") != tigerHex("input B"));
    REQUIRE(tigerHex("") != tigerHex("x"));
}
