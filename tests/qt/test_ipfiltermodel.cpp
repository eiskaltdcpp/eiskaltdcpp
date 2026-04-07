/*
 * Tests for IPFilterModel — Qt data model for IP filter rules.
 * No dcpp dependencies — pure Qt model.
 */

#include <catch2/catch_test_macros.hpp>
#include <QVariant>
#include <QModelIndex>

#include "IPFilterModel.h"

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: initial state", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    REQUIRE(model.rowCount() == 0);
    REQUIRE(model.columnCount() == 2); // RULE_NAME and RULE_DIRECTION
}

// ─── Add / Remove ──────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: addResult adds a rule", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("192.168.1.0/24", "IN");
    REQUIRE(model.rowCount() == 1);
}

TEST_CASE("IPFilterModel: add multiple rules", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("10.0.0.0/8", "IN");
    model.addResult("172.16.0.0/12", "OUT");
    model.addResult("192.168.0.0/16", "BOTH");
    REQUIRE(model.rowCount() == 3);
}

// ─── Data retrieval ────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: data() returns correct values", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("10.0.0.1", "IN");

    QModelIndex ruleIdx = model.index(0, COLUMN_RULE_NAME);
    REQUIRE(model.data(ruleIdx, Qt::DisplayRole).toString() == "10.0.0.1");

    QModelIndex dirIdx = model.index(0, COLUMN_RULE_DIRECTION);
    REQUIRE(model.data(dirIdx, Qt::DisplayRole).toString() == "IN");
}

// ─── Flags ──────────────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: flags are selectable and enabled", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("1.2.3.4", "OUT");

    QModelIndex idx = model.index(0, 0);
    Qt::ItemFlags f = model.flags(idx);
    REQUIRE((f & Qt::ItemIsEnabled));
    REQUIRE((f & Qt::ItemIsSelectable));
}

// ─── Move operations ────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: moveUp and moveDown", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("Rule_A", "IN");
    model.addResult("Rule_B", "OUT");
    model.addResult("Rule_C", "BOTH");
    REQUIRE(model.rowCount() == 3);

    // Move second item up → should swap with first
    QModelIndex idx1 = model.index(1, 0);
    model.moveUp(idx1);

    REQUIRE(model.data(model.index(0, COLUMN_RULE_NAME), Qt::DisplayRole).toString() == "Rule_B");

    // Move it back down
    QModelIndex idx0 = model.index(0, 0);
    model.moveDown(idx0);

    REQUIRE(model.data(model.index(0, COLUMN_RULE_NAME), Qt::DisplayRole).toString() == "Rule_A");
}

// ─── Sort ───────────────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: sort is no-op", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("Zeta", "IN");
    model.addResult("Alpha", "OUT");
    model.addResult("Mu", "IN");

    // sort() is intentionally a no-op in IPFilterModel (rules are order-sensitive)
    model.sort(COLUMN_RULE_NAME, Qt::AscendingOrder);

    // Order should be preserved (insertion order)
    REQUIRE(model.data(model.index(0, COLUMN_RULE_NAME), Qt::DisplayRole).toString() == "Zeta");
    REQUIRE(model.data(model.index(2, COLUMN_RULE_NAME), Qt::DisplayRole).toString() == "Mu");
}

// ─── Clear ──────────────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: clearModel empties all rows", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("1.2.3.4", "IN");
    model.addResult("5.6.7.8", "OUT");
    REQUIRE(model.rowCount() == 2);

    model.clearModel();
    REQUIRE(model.rowCount() == 0);
}

// ─── Header ─────────────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: headerData returns column names", "[qt][ipfiltermodel]") {
    IPFilterModel model;

    QVariant header = model.headerData(COLUMN_RULE_NAME, Qt::Horizontal, Qt::DisplayRole);
    REQUIRE(header.isValid());
    REQUIRE(!header.toString().isEmpty());
}

// ─── Index / Parent ─────────────────────────────────────────────────────

TEST_CASE("IPFilterModel: index and parent", "[qt][ipfiltermodel]") {
    IPFilterModel model;
    model.addResult("10.0.0.0/8", "BOTH");

    QModelIndex idx = model.index(0, 0);
    REQUIRE(idx.isValid());

    QModelIndex parentIdx = model.parent(idx);
    REQUIRE(!parentIdx.isValid());
}
