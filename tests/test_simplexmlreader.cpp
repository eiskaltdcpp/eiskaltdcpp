/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::SimpleXMLReader (SAX-style XML parser)
 *
 * SimpleXMLReader is the core XML parser used for file lists, settings,
 * favorites, and protocol messages. We test it with a CallBack that records
 * events, then verify the event sequence.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "SimpleXMLReader.h"

#include <string>
#include <vector>
#include <sstream>

using namespace dcpp;

// ── Recording callback ──────────────────────────────────────────────────

struct Event {
    enum Type { START, DATA, END } type;
    std::string name;               // tag name (START/END) or data content (DATA)
    StringPairList attribs;          // only for START
    bool simple = false;             // only for START
};

struct RecordingCallback : public SimpleXMLReader::CallBack {
    std::vector<Event> events;

    void startTag(const std::string& name, StringPairList& attribs, bool simple) override {
        Event e;
        e.type = Event::START;
        e.name = name;
        e.attribs = attribs;
        e.simple = simple;
        events.push_back(e);
    }

    void data(const std::string& d) override {
        Event e;
        e.type = Event::DATA;
        e.name = d;
        events.push_back(e);
    }

    void endTag(const std::string& name) override {
        Event e;
        e.type = Event::END;
        e.name = name;
        events.push_back(e);
    }
};

// ── Helper to parse a string ────────────────────────────────────────────

static std::vector<Event> parseXML(const std::string& xml) {
    RecordingCallback cb;
    SimpleXMLReader reader(&cb);
    reader.parse(xml);
    return cb.events;
}

// ── Basic tag parsing ───────────────────────────────────────────────────

TEST_CASE("SimpleXMLReader: simple self-closing tag", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><item/>");
    REQUIRE(events.size() >= 1);
    REQUIRE(events[0].type == Event::START);
    REQUIRE(events[0].name == "item");
    REQUIRE(events[0].simple == true);
}

TEST_CASE("SimpleXMLReader: open and close tag", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><root></root>");
    REQUIRE(events.size() >= 2);
    REQUIRE(events[0].type == Event::START);
    REQUIRE(events[0].name == "root");
    REQUIRE(events[0].simple == false);
    REQUIRE(events[1].type == Event::END);
    REQUIRE(events[1].name == "root");
}

TEST_CASE("SimpleXMLReader: tag with text content", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><msg>Hello</msg>");
    REQUIRE(events.size() >= 3);
    REQUIRE(events[0].type == Event::START);
    REQUIRE(events[0].name == "msg");
    REQUIRE(events[1].type == Event::DATA);
    REQUIRE(events[1].name == "Hello");
    REQUIRE(events[2].type == Event::END);
    REQUIRE(events[2].name == "msg");
}

// ── Attributes ──────────────────────────────────────────────────────────

TEST_CASE("SimpleXMLReader: tag with attributes", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><file name=\"test.txt\" size=\"1024\"/>");
    REQUIRE(events.size() >= 1);
    REQUIRE(events[0].type == Event::START);
    REQUIRE(events[0].name == "file");

    bool foundName = false, foundSize = false;
    for (const auto& p : events[0].attribs) {
        if (p.first == "name") { REQUIRE(p.second == "test.txt"); foundName = true; }
        if (p.first == "size") { REQUIRE(p.second == "1024"); foundSize = true; }
    }
    REQUIRE(foundName);
    REQUIRE(foundSize);
}

TEST_CASE("SimpleXMLReader: attribute with single quotes (apostrophe)", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><item val='hello'/>");
    REQUIRE(events.size() >= 1);
    bool found = false;
    for (const auto& p : events[0].attribs) {
        if (p.first == "val") { REQUIRE(p.second == "hello"); found = true; }
    }
    REQUIRE(found);
}

// ── Nested elements ─────────────────────────────────────────────────────

TEST_CASE("SimpleXMLReader: nested elements", "[SimpleXMLReader]") {
    auto events = parseXML(
        "<?xml version=\"1.0\"?>"
        "<root><child1/><child2>data</child2></root>");

    // root start, child1 start(simple), child2 start, data, child2 end, root end
    REQUIRE(events.size() >= 6);
    REQUIRE(events[0].name == "root");
    REQUIRE(events[0].type == Event::START);
    REQUIRE(events[1].name == "child1");
    REQUIRE(events[1].simple == true);
    REQUIRE(events[2].name == "child2");
    REQUIRE(events[3].type == Event::DATA);
    REQUIRE(events[3].name == "data");
    REQUIRE(events[4].type == Event::END);
    REQUIRE(events[4].name == "child2");
    REQUIRE(events[5].type == Event::END);
    REQUIRE(events[5].name == "root");
}

// ── Entity references ───────────────────────────────────────────────────

TEST_CASE("SimpleXMLReader: entity references in content", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><msg>a&amp;b&lt;c&gt;d&apos;e&quot;f</msg>");
    // Find the DATA event(s) and concatenate
    std::string combined;
    for (const auto& e : events) {
        if (e.type == Event::DATA) combined += e.name;
    }
    REQUIRE(combined == "a&b<c>d'e\"f");
}

TEST_CASE("SimpleXMLReader: entity references in attributes", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><item val=\"a&amp;b\"/>");
    REQUIRE(events.size() >= 1);
    for (const auto& p : events[0].attribs) {
        if (p.first == "val") {
            REQUIRE(p.second == "a&b");
        }
    }
}

// ── Comments & CDATA ────────────────────────────────────────────────────

TEST_CASE("SimpleXMLReader: XML comment is ignored", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><!-- comment --><root/>");
    // Only root start event expected
    REQUIRE(events.size() >= 1);
    REQUIRE(events[0].name == "root");
}

TEST_CASE("SimpleXMLReader: CDATA section", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><msg><![CDATA[<not>xml</not>]]></msg>");
    std::string combined;
    for (const auto& e : events) {
        if (e.type == Event::DATA) combined += e.name;
    }
    REQUIRE(combined == "<not>xml</not>");
}

// ── XML declaration ─────────────────────────────────────────────────────

TEST_CASE("SimpleXMLReader: XML declaration with encoding", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\" encoding=\"UTF-8\"?><root/>");
    REQUIRE(events.size() >= 1);
    REQUIRE(events[0].name == "root");
}

// ── Empty / minimal documents ───────────────────────────────────────────

TEST_CASE("SimpleXMLReader: minimal document", "[SimpleXMLReader]") {
    auto events = parseXML("<?xml version=\"1.0\"?><x/>");
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].name == "x");
    REQUIRE(events[0].simple == true);
}

// ── DC++ file list-like structure ───────────────────────────────────────

TEST_CASE("SimpleXMLReader: DC++ file list structure", "[SimpleXMLReader]") {
    std::string xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<FileListing Version=\"1\" Generator=\"EiskaltDC++\">"
        "<Directory Name=\"Share\">"
        "<File Name=\"test.txt\" Size=\"1234\" TTH=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567ABCDEFG\"/>"
        "</Directory>"
        "</FileListing>";

    auto events = parseXML(xml);

    // FileListing start, Directory start, File start(simple), Directory end, FileListing end
    REQUIRE(events.size() >= 5);
    REQUIRE(events[0].name == "FileListing");
    REQUIRE(events[1].name == "Directory");
    REQUIRE(events[2].name == "File");
    REQUIRE(events[2].simple == true);
    REQUIRE(events[3].type == Event::END);
    REQUIRE(events[3].name == "Directory");
    REQUIRE(events[4].type == Event::END);
    REQUIRE(events[4].name == "FileListing");
}
