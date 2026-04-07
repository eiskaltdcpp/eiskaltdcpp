/*
 * Tests for FavoriteHubParams — Phase 2 data transformation extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "FavoriteHubParams.h"
#include "TestContext.h"

#include <dcpp/stdinc.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/HubEntry.h>

using namespace gtk_favhub;
using dcpp::FavoriteHubEntry;
using dcpp::test::TestContext;

// ── paramsFromEntry ──

TEST_CASE("FavoriteHubParams: paramsFromEntry basic", "[gtk][favhubparams]")
{
    TestContext ctx;

    FavoriteHubEntry entry(*ctx.ownedCtx);
    entry.setName("TestHub");
    entry.setServer("dchub://hub.example.com");
    entry.setHubDescription("A test hub");
    entry.setNick("MyNick");
    entry.setPassword("secret");
    entry.setUserDescription("Just testing");
    entry.setConnect(true);
    entry.setMode(1);
    entry.setDisableChat(false);
    entry.setExternalIP("1.2.3.4");
    entry.setUseInternetIP(true);

    auto p = paramsFromEntry(entry);

    REQUIRE(p["Name"] == "TestHub");
    REQUIRE(p["Address"] == "dchub://hub.example.com");
    REQUIRE(p["Description"] == "A test hub");
    REQUIRE(p["Nick"] == "MyNick");
    REQUIRE(p["Password"] == "secret");
    REQUIRE(p["User Description"] == "Just testing");
    REQUIRE(p["Auto Connect"] == "1");
    REQUIRE(p["Mode"] == "1");
    REQUIRE(p["Disable Chat"] == "0");
    REQUIRE(p["External IP"] == "1.2.3.4");
    REQUIRE(p["Internet IP"] == "1");
}

TEST_CASE("FavoriteHubParams: paramsFromEntry defaults", "[gtk][favhubparams]")
{
    TestContext ctx;

    FavoriteHubEntry entry(*ctx.ownedCtx);

    auto p = paramsFromEntry(entry);

    REQUIRE(p["Auto Connect"] == "0");
    REQUIRE(p["Mode"] == "0");
    REQUIRE(p["Disable Chat"] == "0");
    REQUIRE(p["Internet IP"] == "0");
    // All expected keys present
    REQUIRE(p.count("Name") == 1);
    REQUIRE(p.count("Address") == 1);
    REQUIRE(p.count("Description") == 1);
    REQUIRE(p.count("Nick") == 1);
    REQUIRE(p.count("Password") == 1);
    REQUIRE(p.count("Encoding") == 1);
    REQUIRE(p.count("Search Interval") == 1);
}

TEST_CASE("FavoriteHubParams: paramsFromEntry nick with default suppressed", "[gtk][favhubparams]")
{
    TestContext ctx;

    FavoriteHubEntry entry(*ctx.ownedCtx);
    // Don't set nick → getNick(false) should return empty

    auto p = paramsFromEntry(entry);
    REQUIRE(p["Nick"].empty());
}

// ── validateHubEntry ──

TEST_CASE("FavoriteHubParams: validateHubEntry valid entry", "[gtk][favhubparams]")
{
    std::string err;
    REQUIRE(validateHubEntry("MyHub", "dchub://hub.example.com", err));
    REQUIRE(err.empty());
}

TEST_CASE("FavoriteHubParams: validateHubEntry valid IP:port", "[gtk][favhubparams]")
{
    std::string err;
    REQUIRE(validateHubEntry("Hub", "192.168.1.1:411", err));
}

TEST_CASE("FavoriteHubParams: validateHubEntry empty name", "[gtk][favhubparams]")
{
    std::string err;
    REQUIRE_FALSE(validateHubEntry("", "dchub://hub", err));
    REQUIRE(!err.empty());
}

TEST_CASE("FavoriteHubParams: validateHubEntry empty address", "[gtk][favhubparams]")
{
    std::string err;
    REQUIRE_FALSE(validateHubEntry("Hub", "", err));
    REQUIRE(!err.empty());
}

TEST_CASE("FavoriteHubParams: validateHubEntry invalid address format", "[gtk][favhubparams]")
{
    std::string err;
    REQUIRE_FALSE(validateHubEntry("Hub", "justtext", err));
    REQUIRE(!err.empty());
}
