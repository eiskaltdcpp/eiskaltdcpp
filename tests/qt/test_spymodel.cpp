/*
 * Tests for SpyModel — Qt data model for search spy display.
 * Requires dcpp TestContext because addResult() uses BOOLSETTING(LOG_SPY).
 */

#include <catch2/catch_test_macros.hpp>
#include <QVariant>
#include <QModelIndex>
#include <QMetaObject>
#include <QString>

#include "SpyModel.h"
#include "../TestContext.h"

#include <memory>

// Lazy-init dcpp context so BOOLSETTING() works inside addResult()
static std::unique_ptr<dcpp::test::TestContext> g_tc;
static void ensureContext() {
    if (!g_tc)
        g_tc = std::make_unique<dcpp::test::TestContext>();
}

// Helper: invoke private slot addResult via QMetaObject
static void addSpyResult(SpyModel& model, const QString& file, bool isTTH) {
    QMetaObject::invokeMethod(&model, "addResult",
                              Q_ARG(QString, file),
                              Q_ARG(bool, isTTH));
}

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("SpyModel: initial state", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    REQUIRE(model.rowCount() == 0);
    REQUIRE(model.columnCount() == 2); // COUNT and STRING
}

// ─── Add results ────────────────────────────────────────────────────────

TEST_CASE("SpyModel: addResult adds a row", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "test_search", false);
    REQUIRE(model.rowCount() == 1);
}

TEST_CASE("SpyModel: add multiple rows", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "search_1", false);
    addSpyResult(model, "search_2", false);
    addSpyResult(model, "search_3", true);
    REQUIRE(model.rowCount() == 3);
}

TEST_CASE("SpyModel: duplicate search increments count", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "same_search", false);
    addSpyResult(model, "same_search", false);
    addSpyResult(model, "same_search", false);

    // Should still be 1 row (deduplicated), with count = 3
    REQUIRE(model.rowCount() == 1);

    QModelIndex countIdx = model.index(0, COLUMN_SPY_COUNT);
    int count = model.data(countIdx, Qt::DisplayRole).toInt();
    REQUIRE(count == 3);
}

// ─── Data retrieval ────────────────────────────────────────────────────

TEST_CASE("SpyModel: data() returns correct search string", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "hello_world", false);

    QModelIndex strIdx = model.index(0, COLUMN_SPY_STRING);
    REQUIRE(model.data(strIdx, Qt::DisplayRole).toString() == "hello_world");
}

TEST_CASE("SpyModel: data() initial count is 0 (no duplicates)", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "first_search", false);

    // Count is stored as childItems.size()+1, but only when item has children.
    // A single (non-duplicated) search has no children, so raw data is "" → 0.
    QModelIndex countIdx = model.index(0, COLUMN_SPY_COUNT);
    REQUIRE(model.data(countIdx, Qt::DisplayRole).toInt() == 0);
}

// ─── TTH marking ────────────────────────────────────────────────────────

TEST_CASE("SpyModel: TTH and non-TTH searches", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567ABCDEFGH", true);
    addSpyResult(model, "regular_search", false);

    REQUIRE(model.rowCount() == 2);
}

// ─── Clear ──────────────────────────────────────────────────────────────

TEST_CASE("SpyModel: clearModel empties all rows", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "s1", false);
    addSpyResult(model, "s2", false);
    REQUIRE(model.rowCount() == 2);

    model.clearModel();
    REQUIRE(model.rowCount() == 0);
}

// ─── Sort toggle ────────────────────────────────────────────────────────

TEST_CASE("SpyModel: setSort enables/disables sorting", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    // Just verify these don't crash
    model.setSort(true);
    model.setSort(false);

    addSpyResult(model, "test", false);
    REQUIRE(model.rowCount() == 1);
}

TEST_CASE("SpyModel: sort with multiple items", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "Zulu", false);
    addSpyResult(model, "Alpha", false);
    addSpyResult(model, "Mike", false);

    model.sort(COLUMN_SPY_STRING, Qt::AscendingOrder);

    REQUIRE(model.data(model.index(0, COLUMN_SPY_STRING), Qt::DisplayRole).toString() == "Alpha");
    REQUIRE(model.data(model.index(2, COLUMN_SPY_STRING), Qt::DisplayRole).toString() == "Zulu");
}

// ─── Header ─────────────────────────────────────────────────────────────

TEST_CASE("SpyModel: headerData", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;

    QVariant header = model.headerData(COLUMN_SPY_STRING, Qt::Horizontal, Qt::DisplayRole);
    REQUIRE(header.isValid());
    REQUIRE(!header.toString().isEmpty());
}

// ─── Index / Parent ─────────────────────────────────────────────────────

TEST_CASE("SpyModel: index and parent", "[qt][spymodel]") {
    ensureContext();
    SpyModel model;
    addSpyResult(model, "test", false);

    QModelIndex idx = model.index(0, 0);
    REQUIRE(idx.isValid());

    QModelIndex parentIdx = model.parent(idx);
    REQUIRE(!parentIdx.isValid());
}
