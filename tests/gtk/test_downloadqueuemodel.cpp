/*
 * Tests for DownloadQueueModel — Phase 3 stateful model.
 */
#include <catch2/catch_test_macros.hpp>
#include "DownloadQueueModel.h"

using namespace gtk_queue;

namespace {

std::map<std::string, std::string> makeQueueItem(const std::string &target,
                                                   const std::string &path,
                                                   int64_t size = 1000)
{
    std::map<std::string, std::string> params;
    params["Target"] = target;
    params["Path"] = path;
    params["Filename"] = target.substr(target.rfind('/') + 1);
    params["Real Size"] = std::to_string(size);
    params["Downloaded Sort"] = "0";
    params["Priority"] = "3";
    return params;
}

} // anonymous namespace

// ── addItem / findItem ──

TEST_CASE("DownloadQueueModel: addItem basic", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/"));
    REQUIRE(model.itemCount() == 1);
    REQUIRE(model.findItem("/downloads/file.txt") != nullptr);
}

TEST_CASE("DownloadQueueModel: addItem overwrites same target", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/", 1000));
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/", 2000));
    REQUIRE(model.itemCount() == 1);
    REQUIRE(model.findItem("/downloads/file.txt")->size == 2000);
}

TEST_CASE("DownloadQueueModel: addItem without target ignored", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    std::map<std::string, std::string> params;
    params["Filename"] = "test.txt";
    model.addItem(params);
    REQUIRE(model.itemCount() == 0);
}

// ── updateItem ──

TEST_CASE("DownloadQueueModel: updateItem merges params", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/"));

    std::map<std::string, std::string> updates;
    updates["Priority"] = "5";
    REQUIRE(model.updateItem("/downloads/file.txt", updates));
    REQUIRE(model.findItem("/downloads/file.txt")->priority == 5);
}

TEST_CASE("DownloadQueueModel: updateItem non-existent returns false", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    std::map<std::string, std::string> updates;
    updates["Priority"] = "5";
    REQUIRE_FALSE(model.updateItem("/nope.txt", updates));
}

// ── removeItem ──

TEST_CASE("DownloadQueueModel: removeItem", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/"));
    REQUIRE(model.removeItem("/downloads/file.txt"));
    REQUIRE(model.itemCount() == 0);
}

TEST_CASE("DownloadQueueModel: removeItem cleans up sources", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/"));
    model.setSources("/downloads/file.txt", {{"Alice", "CID1"}});
    model.removeItem("/downloads/file.txt");
    REQUIRE(model.getSources("/downloads/file.txt") == nullptr);
}

// ── itemsInDir ──

TEST_CASE("DownloadQueueModel: itemsInDir filters by directory", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/dl/a/file1.txt", "/dl/a/"));
    model.addItem(makeQueueItem("/dl/a/file2.txt", "/dl/a/"));
    model.addItem(makeQueueItem("/dl/b/file3.txt", "/dl/b/"));

    REQUIRE(model.itemsInDir("/dl/a/").size() == 2);
    REQUIRE(model.itemsInDir("/dl/b/").size() == 1);
    REQUIRE(model.itemsInDir("/dl/c/").empty());
}

// ── directoryTree ──

TEST_CASE("DownloadQueueModel: directoryTree builds hierarchy", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/dl/music/song.mp3", "/dl/music/", 5000));
    model.addItem(makeQueueItem("/dl/music/album.zip", "/dl/music/", 3000));
    model.addItem(makeQueueItem("/dl/video/clip.mp4", "/dl/video/", 10000));

    auto tree = model.directoryTree();
    REQUIRE(tree.name == "/");
    REQUIRE(tree.fileCount == 3);
    REQUIRE(tree.totalSize == 18000);
    REQUIRE_FALSE(tree.children.empty());
}

TEST_CASE("DownloadQueueModel: directories returns unique sorted paths", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/dl/a/f1.txt", "/dl/a/"));
    model.addItem(makeQueueItem("/dl/b/f2.txt", "/dl/b/"));
    model.addItem(makeQueueItem("/dl/a/f3.txt", "/dl/a/"));

    auto dirs = model.directories();
    REQUIRE(dirs.size() == 2);
}

// ── sources ──

TEST_CASE("DownloadQueueModel: source tracking", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/"));

    std::map<std::string, std::string> sources = {{"Alice", "CID1"}, {"Bob", "CID2"}};
    model.setSources("/downloads/file.txt", sources);

    auto s = model.getSources("/downloads/file.txt");
    REQUIRE(s != nullptr);
    REQUIRE(s->size() == 2);
    REQUIRE(s->at("Alice") == "CID1");
}

TEST_CASE("DownloadQueueModel: bad source tracking", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/downloads/file.txt", "/downloads/"));

    model.setBadSources("/downloads/file.txt", {{"BadUser", "CID3"}});
    auto bs = model.getBadSources("/downloads/file.txt");
    REQUIRE(bs != nullptr);
    REQUIRE(bs->size() == 1);
}

// ── stats ──

TEST_CASE("DownloadQueueModel: stats computation", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/dl/f1.txt", "/dl/", 1000));
    model.addItem(makeQueueItem("/dl/f2.txt", "/dl/", 2000));

    auto s = model.stats();
    REQUIRE(s.items == 2);
    REQUIRE(s.totalSize == 3000);
}

// ── updatePriority ──

TEST_CASE("DownloadQueueModel: updatePriority", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/dl/file.txt", "/dl/"));
    REQUIRE(model.updatePriority("/dl/file.txt", 5));
    REQUIRE(model.findItem("/dl/file.txt")->priority == 5);
}

// ── clear ──

TEST_CASE("DownloadQueueModel: clear removes everything", "[gtk][queuemodel]")
{
    DownloadQueueModel model;
    model.addItem(makeQueueItem("/dl/f1.txt", "/dl/"));
    model.setSources("/dl/f1.txt", {{"Alice", "CID1"}});
    model.clear();
    REQUIRE(model.itemCount() == 0);
    REQUIRE(model.getSources("/dl/f1.txt") == nullptr);
}
