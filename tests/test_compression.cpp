/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for BZFilter/UnBZFilter (bzip2) and ZFilter/UnZFilter (zlib)
 *
 * Tests compress/decompress round-trips, empty data, and large data.
 * These filters are used by the DC++ protocol for file list compression.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "BZUtils.h"
#include "ZUtils.h"

#include <cstring>
#include <string>
#include <vector>

using namespace dcpp;

// ── Helper: compress+decompress round-trip using BZ filters ─────────────

static std::string bzRoundTrip(const std::string& input) {
    // Compress
    std::vector<uint8_t> compressed;
    {
        BZFilter compressor;
        const size_t CHUNK = 4096;
        std::vector<uint8_t> outBuf(CHUNK);

        size_t inPos = 0;
        bool more = true;
        while (more) {
            size_t inSize = std::min(CHUNK, input.size() - inPos);
            if (inPos >= input.size()) inSize = 0; // signal EOF

            size_t outSize = outBuf.size();
            more = compressor(
                inSize > 0 ? input.data() + inPos : nullptr,
                inSize, outBuf.data(), outSize);
            inPos += inSize;
            compressed.insert(compressed.end(), outBuf.data(), outBuf.data() + outSize);
        }
    }

    // Decompress
    std::string output;
    {
        UnBZFilter decompressor;
        const size_t CHUNK = 4096;
        std::vector<uint8_t> outBuf(CHUNK);

        size_t inPos = 0;
        bool more = true;
        while (more) {
            size_t inSize = std::min(CHUNK, compressed.size() - inPos);
            if (inPos >= compressed.size()) inSize = 0;

            size_t outSize = outBuf.size();
            more = decompressor(
                inSize > 0 ? compressed.data() + inPos : nullptr,
                inSize, outBuf.data(), outSize);
            inPos += inSize;
            output.append(reinterpret_cast<char*>(outBuf.data()), outSize);
        }
    }

    return output;
}

// ── Helper: compress+decompress round-trip using Z (zlib) filters ───────

static std::string zRoundTrip(const std::string& input) {
    // Compress
    std::vector<uint8_t> compressed;
    {
        ZFilter compressor;
        const size_t CHUNK = 4096;
        std::vector<uint8_t> outBuf(CHUNK);

        size_t inPos = 0;
        bool more = true;
        while (more) {
            size_t inSize = std::min(CHUNK, input.size() - inPos);
            if (inPos >= input.size()) inSize = 0;

            size_t outSize = outBuf.size();
            more = compressor(
                inSize > 0 ? input.data() + inPos : nullptr,
                inSize, outBuf.data(), outSize);
            inPos += inSize;
            compressed.insert(compressed.end(), outBuf.data(), outBuf.data() + outSize);
        }
    }

    // Decompress
    std::string output;
    {
        UnZFilter decompressor;
        const size_t CHUNK = 4096;
        std::vector<uint8_t> outBuf(CHUNK);

        size_t inPos = 0;
        bool more = true;
        while (more) {
            size_t inSize = std::min(CHUNK, compressed.size() - inPos);
            if (inPos >= compressed.size()) inSize = 0;

            size_t outSize = outBuf.size();
            more = decompressor(
                inSize > 0 ? compressed.data() + inPos : nullptr,
                inSize, outBuf.data(), outSize);
            inPos += inSize;
            output.append(reinterpret_cast<char*>(outBuf.data()), outSize);
        }
    }

    return output;
}

// ── BZip2 tests ─────────────────────────────────────────────────────────

TEST_CASE("BZ round-trip: short string", "[Compression][BZ]") {
    REQUIRE(bzRoundTrip("Hello, World!") == "Hello, World!");
}

TEST_CASE("BZ round-trip: repeated data (high compressibility)", "[Compression][BZ]") {
    std::string input(10000, 'A');
    REQUIRE(bzRoundTrip(input) == input);
}

TEST_CASE("BZ round-trip: binary-like data", "[Compression][BZ]") {
    std::string input;
    for (int i = 0; i < 1000; ++i) {
        input += static_cast<char>(i % 256);
    }
    REQUIRE(bzRoundTrip(input) == input);
}

TEST_CASE("BZ compressed size < original for repetitive data", "[Compression][BZ]") {
    std::string input(10000, 'Z');

    BZFilter compressor;
    const size_t CHUNK = 16384;
    std::vector<uint8_t> outBuf(CHUNK);
    size_t totalCompressed = 0;

    size_t inPos = 0;
    bool more = true;
    while (more) {
        size_t inSize = std::min(CHUNK, input.size() - inPos);
        if (inPos >= input.size()) inSize = 0;

        size_t outSize = outBuf.size();
        more = compressor(
            inSize > 0 ? input.data() + inPos : nullptr,
            inSize, outBuf.data(), outSize);
        inPos += inSize;
        totalCompressed += outSize;
    }

    REQUIRE(totalCompressed < input.size());
}

// ── Zlib tests ──────────────────────────────────────────────────────────

TEST_CASE("Z round-trip: short string", "[Compression][Z]") {
    REQUIRE(zRoundTrip("Hello, World!") == "Hello, World!");
}

TEST_CASE("Z round-trip: repeated data", "[Compression][Z]") {
    std::string input(10000, 'B');
    REQUIRE(zRoundTrip(input) == input);
}

TEST_CASE("Z round-trip: binary-like data", "[Compression][Z]") {
    std::string input;
    for (int i = 0; i < 1000; ++i) {
        input += static_cast<char>(i % 256);
    }
    REQUIRE(zRoundTrip(input) == input);
}

TEST_CASE("Z compressed size < original for repetitive data", "[Compression][Z]") {
    std::string input(10000, 'Y');

    ZFilter compressor;
    const size_t CHUNK = 16384;
    std::vector<uint8_t> outBuf(CHUNK);
    size_t totalCompressed = 0;

    size_t inPos = 0;
    bool more = true;
    while (more) {
        size_t inSize = std::min(CHUNK, input.size() - inPos);
        if (inPos >= input.size()) inSize = 0;

        size_t outSize = outBuf.size();
        more = compressor(
            inSize > 0 ? input.data() + inPos : nullptr,
            inSize, outBuf.data(), outSize);
        inPos += inSize;
        totalCompressed += outSize;
    }

    REQUIRE(totalCompressed < input.size());
}

// ── Large data ──────────────────────────────────────────────────────────

TEST_CASE("BZ round-trip: 100KB mixed data", "[Compression][BZ]") {
    std::string input;
    input.reserve(100000);
    for (int i = 0; i < 100000; ++i) {
        input += static_cast<char>((i * 7 + 13) % 256);
    }
    REQUIRE(bzRoundTrip(input) == input);
}

TEST_CASE("Z round-trip: 100KB mixed data", "[Compression][Z]") {
    std::string input;
    input.reserve(100000);
    for (int i = 0; i < 100000; ++i) {
        input += static_cast<char>((i * 7 + 13) % 256);
    }
    REQUIRE(zRoundTrip(input) == input);
}
