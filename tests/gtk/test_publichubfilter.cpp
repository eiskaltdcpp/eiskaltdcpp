/*
 * Tests for PublicHubFilter — Phase 4 complex logic extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "PublicHubFilter.h"

using namespace gtk_pubhub;

namespace {

HubInfo makeHub(const std::string &name, const std::string &desc,
                const std::string &server, int users = 100) {
    HubInfo h;
    h.name = name;
    h.description = desc;
    h.server = server;
    h.users = users;
    return h;
}

std::vector<HubInfo> sampleHubs() {
    return {
        makeHub("Big Hub",    "A large DC++ hub",  "bighub.example.com", 500),
        makeHub("SmallHub",   "Small community",   "small.example.com",  50),
        makeHub("TestDC",     "Testing server",    "test.dc.org",        200),
        makeHub("Private DC", "Invite only",       "private.dc.net",     75),
    };
}

} // anonymous

// ── Empty filter matches all ──

TEST_CASE("PublicHubFilter: empty pattern matches all", "[gtk][pubhub]")
{
    PublicHubFilter filter;
    auto hubs = sampleHubs();
    REQUIRE(filter.countMatching(hubs) == 4);
}

// ── Pattern matching ──

TEST_CASE("PublicHubFilter: filter by name", "[gtk][pubhub]")
{
    PublicHubFilter filter("big");
    auto hubs = sampleHubs();
    auto result = filter.filterCopy(hubs);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].name == "Big Hub");
}

TEST_CASE("PublicHubFilter: filter by description", "[gtk][pubhub]")
{
    PublicHubFilter filter("invite");
    auto hubs = sampleHubs();
    auto result = filter.filterCopy(hubs);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].name == "Private DC");
}

TEST_CASE("PublicHubFilter: filter by server address", "[gtk][pubhub]")
{
    PublicHubFilter filter("dc.org");
    auto hubs = sampleHubs();
    auto result = filter.filterCopy(hubs);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].name == "TestDC");
}

TEST_CASE("PublicHubFilter: case insensitive matching", "[gtk][pubhub]")
{
    PublicHubFilter filter("BIG HUB");
    auto hubs = sampleHubs();
    REQUIRE(filter.countMatching(hubs) == 1);
}

TEST_CASE("PublicHubFilter: pattern matches multiple fields", "[gtk][pubhub]")
{
    // "dc" appears in "TestDC" (name), "DC++" (desc of Big Hub), "Private DC" (name), "dc.org" (server) etc.
    PublicHubFilter filter("dc");
    auto hubs = sampleHubs();
    auto result = filter.filterCopy(hubs);
    // "Big Hub" desc has "DC++", "TestDC" name, "Private DC" name, server "dc.org", "dc.net"
    REQUIRE(result.size() >= 3);
}

TEST_CASE("PublicHubFilter: no matches", "[gtk][pubhub]")
{
    PublicHubFilter filter("nonexistent_xyz");
    auto hubs = sampleHubs();
    REQUIRE(filter.countMatching(hubs) == 0);
}

TEST_CASE("PublicHubFilter: empty hub list", "[gtk][pubhub]")
{
    PublicHubFilter filter("test");
    std::vector<HubInfo> empty;
    REQUIRE(filter.countMatching(empty) == 0);
}

// ── setPattern ──

TEST_CASE("PublicHubFilter: setPattern updates filter", "[gtk][pubhub]")
{
    PublicHubFilter filter("big");
    auto hubs = sampleHubs();
    REQUIRE(filter.countMatching(hubs) == 1);

    filter.setPattern("small");
    REQUIRE(filter.countMatching(hubs) == 1);

    filter.setPattern("");
    REQUIRE(filter.countMatching(hubs) == 4);
}

// ── filter indices ──

TEST_CASE("PublicHubFilter: filter returns correct indices", "[gtk][pubhub]")
{
    PublicHubFilter filter("test");
    auto hubs = sampleHubs();
    auto indices = filter.filter(hubs);
    // "TestDC" is at index 2, "Testing server" matches description
    for (auto idx : indices) {
        REQUIRE(filter.matches(hubs[idx]));
    }
}

// ── stats ──

TEST_CASE("PublicHubFilter: stats computes totals", "[gtk][pubhub]")
{
    PublicHubFilter filter;
    auto hubs = sampleHubs();
    auto s = filter.stats(hubs);
    REQUIRE(s.matchedHubs == 4);
    REQUIRE(s.totalUsers == 500 + 50 + 200 + 75);
}

TEST_CASE("PublicHubFilter: stats with filter", "[gtk][pubhub]")
{
    PublicHubFilter filter("big");
    auto hubs = sampleHubs();
    auto s = filter.stats(hubs);
    REQUIRE(s.matchedHubs == 1);
    REQUIRE(s.totalUsers == 500);
}

// ── matches individual hub ──

TEST_CASE("PublicHubFilter: matches against all 3 fields", "[gtk][pubhub]")
{
    HubInfo hub = makeHub("MyHub", "Great hub for sharing", "myhub.example.com");

    // Match by name
    REQUIRE(PublicHubFilter("myhub").matches(hub));
    // Match by description
    REQUIRE(PublicHubFilter("sharing").matches(hub));
    // Match by server
    REQUIRE(PublicHubFilter("example").matches(hub));
    // No match
    REQUIRE_FALSE(PublicHubFilter("zzzzz").matches(hub));
}

// ── Constructor ──

TEST_CASE("PublicHubFilter: constructor sets pattern", "[gtk][pubhub]")
{
    PublicHubFilter filter("test");
    REQUIRE(filter.pattern() == "test");
}
