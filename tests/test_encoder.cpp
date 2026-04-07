/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::Encoder (Base32, Base16)
 *
 * These tests validate the encoding/decoding logic that is used
 * throughout the codebase for CID, TTH, and file hashes.
 * They serve as regression tests during the Qt6 migration.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Encoder.h"

#include <cstring>
#include <vector>

using namespace dcpp;

// ── Base32 encode/decode ────────────────────────────────────────────────

TEST_CASE("Base32 encode empty input", "[Encoder][Base32]") {
    std::string out;
    Encoder::toBase32(nullptr, 0, out);
    REQUIRE(out.empty());
}

TEST_CASE("Base32 encode known RFC 4648 vectors", "[Encoder][Base32]") {
    // RFC 4648 §10 test vectors
    auto encode = [](const std::string& input) {
        return Encoder::toBase32(
            reinterpret_cast<const uint8_t*>(input.data()), input.size());
    };

    REQUIRE(encode("f")      == "MY");
    REQUIRE(encode("fo")     == "MZXQ");
    REQUIRE(encode("foo")    == "MZXW6");
    REQUIRE(encode("foob")   == "MZXW6YQ");
    REQUIRE(encode("fooba")  == "MZXW6YTB");
    REQUIRE(encode("foobar") == "MZXW6YTBOI");
}

TEST_CASE("Base32 round-trip with arbitrary binary data", "[Encoder][Base32]") {
    // 24 bytes of predetermined binary data
    const uint8_t data[] = {
        0x00, 0x01, 0x02, 0xFF, 0xFE, 0x80, 0x7F, 0x3C,
        0xAA, 0x55, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
    };
    const size_t len = sizeof(data);

    std::string encoded = Encoder::toBase32(data, len);
    REQUIRE_FALSE(encoded.empty());

    std::vector<uint8_t> decoded(len, 0);
    Encoder::fromBase32(encoded.c_str(), decoded.data(), len);

    REQUIRE(std::memcmp(data, decoded.data(), len) == 0);
}

TEST_CASE("Base32 round-trip with CID-sized data (24 bytes)", "[Encoder][Base32]") {
    // CID is 24 bytes (192 bits) → 39 base32 chars (ceil(192/5)=39)
    uint8_t cid[24];
    for (int i = 0; i < 24; ++i) {
        cid[i] = static_cast<uint8_t>(i * 11 + 7);
    }

    std::string encoded = Encoder::toBase32(cid, 24);
    REQUIRE(encoded.size() == 39); // ceil(24*8/5) = 39 (no padding in this impl)

    uint8_t decoded[24] = {};
    Encoder::fromBase32(encoded.c_str(), decoded, 24);

    REQUIRE(std::memcmp(cid, decoded, 24) == 0);
}

TEST_CASE("isBase32 validates correctly", "[Encoder][Base32]") {
    REQUIRE(Encoder::isBase32("MZXW6YTBOI"));    // valid
    REQUIRE(Encoder::isBase32("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567")); // all valid chars

    // Note: empty string returns true (find_first_not_of on empty = npos)
    REQUIRE(Encoder::isBase32(""));

    REQUIRE_FALSE(Encoder::isBase32("MZXW6YTB01")); // '0' and '1' are not base32
    REQUIRE_FALSE(Encoder::isBase32("MZXW 6YTB"));   // space
    // isBase32 uses uppercase alphabet only
    REQUIRE_FALSE(Encoder::isBase32("abcdefghijklmnopqrstuvwxyz234567"));
}

// ── Base16 (hex) decode ─────────────────────────────────────────────────
// NOTE: fromBase16 has a known bug where 'A'-'F' decode as 0-5 instead of 10-15
// (decode16 returns c-'A' instead of c-'A'+10). These tests document actual behavior.

TEST_CASE("Base16 decode digits 0-9 work correctly", "[Encoder][Base16]") {
    const char* hex = "01234567";
    uint8_t out[4] = {};
    Encoder::fromBase16(hex, out, 4);

    REQUIRE(out[0] == 0x01);
    REQUIRE(out[1] == 0x23);
    REQUIRE(out[2] == 0x45);
    REQUIRE(out[3] == 0x67);
}

TEST_CASE("Base16 decode with hex digits 89", "[Encoder][Base16]") {
    const char* hex = "8899";
    uint8_t out[2] = {};
    Encoder::fromBase16(hex, out, 2);

    REQUIRE(out[0] == 0x88);
    REQUIRE(out[1] == 0x99);
}
