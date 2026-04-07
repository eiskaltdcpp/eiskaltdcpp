/*
 * Tests for SearchResult — construction, toRES, getBaseName, getSlotString.
 * No DCContext required (uses the full-parameter constructor).
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "SearchResult.h"
#include "User.h"
#include "CID.h"
#include "TigerHash.h"
#include "Pointer.h"
#include "AdcCommand.h"

using namespace dcpp;

// Helper: make a dummy UserPtr
static UserPtr makeUser(const std::string& nick) {
    TigerHash th;
    th.update(nick.data(), nick.size());
    th.finalize();
    CID cid(th.getResult());
    return UserPtr(new User(cid));
}

// Helper: make a TTHValue from a string
static TTHValue makeTTH(const std::string& seed) {
    TigerHash th;
    th.update(seed.data(), seed.size());
    th.finalize();
    return TTHValue(th.getResult());
}

TEST_CASE("SearchResult: full constructor and getters", "[searchresult]") {
    auto user = makeUser("testuser");
    auto tth = makeTTH("file_content");

    SearchResult sr(user, SearchResult::TYPE_FILE, 5, 3,
                    1024 * 1024, "path\\to\\file.txt", "MyHub",
                    "adc://hub.example.com", "192.168.1.1", tth, "token123");

    REQUIRE(sr.getFile() == "path\\to\\file.txt");
    REQUIRE(sr.getHubName() == "MyHub");
    REQUIRE(sr.getHubURL() == "adc://hub.example.com");
    REQUIRE(sr.getSize() == 1024 * 1024);
    REQUIRE(sr.getType() == SearchResult::TYPE_FILE);
    REQUIRE(sr.getSlots() == 5);
    REQUIRE(sr.getFreeSlots() == 3);
    REQUIRE(sr.getIP() == "192.168.1.1");
    REQUIRE(sr.getTTH() == tth);
    REQUIRE(sr.getToken() == "token123");
}

TEST_CASE("SearchResult: getSlotString", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    SearchResult sr(user, SearchResult::TYPE_FILE, 10, 7,
                    0, "file", "hub", "url", "ip", tth, "t");

    REQUIRE(sr.getSlotString() == "7/10");
}

TEST_CASE("SearchResult: getSlotString zero slots", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    SearchResult sr(user, SearchResult::TYPE_FILE, 0, 0,
                    0, "file", "hub", "url", "ip", tth, "t");

    REQUIRE(sr.getSlotString() == "0/0");
}

TEST_CASE("SearchResult: getBaseName for file", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    SearchResult sr(user, SearchResult::TYPE_FILE, 1, 1,
                    100, "path\\to\\file.txt", "hub", "url", "ip", tth, "t");

    REQUIRE(sr.getBaseName() == "file.txt");
}

TEST_CASE("SearchResult: getBaseName for file no directory", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    SearchResult sr(user, SearchResult::TYPE_FILE, 1, 1,
                    100, "file.txt", "hub", "url", "ip", tth, "t");

    REQUIRE(sr.getBaseName() == "file.txt");
}

TEST_CASE("SearchResult: getBaseName for directory with trailing slash", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    SearchResult sr(user, SearchResult::TYPE_DIRECTORY, 1, 1,
                    0, "path\\to\\somedir\\", "hub", "url", "ip", tth, "t");

    REQUIRE(sr.getBaseName() == "somedir");
}

TEST_CASE("SearchResult: getBaseName for directory no trailing slash", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    // getLastDir looks for the last backslash-delimited segment that ends at a '\'.
    // Without trailing '\', "path\to\somedir" --> getLastDir returns "to"
    // (the last complete directory between two separators)
    SearchResult sr(user, SearchResult::TYPE_DIRECTORY, 1, 1,
                    0, "path\\to\\somedir", "hub", "url", "ip", tth, "t");

    REQUIRE(sr.getBaseName() == "to");
}

TEST_CASE("SearchResult: toRES produces valid AdcCommand", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("content");

    SearchResult sr(user, SearchResult::TYPE_FILE, 5, 3,
                    2048, "path/to/file.txt", "hub", "url", "ip", tth, "t");

    // toRES with CLIENT type
    AdcCommand cmd = sr.toRES(AdcCommand::TYPE_CLIENT);

    // Verify the command has expected parameters
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_RES);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_CLIENT);

    // Check SI (size), SL (slots), FN (filename), TR (TTH) params
    string val;
    REQUIRE(cmd.getParam("SI", 0, val));
    REQUIRE(val == "2048");
    REQUIRE(cmd.getParam("SL", 0, val));
    REQUIRE(val == "3");
    REQUIRE(cmd.getParam("TR", 0, val));
    REQUIRE(val == tth.toBase32());
    REQUIRE(cmd.getParam("FN", 0, val));
    REQUIRE(val == Util::toAdcFile("path/to/file.txt"));
}

TEST_CASE("SearchResult: toRES with UDP type", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");

    SearchResult sr(user, SearchResult::TYPE_FILE, 1, 1,
                    100, "test.bin", "h", "u", "ip", tth, "t");

    AdcCommand cmd = sr.toRES(AdcCommand::TYPE_UDP);
    REQUIRE(cmd.getType() == AdcCommand::TYPE_UDP);
    REQUIRE(cmd.getCommand() == AdcCommand::CMD_RES);
}

TEST_CASE("SearchResult: TYPE_FILE vs TYPE_DIRECTORY", "[searchresult]") {
    REQUIRE(SearchResult::TYPE_FILE == 0);
    REQUIRE(SearchResult::TYPE_DIRECTORY == 1);
}

TEST_CASE("SearchResult: user pointer accessible", "[searchresult]") {
    auto user = makeUser("u1");
    auto tth = makeTTH("x");
    CID expectedCID = user->getCID();

    SearchResult sr(user, SearchResult::TYPE_FILE, 1, 1,
                    0, "f", "h", "u", "ip", tth, "t");

    REQUIRE(sr.getUser()->getCID() == expectedCID);
}
