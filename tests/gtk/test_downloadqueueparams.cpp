/*
 * Tests for DownloadQueueParams — Phase 2 data transformation extraction.
 * All pure functions, no dcpp runtime needed.
 */
#include <catch2/catch_test_macros.hpp>
#include "DownloadQueueParams.h"
#include "TestContext.h"

#include <dcpp/stdinc.h>
#include <dcpp/QueueItem.h>

using namespace gtk_queue;
using dcpp::test::TestContext;

// ── priorityString ──

TEST_CASE("DownloadQueueParams: priorityString all values", "[gtk][queueparams]")
{
    REQUIRE(priorityString(dcpp::QueueItem::PAUSED)  == "Paused");
    REQUIRE(priorityString(dcpp::QueueItem::LOWEST)  == "Lowest");
    REQUIRE(priorityString(dcpp::QueueItem::LOW)     == "Low");
    REQUIRE(priorityString(dcpp::QueueItem::NORMAL)  == "Normal");
    REQUIRE(priorityString(dcpp::QueueItem::HIGH)    == "High");
    REQUIRE(priorityString(dcpp::QueueItem::HIGHEST) == "Highest");
}

TEST_CASE("DownloadQueueParams: priorityString unknown defaults to Normal", "[gtk][queueparams]")
{
    REQUIRE(priorityString(999) == "Normal");
    REQUIRE(priorityString(-2) == "Normal");
}

// ── statusString ──

TEST_CASE("DownloadQueueParams: statusString running", "[gtk][queueparams]")
{
    REQUIRE(statusString(2, 5, true) == "Running...");
}

TEST_CASE("DownloadQueueParams: statusString waiting", "[gtk][queueparams]")
{
    std::string s = statusString(2, 5, false);
    REQUIRE(s == "2 of 5 user(s) online");
}

TEST_CASE("DownloadQueueParams: statusString no sources", "[gtk][queueparams]")
{
    REQUIRE(statusString(0, 0, false) == "0 of 0 user(s) online");
}

// ── downloadedString ──

TEST_CASE("DownloadQueueParams: downloadedString normal", "[gtk][queueparams]")
{
    TestContext ctx;
    std::string s = downloadedString(512, 1024);
    // Should contain the formatted bytes and a percentage
    REQUIRE(s.find("(") != std::string::npos);
    REQUIRE(s.find("%)") != std::string::npos);
}

TEST_CASE("DownloadQueueParams: downloadedString zero size", "[gtk][queueparams]")
{
    TestContext ctx;
    REQUIRE(downloadedString(0, 0) == "0 B (0.00%)");
}

TEST_CASE("DownloadQueueParams: downloadedString complete", "[gtk][queueparams]")
{
    TestContext ctx;
    std::string s = downloadedString(1000, 1000);
    REQUIRE(s.find("100") != std::string::npos);
}

// ── sourceErrorString ──

TEST_CASE("DownloadQueueParams: sourceErrorString file not available", "[gtk][queueparams]")
{
    std::string s = sourceErrorString("Bob", dcpp::QueueItem::Source::FLAG_FILE_NOT_AVAILABLE);
    REQUIRE(s == "Bob (File not available)");
}

TEST_CASE("DownloadQueueParams: sourceErrorString passive", "[gtk][queueparams]")
{
    std::string s = sourceErrorString("Alice", dcpp::QueueItem::Source::FLAG_PASSIVE);
    REQUIRE(s == "Alice (Passive user)");
}

TEST_CASE("DownloadQueueParams: sourceErrorString CRC failed", "[gtk][queueparams]")
{
    std::string s = sourceErrorString("Eve", dcpp::QueueItem::Source::FLAG_CRC_FAILED);
    REQUIRE(s == "Eve (CRC32 inconsistency (SFV-Check))");
}

TEST_CASE("DownloadQueueParams: sourceErrorString bad tree", "[gtk][queueparams]")
{
    std::string s = sourceErrorString("Mallory", dcpp::QueueItem::Source::FLAG_BAD_TREE);
    REQUIRE(s == "Mallory (Full tree does not match TTH root)");
}

TEST_CASE("DownloadQueueParams: sourceErrorString slow source", "[gtk][queueparams]")
{
    std::string s = sourceErrorString("Slow", dcpp::QueueItem::Source::FLAG_SLOW_SOURCE);
    REQUIRE(s == "Slow (Source too slow)");
}

TEST_CASE("DownloadQueueParams: sourceErrorString no TTHF", "[gtk][queueparams]")
{
    std::string s = sourceErrorString("Old", dcpp::QueueItem::Source::FLAG_NO_TTHF);
    REQUIRE(s == "Old (Remote client does not fully support TTH - cannot download)");
}

TEST_CASE("DownloadQueueParams: sourceErrorString no flags returns nick only", "[gtk][queueparams]")
{
    REQUIRE(sourceErrorString("Clean", 0) == "Clean");
}
