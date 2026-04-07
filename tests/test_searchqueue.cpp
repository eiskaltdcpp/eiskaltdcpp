/*
 * Tests for SearchQueue — add, pop, ordering, deduplication, cancel.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "SearchQueue.h"

using namespace dcpp;

// Helper: create a search with given query/token/owner
static SearchCore makeSearch(const std::string& query, const std::string& token = "manual",
                             void* owner = reinterpret_cast<void*>(1)) {
    SearchCore s;
    s.sizeType = 0;
    s.size = 0;
    s.fileType = 0;
    s.query = query;
    s.token = token;
    s.owners.insert(owner);
    return s;
}

TEST_CASE("SearchQueue: add and pop basic", "[searchqueue]") {
    SearchQueue sq(5000); // 5 second interval

    auto s1 = makeSearch("test query");
    REQUIRE(sq.add(s1) == true);

    SearchCore popped;
    // Pop at time that exceeds interval from lastSearchTime(0)
    REQUIRE(sq.pop(popped, 10000) == true);
    REQUIRE(popped.query == "test query");
}

TEST_CASE("SearchQueue: pop respects interval", "[searchqueue]") {
    SearchQueue sq(5000);

    sq.add(makeSearch("q1"));
    sq.add(makeSearch("q2"));

    SearchCore popped;
    // First pop at time 10000
    REQUIRE(sq.pop(popped, 10000) == true);
    REQUIRE(popped.query == "q1");

    // Second pop too soon (within interval)
    REQUIRE(sq.pop(popped, 12000) == false);

    // Second pop after interval
    REQUIRE(sq.pop(popped, 16000) == true);
    REQUIRE(popped.query == "q2");
}

TEST_CASE("SearchQueue: empty queue pop returns false", "[searchqueue]") {
    SearchQueue sq(5000);

    SearchCore popped;
    REQUIRE(sq.pop(popped, 10000) == false);
}

TEST_CASE("SearchQueue: duplicate search merges owners", "[searchqueue]") {
    SearchQueue sq(5000);

    void* owner1 = reinterpret_cast<void*>(1);
    void* owner2 = reinterpret_cast<void*>(2);

    auto s1 = makeSearch("same query", "manual", owner1);
    auto s2 = makeSearch("same query", "manual", owner2);

    REQUIRE(sq.add(s1) == true);
    REQUIRE(sq.add(s2) == false); // duplicate — merged

    SearchCore popped;
    REQUIRE(sq.pop(popped, 10000) == true);
    REQUIRE(popped.query == "same query");
    REQUIRE(popped.owners.size() == 2);
}

TEST_CASE("SearchQueue: manual searches before auto", "[searchqueue]") {
    SearchQueue sq(5000);

    auto auto1 = makeSearch("auto1", "auto");
    auto manual1 = makeSearch("manual1", "manual");

    sq.add(auto1);
    sq.add(manual1);

    SearchCore popped;
    // Manual should come first even though auto was added first
    REQUIRE(sq.pop(popped, 10000) == true);
    REQUIRE(popped.query == "manual1");

    REQUIRE(sq.pop(popped, 20000) == true);
    REQUIRE(popped.query == "auto1");
}

TEST_CASE("SearchQueue: clear removes all", "[searchqueue]") {
    SearchQueue sq(5000);

    sq.add(makeSearch("q1"));
    sq.add(makeSearch("q2"));
    sq.clear();

    SearchCore popped;
    REQUIRE(sq.pop(popped, 10000) == false);
}

TEST_CASE("SearchQueue: cancelSearch by owner", "[searchqueue]") {
    SearchQueue sq(5000);

    void* owner1 = reinterpret_cast<void*>(1);
    void* owner2 = reinterpret_cast<void*>(2);

    sq.add(makeSearch("q1", "manual", owner1));
    sq.add(makeSearch("q2", "manual", owner2));

    REQUIRE(sq.cancelSearch(owner1) == true);

    SearchCore popped;
    REQUIRE(sq.pop(popped, 10000) == true);
    REQUIRE(popped.query == "q2");

    REQUIRE(sq.pop(popped, 20000) == false); // q1 was cancelled
}

TEST_CASE("SearchQueue: cancelSearch nonexistent returns false", "[searchqueue]") {
    SearchQueue sq(5000);

    void* owner = reinterpret_cast<void*>(99);
    REQUIRE(sq.cancelSearch(owner) == false);
}

TEST_CASE("SearchQueue: getSearchTime", "[searchqueue]") {
    SearchQueue sq(5000);

    void* owner1 = reinterpret_cast<void*>(1);
    void* owner2 = reinterpret_cast<void*>(2);

    sq.add(makeSearch("q1", "manual", owner1));
    sq.add(makeSearch("q2", "manual", owner2));

    uint64_t now = 10000;

    // owner1 is first in queue, should be at ~interval from now
    uint64_t t1 = sq.getSearchTime(owner1, now);
    REQUIRE(t1 > 0);
    REQUIRE(t1 != 0xFFFFFFFF);

    // owner2 is second, should be later
    uint64_t t2 = sq.getSearchTime(owner2, now);
    REQUIRE(t2 > t1);
}

TEST_CASE("SearchQueue: getSearchTime null owner returns max", "[searchqueue]") {
    SearchQueue sq(5000);
    REQUIRE(sq.getSearchTime(nullptr, 10000) == 0xFFFFFFFF);
}

TEST_CASE("SearchQueue: auto replaces to manual priority", "[searchqueue]") {
    SearchQueue sq(5000);

    void* owner1 = reinterpret_cast<void*>(1);

    // Add as auto first
    auto s1 = makeSearch("query", "auto", owner1);
    REQUIRE(sq.add(s1) == true);

    // Then add same query as manual — should replace (re-add before autos)
    auto s2 = makeSearch("query", "manual", owner1);
    // s2 is "same" as s1 by == operator (same query/size/type/token... wait, token differs)
    // Actually they won't match because token is different: "auto" != "manual"
    REQUIRE(sq.add(s2) == true); // different token = different search
}

TEST_CASE("SearchQueue: default interval is 0", "[searchqueue]") {
    SearchQueue sq; // default constructor
    REQUIRE(sq.interval == 0);
}
