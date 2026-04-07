/*
 * Tests for FinishedTransferParams — Phase 2 data transformation extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "FinishedTransferParams.h"
#include "TestContext.h"

#include <dcpp/stdinc.h>
#include <dcpp/FinishedItem.h>
#include <dcpp/User.h>
#include <dcpp/CID.h>
#include <dcpp/HintedUser.h>
#include <algorithm>

using namespace gtk_finished;
using dcpp::FinishedFileItem;
using dcpp::test::TestContext;
using dcpp::FinishedUserItem;
using dcpp::UserPtr;
using dcpp::User;
using dcpp::CID;
using dcpp::HintedUser;

namespace {
/// Convert a Unix-style hardcoded path to the platform's native separator.
inline std::string nativePath(const char* p) {
    std::string s(p);
#ifdef _WIN32
    std::replace(s.begin(), s.end(), '/', '\\');
#endif
    return s;
}
} // anon

// ── paramsFromFile ──

TEST_CASE("FinishedTransferParams: paramsFromFile basic", "[gtk][finishedparams]")
{
    TestContext ctx;

    UserPtr user(new User(CID("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1")));
    HintedUser hu(user, "dchub://hub");

    FinishedFileItem item(
        /*transferred*/1048576,
        /*milliSeconds*/5000,
        /*time*/1700000000,
        /*fileSize*/2097152,
        /*actual*/1048576,
        /*crc32Checked*/true,
        hu);

    auto p = paramsFromFile(item, nativePath("/downloads/music/song.mp3"), "TestNick");

    REQUIRE(p["Filename"] == "song.mp3");
    REQUIRE(p["Path"] == nativePath("/downloads/music/"));
    REQUIRE(p["Nicks"] == "TestNick");
    REQUIRE(p["Target"] == nativePath("/downloads/music/song.mp3"));
    REQUIRE(p["CRC Checked"] == "Yes");
    REQUIRE(p["Transferred"] == "1048576");
    REQUIRE(p["Elapsed Time"] == "5000");
    // Speed is transferred/seconds → should be non-zero string
    REQUIRE(!p["Speed"].empty());
    // Not full yet (transferred < fileSize)
    REQUIRE(p["full"] == "0");
    // Time should be formatted
    REQUIRE(!p["Time"].empty());
}

TEST_CASE("FinishedTransferParams: paramsFromFile CRC not checked", "[gtk][finishedparams]")
{
    TestContext ctx;

    UserPtr user(new User(CID("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB1")));
    HintedUser hu(user, "");

    FinishedFileItem item(500, 1000, 1700000000, 500, 500, false, hu);

    auto p = paramsFromFile(item, nativePath("/tmp/file.dat"), "Nick");

    REQUIRE(p["CRC Checked"] == "No");
    // Full (transferred == fileSize)
    REQUIRE(p["full"] == "1");
}

// ── paramsFromUser ──

TEST_CASE("FinishedTransferParams: paramsFromUser basic", "[gtk][finishedparams]")
{
    TestContext ctx;

    FinishedUserItem item(2097152, 3000, 1700000000, "/download/file1.txt");

    auto p = paramsFromUser(item, "UserNick", "TestHub",
                           "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC1");

    REQUIRE(p["Nick"] == "UserNick");
    REQUIRE(p["Hub"] == "TestHub");
    REQUIRE(p["CID"] == "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC1");
    REQUIRE(p["Transferred"] == "2097152");
    REQUIRE(p["Elapsed Time"] == "3000");
    REQUIRE(!p["Speed"].empty());
    REQUIRE(!p["Time"].empty());
    // Files should contain the initial file
    REQUIRE(p["Files"].find("file1.txt") != std::string::npos);
}

TEST_CASE("FinishedTransferParams: paramsFromUser multiple files", "[gtk][finishedparams]")
{
    TestContext ctx;

    FinishedUserItem item(1000, 500, 1700000000, "file1.txt");
    // Add another file by calling update
    item.update(500, 200, 1700001000, "file2.txt");

    auto p = paramsFromUser(item, "Nick", "Hub", "CID");

    // Files should contain both
    REQUIRE(p["Files"].find("file1.txt") != std::string::npos);
    REQUIRE(p["Files"].find("file2.txt") != std::string::npos);
}

// ── computeTotals ──

TEST_CASE("FinishedTransferParams: computeTotals empty", "[gtk][finishedparams]")
{
    auto t = computeTotals({});
    REQUIRE(t.files == 0);
    REQUIRE(t.bytes == 0);
    REQUIRE(t.avgSpeed == 0);
}

TEST_CASE("FinishedTransferParams: computeTotals single item", "[gtk][finishedparams]")
{
    auto t = computeTotals({{1000, 500}});
    REQUIRE(t.files == 1);
    REQUIRE(t.bytes == 1000);
    REQUIRE(t.avgSpeed == 500);
}

TEST_CASE("FinishedTransferParams: computeTotals multiple items", "[gtk][finishedparams]")
{
    auto t = computeTotals({
        {1000, 200},
        {2000, 400},
        {3000, 600}
    });
    REQUIRE(t.files == 3);
    REQUIRE(t.bytes == 6000);
    REQUIRE(t.avgSpeed == 400); // (200+400+600)/3
}
