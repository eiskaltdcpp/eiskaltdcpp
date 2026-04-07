/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Additional Util.cpp coverage: formatBytes, formatExactSize, decodeUrl,
 * encodeURI, getFileName, getFilePath, and sanitizeUrl edge cases.
 *
 * Complements test_util.cpp (basic functions) and test_util_extended.cpp
 * (paths, rounding, hex escape, ADC/NMDC, validation, trimming, timing).
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Util.h"
#include "Text.h"

#include <string>

using namespace dcpp;

// ── formatBytes ─────────────────────────────────────────────────────────
// Note: formatBytes(int64_t) requires SettingsManager (SETTING macro).
// The two-arg version formatBytes(int64_t, uint8_t) is private.
// These are deferred to Phase 2 when TestContext fixture is available.

// ── formatExactSize (uses locale/gettext — needs Util::initialize) ──────
// Skipped: formatExactSize depends on gettext/locale being initialized.
// Covered by integration tests or when TestContext fixture is available.

// ── decodeUrl ───────────────────────────────────────────────────────────

TEST_CASE("decodeUrl: basic HTTP URL", "[Util][decodeUrl]") {
    string protocol, host, port, path, query, fragment;
    Util::decodeUrl("http://example.com:8080/path?q=1#frag",
                    protocol, host, port, path, query, fragment);
    REQUIRE(protocol == "http");
    REQUIRE(host == "example.com");
    REQUIRE(port == "8080");
    REQUIRE(path == "/path");
    REQUIRE(query == "q=1");
    REQUIRE(fragment == "frag");
}

TEST_CASE("decodeUrl: HTTPS URL without port", "[Util][decodeUrl]") {
    string protocol, host, port, path, query, fragment;
    Util::decodeUrl("https://example.com/page",
                    protocol, host, port, path, query, fragment);
    REQUIRE(protocol == "https");
    REQUIRE(host == "example.com");
    REQUIRE(path == "/page");
}

TEST_CASE("decodeUrl: ADC URL", "[Util][decodeUrl]") {
    string protocol, host, port, path, query, fragment;
    Util::decodeUrl("adc://hub.example.com:411",
                    protocol, host, port, path, query, fragment);
    REQUIRE(protocol == "adc");
    REQUIRE(host == "hub.example.com");
    REQUIRE(port == "411");
}

TEST_CASE("decodeUrl: DCHUB URL", "[Util][decodeUrl]") {
    string protocol, host, port, path, query, fragment;
    Util::decodeUrl("dchub://myhub.org:999",
                    protocol, host, port, path, query, fragment);
    REQUIRE(protocol == "dchub");
    REQUIRE(host == "myhub.org");
    REQUIRE(port == "999");
}

TEST_CASE("decodeUrl: empty URL", "[Util][decodeUrl]") {
    string protocol, host, port, path, query, fragment;
    Util::decodeUrl("", protocol, host, port, path, query, fragment);
    REQUIRE(protocol.empty());
    REQUIRE(host.empty());
}

// ── encodeURI ───────────────────────────────────────────────────────────

TEST_CASE("encodeURI: plain ASCII unchanged", "[Util][encodeURI]") {
    REQUIRE(Util::encodeURI("hello") == "hello");
}

TEST_CASE("encodeURI: space is encoded", "[Util][encodeURI]") {
    string encoded = Util::encodeURI("hello world");
    REQUIRE(encoded.find(' ') == string::npos);
    // encodeURI uses + for space (RFC 1630 / magnet-uri)
    REQUIRE(encoded.find('+') != string::npos);
}

TEST_CASE("encodeURI: round-trip with reverse", "[Util][encodeURI]") {
    string original = "hello world & others";
    string encoded = Util::encodeURI(original);
    string decoded = Util::encodeURI(encoded, true);
    REQUIRE(decoded == original);
}

TEST_CASE("encodeURI: special characters", "[Util][encodeURI]") {
    string encoded = Util::encodeURI("a=b&c=d");
    // = and & should be percent-encoded
    REQUIRE(encoded.find('%') != string::npos);
}

// ── getFileName / getFilePath ───────────────────────────────────────────

TEST_CASE("getFileName: Unix path", "[Util][path]") {
    REQUIRE(Util::getFileName("/home/user/file.txt", '/') == "file.txt");
}

TEST_CASE("getFileName: Windows path", "[Util][path]") {
    REQUIRE(Util::getFileName("C:\\Users\\file.txt", '\\') == "file.txt");
}

TEST_CASE("getFileName: no separator", "[Util][path]") {
    REQUIRE(Util::getFileName("file.txt", '/') == "file.txt");
}

TEST_CASE("getFilePath: Unix path", "[Util][path]") {
    REQUIRE(Util::getFilePath("/home/user/file.txt", '/') == "/home/user/");
}

TEST_CASE("getFilePath: trailing separator", "[Util][path]") {
    REQUIRE(Util::getFilePath("/home/user/", '/') == "/home/user/");
}

TEST_CASE("getFilePath: no separator returns full string", "[Util][path]") {
    // getFilePath returns everything up to and including the last separator;
    // if there's no separator, it returns the entire input.
    string result = Util::getFilePath("file.txt", '/');
    REQUIRE(result == "file.txt");
}
