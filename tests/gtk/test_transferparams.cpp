/*
 * Tests for TransferParams — Phase 2 data transformation extraction.
 *
 * Tests the pure helper functions. The full paramsFromTransfer/paramsFromCQI
 * are tested with pre-resolved values (no dcpp runtime needed for these).
 */
#include <catch2/catch_test_macros.hpp>
#include "TransferParams.h"
#include <dcpp/stdinc.h>
#include <dcpp/Util.h> // PATH_SEPARATOR
#include <algorithm>

using namespace gtk_transfer;

namespace {
/// Convert a Unix-style hardcoded path to the platform's native separator.
inline std::string nativePath(const char* p) {
    std::string s(p);
#ifdef _WIN32
    std::replace(s.begin(), s.end(), '/', '\\');
#endif
    return s;
}
} // anon

// ── paramsFromCQI ──

TEST_CASE("TransferParams: paramsFromCQI populates all keys", "[gtk][transferparams]")
{
    auto p = paramsFromCQI("ABCDEFGHIJ1234567890ABCDEFGHIJ12",
                           "TestUser", "MyHub", "dchub://hub.example.com");

    REQUIRE(p["CID"] == "ABCDEFGHIJ1234567890ABCDEFGHIJ12");
    REQUIRE(p["User"] == "TestUser");
    REQUIRE(p["Hub Name"] == "MyHub");
    REQUIRE(p["Failed"] == "0");
    REQUIRE(p["Hub URL"] == "dchub://hub.example.com");
}

TEST_CASE("TransferParams: paramsFromCQI with empty values", "[gtk][transferparams]")
{
    auto p = paramsFromCQI("", "", "", "");

    REQUIRE(p.count("CID") == 1);
    REQUIRE(p.count("User") == 1);
    REQUIRE(p.count("Hub Name") == 1);
    REQUIRE(p.count("Failed") == 1);
    REQUIRE(p.count("Hub URL") == 1);
    REQUIRE(p["Failed"] == "0");
}

// ── paramsFromTransfer ──

TEST_CASE("TransferParams: paramsFromTransfer for regular file", "[gtk][transferparams]")
{
    auto p = paramsFromTransfer(
        "CID123", "Nick", "Hub", nativePath("/path/to/file.txt"),
        TransferType::FILE, /*size*/1024, /*pos*/512,
        /*speed*/100, /*secsLeft*/5,
        "192.168.1.1", "adc://hub.example.com",
        true, "TLS_AES_256_GCM_SHA384");

    REQUIRE(p["Filename"] == "file.txt");
    REQUIRE(p["Path"] == nativePath("/path/to/"));
    REQUIRE(p["Size"] == "1024");
    REQUIRE(p["Download Position"] == "512");
    REQUIRE(p["Speed"] == "100");
    REQUIRE(p["Progress Hidden"] == "50");
    REQUIRE(p["IP"] == "192.168.1.1");
    REQUIRE(p["Time Left"] == "5");
    REQUIRE(p["Target"] == nativePath("/path/to/file.txt"));
    REQUIRE(p["Hub URL"] == "adc://hub.example.com");
    REQUIRE(p["Encryption"] == "TLS_AES_256_GCM_SHA384");
}

TEST_CASE("TransferParams: paramsFromTransfer for file list", "[gtk][transferparams]")
{
    auto p = paramsFromTransfer(
        "CID", "Nick", "Hub", "/some/path",
        TransferType::FULL_LIST, 100, 50, 10, -1,
        "10.0.0.1", "dchub://hub", false, "");

    REQUIRE(p["Filename"] == "File list");
    REQUIRE(p["Encryption"] == "Plain");
    REQUIRE(p["Time Left"] == "-1");
}

TEST_CASE("TransferParams: paramsFromTransfer for partial list", "[gtk][transferparams]")
{
    auto p = paramsFromTransfer(
        "CID", "Nick", "Hub", "/path",
        TransferType::PARTIAL_LIST, 100, 0, 0, -1,
        "", "", false, "");

    REQUIRE(p["Filename"] == "File list");
}

TEST_CASE("TransferParams: paramsFromTransfer for TTH tree", "[gtk][transferparams]")
{
    auto p = paramsFromTransfer(
        "CID", "Nick", "Hub", nativePath("/path/to/data.bin"),
        TransferType::TREE, 100, 10, 5, 18,
        "1.2.3.4", "adc://h", true, "ECDHE-RSA");

    REQUIRE(p["Filename"] == "TTH: data.bin");
    REQUIRE(p["Encryption"] == "ECDHE-RSA");
}

// ── buildTransferFlags ──

TEST_CASE("TransferParams: buildTransferFlags empty for plain transfer", "[gtk][transferparams]")
{
    REQUIRE(buildTransferFlags(false, false, false, false) == "");
}

TEST_CASE("TransferParams: buildTransferFlags secure trusted", "[gtk][transferparams]")
{
    REQUIRE(buildTransferFlags(true, true, false, false) == "[S]");
}

TEST_CASE("TransferParams: buildTransferFlags secure untrusted", "[gtk][transferparams]")
{
    REQUIRE(buildTransferFlags(true, false, false, false) == "[U]");
}

TEST_CASE("TransferParams: buildTransferFlags all flags", "[gtk][transferparams]")
{
    REQUIRE(buildTransferFlags(true, true, true, true) == "[S][T][Z]");
}

TEST_CASE("TransferParams: buildTransferFlags tth+zlib without ssl", "[gtk][transferparams]")
{
    REQUIRE(buildTransferFlags(false, false, true, true) == "[T][Z]");
}

// ── sortOrderPrefix ──

TEST_CASE("TransferParams: sortOrderPrefix download active", "[gtk][transferparams]")
{
    REQUIRE(sortOrderPrefix(true, false) == "d");
}

TEST_CASE("TransferParams: sortOrderPrefix upload active", "[gtk][transferparams]")
{
    REQUIRE(sortOrderPrefix(false, false) == "u");
}

TEST_CASE("TransferParams: sortOrderPrefix completed", "[gtk][transferparams]")
{
    REQUIRE(sortOrderPrefix(true, true) == "w");
    REQUIRE(sortOrderPrefix(false, true) == "w");
}

// ── progressPercent ──

TEST_CASE("TransferParams: progressPercent normal", "[gtk][transferparams]")
{
    REQUIRE(progressPercent(50, 100) == 50);
    REQUIRE(progressPercent(0, 100) == 0);
    REQUIRE(progressPercent(100, 100) == 100);
    REQUIRE(progressPercent(1, 3) == 33);
}

TEST_CASE("TransferParams: progressPercent zero size", "[gtk][transferparams]")
{
    REQUIRE(progressPercent(0, 0) == 0);
    REQUIRE(progressPercent(100, 0) == 0);
    REQUIRE(progressPercent(50, -1) == 0);
}
