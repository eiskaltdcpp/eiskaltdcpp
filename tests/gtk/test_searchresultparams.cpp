/*
 * Tests for SearchResultParams — Phase 2 data transformation extraction.
 *
 * Tests groupingColumn (pure mapping), fileOrderKey, slotsOrderKey,
 * and paramsFromResult with constructed SearchResult objects.
 */
#include <catch2/catch_test_macros.hpp>
#include "SearchResultParams.h"
#include "TestContext.h"

#include <dcpp/stdinc.h>
#include <dcpp/SearchResult.h>
#include <dcpp/MerkleTree.h>
#include <dcpp/User.h>
#include <dcpp/CID.h>

using namespace gtk_search;
using dcpp::SearchResult;
using dcpp::test::TestContext;
using dcpp::TTHValue;
using dcpp::UserPtr;
using dcpp::User;
using dcpp::CID;

// ── groupingColumn ──

TEST_CASE("SearchResultParams: groupingColumn all types", "[gtk][searchparams]")
{
    REQUIRE(groupingColumn(GroupType::NONE) == "");
    REQUIRE(groupingColumn(GroupType::FILENAME) == "Filename");
    REQUIRE(groupingColumn(GroupType::FILEPATH) == "Path");
    REQUIRE(groupingColumn(GroupType::SIZE) == "Size");
    REQUIRE(groupingColumn(GroupType::CONNECTION) == "Connection");
    REQUIRE(groupingColumn(GroupType::TTH) == "TTH");
    REQUIRE(groupingColumn(GroupType::NICK) == "Nick");
    REQUIRE(groupingColumn(GroupType::HUB) == "Hub");
    REQUIRE(groupingColumn(GroupType::TYPE) == "Type");
}

// ── fileOrderKey ──

TEST_CASE("SearchResultParams: fileOrderKey for files", "[gtk][searchparams]")
{
    REQUIRE(fileOrderKey("test.txt", false) == "ftest.txt");
}

TEST_CASE("SearchResultParams: fileOrderKey for directories", "[gtk][searchparams]")
{
    REQUIRE(fileOrderKey("Music/", true) == "dMusic/");
}

// ── slotsOrderKey ──

TEST_CASE("SearchResultParams: slotsOrderKey more free slots sorts first", "[gtk][searchparams]")
{
    // More free slots → more negative → sorts first
    std::string a = slotsOrderKey(5, 10);
    std::string b = slotsOrderKey(2, 10);
    // a (5 free) should be < b (2 free) numerically
    int va = std::stoi(a);
    int vb = std::stoi(b);
    REQUIRE(va < vb);
}

TEST_CASE("SearchResultParams: slotsOrderKey zero free slots", "[gtk][searchparams]")
{
    std::string s = slotsOrderKey(0, 5);
    REQUIRE(std::stoi(s) == -5);
}

// ── paramsFromResult (file) ──

TEST_CASE("SearchResultParams: paramsFromResult for file", "[gtk][searchparams]")
{
    TestContext ctx;

    TTHValue tth("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    UserPtr user(new User(CID("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB1")));

    SearchResult result(user, SearchResult::TYPE_FILE,
                       /*slots*/3, /*freeSlots*/1,
                       /*size*/1048576, "/share/Music/song.mp3",
                       "TestHub", "dchub://hub.test.com",
                       "10.0.0.1", tth, "token1");

    auto p = paramsFromResult(result, "TestNick", "100 Mbit/s", false);

    REQUIRE(p["Filename"] == "song.mp3");
    REQUIRE(p["Path"] == "/share/Music/");
    REQUIRE(p["File Order"] == "fsong.mp3");
    REQUIRE(p["Type"] == "mp3");
    REQUIRE(p["Icon"] == "icon-file");
    REQUIRE(p["Shared"] == "0");
    REQUIRE(p["Nick"] == "TestNick");
    REQUIRE(p["Hub"] == "TestHub");
    REQUIRE(p["Hub URL"] == "dchub://hub.test.com");
    REQUIRE(p["IP"] == "10.0.0.1");
    REQUIRE(p.count("TTH") == 1);
    REQUIRE(!p["TTH"].empty());
    REQUIRE(p["Free Slots"] == "1");
    REQUIRE(!p["Size"].empty());
    REQUIRE(!p["Exact Size"].empty());
    REQUIRE(!p["Real Size"].empty());
}

TEST_CASE("SearchResultParams: paramsFromResult for file without path", "[gtk][searchparams]")
{
    TestContext ctx;

    TTHValue tth("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    UserPtr user(new User(CID("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC1")));

    SearchResult result(user, SearchResult::TYPE_FILE,
                       3, 1, 1024, "readme.txt",
                       "Hub", "adc://hub",
                       "1.2.3.4", tth, "token2");

    auto p = paramsFromResult(result, "Nick", "conn", true);

    // No path separator: Filename = full string, no Path key
    REQUIRE(p["Filename"] == "readme.txt");
    REQUIRE(p["Shared"] == "1");
}

// ── paramsFromResult (directory) ──

TEST_CASE("SearchResultParams: paramsFromResult for directory", "[gtk][searchparams]")
{
    TestContext ctx;

    TTHValue tth("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    UserPtr user(new User(CID("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD1")));

    SearchResult result(user, SearchResult::TYPE_DIRECTORY,
                       5, 2, 0, "Share/Music/",
                       "Hub2", "dchub://h2",
                       "5.6.7.8", tth, "token3");

    auto p = paramsFromResult(result, "DirNick", "LAN", false);

    REQUIRE(p["Type"] == "Directory");
    REQUIRE(p["Icon"] == "icon-directory");
    REQUIRE(p["Shared"] == "0");
    REQUIRE(p["File Order"].substr(0, 1) == "d");
    // Directory with size 0 should not have Size key set
    REQUIRE(p.count("Size") == 0);
    // No TTH for directories
    REQUIRE(p.count("TTH") == 0);
}

TEST_CASE("SearchResultParams: paramsFromResult directory with size", "[gtk][searchparams]")
{
    TestContext ctx;

    TTHValue tth("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    UserPtr user(new User(CID("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE1")));

    SearchResult result(user, SearchResult::TYPE_DIRECTORY,
                       5, 2, 5000, "Share/Videos/",
                       "Hub3", "dchub://h3",
                       "9.8.7.6", tth, "token4");

    auto p = paramsFromResult(result, "Nick2", "DSL", false);

    // Directory with size > 0 SHOULD have Size
    REQUIRE(!p["Size"].empty());
    REQUIRE(!p["Exact Size"].empty());
}

// ── paramsFromResult hub name fallback ──

TEST_CASE("SearchResultParams: paramsFromResult empty hub name uses URL", "[gtk][searchparams]")
{
    TestContext ctx;

    TTHValue tth("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    UserPtr user(new User(CID("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1")));

    SearchResult result(user, SearchResult::TYPE_FILE,
                       1, 0, 100, "test.txt",
                       "", "dchub://fallback.hub",
                       "", tth, "");

    auto p = paramsFromResult(result, "N", "C", false);

    // Empty hub name → falls back to hub URL
    REQUIRE(p["Hub"] == "dchub://fallback.hub");
}
