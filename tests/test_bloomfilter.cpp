/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::BloomFilter<N>
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "BloomFilter.h"

#include <string>
#include <vector>

using namespace dcpp;

// ── Basic add/match ─────────────────────────────────────────────────────

TEST_CASE("BloomFilter: added strings match", "[BloomFilter]") {
    BloomFilter<5> bf(1024);

    bf.add("hello world");
    REQUIRE(bf.match("hello world"));
}

TEST_CASE("BloomFilter: non-added strings may not match", "[BloomFilter]") {
    // A bloom filter can have false positives but not false negatives
    // We test with a large enough table to reduce false positives
    BloomFilter<3> bf(65536);

    bf.add("teststring");

    // The added string must match
    REQUIRE(bf.match("teststring"));

    // Completely unrelated strings shouldn't match (with high probability)
    // We use a string with very different character content
    REQUIRE_FALSE(bf.match("zzzzzzzzzzz"));
}

TEST_CASE("BloomFilter: empty bloom matches everything (no n-grams to check)", "[BloomFilter]") {
    BloomFilter<5> bf(1024);
    // An empty bloom filter should match any string because there are no set bits to fail on
    // Actually, match() checks if all n-grams are present; with no bits set, only strings
    // shorter than N match (because the loop doesn't execute)
    REQUIRE(bf.match("abc")); // shorter than N=5, loop doesn't run
    REQUIRE(bf.match(""));
}

TEST_CASE("BloomFilter: short strings always match", "[BloomFilter]") {
    BloomFilter<5> bf(1024);
    bf.add("some data here");

    // Strings shorter than N always match because no n-grams can be checked
    REQUIRE(bf.match("xy"));
    REQUIRE(bf.match("a"));
    REQUIRE(bf.match(""));
}

// ── StringList match ────────────────────────────────────────────────────

TEST_CASE("BloomFilter: match with StringList - all present", "[BloomFilter]") {
    BloomFilter<3> bf(65536);

    bf.add("hello");
    bf.add("world");

    StringList terms = {"hello", "world"};
    REQUIRE(bf.match(terms));
}

TEST_CASE("BloomFilter: match with StringList - one missing", "[BloomFilter]") {
    BloomFilter<3> bf(65536);

    bf.add("hello");

    StringList terms = {"hello", "xyzxyzxyz"};
    // Should fail because "xyzxyzxyz" is not in the filter (with high probability)
    REQUIRE_FALSE(bf.match(terms));
}

TEST_CASE("BloomFilter: match with empty StringList", "[BloomFilter]") {
    BloomFilter<3> bf(1024);
    StringList empty;
    REQUIRE(bf.match(empty));
}

// ── Clear ───────────────────────────────────────────────────────────────

TEST_CASE("BloomFilter: clear resets the filter", "[BloomFilter]") {
    BloomFilter<3> bf(65536);

    bf.add("data");
    REQUIRE(bf.match("data"));

    bf.clear();

    // After clear, strings of length >= N should no longer match
    // (unless they happen to be false positives, which is unlikely with 65536 table)
    REQUIRE_FALSE(bf.match("data"));
}

// ── Multiple adds ───────────────────────────────────────────────────────

TEST_CASE("BloomFilter: multiple adds accumulate", "[BloomFilter]") {
    BloomFilter<3> bf(65536);

    bf.add("alpha");
    bf.add("bravo");
    bf.add("charlie");

    REQUIRE(bf.match("alpha"));
    REQUIRE(bf.match("bravo"));
    REQUIRE(bf.match("charlie"));
}

// ── Different N values ──────────────────────────────────────────────────

TEST_CASE("BloomFilter<1>: single character n-grams", "[BloomFilter]") {
    BloomFilter<1> bf(256);

    bf.add("abc");

    REQUIRE(bf.match("abc"));
    REQUIRE(bf.match("a"));
    REQUIRE(bf.match("b"));
    REQUIRE(bf.match("c"));
}

TEST_CASE("BloomFilter<2>: two character n-grams", "[BloomFilter]") {
    BloomFilter<2> bf(65536);

    bf.add("hello");
    REQUIRE(bf.match("hello"));
}

// ── Small table size ────────────────────────────────────────────────────

TEST_CASE("BloomFilter: small table has more false positives but still works", "[BloomFilter]") {
    BloomFilter<3> bf(8); // very small table — many collisions

    bf.add("test");
    REQUIRE(bf.match("test"));
    // With only 8 buckets, false positives are very likely
    // but the added string must always match
}
