/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for Wildcard pattern matching
 *
 * Tests Wildcard::wildcardfit(), Wildcard::patternMatch(), and delimiter-based
 * pattern lists. Covers *, ?, character sets [a-z], negation [!abc], and
 * the useSet=false mode.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Wildcards.h"

#include <string>

// ── wildcardfit: star (*) ───────────────────────────────────────────────

TEST_CASE("Star matches everything", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("*", "anything") == 1);
    REQUIRE(Wildcard::wildcardfit("*", "") == 1);
    REQUIRE(Wildcard::wildcardfit("*", "a") == 1);
}

TEST_CASE("Star prefix", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("*.txt", "file.txt") == 1);
    REQUIRE(Wildcard::wildcardfit("*.txt", "file.doc") == 0);
}

TEST_CASE("Star suffix", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("file*", "file.txt") == 1);
    REQUIRE(Wildcard::wildcardfit("file*", "archive.zip") == 0);
}

TEST_CASE("Star in the middle", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("f*e", "file") == 1);
    REQUIRE(Wildcard::wildcardfit("f*e", "frame") == 1);
    REQUIRE(Wildcard::wildcardfit("f*e", "fox") == 0);
}

TEST_CASE("Multiple stars", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("*.*", "a.b") == 1);
    REQUIRE(Wildcard::wildcardfit("*.*", "noext") == 0);
    // *a*b* matches xaybz because * can match any substring including 'z'
    REQUIRE(Wildcard::wildcardfit("*a*b*", "xaybz") == 1);
    REQUIRE(Wildcard::wildcardfit("*a*b*", "xayb") == 1);
    REQUIRE(Wildcard::wildcardfit("*a*b*", "xy") == 0);
}

// ── wildcardfit: question mark (?) ──────────────────────────────────────

TEST_CASE("Question mark matches single char", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("?", "a") == 1);
    REQUIRE(Wildcard::wildcardfit("?", "") == 0);
    REQUIRE(Wildcard::wildcardfit("?", "ab") == 0);
}

TEST_CASE("Question marks for exact length", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("???", "abc") == 1);
    REQUIRE(Wildcard::wildcardfit("???", "ab") == 0);
    REQUIRE(Wildcard::wildcardfit("???", "abcd") == 0);
}

TEST_CASE("Mixed star and question mark", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("?*", "a") == 1);
    REQUIRE(Wildcard::wildcardfit("?*", "ab") == 1);
    REQUIRE(Wildcard::wildcardfit("?*", "") == 0);
    REQUIRE(Wildcard::wildcardfit("*.??", "file.cc") == 1);
    REQUIRE(Wildcard::wildcardfit("*.??", "file.cpp") == 0);
}

// ── wildcardfit: character sets ─────────────────────────────────────────

TEST_CASE("Character set basic", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("[abc]", "a") == 1);
    REQUIRE(Wildcard::wildcardfit("[abc]", "b") == 1);
    REQUIRE(Wildcard::wildcardfit("[abc]", "d") == 0);
}

TEST_CASE("Character set range", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("[a-z]", "m") == 1);
    REQUIRE(Wildcard::wildcardfit("[a-z]", "A") == 0);
    REQUIRE(Wildcard::wildcardfit("[0-9]", "5") == 1);
    REQUIRE(Wildcard::wildcardfit("[0-9]", "a") == 0);
}

TEST_CASE("Character set negation", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("[!abc]", "d") == 1);
    REQUIRE(Wildcard::wildcardfit("[!abc]", "a") == 0);
    REQUIRE(Wildcard::wildcardfit("[!0-9]", "x") == 1);
    REQUIRE(Wildcard::wildcardfit("[!0-9]", "5") == 0);
}

TEST_CASE("Character set combined with wildcards", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit("[a-g]l*i?n", "florian") == 1);
    REQUIRE(Wildcard::wildcardfit("[!abc]*e", "smile") == 1);
}

TEST_CASE("useSet=false disables set matching", "[Wildcard]") {
    // With useSet=false, brackets should be treated as literals
    REQUIRE(Wildcard::wildcardfit("[a]", "a", false) == 0);
    REQUIRE(Wildcard::wildcardfit("[a]", "[a]", false) == 1);
}

// ── patternMatch (string interface) ─────────────────────────────────────

TEST_CASE("patternMatch basic string", "[Wildcard]") {
    REQUIRE(Wildcard::patternMatch("hello.txt", "*.txt"));
    REQUIRE_FALSE(Wildcard::patternMatch("hello.doc", "*.txt"));
}

TEST_CASE("patternMatch is case-insensitive (lowered internally)", "[Wildcard]") {
    // patternMatch uses Text::toLower internally
    REQUIRE(Wildcard::patternMatch("FILE.TXT", "*.txt"));
    REQUIRE(Wildcard::patternMatch("File.Txt", "*.txt"));
}

TEST_CASE("patternMatch with delimiter list", "[Wildcard]") {
    REQUIRE(Wildcard::patternMatch("image.png", "*.jpg;*.png;*.gif", ';'));
    REQUIRE(Wildcard::patternMatch("photo.jpg", "*.jpg;*.png;*.gif", ';'));
    REQUIRE_FALSE(Wildcard::patternMatch("doc.pdf", "*.jpg;*.png;*.gif", ';'));
}

TEST_CASE("patternMatch empty pattern", "[Wildcard]") {
    // Empty pattern should not match non-empty text
    REQUIRE_FALSE(Wildcard::patternMatch("something", ""));
}

TEST_CASE("patternMatch empty text", "[Wildcard]") {
    REQUIRE(Wildcard::patternMatch("", "*"));
    REQUIRE_FALSE(Wildcard::patternMatch("", "?"));
}

// ── wchar_t overloads ───────────────────────────────────────────────────

TEST_CASE("wildcardfit wchar_t star", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit(L"*", L"anything") == 1);
    REQUIRE(Wildcard::wildcardfit(L"*.txt", L"file.txt") == 1);
    REQUIRE(Wildcard::wildcardfit(L"*.txt", L"file.doc") == 0);
}

TEST_CASE("wildcardfit wchar_t question mark", "[Wildcard]") {
    REQUIRE(Wildcard::wildcardfit(L"?", L"a") == 1);
    REQUIRE(Wildcard::wildcardfit(L"???", L"abc") == 1);
    REQUIRE(Wildcard::wildcardfit(L"???", L"ab") == 0);
}

TEST_CASE("patternMatch wchar_t basic", "[Wildcard]") {
    REQUIRE(Wildcard::patternMatch(std::wstring(L"hello.txt"), std::wstring(L"*.txt")));
    REQUIRE_FALSE(Wildcard::patternMatch(std::wstring(L"hello.doc"), std::wstring(L"*.txt")));
}

TEST_CASE("patternMatch wchar_t with delimiter", "[Wildcard]") {
    REQUIRE(Wildcard::patternMatch(std::wstring(L"image.png"), std::wstring(L"*.jpg;*.png"), L';'));
}
