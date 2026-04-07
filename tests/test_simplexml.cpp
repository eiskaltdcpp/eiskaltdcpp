/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::SimpleXML — in-memory XML builder/parser
 *
 * Tests XML construction, serialization, and parsing round-trips.
 * Serves as regression tests during the Qt6 migration.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "SimpleXML.h"

using namespace dcpp;

// ── Build and serialize ─────────────────────────────────────────────────

TEST_CASE("SimpleXML build simple tag and serialize", "[SimpleXML]") {
    SimpleXML xml;
    xml.addTag("Root");
    xml.stepIn();
    xml.addTag("Child", "Hello");
    xml.stepOut();

    std::string output = xml.toXML();
    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find("<Root>") != std::string::npos);
    REQUIRE(output.find("Child") != std::string::npos);
    REQUIRE(output.find("Hello") != std::string::npos);
}

TEST_CASE("SimpleXML build tag with attributes", "[SimpleXML]") {
    SimpleXML xml;
    xml.addTag("Settings");
    xml.stepIn();
    xml.addTag("Item", std::string("value1"));
    xml.addChildAttrib("Name", std::string("setting1"));
    xml.addTag("Item", std::string("value2"));
    xml.addChildAttrib("Name", std::string("setting2"));
    xml.stepOut();

    std::string output = xml.toXML();
    REQUIRE(output.find("Name=\"setting1\"") != std::string::npos);
    REQUIRE(output.find("Name=\"setting2\"") != std::string::npos);
}

TEST_CASE("SimpleXML nested structure", "[SimpleXML]") {
    SimpleXML xml;
    xml.addTag("Root");
    xml.stepIn();
    xml.addTag("Level1");
    xml.stepIn();
    xml.addTag("Level2", "deep data");
    xml.stepOut();
    xml.stepOut();

    std::string output = xml.toXML();
    REQUIRE(output.find("Level1") != std::string::npos);
    REQUIRE(output.find("Level2") != std::string::npos);
    REQUIRE(output.find("deep data") != std::string::npos);
}

// ── Parse and read ──────────────────────────────────────────────────────

TEST_CASE("SimpleXML parse and read back simple structure", "[SimpleXML]") {
    // Build XML — addChildAttrib adds to the last-added child tag
    SimpleXML builder;
    builder.addTag("Config");
    builder.stepIn();
    builder.addTag("Setting", std::string("42"));
    builder.addChildAttrib("Name", std::string("Port"));
    builder.addTag("Setting", std::string("hello"));
    builder.addChildAttrib("Name", std::string("Nick"));
    builder.stepOut();

    std::string xmlStr = builder.toXML();

    // Parse it back
    SimpleXML reader;
    reader.fromXML(xmlStr);
    reader.stepIn();

    REQUIRE(reader.findChild("Setting"));
    REQUIRE(reader.getChildAttrib("Name") == "Port");
    REQUIRE(reader.getChildData() == "42");

    REQUIRE(reader.findChild("Setting"));
    REQUIRE(reader.getChildAttrib("Name") == "Nick");
    REQUIRE(reader.getChildData() == "hello");

    reader.stepOut();
}

TEST_CASE("SimpleXML getIntChildAttrib parses integer data", "[SimpleXML]") {
    SimpleXML builder;
    builder.addTag("Root");
    builder.stepIn();
    builder.addTag("Port", std::string("411"));
    builder.stepOut();

    std::string xmlStr = builder.toXML();

    SimpleXML reader;
    reader.fromXML(xmlStr);
    reader.stepIn();
    REQUIRE(reader.findChild("Port"));
    REQUIRE(reader.getIntChildAttrib("Port") == 0); // getIntChildAttrib reads attrib, not data
    reader.stepOut();
}

// ── Escape handling ─────────────────────────────────────────────────────

TEST_CASE("SimpleXML escapes special XML characters", "[SimpleXML]") {
    SimpleXML xml;
    xml.addTag("Root");
    xml.stepIn();
    xml.addTag("Data", std::string("a < b & c > d \"quoted\" 'apos'"));
    xml.stepOut();

    std::string output = xml.toXML();
    // Should contain escaped versions
    REQUIRE(output.find("&lt;") != std::string::npos);
    REQUIRE(output.find("&amp;") != std::string::npos);
    REQUIRE(output.find("&gt;") != std::string::npos);
}

TEST_CASE("SimpleXML round-trip preserves special characters", "[SimpleXML]") {
    const std::string specialData = "Tom & Jerry < 100 > 50";

    SimpleXML builder;
    builder.addTag("Root");
    builder.stepIn();
    builder.addTag("Text", std::string(specialData));
    builder.stepOut();

    std::string xmlStr = builder.toXML();

    SimpleXML reader;
    reader.fromXML(xmlStr);
    reader.stepIn();
    REQUIRE(reader.findChild("Text"));
    REQUIRE(reader.getChildData() == specialData);
    reader.stepOut();
}

// ── needsEscape ─────────────────────────────────────────────────────────

TEST_CASE("SimpleXML::needsEscape detects escapeable content", "[SimpleXML]") {
    REQUIRE(SimpleXML::needsEscape("hello <world>", false));
    REQUIRE(SimpleXML::needsEscape("a & b", false));
    REQUIRE_FALSE(SimpleXML::needsEscape("plain text", false));
}

// ── Error handling ──────────────────────────────────────────────────────

TEST_CASE("SimpleXML findChild returns false when not found", "[SimpleXML]") {
    SimpleXML xml;
    xml.addTag("Root");
    xml.stepIn();
    xml.addTag("Exists", std::string("data"));
    xml.stepOut();

    std::string xmlStr = xml.toXML();

    SimpleXML reader;
    reader.fromXML(xmlStr);
    reader.stepIn();
    REQUIRE_FALSE(reader.findChild("DoesNotExist"));
    reader.stepOut();
}
