/*
 * Tests for Segment and QueueItem — segment arithmetic, consolidation,
 * downloaded bytes tracking, and basic QueueItem operations.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "stdinc.h"
#include "Segment.h"
#include "QueueItem.h"
#include "TigerHash.h"
#include "MerkleTree.h"
#include "Util.h"
#include "TestContext.h"

using namespace dcpp;

// Helper: create a TTHValue from a string
static TTHValue makeTTH(const std::string& s) {
    TigerHash h;
    h.update(s.data(), s.size());
    h.finalize();
    return TTHValue(h.getResult());
}

// ===== Segment tests =====

TEST_CASE("Segment: default construction", "[segment]") {
    Segment s;
    REQUIRE(s.getStart() == 0);
    REQUIRE(s.getSize() == -1);
    REQUIRE(s.getEnd() == -1);
}

TEST_CASE("Segment: parameterized construction", "[segment]") {
    Segment s(100, 200);
    REQUIRE(s.getStart() == 100);
    REQUIRE(s.getSize() == 200);
    REQUIRE(s.getEnd() == 300);
}

TEST_CASE("Segment: overlaps detection", "[segment]") {
    Segment a(0, 100);    // [0, 100)
    Segment b(50, 100);   // [50, 150)
    Segment c(100, 100);  // [100, 200) — adjacent, not overlapping
    Segment d(200, 100);  // [200, 300) — disjoint

    REQUIRE(a.overlaps(b) == true);
    REQUIRE(b.overlaps(a) == true);
    REQUIRE(a.overlaps(c) == false);  // touching but not overlapping
    REQUIRE(a.overlaps(d) == false);
}

TEST_CASE("Segment: contains", "[segment]") {
    Segment big(0, 200);  // [0, 200)
    Segment sub(0, 200);  // same
    Segment partial(50, 150); // [50, 200) — same end

    REQUIRE(big.contains(sub) == true);
    REQUIRE(big.contains(partial) == true);

    Segment shorter(0, 100); // [0, 100) — different end
    REQUIRE(big.contains(shorter) == false); // contains requires same end
}

TEST_CASE("Segment: trim - overlapping left", "[segment]") {
    Segment a(50, 150);   // [50, 200)
    Segment b(0, 100);    // [0, 100) — overlaps from left

    a.trim(b);
    // After trimming [0,100) from [50,200), we get [100,200)
    REQUIRE(a.getStart() == 100);
    REQUIRE(a.getEnd() == 200);
}

TEST_CASE("Segment: trim - overlapping right", "[segment]") {
    Segment a(0, 200);    // [0, 200)
    Segment b(100, 200);  // [100, 300) — overlaps from right

    a.trim(b);
    // After trimming [100,300) from [0,200), size becomes 100 - 0 = 100
    REQUIRE(a.getStart() == 0);
    REQUIRE(a.getSize() == 100);
    REQUIRE(a.getEnd() == 100);
}

TEST_CASE("Segment: trim - no overlap does nothing", "[segment]") {
    Segment a(0, 100);
    Segment b(200, 100);

    a.trim(b);
    REQUIRE(a.getStart() == 0);
    REQUIRE(a.getSize() == 100);
}

TEST_CASE("Segment: trim - complete overlap zeroes", "[segment]") {
    Segment a(50, 100);   // [50, 150)
    Segment b(0, 200);    // [0, 200) — completely contains a

    a.trim(b);
    REQUIRE(a.getStart() == 0);
    REQUIRE(a.getSize() == 0);
}

TEST_CASE("Segment: equality and ordering", "[segment]") {
    Segment a(0, 100);
    Segment b(0, 100);
    Segment c(0, 200);
    Segment d(50, 100);

    REQUIRE(a == b);
    REQUIRE(!(a == c)); // same start, different size
    REQUIRE(a < c);     // same start, smaller size
    REQUIRE(a < d);     // smaller start
}

// ===== QueueItem tests =====

TEST_CASE("QueueItem: construction and basic getters", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("test-file");
    std::string target = std::string(1, PATH_SEPARATOR) + "downloads" +
                         std::string(1, PATH_SEPARATOR) + "test.txt";
    QueueItem qi(ctx, target, 1024, QueueItem::NORMAL, QueueItem::FLAG_NORMAL,
                 time(nullptr), tth);

    REQUIRE(qi.getTarget() == target);
    REQUIRE(qi.getSize() == 1024);
    REQUIRE(qi.getPriority() == QueueItem::NORMAL);
    REQUIRE(qi.getTTH() == tth);
    REQUIRE(qi.getTargetFileName() == "test.txt");
}

TEST_CASE("QueueItem: priority levels", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("p");
    QueueItem qi(ctx, "/f", 100, QueueItem::PAUSED, 0, 0, tth);
    REQUIRE(qi.getPriority() == QueueItem::PAUSED);

    qi.setPriority(QueueItem::HIGHEST);
    REQUIRE(qi.getPriority() == QueueItem::HIGHEST);

    // Check all priority values are ordered
    REQUIRE(QueueItem::PAUSED < QueueItem::LOWEST);
    REQUIRE(QueueItem::LOWEST < QueueItem::LOW);
    REQUIRE(QueueItem::LOW < QueueItem::NORMAL);
    REQUIRE(QueueItem::NORMAL < QueueItem::HIGH);
    REQUIRE(QueueItem::HIGH < QueueItem::HIGHEST);
}

TEST_CASE("QueueItem: flags", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("f");
    QueueItem qi(ctx, "/f", 100, QueueItem::NORMAL,
                 QueueItem::FLAG_USER_LIST | QueueItem::FLAG_XML_BZLIST, 0, tth);

    REQUIRE(qi.isSet(QueueItem::FLAG_USER_LIST));
    REQUIRE(qi.isSet(QueueItem::FLAG_XML_BZLIST));
    REQUIRE(!qi.isSet(QueueItem::FLAG_TEXT));
}

TEST_CASE("QueueItem: getTargetFileName extracts filename", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("fn");

    SECTION("with path separator") {
        std::string target = std::string(1, PATH_SEPARATOR) + "path" +
                             std::string(1, PATH_SEPARATOR) + "to" +
                             std::string(1, PATH_SEPARATOR) + "movie.avi";
        QueueItem qi(ctx, target, 0, QueueItem::NORMAL, 0, 0, tth);
        REQUIRE(qi.getTargetFileName() == "movie.avi");
    }
    SECTION("no path separator") {
        QueueItem qi(ctx, "just_a_file.txt", 0, QueueItem::NORMAL, 0, 0, tth);
        REQUIRE(qi.getTargetFileName() == "just_a_file.txt");
    }
}

TEST_CASE("QueueItem: addSegment and getDownloadedBytes", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("seg");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    REQUIRE(qi.getDownloadedBytes() == 0);

    qi.addSegment(Segment(0, 100));
    REQUIRE(qi.getDownloadedBytes() == 100);

    qi.addSegment(Segment(200, 100));
    REQUIRE(qi.getDownloadedBytes() == 200); // 100 + 100

    qi.addSegment(Segment(500, 200));
    REQUIRE(qi.getDownloadedBytes() == 400); // 100 + 100 + 200
}

TEST_CASE("QueueItem: addSegment consolidates overlapping", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("con");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(0, 100));    // [0, 100)
    qi.addSegment(Segment(50, 100));   // [50, 150) — overlaps with first
    // Should consolidate to [0, 150)
    REQUIRE(qi.getDownloadedBytes() == 150);
    REQUIRE(qi.getDone().size() == 1);
}

TEST_CASE("QueueItem: addSegment consolidates adjacent", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("adj");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(0, 100));    // [0, 100)
    qi.addSegment(Segment(100, 100));  // [100, 200) — adjacent
    // Should consolidate to [0, 200)
    REQUIRE(qi.getDownloadedBytes() == 200);
    REQUIRE(qi.getDone().size() == 1);
}

TEST_CASE("QueueItem: isFinished", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("fin");
    QueueItem qi(ctx, "/f", 500, QueueItem::NORMAL, 0, 0, tth);

    REQUIRE(qi.isFinished() == false);

    qi.addSegment(Segment(0, 500));
    REQUIRE(qi.isFinished() == true);
}

TEST_CASE("QueueItem: isFinished after consolidation", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("finc");
    QueueItem qi(ctx, "/f", 300, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(0, 100));
    qi.addSegment(Segment(100, 100));
    qi.addSegment(Segment(200, 100));
    // Should consolidate to [0, 300)
    REQUIRE(qi.isFinished() == true);
}

TEST_CASE("QueueItem: getDownloadedFraction", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("frac");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(0, 500));
    REQUIRE(qi.getDownloadedFraction() == Catch::Approx(0.5));

    qi.addSegment(Segment(500, 500));
    REQUIRE(qi.getDownloadedFraction() == Catch::Approx(1.0));
}

TEST_CASE("QueueItem: isChunkDownloaded", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("chunk");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(100, 200)); // [100, 300)

    int64_t len;

    len = 50;
    REQUIRE(qi.isChunkDownloaded(150, len) == true);   // entirely within [100,300)
    REQUIRE(len == 50);  // clipped to min(50, 300-150)

    len = 200;
    REQUIRE(qi.isChunkDownloaded(200, len) == true);
    REQUIRE(len == 100);  // clipped to 300-200

    len = 100;
    REQUIRE(qi.isChunkDownloaded(0, len) == false);    // before the segment

    len = 100;
    REQUIRE(qi.isChunkDownloaded(300, len) == false);   // after the segment
}

TEST_CASE("QueueItem: isChunkDownloaded with zero/negative len", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("z");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);
    qi.addSegment(Segment(0, 500));

    int64_t len = 0;
    REQUIRE(qi.isChunkDownloaded(0, len) == false);

    len = -1;
    REQUIRE(qi.isChunkDownloaded(0, len) == false);
}

TEST_CASE("QueueItem: resetDownloaded", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("reset");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(0, 500));
    REQUIRE(qi.getDownloadedBytes() == 500);

    qi.resetDownloaded();
    REQUIRE(qi.getDownloadedBytes() == 0);
    REQUIRE(qi.isFinished() == false);
}

TEST_CASE("QueueItem: copy constructor", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("copy");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, QueueItem::FLAG_TEXT, 12345, tth);
    qi.addSegment(Segment(0, 300));

    QueueItem copy(qi);
    REQUIRE(copy.getTarget() == "/f");
    REQUIRE(copy.getSize() == 1000);
    REQUIRE(copy.getPriority() == QueueItem::NORMAL);
    REQUIRE(copy.isSet(QueueItem::FLAG_TEXT));
    REQUIRE(copy.getDownloadedBytes() == 300);
    REQUIRE(copy.getTTH() == tth);
}

TEST_CASE("QueueItem: getPartialInfo", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("partial");
    QueueItem qi(ctx, "/f", 10000, QueueItem::NORMAL, 0, 0, tth);

    qi.addSegment(Segment(0, 1024));       // [0, 1024)
    qi.addSegment(Segment(2048, 1024));    // [2048, 3072)

    PartsInfo info;
    int64_t blockSize = 1024;
    qi.getPartialInfo(info, blockSize);

    // Two segments → 4 values: [start1, end1, start2, end2]
    REQUIRE(info.size() == 4);
    REQUIRE(info[0] == 0);    // start of first = 0 / 1024 = 0
    REQUIRE(info[1] == 1);    // end of first = (1024-1)/1024+1 = 1
    REQUIRE(info[2] == 2);    // start of second = 2048 / 1024 = 2
    REQUIRE(info[3] == 3);    // end of second = (3072-1)/1024+1 = 3
}

TEST_CASE("QueueItem: isWaiting and isRunning", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("wait");
    QueueItem qi(ctx, "/f", 1000, QueueItem::NORMAL, 0, 0, tth);

    // No downloads → waiting
    REQUIRE(qi.isWaiting() == true);
    REQUIRE(qi.isRunning() == false);
}

TEST_CASE("QueueItem: getListName", "[queueitem]") {
    test::TestContext tc;
    auto& ctx = *tc.ownedCtx;
    TTHValue tth = makeTTH("list");

    SECTION("XML BZ list") {
        QueueItem qi(ctx, "/user123", 0, QueueItem::NORMAL,
                     QueueItem::FLAG_USER_LIST | QueueItem::FLAG_XML_BZLIST, 0, tth);
        REQUIRE(qi.getListName() == "/user123.xml.bz2");
    }
    SECTION("plain XML list") {
        QueueItem qi(ctx, "/user456", 0, QueueItem::NORMAL,
                     QueueItem::FLAG_USER_LIST, 0, tth);
        REQUIRE(qi.getListName() == "/user456.xml");
    }
}
