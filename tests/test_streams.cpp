/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp stream classes (Streams.h)
 *
 * Tests MemoryInputStream, CountedInputStream, LimitedInputStream,
 * LimitedOutputStream, BufferedOutputStream, StringOutputStream,
 * and StringRefOutputStream.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Streams.h"

#include <cstring>
#include <string>
#include <vector>

using namespace dcpp;

// ── MemoryInputStream ───────────────────────────────────────────────────

TEST_CASE("MemoryInputStream: read all data in one call", "[Streams][MemoryInputStream]") {
    const std::string data = "Hello, streams!";
    MemoryInputStream mis(data);

    REQUIRE(mis.getSize() == data.size());

    char buf[64] = {};
    size_t len = sizeof(buf);
    size_t ret = mis.read(buf, len);

    REQUIRE(ret == data.size());
    REQUIRE(len == data.size());
    REQUIRE(std::string(buf, len) == data);
}

TEST_CASE("MemoryInputStream: read in chunks", "[Streams][MemoryInputStream]") {
    const std::string data = "ABCDEFGHIJ";
    MemoryInputStream mis(data);

    std::string result;
    char buf[3];

    while (true) {
        size_t len = sizeof(buf);
        size_t ret = mis.read(buf, len);
        if (len == 0) break;
        result.append(buf, len);
    }

    REQUIRE(result == data);
}

TEST_CASE("MemoryInputStream: binary data constructor", "[Streams][MemoryInputStream]") {
    const uint8_t bin[] = {0x00, 0xFF, 0x80, 0x7F, 0x01};
    MemoryInputStream mis(bin, sizeof(bin));

    REQUIRE(mis.getSize() == 5);

    uint8_t out[5] = {};
    size_t len = 5;
    mis.read(out, len);
    REQUIRE(len == 5);
    REQUIRE(std::memcmp(bin, out, 5) == 0);
}

TEST_CASE("MemoryInputStream: empty input", "[Streams][MemoryInputStream]") {
    MemoryInputStream mis(std::string(""));
    REQUIRE(mis.getSize() == 0);

    char buf[4];
    size_t len = sizeof(buf);
    mis.read(buf, len);
    REQUIRE(len == 0);
}

// ── CountedInputStream ──────────────────────────────────────────────────

TEST_CASE("CountedInputStream: counts bytes read", "[Streams][CountedInputStream]") {
    const std::string data = "0123456789";
    auto* inner = new MemoryInputStream(data);
    CountedInputStream<true> counted(inner);

    REQUIRE(counted.getReadBytes() == 0);

    char buf[16];
    size_t len = 4;
    counted.read(buf, len);
    REQUIRE(counted.getReadBytes() == 4);

    len = 10; // ask for more than remaining
    counted.read(buf, len);
    REQUIRE(len == 6); // only 6 remaining
    REQUIRE(counted.getReadBytes() == 10);
}

TEST_CASE("CountedInputStream: non-managed does not delete", "[Streams][CountedInputStream]") {
    const std::string data = "test";
    MemoryInputStream inner(data);
    {
        CountedInputStream<false> counted(&inner);
        char buf[4];
        size_t len = 4;
        counted.read(buf, len);
        REQUIRE(counted.getReadBytes() == 4);
    }
    // inner is still valid — read remaining
    char buf[4];
    size_t len = 4;
    inner.read(buf, len);
    REQUIRE(len == 0); // already consumed
}

// ── LimitedInputStream ──────────────────────────────────────────────────

TEST_CASE("LimitedInputStream: limits bytes read", "[Streams][LimitedInputStream]") {
    const std::string data = "ABCDEFGHIJ"; // 10 bytes
    auto* inner = new MemoryInputStream(data);
    LimitedInputStream<true> limited(inner, 5);

    char buf[10];
    size_t len = 10;
    limited.read(buf, len);
    REQUIRE(len == 5);
    REQUIRE(std::string(buf, len) == "ABCDE");

    // After limit reached, reads return 0
    len = 10;
    size_t ret = limited.read(buf, len);
    REQUIRE(len == 0);
    REQUIRE(ret == 0);
}

