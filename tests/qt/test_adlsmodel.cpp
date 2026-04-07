/*
 * Tests for ADLSModel — Qt data model for ADL search rules.
 * No dcpp dependencies — pure Qt model.
 */

#include <catch2/catch_test_macros.hpp>
#include <QVariant>
#include <QModelIndex>

#include "ADLSModel.h"

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("ADLSModel: initial state", "[qt][adlsmodel]") {
    ADLSModel model;
    REQUIRE(model.rowCount() == 0);
    REQUIRE(model.columnCount() == 7); // 7 columns: CHECK, SSTRING, TYPE, DIR, MIN, MAX, TYPESIZE
}

// ─── Add / Remove ──────────────────────────────────────────────────────

TEST_CASE("ADLSModel: addResult adds a row", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> data;
    data << Qt::Checked << "*.mp3" << "Filename" << "Music"
         << "0" << "100" << "MB";
    model.addResult(data);
    REQUIRE(model.rowCount() == 1);
}

TEST_CASE("ADLSModel: add multiple rows", "[qt][adlsmodel]") {
    ADLSModel model;

    for (int i = 0; i < 4; ++i) {
        QList<QVariant> data;
        data << Qt::Checked << QString("*.ext%1").arg(i) << "Filename"
             << "Dir" << "0" << "0" << "B";
        model.addResult(data);
    }
    REQUIRE(model.rowCount() == 4);
}

TEST_CASE("ADLSModel: removeItem by index", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> data;
    data << Qt::Checked << "*.avi" << "Filename" << "Video"
         << "0" << "0" << "B";
    model.addResult(data);
    REQUIRE(model.rowCount() == 1);

    QModelIndex idx = model.index(0, 0);
    model.removeItem(idx);
    REQUIRE(model.rowCount() == 0);
}

// ─── Data retrieval ────────────────────────────────────────────────────

TEST_CASE("ADLSModel: data() returns correct values", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> data;
    data << Qt::Checked << "linux*.iso" << "Filename" << "ISOs"
         << "500" << "5000" << "MB";
    model.addResult(data);

    QModelIndex searchIdx = model.index(0, COLUMN_SSTRING);
    REQUIRE(model.data(searchIdx, Qt::DisplayRole).toString() == "linux*.iso");

    QModelIndex typeIdx = model.index(0, COLUMN_TYPE);
    REQUIRE(model.data(typeIdx, Qt::DisplayRole).toString() == "Filename");

    QModelIndex dirIdx = model.index(0, COLUMN_DIRECTORY);
    REQUIRE(model.data(dirIdx, Qt::DisplayRole).toString() == "ISOs");
}

// ─── Flags ──────────────────────────────────────────────────────────────

TEST_CASE("ADLSModel: flags include checkbox for check column", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> data;
    data << Qt::Checked << "test" << "Filename" << "" << "0" << "0" << "B";
    model.addResult(data);

    QModelIndex checkIdx = model.index(0, COLUMN_CHECK);
    Qt::ItemFlags f = model.flags(checkIdx);
    REQUIRE((f & Qt::ItemIsUserCheckable));
}

// ─── Move operations ────────────────────────────────────────────────────

TEST_CASE("ADLSModel: moveUp and moveDown", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> d1, d2, d3;
    d1 << Qt::Checked << "Rule_A" << "Filename" << "" << "0" << "0" << "B";
    d2 << Qt::Checked << "Rule_B" << "Filename" << "" << "0" << "0" << "B";
    d3 << Qt::Checked << "Rule_C" << "Filename" << "" << "0" << "0" << "B";
    model.addResult(d1);
    model.addResult(d2);
    model.addResult(d3);

    // Move second item up
    QModelIndex idx1 = model.index(1, 0);
    model.moveUp(idx1);

    REQUIRE(model.data(model.index(0, COLUMN_SSTRING), Qt::DisplayRole).toString() == "Rule_B");

    // Move it back down
    QModelIndex idx0 = model.index(0, 0);
    model.moveDown(idx0);

    REQUIRE(model.data(model.index(0, COLUMN_SSTRING), Qt::DisplayRole).toString() == "Rule_A");
}

// ─── Sort ───────────────────────────────────────────────────────────────

TEST_CASE("ADLSModel: sort by search string", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> d1, d2, d3;
    d1 << Qt::Checked << "Zulu" << "FN" << "" << "0" << "0" << "B";
    d2 << Qt::Checked << "Alpha" << "FN" << "" << "0" << "0" << "B";
    d3 << Qt::Checked << "Mike" << "FN" << "" << "0" << "0" << "B";
    model.addResult(d1);
    model.addResult(d2);
    model.addResult(d3);

    model.sort(COLUMN_SSTRING, Qt::AscendingOrder);

    REQUIRE(model.data(model.index(0, COLUMN_SSTRING), Qt::DisplayRole).toString() == "Alpha");
    REQUIRE(model.data(model.index(2, COLUMN_SSTRING), Qt::DisplayRole).toString() == "Zulu");
}

// ─── Clear ──────────────────────────────────────────────────────────────

TEST_CASE("ADLSModel: clearModel empties all rows", "[qt][adlsmodel]") {
    ADLSModel model;

    for (int i = 0; i < 3; ++i) {
        QList<QVariant> data;
        data << Qt::Checked << "test" << "FN" << "" << "0" << "0" << "B";
        model.addResult(data);
    }
    REQUIRE(model.rowCount() == 3);

    model.clearModel();
    REQUIRE(model.rowCount() == 0);
}

// ─── Header ─────────────────────────────────────────────────────────────

TEST_CASE("ADLSModel: headerData returns column names", "[qt][adlsmodel]") {
    ADLSModel model;

    QVariant header = model.headerData(COLUMN_SSTRING, Qt::Horizontal, Qt::DisplayRole);
    REQUIRE(header.isValid());
    REQUIRE(!header.toString().isEmpty());
}

// ─── Index / Parent ─────────────────────────────────────────────────────

TEST_CASE("ADLSModel: index and parent", "[qt][adlsmodel]") {
    ADLSModel model;

    QList<QVariant> data;
    data << Qt::Checked << "test" << "FN" << "" << "0" << "0" << "B";
    model.addResult(data);

    QModelIndex idx = model.index(0, 0);
    REQUIRE(idx.isValid());

    QModelIndex parentIdx = model.parent(idx);
    REQUIRE(!parentIdx.isValid());
}

// ─── Insert / Remove rows ──────────────────────────────────────────────

TEST_CASE("ADLSModel: insertRow is no-op stub", "[qt][adlsmodel]") {
    ADLSModel model;

    // insertRow/removeRow are stubs returning true (used for drag-drop)
    bool ok = model.insertRow(0, QModelIndex());
    REQUIRE(ok);
    REQUIRE(model.rowCount() == 0);
}
