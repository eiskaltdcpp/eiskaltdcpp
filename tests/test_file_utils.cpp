/*
 * Tests for File static utilities: isAbsolute, ensureDirectory,
 * getSize, deleteFile, copyFile, renameFile, findFiles, and
 * File instance read/write round-trips.
 * No DCContext required.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "File.h"

#include <filesystem>
#include <fstream>

using namespace dcpp;
namespace fs = std::filesystem;

// Helper: create a temp directory and return its path
static std::string makeTempDir(const std::string& suffix) {
    auto p = fs::temp_directory_path() / ("eiskalt_test_file_" + suffix);
    fs::create_directories(p);
    return p.string() + "/";
}

// Helper: clean up temp dir
static void removeTempDir(const std::string& path) {
    fs::remove_all(fs::path(path));
}

// ─── isAbsolute ─────────────────────────────────────────────────────────

TEST_CASE("File::isAbsolute: absolute path", "[file]") {
    REQUIRE(File::isAbsolute("/usr/bin/test") == true);
    REQUIRE(File::isAbsolute("/home") == true);
}

TEST_CASE("File::isAbsolute: relative path", "[file]") {
    REQUIRE(File::isAbsolute("relative/path") == false);
    REQUIRE(File::isAbsolute("file.txt") == false);
    REQUIRE(File::isAbsolute("") == false);
}

// ─── ensureDirectory ────────────────────────────────────────────────────

TEST_CASE("File::ensureDirectory: creates nested dirs", "[file]") {
    std::string base = makeTempDir("ensure");
    std::string nested = base + "a/b/c/";

    File::ensureDirectory(nested);
    REQUIRE(fs::is_directory(nested));

    removeTempDir(base);
}

TEST_CASE("File::ensureDirectory: existing dir is no-op", "[file]") {
    std::string base = makeTempDir("ensure_exist");

    File::ensureDirectory(base);
    REQUIRE(fs::is_directory(base));

    removeTempDir(base);
}

// ─── File instance: write + read round-trip ─────────────────────────────

TEST_CASE("File: write and read back", "[file]") {
    std::string base = makeTempDir("rw");
    std::string path = base + "test.txt";

    {
        File f(path, File::WRITE, File::CREATE | File::TRUNCATE);
        std::string data = "Hello, File!";
        f.write(data);
    }

    {
        File f(path, File::READ, File::OPEN);
        std::string content = f.read();
        REQUIRE(content == "Hello, File!");
    }

    removeTempDir(base);
}

TEST_CASE("File: getSize instance", "[file]") {
    std::string base = makeTempDir("size_inst");
    std::string path = base + "sized.dat";

    {
        File f(path, File::WRITE, File::CREATE | File::TRUNCATE);
        std::string data(1234, 'X');
        f.write(data);
    }

    {
        File f(path, File::READ, File::OPEN);
        REQUIRE(f.getSize() == 1234);
    }

    removeTempDir(base);
}

// ─── Static getSize ─────────────────────────────────────────────────────

TEST_CASE("File::getSize static", "[file]") {
    std::string base = makeTempDir("getsize");
    std::string path = base + "data.bin";

    {
        std::ofstream ofs(path, std::ios::binary);
        std::string data(5678, 'A');
        ofs.write(data.data(), data.size());
    }

    REQUIRE(File::getSize(path) == 5678);

    removeTempDir(base);
}

TEST_CASE("File::getSize static: non-existent file", "[file]") {
    REQUIRE(File::getSize("/tmp/nonexistent_eiskalt_test_file_xyz") == -1);
}

// ─── deleteFile ─────────────────────────────────────────────────────────

TEST_CASE("File::deleteFile", "[file]") {
    std::string base = makeTempDir("delete");
    std::string path = base + "todelete.txt";

    {
        std::ofstream ofs(path);
        ofs << "delete me";
    }
    REQUIRE(fs::exists(path));

    File::deleteFile(path);
    REQUIRE_FALSE(fs::exists(path));

    removeTempDir(base);
}

TEST_CASE("File::deleteFile: non-existent file no throw", "[file]") {
    File::deleteFile("/tmp/nonexistent_eiskalt_test_xyz_delete");
    // Should not throw
    REQUIRE(true);
}

// ─── copyFile ───────────────────────────────────────────────────────────

TEST_CASE("File::copyFile", "[file]") {
    std::string base = makeTempDir("copy");
    std::string src = base + "source.txt";
    std::string dst = base + "dest.txt";

    {
        std::ofstream ofs(src);
        ofs << "copy me";
    }

    File::copyFile(src, dst);
    REQUIRE(fs::exists(dst));
    REQUIRE(File::getSize(dst) == File::getSize(src));

    // Verify content
    {
        File f(dst, File::READ, File::OPEN);
        REQUIRE(f.read() == "copy me");
    }

    removeTempDir(base);
}

// ─── renameFile ─────────────────────────────────────────────────────────

TEST_CASE("File::renameFile", "[file]") {
    std::string base = makeTempDir("rename");
    std::string src = base + "old.txt";
    std::string dst = base + "new.txt";

    {
        std::ofstream ofs(src);
        ofs << "rename me";
    }

    File::renameFile(src, dst);
    REQUIRE_FALSE(fs::exists(src));
    REQUIRE(fs::exists(dst));

    {
        File f(dst, File::READ, File::OPEN);
        REQUIRE(f.read() == "rename me");
    }

    removeTempDir(base);
}

// ─── findFiles ──────────────────────────────────────────────────────────

TEST_CASE("File::findFiles", "[file]") {
    std::string base = makeTempDir("find");

    // Create a few files
    for (auto name : {"file1.txt", "file2.txt", "file3.log"}) {
        std::ofstream ofs(base + name);
        ofs << "data";
    }

    auto allFiles = File::findFiles(base, "*");
    REQUIRE(allFiles.size() >= 3);

    auto txtFiles = File::findFiles(base, "*.txt");
    REQUIRE(txtFiles.size() == 2);

    auto logFiles = File::findFiles(base, "*.log");
    REQUIRE(logFiles.size() == 1);

    removeTempDir(base);
}

TEST_CASE("File::findFiles: empty directory", "[file]") {
    std::string base = makeTempDir("find_empty");

    // Use a pattern that won't match . and ..
    auto files = File::findFiles(base, "*.txt");
    REQUIRE(files.empty());

    removeTempDir(base);
}

// ─── Seek / position ────────────────────────────────────────────────────

TEST_CASE("File: setPos and read partial", "[file]") {
    std::string base = makeTempDir("seek");
    std::string path = base + "seektest.txt";

    {
        File f(path, File::WRITE, File::CREATE | File::TRUNCATE);
        f.write(std::string("ABCDEFGHIJ"));
    }

    {
        File f(path, File::READ, File::OPEN);
        f.setPos(5);
        REQUIRE(f.getPos() == 5);
        // read(size_t) doesn't reset position, unlike read() which calls setPos(0)
        std::string rest = f.read(5);
        REQUIRE(rest == "FGHIJ");
    }

    removeTempDir(base);
}

TEST_CASE("File: read with length", "[file]") {
    std::string base = makeTempDir("readlen");
    std::string path = base + "partial.txt";

    {
        File f(path, File::WRITE, File::CREATE | File::TRUNCATE);
        f.write(std::string("0123456789"));
    }

    {
        File f(path, File::READ, File::OPEN);
        std::string result = f.read(5);
        REQUIRE(result.size() == 5);
        REQUIRE(result == "01234");
    }

    removeTempDir(base);
}
