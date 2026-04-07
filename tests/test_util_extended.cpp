/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Extended unit tests for dcpp::Util — static pure functions
 *
 * Supplements test_util.cpp with additional coverage for functions
 * that are pure and can be tested without filesystem/network.
 * Serves as regression tests during the Qt6 migration.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Util.h"
#include "Text.h"

using namespace dcpp;

// ── getLastDir ──────────────────────────────────────────────────────────

TEST_CASE("Util::getLastDir extracts last directory component", "[Util]") {
    REQUIRE(Util::getLastDir("/home/user/downloads/", '/') == "downloads");
    REQUIRE(Util::getLastDir("/home/user/", '/') == "user");
    REQUIRE(Util::getLastDir("/", '/') == "");
}

TEST_CASE("Util::getLastDir with no trailing separator", "[Util]") {
    // When there's no trailing separator, the implementation may vary
    // This documents current behaviour
    std::string result = Util::getLastDir("/home/user/file.txt", '/');
    // The function searches backward from len-2, so it skips the last char
    // and finds the last separator
    CHECK_FALSE(result.empty()); // just ensure it doesn't crash
}

// ── roundDown / roundUp ─────────────────────────────────────────────────

TEST_CASE("Util::roundDown rounds to nearest block boundary", "[Util]") {
    // Note: roundDown is actually "round to nearest" (adds blockSize/2 before dividing)
    REQUIRE(Util::roundDown(int64_t(100), int64_t(32)) == 96);
    REQUIRE(Util::roundDown(int64_t(64), int64_t(32)) == 64);
    REQUIRE(Util::roundDown(int64_t(0), int64_t(32)) == 0);
    REQUIRE(Util::roundDown(int64_t(31), int64_t(32)) == 32); // (31+16)/32*32 = 32
    REQUIRE(Util::roundDown(int64_t(15), int64_t(32)) == 0);  // (15+16)/32*32 = 0
}

TEST_CASE("Util::roundUp rounds up to block boundary", "[Util]") {
    REQUIRE(Util::roundUp(int64_t(100), int64_t(32)) == 128);
    REQUIRE(Util::roundUp(int64_t(64), int64_t(32)) == 64);
    REQUIRE(Util::roundUp(int64_t(0), int64_t(32)) == 0);
    REQUIRE(Util::roundUp(int64_t(1), int64_t(32)) == 32);
}

TEST_CASE("Util::roundDown/roundUp int overloads", "[Util]") {
    REQUIRE(Util::roundDown(100, 16) == 96);
    REQUIRE(Util::roundUp(100, 16) == 112);
}

// ── stricmp / strnicmp ──────────────────────────────────────────────────

TEST_CASE("Util::stricmp case-insensitive comparison", "[Util]") {
    REQUIRE(Util::stricmp("hello", "HELLO") == 0);
    REQUIRE(Util::stricmp("Hello", "hello") == 0);
    REQUIRE(Util::stricmp("abc", "abd") < 0);
    REQUIRE(Util::stricmp("abd", "abc") > 0);
    REQUIRE(Util::stricmp("", "") == 0);
}

TEST_CASE("Util::strnicmp case-insensitive prefix comparison", "[Util]") {
    REQUIRE(Util::strnicmp("Hello World", "hello", 5) == 0);
    REQUIRE(Util::strnicmp("ABCDEF", "abcxyz", 3) == 0);
    REQUIRE(Util::strnicmp("ABCDEF", "abcxyz", 4) != 0);
}

// ── findSubString ───────────────────────────────────────────────────────

TEST_CASE("Util::findSubString case-insensitive search", "[Util]") {
    REQUIRE(Util::findSubString("Hello World", "world") != std::string::npos);
    REQUIRE(Util::findSubString("Hello World", "HELLO") != std::string::npos);
    REQUIRE(Util::findSubString("Hello World", "xyz") == std::string::npos);
    REQUIRE(Util::findSubString("Hello World", "WORLD", 6) != std::string::npos);
}

// ── toHexEscape / fromHexEscape ─────────────────────────────────────────

TEST_CASE("Util::toHexEscape produces percent-encoded hex", "[Util]") {
    std::string result = Util::toHexEscape(' ');
    REQUIRE(result == "%20");
}

TEST_CASE("Util::toHexEscape and fromHexEscape round-trip", "[Util]") {
    // toHexEscape produces "%XX", fromHexEscape expects just "XX" (no %)
    for (int c = 0; c < 256; ++c) {
        std::string escaped = Util::toHexEscape(static_cast<char>(c));
        REQUIRE(escaped[0] == '%');
        REQUIRE(escaped.size() >= 2);
        // fromHexEscape takes the hex digits without the leading %
        char decoded = Util::fromHexEscape(escaped.substr(1));
        REQUIRE(static_cast<unsigned char>(decoded) == static_cast<unsigned char>(c));
    }
}

