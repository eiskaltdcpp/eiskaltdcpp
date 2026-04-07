/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <catch2/catch_test_macros.hpp>
#include "GtkFormatters.h"
#include "TestContext.h"

using namespace dcpp::test;

TEST_CASE("GtkFormatters: formatSpeed", "[gtk][formatters]")
{
    TestContext tc;

    SECTION("zero speed") {
        auto s = gtk_fmt::formatSpeed(0);
        REQUIRE(s.find("/s") != std::string::npos);
        REQUIRE(s.find("0") != std::string::npos);
    }

    SECTION("negative speed returns empty") {
        REQUIRE(gtk_fmt::formatSpeed(-1).empty());
        REQUIRE(gtk_fmt::formatSpeed(-100).empty());
    }

    SECTION("small speed") {
        auto s = gtk_fmt::formatSpeed(500);
        REQUIRE(!s.empty());
        REQUIRE(s.find("/s") != std::string::npos);
    }

    SECTION("megabyte speed") {
        auto s = gtk_fmt::formatSpeed(1048576); // 1 MiB
        REQUIRE(s.find("/s") != std::string::npos);
        // Should contain "1" and "MiB" or similar
        REQUIRE(!s.empty());
    }

    SECTION("gigabyte speed") {
        auto s = gtk_fmt::formatSpeed(1073741824LL); // 1 GiB
        REQUIRE(s.find("/s") != std::string::npos);
        REQUIRE(!s.empty());
    }

    SECTION("very large speed") {
        auto s = gtk_fmt::formatSpeed(INT64_MAX);
        REQUIRE(s.find("/s") != std::string::npos);
    }
}

TEST_CASE("GtkFormatters: formatSize", "[gtk][formatters]")
{
    TestContext tc;

    SECTION("zero size") {
        auto s = gtk_fmt::formatSize(0);
        REQUIRE(!s.empty());
    }

    SECTION("various sizes") {
        REQUIRE(!gtk_fmt::formatSize(1024).empty());
        REQUIRE(!gtk_fmt::formatSize(1048576).empty());
        REQUIRE(!gtk_fmt::formatSize(1073741824LL).empty());
    }
}

TEST_CASE("GtkFormatters: formatTimeLeft", "[gtk][formatters]")
{
    TestContext tc;

    SECTION("zero seconds") {
        auto s = gtk_fmt::formatTimeLeft(0);
        REQUIRE(!s.empty());
    }

    SECTION("small duration") {
        auto s = gtk_fmt::formatTimeLeft(45);
        REQUIRE(!s.empty());
    }

    SECTION("hours and minutes") {
        auto s = gtk_fmt::formatTimeLeft(3600 + 900); // 1h 15m
        REQUIRE(!s.empty());
    }

    SECTION("large duration") {
        auto s = gtk_fmt::formatTimeLeft(86400 * 3); // 3 days
        REQUIRE(!s.empty());
    }
}

TEST_CASE("GtkFormatters: formatProgress", "[gtk][formatters]")
{
    SECTION("zero size") {
        REQUIRE(gtk_fmt::formatProgress(0, 0) == "0.0%");
        REQUIRE(gtk_fmt::formatProgress(100, 0) == "0.0%");
    }

    SECTION("zero progress") {
        REQUIRE(gtk_fmt::formatProgress(0, 1000) == "0.0%");
    }

    SECTION("50% progress") {
        REQUIRE(gtk_fmt::formatProgress(500, 1000) == "50.0%");
    }

    SECTION("100% progress") {
        REQUIRE(gtk_fmt::formatProgress(1000, 1000) == "100.0%");
    }

    SECTION("partial progress") {
        auto s = gtk_fmt::formatProgress(333, 1000);
        REQUIRE(s == "33.3%");
    }

    SECTION("exceeds 100% clamped") {
        REQUIRE(gtk_fmt::formatProgress(1500, 1000) == "100.0%");
    }

    SECTION("negative pos with positive size") {
        // pos < 0 should clamp to 0%
        REQUIRE(gtk_fmt::formatProgress(-50, 1000) == "0.0%");
    }
}

TEST_CASE("GtkFormatters: formatExactSize", "[gtk][formatters]")
{
    TestContext tc;

    SECTION("zero") {
        auto s = gtk_fmt::formatExactSize(0);
        REQUIRE(!s.empty());
    }

    SECTION("various sizes") {
        REQUIRE(!gtk_fmt::formatExactSize(1024).empty());
        REQUIRE(!gtk_fmt::formatExactSize(1048576).empty());
    }
}
