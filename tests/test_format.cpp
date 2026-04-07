/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::formated_string (format.h)
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "format.h"

#include <string>

using namespace dcpp;

// ── Basic substitution ──────────────────────────────────────────────────

TEST_CASE("formated_string: single substitution", "[format]") {
    std::string result = dcpp_fmt("Hello %1%") % "World";
    REQUIRE(result == "Hello World");
}

TEST_CASE("formated_string: multiple substitutions", "[format]") {
    std::string result = dcpp_fmt("%1% + %2% = %3%") % "1" % "2" % "3";
    REQUIRE(result == "1 + 2 = 3");
}

TEST_CASE("formated_string: numeric substitution", "[format]") {
    std::string result = dcpp_fmt("Count: %1%") % 42;
    REQUIRE(result == "Count: 42");
}

TEST_CASE("formated_string: repeated placeholder", "[format]") {
    std::string result = dcpp_fmt("%1% and %1%") % "same";
    REQUIRE(result == "same and same");
}

TEST_CASE("formated_string: no placeholders", "[format]") {
    std::string result = dcpp_fmt("no placeholders here");
    REQUIRE(result == "no placeholders here");
}

TEST_CASE("formated_string: empty format string", "[format]") {
    std::string result = dcpp_fmt("");
    REQUIRE(result.empty());
}

// ── Type variations ─────────────────────────────────────────────────────

TEST_CASE("formated_string: const char* substitution", "[format]") {
    const char* val = "cstring";
    std::string result = dcpp_fmt("type: %1%") % val;
    REQUIRE(result == "type: cstring");
}

TEST_CASE("formated_string: int substitution", "[format]") {
    std::string result = dcpp_fmt("port: %1%") % 8080;
    REQUIRE(result == "port: 8080");
}

TEST_CASE("formated_string: negative number", "[format]") {
    std::string result = dcpp_fmt("offset: %1%") % (-5);
    REQUIRE(result == "offset: -5");
}

TEST_CASE("formated_string: large number", "[format]") {
    std::string result = dcpp_fmt("size: %1%") % 1099511627776LL;
    REQUIRE(result == "size: 1099511627776");
}

// ── str() helper ────────────────────────────────────────────────────────

TEST_CASE("formated_string: str() converts to string", "[format]") {
    formated_string fs = dcpp_fmt("test %1%") % "value";
    std::string s = str(fs);
    REQUIRE(s == "test value");
}

// ── Multiple arguments in sequence ──────────────────────────────────────

TEST_CASE("formated_string: three string args", "[format]") {
    std::string result = dcpp_fmt("[%1%] %2%: %3%")
        % "INFO" % "Server" % "Connected";
    REQUIRE(result == "[INFO] Server: Connected");
}

TEST_CASE("formated_string: mixed types", "[format]") {
    std::string result = dcpp_fmt("User %1% has %2% slots (%3%)")
        % "Alice" % 5 % "active";
    REQUIRE(result == "User Alice has 5 slots (active)");
}

// ── dcpp_fmt from const char* ───────────────────────────────────────────

TEST_CASE("formated_string: dcpp_fmt from const char*", "[format]") {
    const char* fmt = "Hello %1%";
    std::string result = dcpp_fmt(fmt) % "test";
    REQUIRE(result == "Hello test");
}

// ── Placeholder not found ───────────────────────────────────────────────

TEST_CASE("formated_string: extra args ignored", "[format]") {
    // If the format has only %1% but we provide 2 args, %2% is not found
    std::string result = dcpp_fmt("only %1%") % "first" % "second";
    REQUIRE(result == "only first");
}

TEST_CASE("formated_string: missing args leave placeholder", "[format]") {
    std::string result = dcpp_fmt("has %1% and %2%") % "first";
    REQUIRE(result == "has first and %2%");
}