// ── URL type checks ────────────────────────────────────────────────────

TEST_CASE("Util::isAdcUrl identifies ADC URLs", "[Util]") {
    REQUIRE(Util::isAdcUrl("adc://example.com:411"));
    REQUIRE_FALSE(Util::isAdcUrl("adcs://example.com:411"));
    REQUIRE_FALSE(Util::isAdcUrl("dchub://example.com:411"));
    REQUIRE_FALSE(Util::isAdcUrl("http://example.com"));
}

TEST_CASE("Util::isAdcsUrl identifies ADCS URLs", "[Util]") {
    REQUIRE(Util::isAdcsUrl("adcs://example.com:411"));
    REQUIRE_FALSE(Util::isAdcsUrl("adc://example.com:411"));
    REQUIRE_FALSE(Util::isAdcsUrl("dchub://example.com:411"));
}

TEST_CASE("Util::isNmdcUrl identifies NMDC URLs", "[Util]") {
    REQUIRE(Util::isNmdcUrl("dchub://example.com:411"));
    REQUIRE_FALSE(Util::isNmdcUrl("adc://example.com:411"));
    REQUIRE_FALSE(Util::isNmdcUrl("http://example.com"));
}

// ── toAdcFile / toNmdcFile ──────────────────────────────────────────────

TEST_CASE("Util::toAdcFile prepends / and converts backslash", "[Util]") {
    // toAdcFile prepends '/' and converts '\\' to '/'
    REQUIRE(Util::toAdcFile("C:\\Users\\test\\file.txt") == "/C:/Users/test/file.txt");
    REQUIRE(Util::toAdcFile("share\\files\\test.dat") == "/share/files/test.dat");
    // Special files are not modified
    REQUIRE(Util::toAdcFile("files.xml.bz2") == "files.xml.bz2");
    REQUIRE(Util::toAdcFile("files.xml") == "files.xml");
}

TEST_CASE("Util::toNmdcFile strips leading / and converts to backslash", "[Util]") {
    // toNmdcFile strips first char (the leading '/') and converts '/' to '\\'
    REQUIRE(Util::toNmdcFile("/home/user/file.txt") == "home\\user\\file.txt");
    REQUIRE(Util::toNmdcFile("/share/test.dat") == "share\\test.dat");
}

TEST_CASE("Util::toAdcFile and toNmdcFile round-trip", "[Util]") {
    std::string adcPath = "/share/files/test.dat";
    std::string nmdcPath = Util::toNmdcFile(adcPath);
    std::string back = Util::toAdcFile(nmdcPath);
    REQUIRE(back == adcPath);
}

// ── validateFileName ────────────────────────────────────────────────────

TEST_CASE("Util::validateFileName sanitizes bad characters", "[Util]") {
    // On Linux, badChars includes '\\' but NOT '/'
    // Colons are replaced to '_' (except at position 1 for Windows drive letters)
    std::string result = Util::validateFileName("test:file<name>");
    REQUIRE(result.find(':') == std::string::npos);
    REQUIRE(result.find('<') == std::string::npos);
    REQUIRE(result.find('>') == std::string::npos);
    REQUIRE(result == "test_file_name_");
}

TEST_CASE("Util::validateFileName preserves clean filename", "[Util]") {
    REQUIRE(Util::validateFileName("clean_file.txt") == "clean_file.txt");
}

// ── trimCopy ────────────────────────────────────────────────────────────

TEST_CASE("Util::trimCopy trims whitespace", "[Util]") {
    REQUIRE(Util::trimCopy("  hello  ") == "hello");
    REQUIRE(Util::trimCopy("hello") == "hello");
    REQUIRE(Util::trimCopy("   ") == "");
    REQUIRE(Util::trimCopy("") == "");
    REQUIRE(Util::trimCopy("\thello\t") == "hello");
}

// ── formatSeconds ───────────────────────────────────────────────────────

TEST_CASE("Util::formatSeconds formats time durations", "[Util]") {
    // These depend on implementation, just ensure no crash and non-empty
    std::string result = Util::formatSeconds(3661); // 1h 1m 1s
    REQUIRE_FALSE(result.empty());

    std::string zero = Util::formatSeconds(0);
    REQUIRE_FALSE(zero.empty());
}
