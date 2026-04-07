/*
 * Tests for ShareBrowserModel — Phase 4 complex logic extraction.
 */
#include <catch2/catch_test_macros.hpp>
#include "ShareBrowserModel.h"

#include <algorithm>
#include <vector>

using namespace gtk_share;

namespace {

ShareItem makeFile(const std::string &name, int64_t size,
                   const std::string &tth = "") {
    ShareItem item;
    item.name = name;
    item.size = size;
    item.tth = tth;
    item.type = ItemType::FILE;
    return item;
}

ShareItem makeDir(const std::string &name, int64_t totalSize = 0) {
    ShareItem item;
    item.name = name;
    item.size = totalSize;
    item.type = ItemType::DIRECTORY;
    return item;
}

} // anonymous

// ── compareItems ──

TEST_CASE("ShareBrowser: dir sorts before file", "[gtk][share]")
{
    auto dir = makeDir("adir", 100);
    auto file = makeFile("afile.txt", 200);
    REQUIRE(compareItems(dir, file, SortColumn::FILENAME) < 0);
    REQUIRE(compareItems(file, dir, SortColumn::FILENAME) > 0);
}

TEST_CASE("ShareBrowser: two dirs compared by name", "[gtk][share]")
{
    auto a = makeDir("Alpha", 100);
    auto b = makeDir("Beta", 200);
    REQUIRE(compareItems(a, b, SortColumn::FILENAME) < 0);
    REQUIRE(compareItems(b, a, SortColumn::FILENAME) > 0);
}

TEST_CASE("ShareBrowser: two dirs compared by size", "[gtk][share]")
{
    auto a = makeDir("Small", 100);
    auto b = makeDir("Large", 900);
    REQUIRE(compareItems(a, b, SortColumn::SIZE) < 0);
    REQUIRE(compareItems(b, a, SortColumn::SIZE) > 0);
}

TEST_CASE("ShareBrowser: two files compared by name", "[gtk][share]")
{
    auto a = makeFile("alpha.txt", 100);
    auto b = makeFile("beta.txt", 200);
    REQUIRE(compareItems(a, b, SortColumn::FILENAME) < 0);
}

TEST_CASE("ShareBrowser: two files compared by size", "[gtk][share]")
{
    auto a = makeFile("small.txt", 100);
    auto b = makeFile("big.txt", 9000);
    REQUIRE(compareItems(a, b, SortColumn::SIZE) < 0);
    REQUIRE(compareItems(b, a, SortColumn::EXACT_SIZE) > 0);
}

TEST_CASE("ShareBrowser: same name case-insensitive", "[gtk][share]")
{
    auto a = makeFile("Hello.txt", 100);
    auto b = makeFile("hello.txt", 200);
    REQUIRE(compareItems(a, b, SortColumn::FILENAME) == 0);
}

TEST_CASE("ShareBrowser: sort a mixed list", "[gtk][share]")
{
    std::vector<ShareItem> items = {
        makeFile("zfile.txt", 1),
        makeDir("bdir", 2),
        makeFile("afile.txt", 3),
        makeDir("adir", 4),
    };

    std::sort(items.begin(), items.end(),
        [](const ShareItem &a, const ShareItem &b) {
            return compareItems(a, b, SortColumn::FILENAME) < 0;
        });

    // Dirs first (sorted), then files (sorted)
    REQUIRE(items[0].name == "adir");
    REQUIRE(items[1].name == "bdir");
    REQUIRE(items[2].name == "afile.txt");
    REQUIRE(items[3].name == "zfile.txt");
}

// ── computeTotalSize ──

TEST_CASE("ShareBrowser: computeTotalSize", "[gtk][share]")
{
    std::vector<ShareItem> items = {
        makeFile("a", 100),
        makeFile("b", 200),
        makeDir("c", 300),
    };
    REQUIRE(computeTotalSize(items) == 600);
}

TEST_CASE("ShareBrowser: computeTotalSize empty", "[gtk][share]")
{
    std::vector<ShareItem> items;
    REQUIRE(computeTotalSize(items) == 0);
}

// ── fileOrderKey ──

TEST_CASE("ShareBrowser: fileOrderKey", "[gtk][share]")
{
    auto dir = makeDir("Movies");
    auto file = makeFile("readme.txt", 100);
    REQUIRE(fileOrderKey(dir) == "dMovies");
    REQUIRE(fileOrderKey(file) == "freadme.txt");
    // Directory sorts before file alphabetically
    REQUIRE(fileOrderKey(dir) < fileOrderKey(file));
}

// ── typeString ──

TEST_CASE("ShareBrowser: typeString directory", "[gtk][share]")
{
    auto dir = makeDir("stuff");
    REQUIRE(typeString(dir) == "Directory");
}

TEST_CASE("ShareBrowser: typeString file with extension", "[gtk][share]")
{
    auto file = makeFile("song.MP3", 100);
    REQUIRE(typeString(file) == "mp3");
}

TEST_CASE("ShareBrowser: typeString file no extension", "[gtk][share]")
{
    auto file = makeFile("README", 100);
    REQUIRE(typeString(file) == "File");
}

TEST_CASE("ShareBrowser: typeString file dot at end", "[gtk][share]")
{
    auto file = makeFile("file.", 100);
    REQUIRE(typeString(file) == "File");
}
