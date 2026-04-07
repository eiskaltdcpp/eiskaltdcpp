/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::Text encoding utilities
 *
 * Tests UTF-8 ↔ wide string conversions, validation, and case handling.
 * These are regression tests for the Qt6 migration.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "Text.h"

using namespace dcpp;

// ── isAscii ─────────────────────────────────────────────────────────────

TEST_CASE("Text::isAscii accepts pure ASCII", "[Text]") {
    REQUIRE(Text::isAscii("Hello, World!"));
    REQUIRE(Text::isAscii(""));
    REQUIRE(Text::isAscii("0123456789"));
    REQUIRE(Text::isAscii("~")); // 0x7E is valid ASCII
}

TEST_CASE("Text::isAscii rejects non-ASCII", "[Text]") {
    REQUIRE_FALSE(Text::isAscii("\xC3\xA9"));         // é (UTF-8)
    REQUIRE_FALSE(Text::isAscii("caf\xC3\xA9"));      // café
    REQUIRE_FALSE(Text::isAscii("\x80"));               // bare high byte
}

// ── validateUtf8 ────────────────────────────────────────────────────────

TEST_CASE("Text::validateUtf8 accepts valid UTF-8", "[Text]") {
    REQUIRE(Text::validateUtf8("Hello"));
    REQUIRE(Text::validateUtf8(""));
    REQUIRE(Text::validateUtf8("café"));                // 2-byte sequences
    REQUIRE(Text::validateUtf8("日本語"));              // 3-byte sequences
    REQUIRE(Text::validateUtf8("𝕳𝖊𝖑𝖑𝖔"));    // 4-byte sequences (Mathematical Fraktur)
}

TEST_CASE("Text::validateUtf8 rejects invalid sequences", "[Text]") {
    REQUIRE_FALSE(Text::validateUtf8("\x80"));          // bare continuation byte
    REQUIRE_FALSE(Text::validateUtf8("\xFE\xFF"));      // invalid lead bytes
    REQUIRE_FALSE(Text::validateUtf8("\xC3"));          // truncated 2-byte sequence
    // Note: overlong sequences like \xC0\xAF may pass validation depending
    // on the iconv-based implementation; only test clearly invalid bytes.
}

// ── UTF-8 ↔ Wide round-trip ─────────────────────────────────────────────

TEST_CASE("Text::utf8ToWide and wideToUtf8 round-trip ASCII", "[Text]") {
    const std::string original = "Hello, World!";
    std::wstring wide = Text::utf8ToWide(original);
    std::string back = Text::wideToUtf8(wide);
    REQUIRE(back == original);
}

TEST_CASE("Text::utf8ToWide and wideToUtf8 round-trip multibyte", "[Text]") {
    const std::string original = "café résumé naïve";
    std::wstring wide = Text::utf8ToWide(original);
    REQUIRE_FALSE(wide.empty());
    std::string back = Text::wideToUtf8(wide);
    REQUIRE(back == original);
}

TEST_CASE("Text::utf8ToWide and wideToUtf8 round-trip CJK", "[Text]") {
    const std::string original = "日本語テスト";
    std::wstring wide = Text::utf8ToWide(original);
    std::string back = Text::wideToUtf8(wide);
    REQUIRE(back == original);
}

TEST_CASE("Text::utf8ToWide and wideToUtf8 round-trip empty", "[Text]") {
    const std::string original;
    std::wstring wide = Text::utf8ToWide(original);
    std::string back = Text::wideToUtf8(wide);
    REQUIRE(back.empty());
}

// ── utf8ToWc / wcToUtf8 ────────────────────────────────────────────────

TEST_CASE("Text::utf8ToWc decodes ASCII character", "[Text]") {
    wchar_t c = 0;
    int bytes = Text::utf8ToWc("A", c);
    REQUIRE(bytes == 1);
    REQUIRE(c == L'A');
}

TEST_CASE("Text::utf8ToWc decodes 2-byte character", "[Text]") {
    wchar_t c = 0;
    int bytes = Text::utf8ToWc("\xC3\xA9", c); // é = U+00E9
    REQUIRE(bytes == 2);
    REQUIRE(c == L'\u00E9');
}

TEST_CASE("Text::utf8ToWc decodes 3-byte character", "[Text]") {
    wchar_t c = 0;
    int bytes = Text::utf8ToWc("\xE6\x97\xA5", c); // 日 = U+65E5
    REQUIRE(bytes == 3);
    REQUIRE(c == L'\u65E5');
}

