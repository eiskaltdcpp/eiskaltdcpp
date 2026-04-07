/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp version strings (version.h / version.cpp)
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "version.h"

#include <string>

using namespace dcpp;

TEST_CASE("version: fullNMDCVersionString is non-empty", "[version]") {
    REQUIRE_FALSE(fullNMDCVersionString.empty());
}

TEST_CASE("version: fullADCVersionString is non-empty", "[version]") {
    REQUIRE_FALSE(fullADCVersionString.empty());
}

TEST_CASE("version: fullHTTPVersionString is non-empty", "[version]") {
    REQUIRE_FALSE(fullHTTPVersionString.empty());
}

TEST_CASE("version: NMDC version contains 'V:'", "[version]") {
    // Format: "APPNAME V:VERSION"
    REQUIRE(fullNMDCVersionString.find("V:") != std::string::npos);
}

TEST_CASE("version: ADC version contains space separator", "[version]") {
    // Format: "APPNAME VERSION"
    REQUIRE(fullADCVersionString.find(' ') != std::string::npos);
}

TEST_CASE("version: HTTP version contains 'v'", "[version]") {
    // Format: "APPNAME vVERSION"
    REQUIRE(fullHTTPVersionString.find('v') != std::string::npos);
}

TEST_CASE("version: all strings contain app name", "[version]") {
    // All version strings should contain the app name (APPNAME macro)
    // The app name is "EiskaltDC++" based on the CMake project name
    // We just check they share the same prefix
    REQUIRE(fullNMDCVersionString.substr(0, 5) == fullADCVersionString.substr(0, 5));
    REQUIRE(fullNMDCVersionString.substr(0, 5) == fullHTTPVersionString.substr(0, 5));
}

TEST_CASE("version: strings are distinct", "[version]") {
    // The three formats are different
    REQUIRE(fullNMDCVersionString != fullADCVersionString);
    REQUIRE(fullNMDCVersionString != fullHTTPVersionString);
    REQUIRE(fullADCVersionString != fullHTTPVersionString);
}
