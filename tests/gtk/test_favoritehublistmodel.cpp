/*
 * Tests for FavoriteHubListModel — Phase 3 stateful model.
 */
#include <catch2/catch_test_macros.hpp>
#include "FavoriteHubListModel.h"

using namespace gtk_favhub;

namespace {

std::map<std::string, std::string> makeHub(const std::string &name,
                                            const std::string &address,
                                            bool autoConnect = false)
{
    std::map<std::string, std::string> params;
    params["Name"] = name;
    params["Address"] = address;
    params["Auto Connect"] = autoConnect ? "1" : "0";
    return params;
}

} // anonymous namespace

// ── addHub ──

TEST_CASE("FavoriteHubListModel: addHub basic", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    REQUIRE(model.addHub(makeHub("Hub1", "dchub://hub1.example.com")));
    REQUIRE(model.count() == 1);
}

TEST_CASE("FavoriteHubListModel: addHub duplicate rejected", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    REQUIRE(model.addHub(makeHub("Hub1", "dchub://hub1.example.com")));
    REQUIRE_FALSE(model.addHub(makeHub("Hub1b", "dchub://hub1.example.com")));
    REQUIRE(model.count() == 1);
}

TEST_CASE("FavoriteHubListModel: addHub auto connect flag", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    model.addHub(makeHub("Hub1", "dchub://hub1.example.com", true));
    auto h = model.findByAddress("dchub://hub1.example.com");
    REQUIRE(h != nullptr);
    REQUIRE(h->autoConnect == true);
}

// ── removeHub ──

TEST_CASE("FavoriteHubListModel: removeHub existing", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    model.addHub(makeHub("Hub1", "dchub://hub1.example.com"));
    REQUIRE(model.removeHub("dchub://hub1.example.com"));
    REQUIRE(model.count() == 0);
}

TEST_CASE("FavoriteHubListModel: removeHub non-existent", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    REQUIRE_FALSE(model.removeHub("dchub://nonexistent.example.com"));
}

// ── updateHub ──

TEST_CASE("FavoriteHubListModel: updateHub modifies params", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    model.addHub(makeHub("Hub1", "dchub://hub1.example.com"));

    auto updated = makeHub("Hub1 Updated", "dchub://hub1.example.com", true);
    REQUIRE(model.updateHub("dchub://hub1.example.com", updated));

    auto h = model.findByAddress("dchub://hub1.example.com");
    REQUIRE(h != nullptr);
    REQUIRE(h->name() == "Hub1 Updated");
    REQUIRE(h->autoConnect == true);
}

TEST_CASE("FavoriteHubListModel: updateHub non-existent returns false", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    REQUIRE_FALSE(model.updateHub("dchub://none.com", makeHub("X", "dchub://none.com")));
}

// ── findByAddress ──

TEST_CASE("FavoriteHubListModel: findByAddress returns nullptr for missing", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    REQUIRE(model.findByAddress("dchub://none.com") == nullptr);
}

// ── allHubs ──

TEST_CASE("FavoriteHubListModel: allHubs preserves insertion order", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    model.addHub(makeHub("Hub1", "dchub://hub1.example.com"));
    model.addHub(makeHub("Hub2", "dchub://hub2.example.com"));
    model.addHub(makeHub("Hub3", "dchub://hub3.example.com"));

    auto all = model.allHubs();
    REQUIRE(all.size() == 3);
    REQUIRE(all[0]->name() == "Hub1");
    REQUIRE(all[1]->name() == "Hub2");
    REQUIRE(all[2]->name() == "Hub3");
}

// ── clear ──

TEST_CASE("FavoriteHubListModel: clear empties model", "[gtk][favhublist]")
{
    FavoriteHubListModel model;
    model.addHub(makeHub("Hub1", "dchub://hub1.example.com"));
    model.addHub(makeHub("Hub2", "dchub://hub2.example.com"));
    model.clear();
    REQUIRE(model.count() == 0);
    REQUIRE(model.allHubs().empty());
}

// ── HubEntry convenience ──

TEST_CASE("FavoriteHubListModel: HubEntry name and address", "[gtk][favhublist]")
{
    HubEntry entry;
    entry.params["Name"] = "TestHub";
    entry.params["Address"] = "adcs://test.hub:411";
    REQUIRE(entry.name() == "TestHub");
    REQUIRE(entry.address() == "adcs://test.hub:411");
}

TEST_CASE("FavoriteHubListModel: HubEntry empty params", "[gtk][favhublist]")
{
    HubEntry entry;
    REQUIRE(entry.name().empty());
    REQUIRE(entry.address().empty());
}
