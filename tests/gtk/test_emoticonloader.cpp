/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <catch2/catch_test_macros.hpp>
#include "TestContext.h"
#include "EmoticonLoader.h"

#include <filesystem>
#include <fstream>

using namespace dcpp::test;

namespace {

/// Helper: create a sample emoticon XML file and (optionally) image stub files.
void createTestPack(const std::string &baseDir,
                    const std::string &packName,
                    bool createImages = true)
{
    namespace fs = std::filesystem;

    // Create pack XML file
    std::string xmlPath = baseDir + PATH_SEPARATOR_STR + packName + ".xml";
    std::string packDir = baseDir + PATH_SEPARATOR_STR + packName + PATH_SEPARATOR_STR;
    fs::create_directories(packDir);

    std::string xml;
    xml += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xml += "<emoticons-map name=\"" + packName + "\">\n";
    xml += "  <emoticon file=\"smile.png\">\n";
    xml += "    <name text=\":)\"/>\n";
    xml += "    <name text=\":-)\"/>\n";
    xml += "  </emoticon>\n";
    xml += "  <emoticon file=\"sad.png\">\n";
    xml += "    <name text=\":(\"/>\n";
    xml += "    <name text=\":-(\"/>\n";
    xml += "  </emoticon>\n";
    xml += "  <emoticon file=\"wink.png\">\n";
    xml += "    <name text=\";)\"/>\n";
    xml += "  </emoticon>\n";
    xml += "</emoticons-map>\n";

    {
        std::ofstream ofs(xmlPath);
        ofs << xml;
    }

    if (createImages) {
        // Create minimal stub files (just need to exist)
        for (const char *f : {"smile.png", "sad.png", "wink.png"}) {
            std::ofstream ofs(packDir + f);
            ofs << "PNG_STUB";
        }
    }
}

} // anonymous namespace

TEST_CASE("EmoticonLoader: listPacks", "[gtk][emoticons]")
{
    TestContext tc;
    std::string emoticonDir = tc.tmpDir.string() + PATH_SEPARATOR_STR "emoticons";
    std::filesystem::create_directories(emoticonDir);

    SECTION("empty directory") {
        auto packs = gtk_emoticon::listPacks(emoticonDir);
        REQUIRE(packs.empty());
    }

    SECTION("finds XML packs") {
        createTestPack(emoticonDir, "default");
        createTestPack(emoticonDir, "kolobok");

        auto packs = gtk_emoticon::listPacks(emoticonDir);
        REQUIRE(packs.size() == 2);

        // Sort for deterministic comparison
        std::sort(packs.begin(), packs.end());
        REQUIRE(packs[0] == "default");
        REQUIRE(packs[1] == "kolobok");
    }

    SECTION("non-existent directory") {
        auto packs = gtk_emoticon::listPacks("/nonexistent/path/emoticons");
        REQUIRE(packs.empty());
    }
}

TEST_CASE("EmoticonLoader: loadPack", "[gtk][emoticons]")
{
    TestContext tc;
    std::string emoticonDir = tc.tmpDir.string() + PATH_SEPARATOR_STR "emoticons";
    std::filesystem::create_directories(emoticonDir);

    SECTION("valid pack with images") {
        createTestPack(emoticonDir, "testpack");
        auto pack = gtk_emoticon::loadPack(emoticonDir + PATH_SEPARATOR_STR "testpack.xml", emoticonDir, true);

        REQUIRE(pack.packName == "testpack");
        REQUIRE(pack.entries.size() == 3);
        REQUIRE(pack.fileCount == 3);

        // Check first emoticon
        REQUIRE(pack.entries[0].file == "smile.png");
        REQUIRE(pack.entries[0].names.size() == 2);
        REQUIRE(pack.entries[0].names[0] == ":)");
        REQUIRE(pack.entries[0].names[1] == ":-)");
    }

    SECTION("pack without images (checkFileExists=false)") {
        createTestPack(emoticonDir, "noimg", false);
        auto pack = gtk_emoticon::loadPack(emoticonDir + PATH_SEPARATOR_STR "noimg.xml", emoticonDir, false);

        REQUIRE(pack.packName == "noimg");
        REQUIRE(pack.entries.size() == 3);
    }

    SECTION("pack without images (checkFileExists=true) skips missing") {
        createTestPack(emoticonDir, "noimg2", false);
        auto pack = gtk_emoticon::loadPack(emoticonDir + PATH_SEPARATOR_STR "noimg2.xml", emoticonDir, true);

        // All entries should be skipped because image files don't exist
        REQUIRE(pack.entries.empty());
    }

    SECTION("empty XML path") {
        auto pack = gtk_emoticon::loadPack("", emoticonDir);
        REQUIRE(pack.entries.empty());
    }

    SECTION("non-existent XML file") {
        auto pack = gtk_emoticon::loadPack("/nonexistent.xml", emoticonDir);
        REQUIRE(pack.entries.empty());
    }
}

TEST_CASE("EmoticonLoader: collectTriggers", "[gtk][emoticons]")
{
    TestContext tc;
    std::string emoticonDir = tc.tmpDir.string() + PATH_SEPARATOR_STR "emoticons";
    std::filesystem::create_directories(emoticonDir);
    createTestPack(emoticonDir, "triggertest");

    auto pack = gtk_emoticon::loadPack(emoticonDir + PATH_SEPARATOR_STR "triggertest.xml", emoticonDir, true);
    auto triggers = gtk_emoticon::collectTriggers(pack);

    REQUIRE(triggers.size() == 5); // :) :-) :( :-( ;)
    REQUIRE(triggers.count(":)") == 1);
    REQUIRE(triggers.count(":-)") == 1);
    REQUIRE(triggers.count(":(") == 1);
    REQUIRE(triggers.count(":-(") == 1);
    REQUIRE(triggers.count(";)") == 1);
}

TEST_CASE("EmoticonLoader: name deduplication", "[gtk][emoticons]")
{
    TestContext tc;
    std::string emoticonDir = tc.tmpDir.string() + PATH_SEPARATOR_STR "emoticons";
    std::string packDir = emoticonDir + PATH_SEPARATOR_STR "dedup" PATH_SEPARATOR_STR;
    std::filesystem::create_directories(packDir);

    // Create a pack with duplicate names across emoticons
    std::string xml = R"XML(<?xml version="1.0"?>
<emoticons-map name="dedup">
  <emoticon file="a.png">
    <name text=":)"/>
    <name text=":D"/>
  </emoticon>
  <emoticon file="b.png">
    <name text=":)"/>
    <name text=";)"/>
  </emoticon>
</emoticons-map>
)XML";

    {
        std::ofstream ofs(emoticonDir + PATH_SEPARATOR_STR "dedup.xml");
        ofs << xml;
    }
    for (const char *f : {"a.png", "b.png"}) {
        std::ofstream ofs(packDir + f);
        ofs << "STUB";
    }

    auto pack = gtk_emoticon::loadPack(emoticonDir + PATH_SEPARATOR_STR "dedup.xml", emoticonDir, true);

    // :) should only appear once (in the first emoticon)
    auto triggers = gtk_emoticon::collectTriggers(pack);
    REQUIRE(triggers.count(":)") == 1);
    REQUIRE(triggers.count(":D") == 1);
    REQUIRE(triggers.count(";)") == 1);
    REQUIRE(triggers.size() == 3);
}
