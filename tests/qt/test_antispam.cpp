/*
 * Tests for AntiSpam — user black/gray/white list management.
 * Requires dcpp TestContext + WulforSettings (WBGET used in isInGray/isInWhite).
 * Does NOT test checkUser() — requires ClientManager.
 */

#include <catch2/catch_test_macros.hpp>
#include <QString>
#include <QList>

#include "Antispam.h"
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

// Helper: create a fresh AntiSpam, destroying any previous one
static AntiSpam* freshAntiSpam() {
    if (qtCtx()->antiSpam())
        qtCtx()->destroyAntiSpam();
    qtCtx()->createAntiSpam();
    return qtCtx()->antiSpam();
}

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("AntiSpam: singleton creation", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();
    REQUIRE(as != nullptr);
}

TEST_CASE("AntiSpam: empty lists on init", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();
    REQUIRE(as->getBlack().isEmpty());
    REQUIRE(as->getGray().isEmpty());
    REQUIRE(as->getWhite().isEmpty());
}

// ─── Add to lists ───────────────────────────────────────────────────────

TEST_CASE("AntiSpam: addToBlack", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"user1", "user2"});
    REQUIRE(as->getBlack().size() == 2);
    REQUIRE(as->isInBlack("user1"));
    REQUIRE(as->isInBlack("user2"));
}

TEST_CASE("AntiSpam: addToGray", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToGray({"grayuser"});
    REQUIRE(as->getGray().size() == 1);
}

TEST_CASE("AntiSpam: addToWhite", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToWhite({"whiteuser"});
    REQUIRE(as->getWhite().size() == 1);
}

// ─── isIn queries ───────────────────────────────────────────────────────

TEST_CASE("AntiSpam: isInBlack with present/absent users", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"bad_user"});
    REQUIRE(as->isInBlack("bad_user"));
    REQUIRE_FALSE(as->isInBlack("good_user"));
}

TEST_CASE("AntiSpam: isInAny checks all lists", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"b"});
    as->addToGray({"g"});
    as->addToWhite({"w"});

    REQUIRE(as->isInAny("b"));
    REQUIRE(as->isInAny("g"));
    REQUIRE(as->isInAny("w"));
    REQUIRE_FALSE(as->isInAny("x"));
}

// ─── Remove from lists ─────────────────────────────────────────────────

TEST_CASE("AntiSpam: remFromBlack", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"user1", "user2"});
    as->remFromBlack({"user1"});
    REQUIRE_FALSE(as->isInBlack("user1"));
    REQUIRE(as->isInBlack("user2"));
}

TEST_CASE("AntiSpam: remFromGray", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToGray({"g1", "g2"});
    as->remFromGray({"g2"});
    REQUIRE(as->getGray().size() == 1);
}

TEST_CASE("AntiSpam: remFromWhite", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToWhite({"w1", "w2"});
    as->remFromWhite({"w1"});
    REQUIRE(as->getWhite().size() == 1);
}

// ─── Clear ──────────────────────────────────────────────────────────────

TEST_CASE("AntiSpam: clearBlack", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"a", "b", "c"});
    as->clearBlack();
    REQUIRE(as->getBlack().isEmpty());
}

TEST_CASE("AntiSpam: clearAll clears all lists", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"b"});
    as->addToGray({"g"});
    as->addToWhite({"w"});
    as->clearAll();

    REQUIRE(as->getBlack().isEmpty());
    REQUIRE(as->getGray().isEmpty());
    REQUIRE(as->getWhite().isEmpty());
}

// ─── Phrase / Keys / Attempts ───────────────────────────────────────────

TEST_CASE("AntiSpam: setPhrase and getPhrase", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    // Bug in AntiSpam::setPhrase: checks `phrase.isEmpty()` not `phrase_.isEmpty()`
    // So on first call (phrase is empty), it always sets "5+5=?" regardless of input
    as->setPhrase("Type 'hello' to chat");
    REQUIRE(as->getPhrase() == "5+5=?");

    // Second call works because phrase is no longer empty
    as->setPhrase("custom phrase");
    REQUIRE(as->getPhrase() == "custom phrase");
}

TEST_CASE("AntiSpam: setKeys and getKeys", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    QList<QString> keys = {"hello", "hi", "hey"};
    as->setKeys(keys);
    REQUIRE(as->getKeys().size() == 3);
    REQUIRE(as->getKeys().contains("hello"));
}

TEST_CASE("AntiSpam: setAttempts and getAttempts", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->setAttempts(5);
    REQUIRE(as->getAttempts() == 5);
}

// ─── Move between lists ────────────────────────────────────────────────

TEST_CASE("AntiSpam: move from black to white", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToBlack({"user1"});
    REQUIRE(as->isInBlack("user1"));

    as->move("user1", eIN_WHITE);
    REQUIRE_FALSE(as->isInBlack("user1"));
    REQUIRE(as->isInWhite("user1"));
}

TEST_CASE("AntiSpam: move from gray to black", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    as->addToGray({"user1"});
    as->move("user1", eIN_BLACK);
    REQUIRE_FALSE(as->isInGray("user1"));
    REQUIRE(as->isInBlack("user1"));
}

// ─── Operator<< ────────────────────────────────────────────────────────

TEST_CASE("AntiSpam: operator<< to add to list", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    // operator<< sets state, then adds a list
    *as << eIN_BLACK << QList<QString>{"op_user1", "op_user2"};
    REQUIRE(as->isInBlack("op_user1"));
    REQUIRE(as->isInBlack("op_user2"));
}

TEST_CASE("AntiSpam: operator<< with single string", "[qt][antispam]") {
    ensureContext();
    auto *as = freshAntiSpam();

    *as << eIN_WHITE << QString("single_user");
    REQUIRE(as->isInWhite("single_user"));
}
