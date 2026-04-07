/*
 * Tests for TransferListModel — Phase 3 stateful model.
 */
#include <catch2/catch_test_macros.hpp>
#include "TransferListModel.h"

using namespace gtk_transfer;

namespace {

std::map<std::string, std::string> makeDL(const std::string &cid,
                                           const std::string &user,
                                           const std::string &target,
                                           int64_t size = 1000,
                                           int64_t speed = 100)
{
    std::map<std::string, std::string> params;
    params["CID"] = cid;
    params["User"] = user;
    params["Hub Name"] = "TestHub";
    params["Filename"] = "file.txt";
    params["Target"] = target;
    params["Size"] = std::to_string(size);
    params["Speed"] = std::to_string(speed);
    params["Download Position"] = "500";
    params["Progress"] = "50";
    return params;
}

std::map<std::string, std::string> makeUL(const std::string &cid,
                                           const std::string &user)
{
    std::map<std::string, std::string> params;
    params["CID"] = cid;
    params["User"] = user;
    params["Hub Name"] = "TestHub";
    params["Filename"] = "upload.txt";
    params["Size"] = "2000";
    params["Speed"] = "200";
    return params;
}

} // anonymous namespace

// ── addOrUpdate / find ──

TEST_CASE("TransferListModel: add download", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt"), true);
    REQUIRE(model.count() == 1);
    REQUIRE(model.downloads().size() == 1);
    REQUIRE(model.uploads().empty());
}

TEST_CASE("TransferListModel: add upload", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeUL("CID2", "Bob"), false);
    REQUIRE(model.count() == 1);
    REQUIRE(model.uploads().size() == 1);
    REQUIRE(model.downloads().empty());
}

TEST_CASE("TransferListModel: update existing transfer", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt", 1000, 100), true);
    auto params = makeDL("CID1", "Alice", "/tmp/file.txt", 1000, 500);
    model.addOrUpdate(params, true);

    REQUIRE(model.count() == 1);
    auto t = model.find("CID1", true);
    REQUIRE(t != nullptr);
    REQUIRE(t->speed == 500);
}

TEST_CASE("TransferListModel: find returns nullptr for missing", "[gtk][transferlist]")
{
    TransferListModel model;
    REQUIRE(model.find("NOPE", true) == nullptr);
    REQUIRE(model.find("NOPE", false) == nullptr);
}

// ── remove ──

TEST_CASE("TransferListModel: remove download", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt"), true);
    REQUIRE(model.remove("CID1", true));
    REQUIRE(model.count() == 0);
}

TEST_CASE("TransferListModel: remove non-existent", "[gtk][transferlist]")
{
    TransferListModel model;
    REQUIRE_FALSE(model.remove("NOPE", true));
}

TEST_CASE("TransferListModel: remove download does not affect upload", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt"), true);
    model.addOrUpdate(makeUL("CID1", "Alice"), false);
    REQUIRE(model.count() == 2);

    REQUIRE(model.remove("CID1", true));
    REQUIRE(model.count() == 1);
    REQUIRE(model.find("CID1", false) != nullptr);
}

// ── allTransfers ──

TEST_CASE("TransferListModel: allTransfers combines both", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt"), true);
    model.addOrUpdate(makeUL("CID2", "Bob"), false);
    REQUIRE(model.allTransfers().size() == 2);
}

// ── clear ──

TEST_CASE("TransferListModel: clear empties everything", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt"), true);
    model.addOrUpdate(makeUL("CID2", "Bob"), false);
    model.finishTarget("/tmp/file.txt", "Done");
    model.clear();
    REQUIRE(model.count() == 0);
}

// ── parentSummary / segmented downloads ──

TEST_CASE("TransferListModel: parentSummary for segmented download", "[gtk][transferlist]")
{
    TransferListModel model;
    auto p1 = makeDL("CID1", "Alice", "/tmp/big.bin", 10000, 100);
    p1["Download Position"] = "2000";
    auto p2 = makeDL("CID2", "Bob", "/tmp/big.bin", 10000, 200);
    p2["Download Position"] = "3000";
    p2["Hub Name"] = "Hub2";

    model.addOrUpdate(p1, true);
    model.addOrUpdate(p2, true);

    auto summary = model.parentSummary("/tmp/big.bin");
    REQUIRE(summary.totalCount == 2);
    REQUIRE(summary.activeCount == 2);
    REQUIRE(summary.totalSpeed == 300);
    REQUIRE(summary.totalPosition == 5000);
    REQUIRE(summary.totalSize == 10000);
    REQUIRE(summary.progressPercent == 50);
    REQUIRE_FALSE(summary.users.empty());
}

TEST_CASE("TransferListModel: segmentedTargets lists multi-connection files", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/big.bin"), true);
    model.addOrUpdate(makeDL("CID2", "Bob", "/tmp/big.bin"), true);
    model.addOrUpdate(makeDL("CID3", "Carol", "/tmp/small.bin"), true);

    auto segs = model.segmentedTargets();
    REQUIRE(segs.size() == 1);
    REQUIRE(segs[0] == "/tmp/big.bin");
}

// ── finishTarget ──

TEST_CASE("TransferListModel: finishTarget marks parent finished", "[gtk][transferlist]")
{
    TransferListModel model;
    model.addOrUpdate(makeDL("CID1", "Alice", "/tmp/file.txt"), true);
    model.finishTarget("/tmp/file.txt", "Finished");

    auto summary = model.parentSummary("/tmp/file.txt");
    REQUIRE(summary.finished);
    REQUIRE(summary.status == "Finished");
}

// ── failed flag ──

TEST_CASE("TransferListModel: failed flag stored", "[gtk][transferlist]")
{
    TransferListModel model;
    auto params = makeDL("CID1", "Alice", "/tmp/file.txt");
    params["Failed"] = "1";
    model.addOrUpdate(params, true);

    auto t = model.find("CID1", true);
    REQUIRE(t != nullptr);
    REQUIRE(t->failed == true);
}
