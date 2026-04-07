/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::AdcCommand — ADC protocol parser/serializer
 *
 * Tests the parse/escape/param logic used in ADC hub communication.
 * Serves as regression tests during the Qt6 migration.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "AdcCommand.h"

using namespace dcpp;

// ── Command construction ────────────────────────────────────────────────

TEST_CASE("AdcCommand basic construction with CMD constant", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_MSG);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_BROADCAST);
}

TEST_CASE("AdcCommand construction with target", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_MSG, 0x12345678, AdcCommand::TYPE_DIRECT);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_MSG);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_DIRECT);
    REQUIRE(cmd.getTo() == 0x12345678);
}

// ── Parameter handling ──────────────────────────────────────────────────

TEST_CASE("AdcCommand addParam and getParam", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_INF, AdcCommand::TYPE_BROADCAST);
    cmd.addParam("NIhello");
    cmd.addParam("DE", "some description");

    REQUIRE(cmd.getParameters().size() == 2);
    REQUIRE(cmd.getParam(0) == "NIhello");
    REQUIRE(cmd.getParam(1) == "DEsome description");
}

TEST_CASE("AdcCommand getParam by name", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_INF, AdcCommand::TYPE_BROADCAST);
    cmd.addParam("NI", "TestNick");
    cmd.addParam("DE", "TestDesc");
    cmd.addParam("SS", "12345");

    std::string val;
    REQUIRE(cmd.getParam("NI", 0, val));
    REQUIRE(val == "TestNick");

    REQUIRE(cmd.getParam("DE", 0, val));
    REQUIRE(val == "TestDesc");

    REQUIRE(cmd.getParam("SS", 0, val));
    REQUIRE(val == "12345");

    REQUIRE_FALSE(cmd.getParam("XX", 0, val)); // not present
}

TEST_CASE("AdcCommand hasFlag", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_SUP, AdcCommand::TYPE_HUB);
    // hasFlag checks for 2-char code with value '1' at position [2], total length 3
    cmd.addParam("TC1"); // flag TC set (value '1')
    cmd.addParam("UC0"); // flag UC not set (value '0')

    REQUIRE(cmd.hasFlag("TC", 0));
    REQUIRE_FALSE(cmd.hasFlag("UC", 0));
    REQUIRE_FALSE(cmd.hasFlag("XX", 0));
}

// ── escape / unescape ──────────────────────────────────────────────────

TEST_CASE("AdcCommand::escape escapes special characters", "[AdcCommand]") {
    // ADC escapes: space->\s, newline->\n, backslash->two backslashes
    std::string result = AdcCommand::escape("hello world", false);
    REQUIRE(result == "hello\\sworld");

    result = AdcCommand::escape("line1\nline2", false);
    REQUIRE(result == "line1\\nline2");

    result = AdcCommand::escape("back\\slash", false);
    REQUIRE(result == "back\\\\slash");
}

TEST_CASE("AdcCommand::escape with no special chars is identity", "[AdcCommand]") {
    std::string result = AdcCommand::escape("simpletext", false);
    REQUIRE(result == "simpletext");
}

TEST_CASE("AdcCommand::escape empty string", "[AdcCommand]") {
    REQUIRE(AdcCommand::escape("", false).empty());
}

// ── SID conversion ──────────────────────────────────────────────────────

TEST_CASE("AdcCommand::toSID and fromSID round-trip", "[AdcCommand]") {
    std::string sid_str = "ABCD";
    uint32_t sid = AdcCommand::toSID(sid_str);
    std::string back = AdcCommand::fromSID(sid);
    REQUIRE(back == sid_str);
}

TEST_CASE("AdcCommand::fromSID produces 4-byte string", "[AdcCommand]") {
    std::string result = AdcCommand::fromSID(0);
    REQUIRE(result.size() == 4);
}

// ── Parse from string ───────────────────────────────────────────────────

