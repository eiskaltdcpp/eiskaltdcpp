/*
 * Tests for FinishedItemBase, FinishedFileItem, FinishedUserItem —
 * pure data classes, no DCContext required.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "stdinc.h"
#include "FinishedItem.h"
#include "HintedUser.h"
#include "User.h"
#include "CID.h"
#include "TigerHash.h"
#include "Pointer.h"

using namespace dcpp;

// Helper: make a dummy HintedUser with a unique CID
static HintedUser makeHintedUser(const std::string& nick) {
    TigerHash th;
    th.update(nick.data(), nick.size());
    th.finalize();
    CID cid(th.getResult());
    UserPtr user(new User(cid));
    return HintedUser(user, "adc://hub.example.com");
}

// ============ FinishedItemBase (via FinishedFileItem) ============

TEST_CASE("FinishedItemBase: construction", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    FinishedFileItem item(1000, 500, 12345, 2000, 1100, false, u);

    REQUIRE(item.getTransferred() == 1000);
    REQUIRE(item.getMilliSeconds() == 500);
    REQUIRE(item.getTime() == 12345);
}

TEST_CASE("FinishedItemBase: update accumulates", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    FinishedFileItem item(1000, 500, 100, 5000, 1100, false, u);

    // Add 500 bytes over 250ms at time 200
    item.FinishedItemBase::update(500, 250, 200);

    REQUIRE(item.getTransferred() == 1500);   // 1000 + 500
    REQUIRE(item.getMilliSeconds() == 750);    // 500 + 250
    REQUIRE(item.getTime() == 200);            // updated to latest
}

TEST_CASE("FinishedItemBase: getAverageSpeed", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    // 10000 bytes in 2000ms = 5000 bytes/sec
    FinishedFileItem item(10000, 2000, 100, 10000, 10000, false, u);

    REQUIRE(item.getAverageSpeed() == 5000);
}

TEST_CASE("FinishedItemBase: getAverageSpeed zero time", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    FinishedFileItem item(10000, 0, 100, 10000, 10000, false, u);

    REQUIRE(item.getAverageSpeed() == 0);
}

// ============ FinishedFileItem ============

TEST_CASE("FinishedFileItem: construction", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    FinishedFileItem item(1000, 500, 100, 5000, 1100, true, u);

    REQUIRE(item.getFileSize() == 5000);
    REQUIRE(item.getActual() == 1100);
    REQUIRE(item.getCrc32Checked() == true);
    REQUIRE(item.getUsers().size() == 1);
}

TEST_CASE("FinishedFileItem: update adds new user", "[finisheditem]") {
    auto u1 = makeHintedUser("user1");
    auto u2 = makeHintedUser("user2");
    FinishedFileItem item(1000, 500, 100, 5000, 1100, false, u1);

    item.update(500, 250, 200, 600, false, u2);

    REQUIRE(item.getUsers().size() == 2);
    REQUIRE(item.getTransferred() == 1500);
    REQUIRE(item.getActual() == 1700);   // 1100 + 600
}

TEST_CASE("FinishedFileItem: update same user updates hint", "[finisheditem]") {
    auto u1 = makeHintedUser("user1");
    FinishedFileItem item(1000, 500, 100, 5000, 1100, false, u1);

    // Same user, different hint
    HintedUser u1b(u1.user, "adc://other-hub.com");
    item.update(500, 250, 200, 600, false, u1b);

    REQUIRE(item.getUsers().size() == 1);  // not duplicated
    REQUIRE(item.getUsers()[0].hint == "adc://other-hub.com"); // hint updated
}

TEST_CASE("FinishedFileItem: crc32Checked sticks true", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    FinishedFileItem item(1000, 500, 100, 5000, 1100, false, u);
    REQUIRE(item.getCrc32Checked() == false);

    item.update(500, 250, 200, 600, true, u);
    REQUIRE(item.getCrc32Checked() == true);

    // Once true, stays true even if next update passes false
    item.update(500, 250, 300, 600, false, u);
    REQUIRE(item.getCrc32Checked() == true);
}

TEST_CASE("FinishedFileItem: getTransferredPercentage", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    // 2500 of 5000 = 50%
    FinishedFileItem item(2500, 500, 100, 5000, 2600, false, u);

    REQUIRE(item.getTransferredPercentage() == Catch::Approx(50.0));
}

TEST_CASE("FinishedFileItem: getTransferredPercentage zero fileSize", "[finisheditem]") {
    auto u = makeHintedUser("user1");
    FinishedFileItem item(1000, 500, 100, 0, 1100, false, u);

    REQUIRE(item.getTransferredPercentage() == Catch::Approx(0.0));
}

TEST_CASE("FinishedFileItem: isFull", "[finisheditem]") {
    auto u = makeHintedUser("user1");

    SECTION("not full") {
        FinishedFileItem item(2500, 500, 100, 5000, 2600, false, u);
        REQUIRE(item.isFull() == false);
    }

    SECTION("exactly full") {
        FinishedFileItem item(5000, 500, 100, 5000, 5100, false, u);
        REQUIRE(item.isFull() == true);
    }

    SECTION("over-transferred") {
        FinishedFileItem item(6000, 500, 100, 5000, 6100, false, u);
        REQUIRE(item.isFull() == true);
    }
}

// ============ FinishedUserItem ============

TEST_CASE("FinishedUserItem: construction", "[finisheditem]") {
    FinishedUserItem item(1000, 500, 100, "file1.txt");

    REQUIRE(item.getTransferred() == 1000);
    REQUIRE(item.getMilliSeconds() == 500);
    REQUIRE(item.getTime() == 100);
    REQUIRE(item.getFiles().size() == 1);
    REQUIRE(item.getFiles()[0] == "file1.txt");
}

TEST_CASE("FinishedUserItem: update adds new file", "[finisheditem]") {
    FinishedUserItem item(1000, 500, 100, "file1.txt");
    item.update(2000, 300, 200, "file2.txt");

    REQUIRE(item.getFiles().size() == 2);
    REQUIRE(item.getTransferred() == 3000);
    REQUIRE(item.getMilliSeconds() == 800);
    REQUIRE(item.getTime() == 200);
}

TEST_CASE("FinishedUserItem: update deduplicates files", "[finisheditem]") {
    FinishedUserItem item(1000, 500, 100, "file1.txt");
    item.update(2000, 300, 200, "file1.txt"); // same file

    REQUIRE(item.getFiles().size() == 1); // not duplicated
    REQUIRE(item.getTransferred() == 3000); // but bytes still accumulated
}

TEST_CASE("FinishedUserItem: averageSpeed", "[finisheditem]") {
    // 8000 bytes in 4000ms = 2000 B/s
    FinishedUserItem item(8000, 4000, 100, "file.txt");
    REQUIRE(item.getAverageSpeed() == 2000);
}
