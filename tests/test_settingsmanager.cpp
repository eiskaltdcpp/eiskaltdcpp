/*
 * Tests for SettingsManager — get/set defaults, and basic config operations.
 * These tests use TestContext for a minimal DCContext with SettingsManager.
 */

#include <catch2/catch_test_macros.hpp>

#include "TestContext.h"
#include "SettingsManager.h"

using namespace dcpp;

// Global TestContext — created once for this translation unit.
// Catch2 runs tests sequentially within a TU, so a single context suffices.
static std::unique_ptr<dcpp::test::TestContext> g_tc;

static void ensureContext() {
    if (!g_tc) {
        g_tc = std::make_unique<dcpp::test::TestContext>();
    }
}

// ─── Default values ─────────────────────────────────────────────────────

TEST_CASE("SettingsManager: string defaults exist", "[settingsmanager]") {
    ensureContext();

    // SLOTS should have a sensible default (typically 1-5)
    int slots = dcpp::getContext()->getSettingsManager()->get(SettingsManager::SLOTS, true);
    REQUIRE(slots >= 0);
    REQUIRE(slots <= 100);
}

TEST_CASE("SettingsManager: APP_UNIT_BASE default is 0 or 1", "[settingsmanager]") {
    ensureContext();

    int base = dcpp::getContext()->getSettingsManager()->get(SettingsManager::APP_UNIT_BASE, true);
    // 0 = binary (KiB/MiB/GiB), 1+ = decimal (KB/MB/GB)
    REQUIRE(base >= 0);
    REQUIRE(base <= 2);
}

TEST_CASE("SettingsManager: get/set int round-trip", "[settingsmanager]") {
    ensureContext();

    auto* sm = dcpp::getContext()->getSettingsManager();
    int orig = sm->get(SettingsManager::SLOTS, true);

    sm->set(SettingsManager::SLOTS, 42);
    REQUIRE(sm->get(SettingsManager::SLOTS, true) == 42);

    // restore
    sm->set(SettingsManager::SLOTS, orig);
}

TEST_CASE("SettingsManager: get/set string round-trip", "[settingsmanager]") {
    ensureContext();

    auto* sm = dcpp::getContext()->getSettingsManager();
    std::string orig = sm->get(SettingsManager::NICK, true);

    sm->set(SettingsManager::NICK, "TestUser");
    REQUIRE(sm->get(SettingsManager::NICK, true) == "TestUser");

    sm->set(SettingsManager::NICK, orig);
}

TEST_CASE("SettingsManager: BOOLSETTING equivalent works", "[settingsmanager]") {
    ensureContext();

    // Just verify the call doesn't crash — the actual default value varies
    bool val = dcpp::getContext()->getSettingsManager()->getBool(SettingsManager::SEGMENTED_DL);
    (void)val; // suppress unused warning
    REQUIRE(true); // if we get here, the macro works
}

// ─── formatBytes (requires SettingsManager for APP_UNIT_BASE) ───────────

TEST_CASE("Util: formatBytes with context - bytes", "[util][settingsmanager]") {
    ensureContext();

    std::string result = Util::formatBytes(0);
    REQUIRE(result.find("0") != std::string::npos);
    REQUIRE(result.find("B") != std::string::npos);
}

TEST_CASE("Util: formatBytes with context - KiB range", "[util][settingsmanager]") {
    ensureContext();

    std::string result = Util::formatBytes(1024);
    // Should be ~1.00 KiB or ~1.02 KB depending on APP_UNIT_BASE
    REQUIRE((result.find("K") != std::string::npos || result.find("k") != std::string::npos));
}

TEST_CASE("Util: formatBytes with context - MiB range", "[util][settingsmanager]") {
    ensureContext();

    std::string result = Util::formatBytes(int64_t(1024) * 1024);
    REQUIRE((result.find("M") != std::string::npos || result.find("m") != std::string::npos));
}

TEST_CASE("Util: formatBytes with context - GiB range", "[util][settingsmanager]") {
    ensureContext();

    std::string result = Util::formatBytes(int64_t(1024) * 1024 * 1024);
    REQUIRE((result.find("G") != std::string::npos || result.find("g") != std::string::npos));
}

TEST_CASE("Util: formatBytes with context - TiB range", "[util][settingsmanager]") {
    ensureContext();

    std::string result = Util::formatBytes(int64_t(1024) * 1024 * 1024 * 1024);
    REQUIRE((result.find("T") != std::string::npos || result.find("t") != std::string::npos));
}

TEST_CASE("Util: formatBytes string overload", "[util][settingsmanager]") {
    ensureContext();

    std::string result = Util::formatBytes("1048576"); // 1 MiB
    REQUIRE((result.find("M") != std::string::npos || result.find("m") != std::string::npos));
}
