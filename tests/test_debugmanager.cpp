/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::DebugManager
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "DebugManager.h"
#include "TestContext.h"

#include <string>
#include <vector>

using namespace dcpp;

// ── Listener for capturing events ───────────────────────────────────────

namespace {

struct TestListener : public DebugManagerListener {
    struct CommandEvent {
        std::string message;
        int typeDir;
        std::string ip;
    };

    std::vector<CommandEvent> commandEvents;
    std::vector<std::string> detectionEvents;

    void on(DebugCommand, const std::string& msg, int typeDir, const std::string& ip) noexcept override {
        commandEvents.push_back({msg, typeDir, ip});
    }

    void on(DebugDetection, const std::string& msg) noexcept override {
        detectionEvents.push_back(msg);
    }
};

} // anonymous namespace

// ── SendCommandMessage ──────────────────────────────────────────────────

TEST_CASE("DebugManager: SendCommandMessage fires listener", "[DebugManager]") {
    test::TestContext tc;

    DebugManager dm(*tc.ownedCtx);
    TestListener listener;
    dm.addListener(&listener);

    dm.SendCommandMessage("test msg", DebugManager::HUB_IN, "192.168.1.1");

    REQUIRE(listener.commandEvents.size() == 1);
    REQUIRE(listener.commandEvents[0].message == "test msg");
    REQUIRE(listener.commandEvents[0].typeDir == DebugManager::HUB_IN);
    REQUIRE(listener.commandEvents[0].ip == "192.168.1.1");

    dm.removeListener(&listener);
}

TEST_CASE("DebugManager: multiple command messages", "[DebugManager]") {
    test::TestContext tc;

    DebugManager dm(*tc.ownedCtx);
    TestListener listener;
    dm.addListener(&listener);

    dm.SendCommandMessage("msg1", DebugManager::HUB_OUT, "10.0.0.1");
    dm.SendCommandMessage("msg2", DebugManager::CLIENT_IN, "10.0.0.2");
    dm.SendCommandMessage("msg3", DebugManager::CLIENT_OUT, "10.0.0.3");

    REQUIRE(listener.commandEvents.size() == 3);
    REQUIRE(listener.commandEvents[0].typeDir == DebugManager::HUB_OUT);
    REQUIRE(listener.commandEvents[1].typeDir == DebugManager::CLIENT_IN);
    REQUIRE(listener.commandEvents[2].typeDir == DebugManager::CLIENT_OUT);

    dm.removeListener(&listener);
}

// ── SendDetectionMessage ────────────────────────────────────────────────

TEST_CASE("DebugManager: SendDetectionMessage fires listener", "[DebugManager]") {
    test::TestContext tc;

    DebugManager dm(*tc.ownedCtx);
    TestListener listener;
    dm.addListener(&listener);

    dm.SendDetectionMessage("Detection: fake client");

    REQUIRE(listener.detectionEvents.size() == 1);
    REQUIRE(listener.detectionEvents[0] == "Detection: fake client");

    dm.removeListener(&listener);
}

// ── Multiple listeners ──────────────────────────────────────────────────

TEST_CASE("DebugManager: multiple listeners receive events", "[DebugManager]") {
    test::TestContext tc;

    DebugManager dm(*tc.ownedCtx);
    TestListener l1, l2;
    dm.addListener(&l1);
    dm.addListener(&l2);

    dm.SendCommandMessage("broadcast", DebugManager::HUB_IN, "127.0.0.1");

    REQUIRE(l1.commandEvents.size() == 1);
    REQUIRE(l2.commandEvents.size() == 1);

    dm.removeListener(&l1);
    dm.removeListener(&l2);
}

// ── removeListener stops events ─────────────────────────────────────────

TEST_CASE("DebugManager: removing listener stops events", "[DebugManager]") {
    test::TestContext tc;

    DebugManager dm(*tc.ownedCtx);
    TestListener listener;
    dm.addListener(&listener);

    dm.SendCommandMessage("first", DebugManager::HUB_IN, "");
    REQUIRE(listener.commandEvents.size() == 1);

    dm.removeListener(&listener);
    dm.SendCommandMessage("second", DebugManager::HUB_IN, "");
    REQUIRE(listener.commandEvents.size() == 1); // still 1
}

// ── removeListeners clears all ──────────────────────────────────────────

TEST_CASE("DebugManager: removeListeners clears all", "[DebugManager]") {
    test::TestContext tc;

    DebugManager dm(*tc.ownedCtx);
    TestListener l1, l2;
    dm.addListener(&l1);
    dm.addListener(&l2);

    dm.removeListeners();

    dm.SendCommandMessage("nobody hears", DebugManager::HUB_IN, "");
    REQUIRE(l1.commandEvents.empty());
    REQUIRE(l2.commandEvents.empty());
}

// ── Type direction enum values ──────────────────────────────────────────

TEST_CASE("DebugManager: enum values are distinct", "[DebugManager]") {
    REQUIRE(DebugManager::HUB_IN != DebugManager::HUB_OUT);
    REQUIRE(DebugManager::HUB_OUT != DebugManager::CLIENT_IN);
    REQUIRE(DebugManager::CLIENT_IN != DebugManager::CLIENT_OUT);
}