TEST_CASE("AdcCommand parse BMSG broadcast message", "[AdcCommand]") {
    // Format: BMSG <sid> <message> (no trailing newline — socket layer strips it)
    std::string line = "BMSG AAAB Hello\\sWorld";
    AdcCommand cmd(line);

    REQUIRE(cmd.getType() == AdcCommand::TYPE_BROADCAST);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_MSG);
    REQUIRE(cmd.getParameters().size() >= 1);
}

TEST_CASE("AdcCommand parse IINF hub info", "[AdcCommand]") {
    std::string line = "IINF NITestHub DEWelcome\\sto\\sour\\shub";
    AdcCommand cmd(line);

    REQUIRE(cmd.getType() == AdcCommand::TYPE_INFO);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_INF);

    std::string nick;
    REQUIRE(cmd.getParam("NI", 0, nick));
    REQUIRE(nick == "TestHub");

    std::string desc;
    REQUIRE(cmd.getParam("DE", 0, desc));
    REQUIRE(desc == "Welcome to our hub");
}

TEST_CASE("AdcCommand parse throws on too-short input", "[AdcCommand]") {
    REQUIRE_THROWS_AS(AdcCommand("AB"), ParseException);
    REQUIRE_THROWS_AS(AdcCommand(""), ParseException);
}

TEST_CASE("AdcCommand parse STA status command", "[AdcCommand]") {
    std::string line = "ISTA 000 Welcome";
    AdcCommand cmd(line);

    REQUIRE(cmd.getType() == AdcCommand::TYPE_INFO);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_STA);
}

// ── Serialization round-trip ────────────────────────────────────────────

TEST_CASE("AdcCommand serialize and re-parse round-trip", "[AdcCommand]") {
    AdcCommand original(AdcCommand::CMD_INF, AdcCommand::TYPE_BROADCAST);
    original.addParam("NI", "TestUser");
    original.addParam("DE", "Hello World");

    uint32_t fakeSid = AdcCommand::toSID("TEST");
    std::string serialized = original.toString(fakeSid);

    // Should start with BINF
    REQUIRE(serialized.substr(0, 4) == "BINF");
    REQUIRE(serialized.back() == '\n');

    // Strip trailing newline before re-parsing (socket layer does this)
    serialized.pop_back();

    // Re-parse
    AdcCommand reparsed(serialized);
    REQUIRE(reparsed.getCommand() == AdcCommand::CMD_INF);
    REQUIRE(reparsed.getType() == AdcCommand::TYPE_BROADCAST);

    std::string nick;
    REQUIRE(reparsed.getParam("NI", 0, nick));
    REQUIRE(nick == "TestUser");

    std::string desc;
    REQUIRE(reparsed.getParam("DE", 0, desc));
    REQUIRE(desc == "Hello World");
}

// ── Additional command types ────────────────────────────────────────────

TEST_CASE("AdcCommand construct STA with severity/error", "[AdcCommand]") {
    // Severity::SUCCESS = 0, Error::SUCCESS = 0
    AdcCommand cmd(AdcCommand::SEV_SUCCESS, AdcCommand::ERROR_GENERIC, "All good");
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_STA);
    REQUIRE(cmd.getParameters().size() >= 1);
}

TEST_CASE("AdcCommand construct STA with fatal error", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::SEV_FATAL, AdcCommand::ERROR_BAD_STATE, "Bad state");
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_STA);
}

TEST_CASE("AdcCommand parse DCTM direct connect-to-me", "[AdcCommand]") {
    std::string line = "DCTM AAAB AAAC ADC/1.0 192.168.1.1 12345";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_DIRECT);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_CTM);
}

TEST_CASE("AdcCommand parse DRCM direct reverse connect", "[AdcCommand]") {
    std::string line = "DRCM AAAB AAAC ADC/1.0 TOKN";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_DIRECT);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_RCM);
}

TEST_CASE("AdcCommand parse HSUP hub supports", "[AdcCommand]") {
    std::string line = "HSUP ADBASE ADTIGR";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_HUB);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_SUP);
    REQUIRE(cmd.getParameters().size() == 2);
}