TEST_CASE("Text::wcToUtf8 encodes ASCII", "[Text]") {
    std::string out;
    Text::wcToUtf8(L'A', out);
    REQUIRE(out == "A");
}

TEST_CASE("Text::wcToUtf8 encodes 2-byte character", "[Text]") {
    std::string out;
    Text::wcToUtf8(L'\u00E9', out); // é
    REQUIRE(out == "\xC3\xA9");
}

// ── toLower ─────────────────────────────────────────────────────────────

TEST_CASE("Text::toLower converts ASCII to lowercase", "[Text]") {
    std::string result = Text::toLower("HELLO WORLD");
    REQUIRE(result == "hello world");
}

TEST_CASE("Text::toLower preserves already-lowercase", "[Text]") {
    std::string result = Text::toLower("hello");
    REQUIRE(result == "hello");
}

TEST_CASE("Text::toLower handles mixed case with numbers", "[Text]") {
    std::string result = Text::toLower("Test123ABC");
    REQUIRE(result == "test123abc");
}

TEST_CASE("Text::toLower handles empty string", "[Text]") {
    std::string result = Text::toLower("");
    REQUIRE(result.empty());
}

// ── toDOS ───────────────────────────────────────────────────────────────

TEST_CASE("Text::toDOS converts LF to CRLF", "[Text]") {
    std::string result = Text::toDOS("line1\nline2\nline3");
    REQUIRE(result == "line1\r\nline2\r\nline3");
}

TEST_CASE("Text::toDOS preserves existing CRLF", "[Text]") {
    std::string result = Text::toDOS("line1\r\nline2\r\n");
    REQUIRE(result == "line1\r\nline2\r\n");
}

TEST_CASE("Text::toDOS handles empty string", "[Text]") {
    std::string result = Text::toDOS(std::string(""));
    REQUIRE(result.empty());
}

TEST_CASE("Text::toDOS handles string with no newlines", "[Text]") {
    std::string result = Text::toDOS("hello world");
    REQUIRE(result == "hello world");
}

// ── toUtf8 / fromUtf8 / convert (iconv-based charset conversion) ────────

TEST_CASE("Text::toUtf8 converts Latin-1 to UTF-8", "[Text]") {
    // Latin-1 byte 0xE9 is 'e with acute' = U+00E9 = UTF-8 C3 A9
    std::string latin1 = {'\xe9'};
    std::string result = Text::toUtf8(latin1, "ISO-8859-1");
    REQUIRE(result == "\xc3\xa9");
}

TEST_CASE("Text::toUtf8 converts CP1251 Cyrillic to UTF-8", "[Text]") {
    // CP1251: 0xCF 0xF0 0xE8 0xE2 0xE5 0xF2 = "Привет" (Privet)
    std::string cp1251 = {'\xcf', '\xf0', '\xe8', '\xe2', '\xe5', '\xf2'};
    std::string result = Text::toUtf8(cp1251, "CP1251");
    // Expected UTF-8: П=D0 9F, р=D1 80, и=D0 B8, в=D0 B2, е=D0 B5, т=D1 82
    REQUIRE(result == "\xd0\x9f\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82");
}

TEST_CASE("Text::fromUtf8 converts UTF-8 to Latin-1", "[Text]") {
    std::string utf8str = "\xc3\xa9"; // é in UTF-8
    std::string result = Text::fromUtf8(utf8str, "ISO-8859-1");
    REQUIRE(result == std::string(1, '\xe9'));
}

TEST_CASE("Text::toUtf8 returns UTF-8 input unchanged", "[Text]") {
    std::string utf8str = "\xd0\x9f\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82";
    std::string result = Text::toUtf8(utf8str, "UTF-8");
    REQUIRE(result == utf8str);
}

TEST_CASE("Text::convert handles ASCII passthrough", "[Text]") {
    std::string ascii = "Hello";
    std::string tmp;
    const std::string& result = Text::convert(ascii, tmp, "ASCII", "UTF-8");
    REQUIRE(result == "Hello");
}

TEST_CASE("Text::convert replaces unconvertible chars with underscore", "[Text]") {
    // Cyrillic П = UTF-8 0xD0 0x9F cannot be represented in ASCII;
    // each source byte that fails produces one '_' replacement.
    std::string utf8_cyrillic = "\xd0\x9f"; // П in UTF-8
    std::string tmp;
    const std::string& result = Text::convert(utf8_cyrillic, tmp, "UTF-8", "ASCII");
    REQUIRE(result == "__");
}

