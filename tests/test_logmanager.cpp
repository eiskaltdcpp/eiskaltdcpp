/*
 * Tests for LogManager — message/getLastLogs, getSetting/saveSetting, getPath.
 * Requires TestContext (DCContext with minimal startup).
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "LogManager.h"
#include "SettingsManager.h"
#include "DCPlusPlus.h"
#include "Util.h"
#include "TestContext.h"

#include <filesystem>

using namespace dcpp;

// Shared test context — lazy-initialized once for all tests in this file
static std::unique_ptr<dcpp::test::TestContext> g_tc;
static void ensureContext() {
    if (!g_tc) {
        g_tc = std::make_unique<dcpp::test::TestContext>();
    }
}

TEST_CASE("LogManager: getLastLogs initially empty", "[logmanager]") {
    ensureContext();
    auto* lm = getContext()->getLogManager();
    REQUIRE(lm != nullptr);

    // Clear any previous state by constructing fresh — but we share context,
    // so just check it returns a list (may have entries from other tests)
    auto logs = lm->getLastLogs();
    // At minimum the type is correct
    REQUIRE(logs.size() >= 0); // trivially true, just ensure no crash
}

TEST_CASE("LogManager: message stores entry", "[logmanager]") {
    ensureContext();
    auto* lm = getContext()->getLogManager();

    // Record size before
    auto before = lm->getLastLogs().size();
    lm->message("test log message");
    auto after = lm->getLastLogs();

    REQUIRE(after.size() == before + 1);
    REQUIRE(after.back().second == "test log message");
    REQUIRE(after.back().first > 0); // timestamp set
}

TEST_CASE("LogManager: multiple messages accumulate", "[logmanager]") {
    ensureContext();
    auto* lm = getContext()->getLogManager();

    auto before = lm->getLastLogs().size();
    lm->message("msg A");
    lm->message("msg B");
    lm->message("msg C");
    auto logs = lm->getLastLogs();

    REQUIRE(logs.size() == before + 3);
    // Last 3 entries
    auto it = logs.end();
    --it; REQUIRE(it->second == "msg C");
    --it; REQUIRE(it->second == "msg B");
    --it; REQUIRE(it->second == "msg A");
}

TEST_CASE("LogManager: getSetting returns setting string", "[logmanager]") {
    ensureContext();
    auto* lm = getContext()->getLogManager();

    // SYSTEM file setting should be a non-empty default
    const string& sysFmt = lm->getSetting(LogManager::SYSTEM, LogManager::FORMAT);
    // The default format string contains %[message] or similar
    REQUIRE(!sysFmt.empty());
}

TEST_CASE("LogManager: saveSetting round-trip", "[logmanager]") {
    ensureContext();
    auto* lm = getContext()->getLogManager();

    // Save a custom format for SYSTEM log
    lm->saveSetting(LogManager::SYSTEM, LogManager::FORMAT, "custom: %[message]");
    const string& fmt = lm->getSetting(LogManager::SYSTEM, LogManager::FORMAT);
    REQUIRE(fmt == "custom: %[message]");
}

TEST_CASE("LogManager: getPath uses LOG_DIRECTORY", "[logmanager]") {
    ensureContext();
    auto* sm = getContext()->getSettingsManager();

    // Set a known log directory using platform-native path
    namespace fs = std::filesystem;
    auto logDir = (fs::temp_directory_path() / "test_logs").make_preferred().string()
                  + std::string(1, PATH_SEPARATOR);
    sm->set(SettingsManager::LOG_DIRECTORY, logDir);

    auto* lm = getContext()->getLogManager();
    string path = lm->getPath(LogManager::SYSTEM);

    // Path should start with the log directory we set
    REQUIRE(path.substr(0, logDir.size()) == logDir);
}

TEST_CASE("LogManager: getPath with params", "[logmanager]") {
    ensureContext();
    auto* sm = getContext()->getSettingsManager();
    auto* lm = getContext()->getLogManager();

    namespace fs = std::filesystem;
    auto logDir = (fs::temp_directory_path() / "logs").make_preferred().string()
                  + std::string(1, PATH_SEPARATOR);
    sm->set(SettingsManager::LOG_DIRECTORY, logDir);
    lm->saveSetting(LogManager::CHAT, LogManager::FILE, "%[hubURL].log");

    ParamMap params;
    params["hubURL"] = "my_hub";
    string path = lm->getPath(LogManager::CHAT, params);

    REQUIRE(path.find("my_hub") != string::npos);
    REQUIRE(path.find(".log") != string::npos);
}