TEST_CASE("AdcCommand parse IGPA hub password challenge", "[AdcCommand]") {
    std::string line = "IGPA ABCDEFGHIJ";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_INFO);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_GPA);
    REQUIRE(cmd.getParameters().size() == 1);
}

TEST_CASE("AdcCommand parse IQUI hub quit", "[AdcCommand]") {
    std::string line = "IQUI AAAB";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_INFO);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_QUI);
}

TEST_CASE("AdcCommand parse BSCH broadcast search", "[AdcCommand]") {
    std::string line = "BSCH AAAB ANsearch\\sterm TY1";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_BROADCAST);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_SCH);

    std::string an;
    REQUIRE(cmd.getParam("AN", 0, an));
    REQUIRE(an == "search term"); // unescape \\s → space
}

TEST_CASE("AdcCommand parse CGET client-to-client get", "[AdcCommand]") {
    std::string line = "CGET file files.xml.bz2 0 -1";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_CLIENT);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_GET);
}

TEST_CASE("AdcCommand parse CSND client-to-client send", "[AdcCommand]") {
    std::string line = "CSND file files.xml.bz2 0 12345";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_CLIENT);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_SND);
}

// ── Features ────────────────────────────────────────────────────────────

TEST_CASE("AdcCommand setFeatures and getFeatures", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_FEATURE);
    cmd.setFeatures("+SEGA");
    REQUIRE(cmd.getFeatures() == "+SEGA");
}

TEST_CASE("AdcCommand parse FSCH feature search", "[AdcCommand]") {
    std::string line = "FSCH AAAB +SEGA ANtest";
    AdcCommand cmd(line);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_FEATURE);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_SCH);
    // Note: the parser discards the feature string (// Skip... in parse())
    // Features are only stored when set via setFeatures() for serialization

    std::string an;
    REQUIRE(cmd.getParam("AN", 0, an));
    REQUIRE(an == "test");
}

// ── Edge cases ──────────────────────────────────────────────────────────

TEST_CASE("AdcCommand escape special characters are reversible", "[AdcCommand]") {
    // Round-trip with spaces, newlines, and backslashes
    std::string original = "hello world\nback\\slash";
    std::string escaped = AdcCommand::escape(original, false);

    REQUIRE(escaped.find(' ') == std::string::npos);
    REQUIRE(escaped.find('\n') == std::string::npos);

    // Parsing reconstructs the original via unescape
    // Build a minimal command string with the escaped param
    std::string line = "IMSG " + escaped;
    AdcCommand cmd(line);
    REQUIRE(cmd.getParam(0) == original);
}

TEST_CASE("AdcCommand toFourCC and fromFourCC", "[AdcCommand]") {
    std::string fourcc = "BMSG";
    uint32_t code = AdcCommand::toFourCC(fourcc.c_str());
    std::string back = AdcCommand::fromFourCC(code);
    REQUIRE(back == fourcc);
}

TEST_CASE("AdcCommand getFourCC", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST);
    std::string fourcc = cmd.getFourCC();
    REQUIRE(fourcc.length() == 4);
    REQUIRE(fourcc[0] == 'B'); // TYPE_BROADCAST
}

TEST_CASE("AdcCommand setType", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_BROADCAST);
    cmd.setType(AdcCommand::TYPE_HUB);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_HUB);
}

TEST_CASE("AdcCommand setTo", "[AdcCommand]") {
    AdcCommand cmd(AdcCommand::CMD_MSG, AdcCommand::TYPE_BROADCAST);
    cmd.setTo(0xDEADBEEF);
    REQUIRE(cmd.getTo() == 0xDEADBEEF);
}

TEST_CASE("AdcCommand parse single param only 3-char type+cmd", "[AdcCommand]") {
    // A command with exactly 4 chars and nothing else
    REQUIRE_NOTHROW(AdcCommand("ISUP"));
}

TEST_CASE("AdcCommand NMDC mode parsing", "[AdcCommand]") {
    // In NMDC mode, the $ prefix is stripped
    std::string line = "$ADCGET file files.xml.bz2 0 -1";
    AdcCommand cmd(line, true);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_GET);
}
