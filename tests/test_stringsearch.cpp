/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::StringSearch (Quick Search / Boyer-Moore variant)
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "StringSearch.h"

using namespace dcpp;

// ── Basic matching ──────────────────────────────────────────────────────

TEST_CASE("StringSearch: exact match", "[StringSearch]") {
    StringSearch ss("hello");
    REQUIRE(ss.match("hello"));
}

TEST_CASE("StringSearch: substring match", "[StringSearch]") {
    StringSearch ss("world");
    REQUIRE(ss.match("hello world!"));
}

TEST_CASE("StringSearch: case-insensitive match", "[StringSearch]") {
    StringSearch ss("hello");
    REQUIRE(ss.match("HELLO"));
    REQUIRE(ss.match("Hello"));
    REQUIRE(ss.match("hElLo"));
}

TEST_CASE("StringSearch: no match", "[StringSearch]") {
    StringSearch ss("xyz");
    REQUIRE_FALSE(ss.match("hello world"));
}

TEST_CASE("StringSearch: pattern longer than text", "[StringSearch]") {
    StringSearch ss("a very long pattern");
    REQUIRE_FALSE(ss.match("short"));
}

TEST_CASE("StringSearch: match at start of text", "[StringSearch]") {
    StringSearch ss("abc");
    REQUIRE(ss.match("abcdefgh"));
}

TEST_CASE("StringSearch: match at end of text", "[StringSearch]") {
    StringSearch ss("xyz");
    REQUIRE(ss.match("abcxyz"));
}

TEST_CASE("StringSearch: single character pattern", "[StringSearch]") {
    StringSearch ss("a");
    REQUIRE(ss.match("a"));
    REQUIRE(ss.match("bac"));
    REQUIRE_FALSE(ss.match("xyz"));
}

TEST_CASE("StringSearch: text equals pattern", "[StringSearch]") {
    StringSearch ss("exact");
    REQUIRE(ss.match("exact"));
}

// ── getPattern ──────────────────────────────────────────────────────────

TEST_CASE("StringSearch: getPattern returns lowercase", "[StringSearch]") {
    StringSearch ss("MiXeD");
    REQUIRE(ss.getPattern() == "mixed");
}

// ── Copy and assignment ─────────────────────────────────────────────────

TEST_CASE("StringSearch: copy constructor", "[StringSearch]") {
    StringSearch original("test");
    StringSearch copy(original);

    REQUIRE(copy.getPattern() == "test");
    REQUIRE(copy.match("This is a TEST"));
}

TEST_CASE("StringSearch: assignment operator from StringSearch", "[StringSearch]") {
    StringSearch a("first");
    StringSearch b("second");

    a = b;
    REQUIRE(a.getPattern() == "second");
    REQUIRE(a.match("the second one"));
}

TEST_CASE("StringSearch: assignment operator from string", "[StringSearch]") {
    StringSearch ss("old");
    ss = std::string("new");

    REQUIRE(ss.getPattern() == "new");
    REQUIRE(ss.match("brand new"));
    REQUIRE_FALSE(ss.match("old"));
}

// ── Equality ────────────────────────────────────────────────────────────

TEST_CASE("StringSearch: equality operator", "[StringSearch]") {
    StringSearch a("hello");
    StringSearch b("hello");
    StringSearch c("world");

    REQUIRE(a.getPattern() == b.getPattern());
    REQUIRE_FALSE(a.getPattern() == c.getPattern());
}

// ── Default constructor ─────────────────────────────────────────────────

TEST_CASE("StringSearch: default constructor has empty pattern", "[StringSearch]") {
    StringSearch ss;
    REQUIRE(ss.getPattern().empty());
}

// ── Multiple matches in text ────────────────────────────────────────────

TEST_CASE("StringSearch: pattern appears multiple times", "[StringSearch]") {
    StringSearch ss("ab");
    REQUIRE(ss.match("ababab"));
}

// ── Special characters ──────────────────────────────────────────────────

TEST_CASE("StringSearch: pattern with spaces", "[StringSearch]") {
    StringSearch ss("hello world");
    REQUIRE(ss.match("say hello world!"));
    REQUIRE_FALSE(ss.match("helloworld"));
}

TEST_CASE("StringSearch: pattern with numbers", "[StringSearch]") {
    StringSearch ss("v2.0");
    REQUIRE(ss.match("Release v2.0 is here"));
}
