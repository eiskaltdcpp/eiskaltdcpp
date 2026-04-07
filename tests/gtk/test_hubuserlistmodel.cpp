/*
 * Tests for HubUserListModel — Phase 3 stateful model.
 */
#include <catch2/catch_test_macros.hpp>
#include "HubUserListModel.h"

using namespace gtk_hub;

namespace {

std::map<std::string, std::string> makeUser(const std::string &cid,
                                             const std::string &nick,
                                             int64_t shared = 0)
{
    std::map<std::string, std::string> params;
    params["CID"] = cid;
    params["Nick"] = nick;
    params["Shared"] = std::to_string(shared);
    params["Description"] = "test user";
    params["Icon"] = "dc++";
    params["Nick Order"] = "U" + nick;
    return params;
}

} // anonymous namespace

// ── addOrUpdateUser ──

TEST_CASE("HubUserListModel: add new user returns true", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    REQUIRE(model.addOrUpdateUser(makeUser("CID1", "Alice", 1000)));
    REQUIRE(model.userCount() == 1);
}

TEST_CASE("HubUserListModel: update existing user returns false", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice", 1000));
    REQUIRE_FALSE(model.addOrUpdateUser(makeUser("CID1", "Alice", 2000)));
    REQUIRE(model.userCount() == 1);
}

TEST_CASE("HubUserListModel: update updates shared and totalShared", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice", 1000));
    REQUIRE(model.totalShared() == 1000);

    model.addOrUpdateUser(makeUser("CID1", "Alice", 3000));
    REQUIRE(model.totalShared() == 3000);
    auto u = model.findByCid("CID1");
    REQUIRE(u != nullptr);
    REQUIRE(u->shared == 3000);
}

TEST_CASE("HubUserListModel: nick change updates nickToCid", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice"));
    REQUIRE(model.findByNick("Alice") != nullptr);

    // Change nick
    model.addOrUpdateUser(makeUser("CID1", "AliceRenamed"));
    REQUIRE(model.findByNick("AliceRenamed") != nullptr);
    REQUIRE(model.findByNick("Alice") == nullptr);
}

TEST_CASE("HubUserListModel: add ignored without CID", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    std::map<std::string, std::string> params;
    params["Nick"] = "Ghost";
    REQUIRE_FALSE(model.addOrUpdateUser(params));
    REQUIRE(model.userCount() == 0);
}

// ── removeUser ──

TEST_CASE("HubUserListModel: removeUser existing", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice", 5000));
    REQUIRE(model.removeUser("CID1"));
    REQUIRE(model.userCount() == 0);
    REQUIRE(model.totalShared() == 0);
    REQUIRE(model.findByNick("Alice") == nullptr);
}

TEST_CASE("HubUserListModel: removeUser non-existent", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    REQUIRE_FALSE(model.removeUser("NOPE"));
}

// ── findByCid / findByNick ──

TEST_CASE("HubUserListModel: findByCid and findByNick", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice", 1000));

    auto byCid = model.findByCid("CID1");
    auto byNick = model.findByNick("Alice");
    REQUIRE(byCid != nullptr);
    REQUIRE(byNick != nullptr);
    REQUIRE(byCid == byNick);
    REQUIRE(byCid->nick == "Alice");
    REQUIRE(byCid->description == "test user");
}

// ── allUsers / totalShared ──

TEST_CASE("HubUserListModel: allUsers and totalShared", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice", 1000));
    model.addOrUpdateUser(makeUser("CID2", "Bob", 2000));
    model.addOrUpdateUser(makeUser("CID3", "Carol", 3000));

    REQUIRE(model.allUsers().size() == 3);
    REQUIRE(model.totalShared() == 6000);
}

// ── clearUsers ──

TEST_CASE("HubUserListModel: clearUsers resets everything", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice", 1000));
    model.addOrUpdateUser(makeUser("CID2", "Bob", 2000));
    model.clearUsers();
    REQUIRE(model.userCount() == 0);
    REQUIRE(model.totalShared() == 0);
    REQUIRE(model.findByNick("Alice") == nullptr);
}

// ── Favorites ──

TEST_CASE("HubUserListModel: favorite tracking", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addOrUpdateUser(makeUser("CID1", "Alice"));

    REQUIRE_FALSE(model.isFavorite("CID1"));
    REQUIRE(model.setFavorite("CID1", true));
    REQUIRE(model.isFavorite("CID1"));

    auto favs = model.favoriteCids();
    REQUIRE(favs.size() == 1);
    REQUIRE(favs[0] == "CID1");

    REQUIRE(model.setFavorite("CID1", false));
    REQUIRE_FALSE(model.isFavorite("CID1"));
    REQUIRE(model.favoriteCids().empty());
}

TEST_CASE("HubUserListModel: setFavorite on non-existent user", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    REQUIRE_FALSE(model.setFavorite("NOPE", true));
}

// ── Chat history ──

TEST_CASE("HubUserListModel: chat history add and navigate", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addHistoryEntry("hello");
    model.addHistoryEntry("world");
    REQUIRE(model.historySize() == 2);

    // Navigate backward
    std::string s = model.historyPrev();
    REQUIRE(s == "world");
    s = model.historyPrev();
    REQUIRE(s == "hello");

    // Navigate forward
    s = model.historyNext();
    REQUIRE(s == "world");
}

TEST_CASE("HubUserListModel: chat history max limit", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    for (int i = 0; i < 30; ++i)
        model.addHistoryEntry("msg" + std::to_string(i));

    REQUIRE(model.historySize() == HubUserListModel::MAX_HISTORY);
}

TEST_CASE("HubUserListModel: chat history empty string ignored", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addHistoryEntry("");
    REQUIRE(model.historySize() == 0);
}

TEST_CASE("HubUserListModel: history boundary navigation", "[gtk][hubuserlist]")
{
    HubUserListModel model;
    model.addHistoryEntry("only");

    // Going further back than available stays at first
    model.historyPrev();
    model.historyPrev();
    model.historyPrev();
    REQUIRE(model.historyPrev() == "only");

    // Reset and verify
    model.historyReset();
    REQUIRE(model.historyPrev() == "only");
}
