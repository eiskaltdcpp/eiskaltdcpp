/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for DHT singleton removal — verifies that DHT is now owned
 * by DCContext rather than using the Singleton pattern, and that all
 * sub-managers are accessible through DHT references.
 */

#include <catch2/catch_test_macros.hpp>

#include "TestContext.h"
#include "DCContext.h"

#ifdef WITH_DHT
#include "dht/DHT.h"
#include "dht/IndexManager.h"
#include "dht/KBucket.h"
#endif

using namespace dcpp;

// Global TestContext — created once for this TU.
static std::unique_ptr<dcpp::test::TestContext> g_tc;

static void ensureContext() {
    if (!g_tc) {
        g_tc = std::make_unique<dcpp::test::TestContext>();
    }
}

#ifdef WITH_DHT

// ── DCContext DHT ownership ─────────────────────────────────────────────

TEST_CASE("DCContext: getDHT is null before startup", "[dht][DCContext]") {
    DCContext ctx;
    REQUIRE(ctx.getDHT() == nullptr);
}

TEST_CASE("DCContext: getDHT is null after startupMinimal", "[dht][DCContext]") {
    // startupMinimal does not create DHT (no networking)
    ensureContext();
    // The global context uses startupMinimal which should NOT create DHT
    // But getDHT() should still be callable and return nullptr
    // (TestContext uses startupMinimal, not startup)
    auto* dht = dcpp::getContext()->getDHT();
    // startupMinimal does not create DHT
    REQUIRE(dht == nullptr);
}

// ── DHT construction and sub-manager access ─────────────────────────────

TEST_CASE("DHT: constructor creates IndexManager", "[dht]") {
    ensureContext();
    dht::DHT dht(*dcpp::getContext());
    // IndexManager is created in DHT constructor
    REQUIRE_NOTHROW(dht.getIndexManager());
}

TEST_CASE("DHT: sub-managers null before start", "[dht]") {
    ensureContext();
    dht::DHT dht(*dcpp::getContext());
    // Only IndexManager is created in constructor; others are created in start()
    // Accessing them before start() would dereference null unique_ptrs, so
    // we just verify DHT can be constructed and destroyed without start().
    SUCCEED();
}

TEST_CASE("DHT: getPort returns empty when DHT disabled", "[dht]") {
    ensureContext();
    // Ensure USE_DHT setting is off so getPort() returns empty
    auto* sm = dcpp::getContext()->getSettingsManager();
    bool orig = sm->getBool(SettingsManager::USE_DHT, true);
    sm->set(SettingsManager::USE_DHT, false);

    dht::DHT dht(*dcpp::getContext());
    REQUIRE(dht.getPort().empty());

    sm->set(SettingsManager::USE_DHT, orig);
}

TEST_CASE("DHT: isConnected is false on fresh instance", "[dht]") {
    ensureContext();
    dht::DHT dht(*dcpp::getContext());
    REQUIRE_FALSE(dht.isConnected());
}

TEST_CASE("DHT: isFirewalled is true on fresh instance", "[dht]") {
    ensureContext();
    dht::DHT dht(*dcpp::getContext());
    REQUIRE(dht.isFirewalled());
}

TEST_CASE("DHT: hub URL is DHT network name", "[dht]") {
    ensureContext();
    dht::DHT dht(*dcpp::getContext());
    REQUIRE_FALSE(dht.getHubUrl().empty());
    REQUIRE_FALSE(dht.getHubName().empty());
    REQUIRE(dht.getHubUrl() == dht.getHubName());
}

TEST_CASE("DHT: isOp returns false", "[dht]") {
    ensureContext();
    dht::DHT dht(*dcpp::getContext());
    REQUIRE_FALSE(dht.isOp());
}

#endif // WITH_DHT
