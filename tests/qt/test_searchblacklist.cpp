/*
 * Tests for SearchBlacklist — wildcard-based search filter.
 * Requires dcpp TestContext (Util::getPath for load/save) & WulforSettings.
 */

#include <catch2/catch_test_macros.hpp>
#include <QString>
#include <QList>

#include "SearchBlacklist.h"
#include "WulforSettings.h"
#include "QtContext.h"
#include "QtContextAware.h"
#include "../TestContext.h"

#include <memory>

// Lazy-init dcpp context + QtContext with WulforSettings
static std::unique_ptr<dcpp::test::TestContext> g_tc;
static std::unique_ptr<QtContext> g_qtCtx;

static void cleanupQtContext() {
    g_qtCtx.reset();     // ~QtContext auto-deregisters
}

static void ensureContext() {
    if (!g_tc)
        g_tc = std::make_unique<dcpp::test::TestContext>();
    if (!qtCtx()) {
        g_qtCtx = std::make_unique<QtContext>(*dcpp::getContext());  // auto-registers
        g_qtCtx->createSettings();
        std::atexit(cleanupQtContext);
    }
}

// Helper: create a fresh SearchBlacklist with empty lists
static SearchBlacklist* freshBlacklist() {
    qtCtx()->createSearchBlacklist();
    auto *sb = qtCtx()->searchBlacklist();
    // Clear any lists that may have been loaded from a previous test's save
    sb->setList(SearchBlacklist::NAME, {});
    sb->setList(SearchBlacklist::TTH, {});
    return sb;
}

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("SearchBlacklist: context creation", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();
    REQUIRE(sb != nullptr);
}

TEST_CASE("SearchBlacklist: empty lists by default", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();
    REQUIRE(sb->getList(SearchBlacklist::NAME).isEmpty());
    REQUIRE(sb->getList(SearchBlacklist::TTH).isEmpty());
}

// ─── setList / getList ──────────────────────────────────────────────────

TEST_CASE("SearchBlacklist: setList and getList for NAME", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    QList<QString> names = {"*.exe", "spam*", "bad_file"};
    sb->setList(SearchBlacklist::NAME, names);

    REQUIRE(sb->getList(SearchBlacklist::NAME).size() == 3);
    REQUIRE(sb->getList(SearchBlacklist::NAME).contains("*.exe"));
}

TEST_CASE("SearchBlacklist: setList and getList for TTH", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    QList<QString> tths = {"ABCDEF1234567890", "DEADBEEF"};
    sb->setList(SearchBlacklist::TTH, tths);

    REQUIRE(sb->getList(SearchBlacklist::TTH).size() == 2);
}

TEST_CASE("SearchBlacklist: NAME and TTH lists are independent", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    sb->setList(SearchBlacklist::NAME, {"name_entry"});
    sb->setList(SearchBlacklist::TTH, {"tth_entry"});

    REQUIRE(sb->getList(SearchBlacklist::NAME).size() == 1);
    REQUIRE(sb->getList(SearchBlacklist::TTH).size() == 1);
    REQUIRE(sb->getList(SearchBlacklist::NAME).first() == "name_entry");
    REQUIRE(sb->getList(SearchBlacklist::TTH).first() == "tth_entry");
}

// ─── ok() — wildcard matching ───────────────────────────────────────────

TEST_CASE("SearchBlacklist: ok returns true when blacklist is empty", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    REQUIRE(sb->ok("any_file.txt", SearchBlacklist::NAME));
    REQUIRE(sb->ok("ANYTTH12345", SearchBlacklist::TTH));
}

TEST_CASE("SearchBlacklist: ok returns false for exact match", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    sb->setList(SearchBlacklist::NAME, {"virus.exe"});
    REQUIRE_FALSE(sb->ok("virus.exe", SearchBlacklist::NAME));
}

TEST_CASE("SearchBlacklist: ok supports wildcard patterns", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    sb->setList(SearchBlacklist::NAME, {"*.exe"});
    REQUIRE_FALSE(sb->ok("virus.exe", SearchBlacklist::NAME));
    REQUIRE_FALSE(sb->ok("program.exe", SearchBlacklist::NAME));
    REQUIRE(sb->ok("document.pdf", SearchBlacklist::NAME));
}

TEST_CASE("SearchBlacklist: ok is case-insensitive", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    sb->setList(SearchBlacklist::NAME, {"SPAM*"});
    REQUIRE_FALSE(sb->ok("spam_file", SearchBlacklist::NAME));
    REQUIRE_FALSE(sb->ok("SPAM_file", SearchBlacklist::NAME));
    REQUIRE_FALSE(sb->ok("Spam_File", SearchBlacklist::NAME));
}

TEST_CASE("SearchBlacklist: ok with multiple patterns", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    sb->setList(SearchBlacklist::NAME, {"*.exe", "*.bat", "spam*"});

    REQUIRE_FALSE(sb->ok("virus.exe", SearchBlacklist::NAME));
    REQUIRE_FALSE(sb->ok("run.bat", SearchBlacklist::NAME));
    REQUIRE_FALSE(sb->ok("spam_today", SearchBlacklist::NAME));
    REQUIRE(sb->ok("document.pdf", SearchBlacklist::NAME));
}

TEST_CASE("SearchBlacklist: ok with TTH type", "[qt][searchblacklist]") {
    ensureContext();
    auto *sb = freshBlacklist();

    sb->setList(SearchBlacklist::TTH, {"ABCDEF*"});
    REQUIRE_FALSE(sb->ok("ABCDEF1234567890", SearchBlacklist::TTH));
    REQUIRE(sb->ok("ZZZZZ1234567890", SearchBlacklist::TTH));
}
