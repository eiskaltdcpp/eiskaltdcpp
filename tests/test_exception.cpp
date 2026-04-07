/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::Exception and derived exception macros
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Exception.h"
#include "Streams.h"    // for FileException (STANDARD_EXCEPTION)

#include <string>
#include <stdexcept>

using namespace dcpp;

// ── Base Exception ──────────────────────────────────────────────────────

TEST_CASE("Exception: default constructor", "[Exception]") {
    Exception e;
    REQUIRE(e.getError().empty());
    // what() returns pointer to empty string
    REQUIRE(std::string(e.what()).empty());
}

TEST_CASE("Exception: construct with message", "[Exception]") {
    Exception e("something went wrong");
    REQUIRE(e.getError() == "something went wrong");
    REQUIRE(std::string(e.what()) == "something went wrong");
}

TEST_CASE("Exception: is a std::exception", "[Exception]") {
    try {
        throw Exception("test error");
    } catch (const std::exception& e) {
        REQUIRE(std::string(e.what()).find("test error") != std::string::npos);
        return;
    }
    FAIL("Exception should be catchable as std::exception");
}

TEST_CASE("Exception: copy preserves message", "[Exception]") {
    Exception original("original message");
    Exception copy = original;

    REQUIRE(copy.getError() == "original message");
}

// ── STANDARD_EXCEPTION derived types ────────────────────────────────────

TEST_CASE("FileException: default constructor", "[Exception][FileException]") {
    FileException fe;
    // In debug mode, default msg is "FileException"; in release, empty
#ifdef _DEBUG
    REQUIRE(fe.getError() == "FileException");
#else
    REQUIRE(fe.getError().empty());
#endif
}

TEST_CASE("FileException: construct with message", "[Exception][FileException]") {
    FileException fe("disk full");
    const std::string& err = fe.getError();
#ifdef _DEBUG
    REQUIRE(err.find("disk full") != std::string::npos);
#else
    REQUIRE(err == "disk full");
#endif
}

TEST_CASE("FileException: catchable as Exception", "[Exception][FileException]") {
    try {
        throw FileException("test");
    } catch (const Exception& e) {
        REQUIRE(std::string(e.what()).find("test") != std::string::npos);
        return;
    }
    FAIL("FileException should be catchable as Exception");
}

TEST_CASE("FileException: catchable as std::exception", "[Exception][FileException]") {
    try {
        throw FileException("IO error");
    } catch (const std::exception& e) {
        REQUIRE(std::string(e.what()).find("IO error") != std::string::npos);
        return;
    }
    FAIL("FileException should be catchable as std::exception");
}

// ── Custom STANDARD_EXCEPTION for testing ───────────────────────────────
namespace {
    STANDARD_EXCEPTION(TestCustomException);
}

TEST_CASE("Custom STANDARD_EXCEPTION: basic usage", "[Exception]") {
    TestCustomException tce("custom problem");
    const std::string& err = tce.getError();
    REQUIRE(err.find("custom problem") != std::string::npos);
}

TEST_CASE("Custom STANDARD_EXCEPTION: throw and catch", "[Exception]") {
    REQUIRE_THROWS_AS(
        throw TestCustomException("oops"),
        Exception
    );
}

// ── Custom EXTEND_EXCEPTION for testing ─────────────────────────────────
namespace {
    EXTEND_EXCEPTION(ExtendedTestException, FileException);
}

TEST_CASE("EXTEND_EXCEPTION: inherits from parent", "[Exception]") {
    try {
        throw ExtendedTestException("extended error");
    } catch (const FileException& fe) {
        REQUIRE(std::string(fe.what()).find("extended error") != std::string::npos);
        return;
    }
    FAIL("ExtendedTestException should be catchable as FileException");
}

TEST_CASE("EXTEND_EXCEPTION: catchable as base Exception", "[Exception]") {
    REQUIRE_THROWS_AS(
        throw ExtendedTestException("test"),
        Exception
    );
}
