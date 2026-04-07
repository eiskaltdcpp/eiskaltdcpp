/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::CID
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "CID.h"

using namespace dcpp;

TEST_CASE("CID default constructor creates zero CID", "[CID]") {
    CID cid;
    // A default-constructed CID should be all zeros
    REQUIRE_FALSE(static_cast<bool>(cid));

    // All bytes should be zero
    for (int i = 0; i < CID::SIZE; ++i) {
        REQUIRE(cid.data()[i] == 0);
    }
}

TEST_CASE("CID from raw bytes round-trips", "[CID]") {
    uint8_t raw[CID::SIZE];
    for (int i = 0; i < CID::SIZE; ++i) {
        raw[i] = static_cast<uint8_t>(i);
    }

    CID cid(raw);
    REQUIRE(static_cast<bool>(cid));
    REQUIRE(memcmp(cid.data(), raw, CID::SIZE) == 0);
}

TEST_CASE("CID base32 round-trip", "[CID]") {
    uint8_t raw[CID::SIZE];
    for (int i = 0; i < CID::SIZE; ++i) {
        raw[i] = static_cast<uint8_t>(i * 7 + 3);
    }

    CID original(raw);
    std::string b32 = original.toBase32();
    REQUIRE_FALSE(b32.empty());

    CID decoded(b32);
    REQUIRE(original == decoded);
}

TEST_CASE("CID equality and comparison", "[CID]") {
    uint8_t raw1[CID::SIZE] = {};
    uint8_t raw2[CID::SIZE] = {};
    raw1[0] = 1;
    raw2[0] = 2;

    CID cid1(raw1);
    CID cid2(raw2);
    CID cid1_copy(raw1);

    REQUIRE(cid1 == cid1_copy);
    REQUIRE_FALSE(cid1 == cid2);
    REQUIRE(cid1 < cid2);
}

TEST_CASE("CID::generate creates unique non-zero CIDs", "[CID]") {
    CID a = CID::generate();
    CID b = CID::generate();

    REQUIRE(static_cast<bool>(a));
    REQUIRE(static_cast<bool>(b));
    REQUIRE_FALSE(a == b);
}
