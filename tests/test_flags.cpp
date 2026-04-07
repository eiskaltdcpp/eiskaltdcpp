/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::Flags (bitmask helper class)
 */

#include <catch2/catch_test_macros.hpp>

#include "Flags.h"

using namespace dcpp;

// ── Construction ────────────────────────────────────────────────────────

TEST_CASE("Flags: default constructor initializes to 0", "[Flags]") {
    Flags f;
    REQUIRE(f.getFlags() == 0);
}

TEST_CASE("Flags: construct with initial value", "[Flags]") {
    Flags f(0x0F);
    REQUIRE(f.getFlags() == 0x0F);
}

// ── setFlag / isSet ─────────────────────────────────────────────────────

TEST_CASE("Flags: setFlag and isSet", "[Flags]") {
    Flags f;

    f.setFlag(0x01);
    REQUIRE(f.isSet(0x01));
    REQUIRE_FALSE(f.isSet(0x02));

    f.setFlag(0x02);
    REQUIRE(f.isSet(0x01));
    REQUIRE(f.isSet(0x02));
    REQUIRE(f.isSet(0x03)); // both bits
}

// ── unsetFlag ───────────────────────────────────────────────────────────

TEST_CASE("Flags: unsetFlag clears bit", "[Flags]") {
    Flags f(0x0F);

    f.unsetFlag(0x01);
    REQUIRE_FALSE(f.isSet(0x01));
    REQUIRE(f.isSet(0x0E));
}

TEST_CASE("Flags: unsetFlag already-unset bit is no-op", "[Flags]") {
    Flags f(0x02);
    f.unsetFlag(0x01); // wasn't set
    REQUIRE(f.getFlags() == 0x02); // unchanged
}

// ── isAnySet ────────────────────────────────────────────────────────────

TEST_CASE("Flags: isAnySet checks any bit in mask", "[Flags]") {
    Flags f(0x05); // bits 0 and 2

    REQUIRE(f.isAnySet(0x01)); // bit 0 set
    REQUIRE(f.isAnySet(0x04)); // bit 2 set
    REQUIRE(f.isAnySet(0x03)); // bit 0 set (even though bit 1 isn't)
    REQUIRE_FALSE(f.isAnySet(0x02)); // only bit 1, which isn't set
}

// ── isSet requires ALL bits ─────────────────────────────────────────────

TEST_CASE("Flags: isSet requires all bits in mask", "[Flags]") {
    Flags f(0x05); // bits 0 and 2

    REQUIRE(f.isSet(0x01));
    REQUIRE(f.isSet(0x04));
    REQUIRE(f.isSet(0x05)); // both bits present
    REQUIRE_FALSE(f.isSet(0x07)); // bit 1 is missing
}

// ── Multiple flag operations ────────────────────────────────────────────

TEST_CASE("Flags: complex flag manipulation", "[Flags]") {
    Flags f;

    // Set multiple flags
    f.setFlag(0x01);
    f.setFlag(0x10);
    f.setFlag(0x100);

    REQUIRE(f.getFlags() == 0x111);

    // Clear middle flag
    f.unsetFlag(0x10);
    REQUIRE(f.getFlags() == 0x101);

    // isAnySet still finds outer flags
    REQUIRE(f.isAnySet(0x10) == false);
    REQUIRE(f.isAnySet(0x01));
    REQUIRE(f.isAnySet(0x100));
}

// ── Edge cases ──────────────────────────────────────────────────────────

TEST_CASE("Flags: zero flag operations", "[Flags]") {
    Flags f(0xFF);

    // isSet(0) should be true (0 & anything == 0 == 0)
    REQUIRE(f.isSet(0));
    // isAnySet(0) should be false (0 & anything == 0 != 0 is false)
    REQUIRE_FALSE(f.isAnySet(0));
}

TEST_CASE("Flags: set same flag twice", "[Flags]") {
    Flags f;
    f.setFlag(0x04);
    f.setFlag(0x04);
    REQUIRE(f.getFlags() == 0x04);
}
