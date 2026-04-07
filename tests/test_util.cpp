/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::Util
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "stdinc.h"
#include "Util.h"

using namespace dcpp;

TEST_CASE("Util::getFileName extracts filename from path", "[Util]") {
    REQUIRE(Util::getFileName("/home/user/file.txt", '/') == "file.txt");
    REQUIRE(Util::getFileName("/home/user/", '/') == "");
    REQUIRE(Util::getFileName("file.txt", '/') == "file.txt");
    REQUIRE(Util::getFileName("", '/') == "");
}

TEST_CASE("Util::getFilePath extracts directory from path", "[Util]") {
    REQUIRE(Util::getFilePath("/home/user/file.txt", '/') == "/home/user/");
    REQUIRE(Util::getFilePath("/home/user/", '/') == "/home/user/");
    REQUIRE(Util::getFilePath("file.txt", '/') == "file.txt");
}

TEST_CASE("Util::getFileExt extracts extension", "[Util]") {
    REQUIRE(Util::getFileExt("file.txt") == ".txt");
    REQUIRE(Util::getFileExt("file.tar.gz") == ".gz");
    REQUIRE(Util::getFileExt("file") == "");
    REQUIRE(Util::getFileExt("") == "");
}

TEST_CASE("Util::addBrackets wraps string in brackets", "[Util]") {
    REQUIRE(Util::addBrackets("test") == "<test>");
    REQUIRE(Util::addBrackets("") == "<>");
}

TEST_CASE("Util::toInt and toInt64 parse numbers", "[Util]") {
    REQUIRE(Util::toInt("42") == 42);
    REQUIRE(Util::toInt("0") == 0);
    REQUIRE(Util::toInt("-1") == -1);
    REQUIRE(Util::toInt("") == 0);

    REQUIRE(Util::toInt64("1234567890123") == 1234567890123LL);
    REQUIRE(Util::toInt64("") == 0);
}

TEST_CASE("Util::toDouble parses floating point", "[Util]") {
    REQUIRE(Util::toDouble("3.14") == Catch::Approx(3.14));
    REQUIRE(Util::toDouble("0") == Catch::Approx(0.0));
    REQUIRE(Util::toDouble("") == Catch::Approx(0.0));
}

TEST_CASE("Util::toString converts numbers to string", "[Util]") {
    REQUIRE(Util::toString(42) == "42");
    REQUIRE(Util::toString(0) == "0");
    REQUIRE(Util::toString(-1) == "-1");
}
