/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for FilteredFile.h stream wrappers:
 * CountOutputStream, CalcOutputStream, CalcInputStream,
 * FilteredOutputStream, FilteredInputStream
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "FilteredFile.h"
#include "Streams.h"

#include <cstring>
#include <string>

using namespace dcpp;

// ── CountOutputStream ───────────────────────────────────────────────────

TEST_CASE("CountOutputStream: counts bytes written", "[FilteredFile][CountOutputStream]") {
    StringOutputStream sout;
    CountOutputStream<false> counted(&sout);

    REQUIRE(counted.getCount() == 0);

    counted.write("hello", 5);
    REQUIRE(counted.getCount() == 5);

    counted.write(" world", 6);
    REQUIRE(counted.getCount() == 11);
}

TEST_CASE("CountOutputStream: data passes through", "[FilteredFile][CountOutputStream]") {
    StringOutputStream sout;
    CountOutputStream<false> counted(&sout);

    counted.write("test", 4);
    counted.flush();

    REQUIRE(sout.getString() == "test");
}

TEST_CASE("CountOutputStream: flush adds to count", "[FilteredFile][CountOutputStream]") {
    StringOutputStream sout;
    CountOutputStream<false> counted(&sout);

    counted.write("abc", 3);
    size_t flushed = counted.flush();
    // flush returns what inner flush returns (0 for StringOutputStream)
    REQUIRE(counted.getCount() == 3 + flushed);
}

TEST_CASE("CountOutputStream: managed deletes inner stream", "[FilteredFile][CountOutputStream]") {
    auto* sout = new StringOutputStream();
    {
        CountOutputStream<true> counted(sout);
        counted.write("data", 4);
    }
    // sout is deleted by CountOutputStream<true> destructor
    // (if we get here without crash, managed deletion worked)
    SUCCEED("managed stream deleted successfully");
}

// ── CalcOutputStream with a simple filter ───────────────────────────────

namespace {

// A simple filter that XORs each byte with 0x42
struct XorFilter {
    void operator()(const void* buf, size_t len) {
        // CalcOutputStream calls filter(buf, len) before writing through
        // We just track the total processed bytes
        totalBytes += len;
    }
    size_t totalBytes = 0;
};

// A simple checksumming filter for CalcInputStream
struct SumFilter {
    void operator()(const void* buf, size_t len) {
        auto* p = static_cast<const uint8_t*>(buf);
        for (size_t i = 0; i < len; i++) {
            sum += p[i];
        }
    }
    uint64_t sum = 0;
};

} // anonymous namespace

TEST_CASE("CalcOutputStream: filter sees all written data", "[FilteredFile][CalcOutputStream]") {
    StringOutputStream sout;
    CalcOutputStream<XorFilter, false> calc(&sout);

    calc.write("hello", 5);
    calc.write(" world!", 7);

    REQUIRE(calc.getFilter().totalBytes == 12);
    // Data still passes through unmodified
    REQUIRE(sout.getString() == "hello world!");
}

TEST_CASE("CalcInputStream: filter sees all read data", "[FilteredFile][CalcInputStream]") {
    auto* mis = new MemoryInputStream(std::string("ABCD"));
    CalcInputStream<SumFilter, true> calc(mis);

    char buf[10];
    size_t len = sizeof(buf);
    calc.read(buf, len);

    // 'A'=65 + 'B'=66 + 'C'=67 + 'D'=68 = 266
    REQUIRE(calc.getFilter().sum == 266);
}

// ── FilteredOutputStream with identity filter ───────────────────────────

namespace {

// Identity filter — copies input to output unchanged
struct IdentityFilter {
    // Returns true if more data may follow, false if done
    bool operator()(const void* in, size_t& inLen, void* out, size_t& outLen) {
        if (!in) {
            // Flush call: no more data
            outLen = 0;
            return false;
        }
        size_t n = std::min(inLen, outLen);
        if (n > 0) {
            std::memcpy(out, in, n);
        }
        inLen = n;
        outLen = n;
        // Return false when we consumed zero bytes (input exhausted)
        return (n > 0);
    }
};

} // anonymous namespace

TEST_CASE("FilteredOutputStream: identity filter passes data through", "[FilteredFile][FilteredOutputStream]") {
    StringOutputStream sout;
    FilteredOutputStream<IdentityFilter, false> filtered(&sout);

    filtered.write("hello", 5);
    filtered.flush();

    REQUIRE(sout.getString() == "hello");
}

TEST_CASE("FilteredOutputStream: multiple writes then flush", "[FilteredFile][FilteredOutputStream]") {
    StringOutputStream sout;
    FilteredOutputStream<IdentityFilter, false> filtered(&sout);

    filtered.write("abc", 3);
    filtered.write("def", 3);
    filtered.flush();

    REQUIRE(sout.getString() == "abcdef");
}

TEST_CASE("FilteredOutputStream: writing after flush throws", "[FilteredFile][FilteredOutputStream]") {
    StringOutputStream sout;
    FilteredOutputStream<IdentityFilter, false> filtered(&sout);

    filtered.write("data", 4);
    filtered.flush();

    REQUIRE_THROWS_AS(
        filtered.write("more", 4),
        Exception
    );
}

// ── FilteredInputStream with identity filter ────────────────────────────

TEST_CASE("FilteredInputStream: identity filter reads data through", "[FilteredFile][FilteredInputStream]") {
    auto* mis = new MemoryInputStream(std::string("test data"));
    FilteredInputStream<IdentityFilter, true> filtered(mis);

    std::string result;
    char buf[64] = {};
    while (true) {
        size_t len = sizeof(buf);
        size_t produced = filtered.read(buf, len);
        if (produced == 0) break;
        result.append(buf, produced);
    }

    REQUIRE(result == "test data");
}
