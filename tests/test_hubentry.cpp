/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::HubEntry and dcpp::FavoriteHubEntry
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "SettingsManager.h"
#include "Text.h"
#include "Util.h"
#include "HubEntry.h"
#include "TestContext.h"

#include <cmath>

using namespace dcpp;

// ── HubEntry ────────────────────────────────────────────────────────────

TEST_CASE("HubEntry: 4-arg constructor", "[HubEntry]") {
    test::TestContext tc;

    HubEntry he("TestHub", "adc://example.com", "A test hub", "150");

    REQUIRE(he.getName() == "TestHub");
    REQUIRE(he.getServer() == "adc://example.com");
    REQUIRE(he.getDescription() == "A test hub");
    REQUIRE(he.getUsers() == 150);
    REQUIRE(he.getShared() == 0);
    REQUIRE(he.getMinShare() == 0);
    REQUIRE(he.getMinSlots() == 0);
    REQUIRE(he.getMaxHubs() == 0);
    REQUIRE(he.getMaxUsers() == 0);
    REQUIRE(he.getReliability() == 0.0f);
}

TEST_CASE("HubEntry: 12-arg constructor parses string values", "[HubEntry]") {
    test::TestContext tc;

    HubEntry he(
        "BigHub",                // name
        "dchub://big.hub.org",   // server
        "The biggest hub",       // description
        "5000",                  // users
        "US",                    // country
        "1099511627776",         // shared (1 TB)
        "1073741824",            // minShare (1 GB)
        "3",                     // minSlots
        "5",                     // maxHubs
        "10000",                 // maxUsers
        "9500",                  // reliability (as percentage * 100)
        "A+"                     // rating
    );

    REQUIRE(he.getName() == "BigHub");
    REQUIRE(he.getServer() == "dchub://big.hub.org");
    REQUIRE(he.getDescription() == "The biggest hub");
    REQUIRE(he.getUsers() == 5000);
    REQUIRE(he.getCountry() == "US");
    REQUIRE(he.getShared() == 1099511627776LL);
    REQUIRE(he.getMinShare() == 1073741824LL);
    REQUIRE(he.getMinSlots() == 3);
    REQUIRE(he.getMaxHubs() == 5);
    REQUIRE(he.getMaxUsers() == 10000);
    REQUIRE(std::fabs(he.getReliability() - 95.0f) < 0.01f);
    REQUIRE(he.getRating() == "A+");
}

TEST_CASE("HubEntry: default constructor", "[HubEntry]") {
    test::TestContext tc;
    HubEntry he;

    // Default-constructed HubEntry has default-initialized members.
    // Strings are empty; numeric fields are indeterminate with = default,
    // so we only check the string members here.
    REQUIRE(he.getName().empty());
    REQUIRE(he.getServer().empty());
}

TEST_CASE("HubEntry: GETSET accessors", "[HubEntry]") {
    test::TestContext tc;
    HubEntry he;

    he.setName("NewName");
    he.setServer("adc://new.server");
    he.setUsers(42);
    he.setShared(1000000);

    REQUIRE(he.getName() == "NewName");
    REQUIRE(he.getServer() == "adc://new.server");
    REQUIRE(he.getUsers() == 42);
    REQUIRE(he.getShared() == 1000000);
}

// ── FavoriteHubEntry ────────────────────────────────────────────────────

TEST_CASE("FavoriteHubEntry: default constructor", "[FavoriteHubEntry]") {
    test::TestContext tc;

    FavoriteHubEntry fhe(*tc.ownedCtx);

    REQUIRE(fhe.getName().empty());
    REQUIRE(fhe.getServer().empty());
    REQUIRE(fhe.getConnect() == false);
    REQUIRE(fhe.getMode() == 0);
    REQUIRE(fhe.getOverrideId() == false);
    REQUIRE(fhe.getUseInternetIP() == false);
    REQUIRE(fhe.getDisableChat() == false);
}

TEST_CASE("FavoriteHubEntry: construct from HubEntry", "[FavoriteHubEntry]") {
    test::TestContext tc;

    HubEntry he("TestHub", "adc://test.com", "desc", "100");
    FavoriteHubEntry fhe(*tc.ownedCtx, he);

    REQUIRE(fhe.getName() == "TestHub");
    REQUIRE(fhe.getServer() == "adc://test.com");
    REQUIRE(fhe.getHubDescription() == "desc");
    REQUIRE(fhe.getConnect() == false);
}

TEST_CASE("FavoriteHubEntry: copy constructor", "[FavoriteHubEntry]") {
    test::TestContext tc;

    FavoriteHubEntry orig(*tc.ownedCtx);
    orig.setName("MyHub");
    orig.setServer("dchub://my.hub");
    orig.setPassword("secret");
    orig.setMode(1);
    orig.setNick("TestNick");

    FavoriteHubEntry copy(orig);

    REQUIRE(copy.getName() == "MyHub");
    REQUIRE(copy.getServer() == "dchub://my.hub");
    REQUIRE(copy.getPassword() == "secret");
    REQUIRE(copy.getMode() == 1);
    REQUIRE(copy.getNick(false) == "TestNick");
}

TEST_CASE("FavoriteHubEntry: getNick with default", "[FavoriteHubEntry]") {
    test::TestContext tc;

    FavoriteHubEntry fhe(*tc.ownedCtx);

    // No nick set — should fall back to SETTING(NICK) when useDefault=true
    std::string defaultNick = fhe.getNick(true);
    // Should return the SettingsManager default nick (which is some value)
    // With useDefault=false, should return empty
    REQUIRE(fhe.getNick(false).empty());

    // Set a nick
    fhe.setNick("CustomNick");
    REQUIRE(fhe.getNick(true) == "CustomNick");
    REQUIRE(fhe.getNick(false) == "CustomNick");
}

TEST_CASE("FavoriteHubEntry: GETSET accessors", "[FavoriteHubEntry]") {
    test::TestContext tc;

    FavoriteHubEntry fhe(*tc.ownedCtx);

    fhe.setUserDescription("My description");
    fhe.setPassword("p@ss");
    fhe.setConnect(true);
    fhe.setMode(2); // passive
    fhe.setGroup("Friends");
    fhe.setSearchInterval(30);
    fhe.setDisableChat(true);
    fhe.setExternalIP("1.2.3.4");

    REQUIRE(fhe.getUserDescription() == "My description");
    REQUIRE(fhe.getPassword() == "p@ss");
    REQUIRE(fhe.getConnect() == true);
    REQUIRE(fhe.getMode() == 2);
    REQUIRE(fhe.getGroup() == "Friends");
    REQUIRE(fhe.getSearchInterval() == 30);
    REQUIRE(fhe.getDisableChat() == true);
    REQUIRE(fhe.getExternalIP() == "1.2.3.4");
}
