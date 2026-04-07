/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::StringTokenizer
 *
 * Pure STL header-only tokenizer — zero external dependencies.
 * Regression tests for the Qt6 migration.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "StringTokenizer.h"

using namespace dcpp;

// ── Single-character delimiter ──────────────────────────────────────────

TEST_CASE("StringTokenizer splits by single char", "[StringTokenizer]") {
    StringTokenizer<std::string> st("one,two,three", ',');
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "one");
    REQUIRE(tokens[1] == "two");
    REQUIRE(tokens[2] == "three");
}

TEST_CASE("StringTokenizer leading delimiter produces empty first token", "[StringTokenizer]") {
    StringTokenizer<std::string> st(",hello,world", ',');
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "");
    REQUIRE(tokens[1] == "hello");
    REQUIRE(tokens[2] == "world");
}

TEST_CASE("StringTokenizer trailing delimiter omits trailing empty", "[StringTokenizer]") {
    // The implementation only appends remaining if j < aString.size()
    // So a trailing delimiter means the last part is empty → not appended
    StringTokenizer<std::string> st("hello,world,", ',');
    auto& tokens = st.getTokens();

    // Trailing delimiter: "hello,world," → ["hello", "world"]
    // (the substr after last ',' is empty and j == aString.size(), not appended)
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0] == "hello");
    REQUIRE(tokens[1] == "world");
}

TEST_CASE("StringTokenizer consecutive delimiters produce empty tokens", "[StringTokenizer]") {
    StringTokenizer<std::string> st("a,,b", ',');
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "a");
    REQUIRE(tokens[1] == "");
    REQUIRE(tokens[2] == "b");
}

TEST_CASE("StringTokenizer no delimiter returns one token", "[StringTokenizer]") {
    StringTokenizer<std::string> st("hello", ',');
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "hello");
}

TEST_CASE("StringTokenizer empty string gives no tokens", "[StringTokenizer]") {
    StringTokenizer<std::string> st("", ',');
    auto& tokens = st.getTokens();

    REQUIRE(tokens.empty());
}

// ── Multi-character delimiter ───────────────────────────────────────────

TEST_CASE("StringTokenizer splits by multi-char delimiter", "[StringTokenizer]") {
    StringTokenizer<std::string> st("one::two::three", "::");
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "one");
    REQUIRE(tokens[1] == "two");
    REQUIRE(tokens[2] == "three");
}

TEST_CASE("StringTokenizer multi-char CRLF delimiter", "[StringTokenizer]") {
    StringTokenizer<std::string> st("line1\r\nline2\r\nline3", "\r\n");
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "line1");
    REQUIRE(tokens[1] == "line2");
    REQUIRE(tokens[2] == "line3");
}

TEST_CASE("StringTokenizer multi-char delimiter not found", "[StringTokenizer]") {
    StringTokenizer<std::string> st("hello world", "::");
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "hello world");
}

// ── ADC-style space-delimited parameters ────────────────────────────────

TEST_CASE("StringTokenizer space-delimited ADC params", "[StringTokenizer]") {
    StringTokenizer<std::string> st("NIhello DEworld FLfeature", ' ');
    auto& tokens = st.getTokens();

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0] == "NIhello");
    REQUIRE(tokens[1] == "DEworld");
    REQUIRE(tokens[2] == "FLfeature");
}
