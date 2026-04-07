/*
 * Tests for SFVReader -- simple file verification CRC lookup.
 * No DCContext required -- pure file I/O.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "SFVReader.h"
#include "Util.h"

#include <filesystem>
#include <fstream>

using namespace dcpp;
namespace fs = std::filesystem;

static std::string makeTempDir(const std::string& suffix) {
    auto p = fs::temp_directory_path() / ("eiskalt_test_sfv_" + suffix);
    fs::create_directories(p);
    p = p.make_preferred();
    auto s = p.string();
    if (!s.empty() && s.back() != PATH_SEPARATOR)
        s += PATH_SEPARATOR;
    return s;
}

static void removeTempDir(const std::string& path) {
    fs::remove_all(fs::path(path));
}

TEST_CASE("SFVReader: no SFV file - hasCRC is false", "[sfvreader]") {
    std::string base = makeTempDir("no_sfv");
    std::string dataFile = base + "somedata.bin";

    // Create a data file with no corresponding .sfv
    {
        std::ofstream ofs(dataFile);
        ofs << "test data";
    }

    SFVReader reader(dataFile);
    REQUIRE_FALSE(reader.hasCRC());

    removeTempDir(base);
}

TEST_CASE("SFVReader: finds CRC from .sfv in same directory", "[sfvreader]") {
    std::string base = makeTempDir("with_sfv");
    std::string dataFile = base + "myfile.txt";
    std::string sfvFile = base + "checksum.sfv";

    // Create the data file
    {
        std::ofstream ofs(dataFile);
        ofs << "hello sfv";
    }

    // Create an SFV file with the CRC for myfile.txt
    // Format: filename<spaces>HEXCRC
    {
        std::ofstream ofs(sfvFile);
        ofs << "; This is a comment line\n";
        ofs << "myfile.txt  DEADBEEF\n";
    }

    SFVReader reader(dataFile);
    REQUIRE(reader.hasCRC());
    REQUIRE(reader.getCRC() == 0xDEADBEEF);

    removeTempDir(base);
}

TEST_CASE("SFVReader: CRC case-insensitive hex", "[sfvreader]") {
    std::string base = makeTempDir("hex_case");
    std::string dataFile = base + "test.dat";
    std::string sfvFile = base + "test.sfv";

    {
        std::ofstream ofs(dataFile);
        ofs << "data";
    }
    {
        std::ofstream ofs(sfvFile);
        ofs << "test.dat  aabbccdd\n";
    }

    SFVReader reader(dataFile);
    REQUIRE(reader.hasCRC());
    REQUIRE(reader.getCRC() == 0xAABBCCDD);

    removeTempDir(base);
}

TEST_CASE("SFVReader: multiple entries, correct one is found", "[sfvreader]") {
    std::string base = makeTempDir("multi_entry");
    std::string dataFile = base + "target.zip";
    std::string sfvFile = base + "archive.sfv";

    {
        std::ofstream ofs(dataFile);
        ofs << "zip data";
    }
    {
        std::ofstream ofs(sfvFile);
        ofs << "other.zip  11111111\n";
        ofs << "target.zip 22222222\n";
        ofs << "third.zip  33333333\n";
    }

    SFVReader reader(dataFile);
    REQUIRE(reader.hasCRC());
    REQUIRE(reader.getCRC() == 0x22222222);

    removeTempDir(base);
}

TEST_CASE("SFVReader: comment lines are skipped", "[sfvreader]") {
    std::string base = makeTempDir("comments");
    std::string dataFile = base + "file.bin";
    std::string sfvFile = base + "file.sfv";

    {
        std::ofstream ofs(dataFile);
        ofs << "binary";
    }
    {
        std::ofstream ofs(sfvFile);
        ofs << "; comment 1\n";
        ofs << "; comment 2\n";
        ofs << "file.bin  CAFEBABE\n";
    }

    SFVReader reader(dataFile);
    REQUIRE(reader.hasCRC());
    REQUIRE(reader.getCRC() == 0xCAFEBABE);

    removeTempDir(base);
}

TEST_CASE("SFVReader: non-existent file - no CRC", "[sfvreader]") {
    SFVReader reader("/tmp/eiskalt_nonexistent_file_for_sfv_test.dat");
    REQUIRE_FALSE(reader.hasCRC());
}

TEST_CASE("SFVReader: CRC value 0 is valid", "[sfvreader]") {
    std::string base = makeTempDir("zero_crc");
    std::string dataFile = base + "zero.dat";
    std::string sfvFile = base + "zero.sfv";

    {
        std::ofstream ofs(dataFile);
        ofs << "zero crc file";
    }
    {
        std::ofstream ofs(sfvFile);
        ofs << "zero.dat  00000000\n";
    }

    SFVReader reader(dataFile);
    REQUIRE(reader.hasCRC());
    REQUIRE(reader.getCRC() == 0);

    removeTempDir(base);
}
