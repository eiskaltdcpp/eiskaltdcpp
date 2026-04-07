/*
 * Tests for NmdcHub::escape / unescape / validateMessage
 * Static methods — no DCContext needed.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "NmdcHub.h"

using namespace dcpp;

// ─── Basic escape ───────────────────────────────────────────────────────

TEST_CASE("NmdcHub::escape: dollar sign", "[nmdc]") {
    REQUIRE(NmdcHub::escape("hello$world") == "hello&#36;world");
}

TEST_CASE("NmdcHub::escape: pipe character", "[nmdc]") {
    REQUIRE(NmdcHub::escape("hello|world") == "hello&#124;world");
}

TEST_CASE("NmdcHub::escape: ampersand in plain text", "[nmdc]") {
    // Plain & stays as & (only pre-existing entities get extra-escaped)
    std::string result = NmdcHub::escape("a&b");
    // $ and | should be the only chars escaped; & not preceded by entity patterns stays
    REQUIRE(result.find("&#36;") == std::string::npos);
    REQUIRE(result.find("&#124;") == std::string::npos);
}

TEST_CASE("NmdcHub::escape: empty string", "[nmdc]") {
    REQUIRE(NmdcHub::escape("") == "");
}

TEST_CASE("NmdcHub::escape: no special chars", "[nmdc]") {
    REQUIRE(NmdcHub::escape("plain text") == "plain text");
}

TEST_CASE("NmdcHub::escape: multiple special chars", "[nmdc]") {
    std::string result = NmdcHub::escape("$|$|");
    // Each $ → &#36;, each | → &#124;
    REQUIRE(result == "&#36;&#124;&#36;&#124;");
}

// ─── Basic unescape ────────────────────────────────────────────────────

TEST_CASE("NmdcHub::unescape: dollar entity", "[nmdc]") {
    REQUIRE(NmdcHub::unescape("hello&#36;world") == "hello$world");
}

TEST_CASE("NmdcHub::unescape: pipe entity", "[nmdc]") {
    REQUIRE(NmdcHub::unescape("hello&#124;world") == "hello|world");
}

TEST_CASE("NmdcHub::unescape: amp entity", "[nmdc]") {
    REQUIRE(NmdcHub::unescape("a&amp;b") == "a&b");
}

TEST_CASE("NmdcHub::unescape: empty string", "[nmdc]") {
    REQUIRE(NmdcHub::unescape("") == "");
}

TEST_CASE("NmdcHub::unescape: no entities", "[nmdc]") {
    REQUIRE(NmdcHub::unescape("plain text") == "plain text");
}

// ─── Round-trip ─────────────────────────────────────────────────────────

TEST_CASE("NmdcHub: escape-unescape round-trip", "[nmdc]") {
    std::string original = "money$pipe|amp&end";
    // Note: escape() encodes $ and |; the & in middle doesn't form an
    // existing entity pattern so the exact round-trip depends on order.
    // The key invariant is: unescape(escape(s)) recovers $, | from entities.
    std::string escaped = NmdcHub::escape(original);
    std::string roundtrip = NmdcHub::unescape(escaped);
    REQUIRE(roundtrip == original);
}

TEST_CASE("NmdcHub: unescape-escape round-trip of entities", "[nmdc]") {
    std::string original = "&#36;&#124;&amp;";
    // unescape first
    std::string plain = NmdcHub::unescape(original);
    REQUIRE(plain == "$|&");
    // escape back
    std::string reescaped = NmdcHub::escape(plain);
    std::string back = NmdcHub::unescape(reescaped);
    REQUIRE(back == "$|&");
}

// ─── Edge cases ─────────────────────────────────────────────────────────

TEST_CASE("NmdcHub::escape: only dollar signs", "[nmdc]") {
    REQUIRE(NmdcHub::escape("$$$") == "&#36;&#36;&#36;");
}

TEST_CASE("NmdcHub::escape: only pipes", "[nmdc]") {
    REQUIRE(NmdcHub::escape("|||") == "&#124;&#124;&#124;");
}

TEST_CASE("NmdcHub::escape: pre-existing entities get escaped", "[nmdc]") {
    // If input already contains &#36;, the & gets escaped to &amp;
    // so the literal entity survives the escape process
    std::string result = NmdcHub::escape("&#36;");
    // The & in &#36; doesn't match &amp; pattern, but &#36; is found
    // and its & is replaced with &amp; → &amp;#36;
    REQUIRE(result.find("&amp;") != std::string::npos);
}

TEST_CASE("NmdcHub::unescape: partial entity left alone", "[nmdc]") {
    // &#3 (incomplete entity) should not be modified
    REQUIRE(NmdcHub::unescape("&#3") == "&#3");
}

TEST_CASE("NmdcHub::unescape: nested amp entities", "[nmdc]") {
    // &amp;#36; → &#36; (just one level of decoding: &amp; → &)
    std::string result = NmdcHub::unescape("&amp;#36;");
    REQUIRE(result == "&#36;");
}