TEST_CASE("LimitedInputStream: limit larger than data", "[Streams][LimitedInputStream]") {
    const std::string data = "short";
    auto* inner = new MemoryInputStream(data);
    LimitedInputStream<true> limited(inner, 100);

    char buf[64];
    size_t len = sizeof(buf);
    limited.read(buf, len);
    REQUIRE(len == 5);
    REQUIRE(std::string(buf, len) == "short");
}

// ── LimitedOutputStream ─────────────────────────────────────────────────

TEST_CASE("LimitedOutputStream: writes within limit", "[Streams][LimitedOutputStream]") {
    StringOutputStream sout;
    LimitedOutputStream<false> limited(&sout, 10);

    const std::string data = "hello";
    limited.write(data.c_str(), data.size());

    REQUIRE(sout.getString() == "hello");
    REQUIRE_FALSE(limited.eof());
}

TEST_CASE("LimitedOutputStream: throws on overflow", "[Streams][LimitedOutputStream]") {
    StringOutputStream sout;
    LimitedOutputStream<false> limited(&sout, 5);

    REQUIRE_THROWS_AS(
        limited.write("toolong!", 8),
        FileException
    );
}

TEST_CASE("LimitedOutputStream: eof when exactly at limit", "[Streams][LimitedOutputStream]") {
    StringOutputStream sout;
    LimitedOutputStream<false> limited(&sout, 5);

    limited.write("12345", 5);
    REQUIRE(limited.eof());
}

// ── StringOutputStream ──────────────────────────────────────────────────

TEST_CASE("StringOutputStream: accumulates writes", "[Streams][StringOutputStream]") {
    StringOutputStream sout;

    sout.write("Hello", 5);
    sout.write(", ", 2);
    sout.write("World!", 6);

    REQUIRE(sout.getString() == "Hello, World!");
}

TEST_CASE("StringOutputStream: write via string overload", "[Streams][StringOutputStream]") {
    StringOutputStream sout;
    sout.write(std::string("test data"));
    REQUIRE(sout.stringRef() == "test data");
}

TEST_CASE("StringOutputStream: flush returns 0", "[Streams][StringOutputStream]") {
    StringOutputStream sout;
    REQUIRE(sout.flush() == 0);
}

// ── StringRefOutputStream ───────────────────────────────────────────────

TEST_CASE("StringRefOutputStream: writes to external string", "[Streams][StringRefOutputStream]") {
    std::string result;
    StringRefOutputStream sout(result);

    sout.write("abc", 3);
    sout.write("def", 3);

    REQUIRE(result == "abcdef");
}

TEST_CASE("StringRefOutputStream: flush returns 0", "[Streams][StringRefOutputStream]") {
    std::string result;
    StringRefOutputStream sout(result);
    REQUIRE(sout.flush() == 0);
}

// ── BufferedOutputStream ────────────────────────────────────────────────

TEST_CASE("BufferedOutputStream: small writes buffered until flush", "[Streams][BufferedOutputStream]") {
    StringOutputStream sout;
    {
        BufferedOutputStream<false> buffered(&sout, 64);
        buffered.write("hello", 5);
        // Data may still be in buffer
        buffered.flush();
    }
    REQUIRE(sout.getString() == "hello");
}

TEST_CASE("BufferedOutputStream: large write bypasses buffer", "[Streams][BufferedOutputStream]") {
    StringOutputStream sout;
    {
        BufferedOutputStream<false> buffered(&sout, 4); // tiny buffer
        std::string big(100, 'X');
        buffered.write(big.c_str(), big.size());
    }
    // The large write should have gone through
    REQUIRE(sout.stringRef().size() == 100);
}

TEST_CASE("BufferedOutputStream: multiple small writes coalesced", "[Streams][BufferedOutputStream]") {
    StringOutputStream sout;
    {
        BufferedOutputStream<false> buffered(&sout, 1024);
        for (int i = 0; i < 100; i++) {
            buffered.write("X", 1);
        }
        buffered.flush();
    }
    REQUIRE(sout.getString() == std::string(100, 'X'));
}

TEST_CASE("BufferedOutputStream: fills buffer triggers write-through", "[Streams][BufferedOutputStream]") {
    StringOutputStream sout;
    {
        BufferedOutputStream<false> buffered(&sout, 8);
        // Write exactly buffer size to trigger flush
        buffered.write("12345678", 8);
        // Write a bit more
        buffered.write("AB", 2);
        buffered.flush();
    }
    REQUIRE(sout.getString() == "12345678AB");
}
