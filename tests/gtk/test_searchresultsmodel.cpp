/*
 * Tests for SearchResultsModel — Phase 3 stateful model.
 */
#include <catch2/catch_test_macros.hpp>
#include "SearchResultsModel.h"

using namespace gtk_search;

namespace {

std::map<std::string, std::string> makeResult(const std::string &cid,
                                                const std::string &filename,
                                                const std::string &tth = "",
                                                int64_t size = 1000,
                                                int freeSlots = 3)
{
    std::map<std::string, std::string> params;
    params["CID"] = cid;
    params["Filename"] = filename;
    params["Path"] = "/share/";
    params["Nick"] = "User_" + cid;
    params["Hub"] = "TestHub";
    params["Type"] = "mp3";
    params["Real Size"] = std::to_string(size);
    params["Free Slots"] = std::to_string(freeSlots);
    params["TTH"] = tth.empty() ? ("TTH" + filename) : tth;
    params["Connection"] = "Cable";
    params["Shared"] = "0";
    return params;
}

} // anonymous namespace

// ── addResult / dedup ──

TEST_CASE("SearchResultsModel: add result basic", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    REQUIRE(model.addResult(makeResult("CID1", "song.mp3")));
    REQUIRE(model.resultCount() == 1);
}

TEST_CASE("SearchResultsModel: duplicate rejected same CID same file", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "song.mp3"));
    REQUIRE_FALSE(model.addResult(makeResult("CID1", "song.mp3")));
    REQUIRE(model.resultCount() == 1);
    REQUIRE(model.droppedCount() == 1);
}

TEST_CASE("SearchResultsModel: same CID different file accepted", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "song1.mp3"));
    REQUIRE(model.addResult(makeResult("CID1", "song2.mp3")));
    REQUIRE(model.resultCount() == 2);
}

TEST_CASE("SearchResultsModel: different CID same file accepted", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "song.mp3"));
    REQUIRE(model.addResult(makeResult("CID2", "song.mp3")));
    REQUIRE(model.resultCount() == 2);
}

// ── clear ──

TEST_CASE("SearchResultsModel: clear resets everything", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "song.mp3"));
    model.addResult(makeResult("CID1", "song.mp3")); // duplicate
    model.clear();
    REQUIRE(model.resultCount() == 0);
    REQUIRE(model.droppedCount() == 0);
}

// ── allResults ──

TEST_CASE("SearchResultsModel: allResults returns all", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "a.mp3"));
    model.addResult(makeResult("CID2", "b.mp3"));
    REQUIRE(model.allResults().size() == 2);
}

// ── grouping ──

TEST_CASE("SearchResultsModel: groupBy NONE returns single group", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "a.mp3"));
    model.addResult(makeResult("CID2", "b.mp3"));
    model.setGroupBy(GroupType::NONE);

    auto groups = model.groupedResults();
    REQUIRE(groups.size() == 1);
    REQUIRE(groups[0].entries.size() == 2);
}

TEST_CASE("SearchResultsModel: groupBy NICK groups by user", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "a.mp3"));
    model.addResult(makeResult("CID1", "b.mp3"));
    model.addResult(makeResult("CID2", "c.mp3"));
    model.setGroupBy(GroupType::NICK);

    auto groups = model.groupedResults();
    REQUIRE(groups.size() == 2);
}

TEST_CASE("SearchResultsModel: groupBy TYPE groups by extension", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    auto r1 = makeResult("CID1", "a.mp3");
    auto r2 = makeResult("CID2", "b.flac");
    r2["Type"] = "flac";
    model.addResult(r1);
    model.addResult(r2);
    model.setGroupBy(GroupType::TYPE);

    auto groups = model.groupedResults();
    REQUIRE(groups.size() == 2);
}

TEST_CASE("SearchResultsModel: groupBy TTH groups by hash", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "a.mp3", "TTH_SAME"));
    model.addResult(makeResult("CID2", "b.mp3", "TTH_SAME"));
    model.addResult(makeResult("CID3", "c.mp3", "TTH_OTHER"));
    model.setGroupBy(GroupType::TTH);

    auto groups = model.groupedResults();
    REQUIRE(groups.size() == 2);
    // The group with matching TTH should have 2 entries
    bool foundPair = false;
    for (const auto &g : groups) {
        if (g.entries.size() == 2)
            foundPair = true;
    }
    REQUIRE(foundPair);
}

// ── filtering ──

TEST_CASE("SearchResultsModel: filter by free slots", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "a.mp3", "", 1000, 3));
    model.addResult(makeResult("CID2", "b.mp3", "", 1000, 0));

    SearchFilter f;
    f.onlyFreeSlots = true;
    model.setFilter(f);

    auto filtered = model.filteredResults();
    REQUIRE(filtered.size() == 1);
}

TEST_CASE("SearchResultsModel: filter by size range", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "small.mp3", "", 500));
    model.addResult(makeResult("CID2", "medium.mp3", "", 1500));
    model.addResult(makeResult("CID3", "large.mp3", "", 5000));

    SearchFilter f;
    f.minSize = 1000;
    f.maxSize = 2000;
    model.setFilter(f);

    auto filtered = model.filteredResults();
    REQUIRE(filtered.size() == 1);
}

TEST_CASE("SearchResultsModel: filter by text positive match", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "favorite_song.mp3"));
    model.addResult(makeResult("CID2", "random_noise.mp3"));

    SearchFilter f;
    f.text = "favorite";
    model.setFilter(f);

    auto filtered = model.filteredResults();
    REQUIRE(filtered.size() == 1);
}

TEST_CASE("SearchResultsModel: filter by text negative match", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "good_song.mp3"));
    model.addResult(makeResult("CID2", "bad_song.mp3"));

    SearchFilter f;
    f.text = "song -bad";
    model.setFilter(f);

    auto filtered = model.filteredResults();
    REQUIRE(filtered.size() == 1);
}

TEST_CASE("SearchResultsModel: filter hides shared", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    auto r1 = makeResult("CID1", "a.mp3");
    r1["Shared"] = "1";
    auto r2 = makeResult("CID2", "b.mp3");
    r2["Shared"] = "0";
    model.addResult(r1);
    model.addResult(r2);

    SearchFilter f;
    f.hideShared = true;
    model.setFilter(f);

    auto filtered = model.filteredResults();
    REQUIRE(filtered.size() == 1);
}

TEST_CASE("SearchResultsModel: empty filter passes all", "[gtk][searchmodel]")
{
    SearchResultsModel model;
    model.addResult(makeResult("CID1", "a.mp3"));
    model.addResult(makeResult("CID2", "b.mp3"));

    model.setFilter(SearchFilter{});
    REQUIRE(model.filteredResults().size() == 2);
}
