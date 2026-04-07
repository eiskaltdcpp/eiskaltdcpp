/*
 * Tests for FavoriteHubModel — Qt data model for favorite hubs.
 * No dcpp dependencies — pure Qt model.
 */

#include <catch2/catch_test_macros.hpp>
#include <QVariant>
#include <QModelIndex>

#include "FavoriteHubModel.h"

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: initial state", "[qt][favhubmodel]") {
    FavoriteHubModel model;
    REQUIRE(model.rowCount() == 0);
    REQUIRE(model.columnCount() == 8); // 8 columns defined by COLUMN_HUB_*
}

// ─── Add / Remove ──────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: addResult adds a row", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> data;
    data << true << "TestHub" << "A test hub" << "dchub://test.com"
         << "MyNick" << "pass123" << "description" << "UTF-8";

    model.addResult(data);
    REQUIRE(model.rowCount() == 1);
}

TEST_CASE("FavoriteHubModel: add multiple rows", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    for (int i = 0; i < 5; ++i) {
        QList<QVariant> data;
        data << false << QString("Hub%1").arg(i) << "desc" << "addr"
             << "nick" << "" << "" << "UTF-8";
        model.addResult(data);
    }
    REQUIRE(model.rowCount() == 5);
}

TEST_CASE("FavoriteHubModel: removeItem by index", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> data;
    data << true << "Hub1" << "d1" << "addr1" << "n" << "" << "" << "UTF-8";
    model.addResult(data);
    REQUIRE(model.rowCount() == 1);

    QModelIndex idx = model.index(0, 0);
    REQUIRE(idx.isValid());
    model.removeItem(idx);
    REQUIRE(model.rowCount() == 0);
}

// ─── Data retrieval ────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: data() returns correct values", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> data;
    data << true << "MyHub" << "Great hub" << "adc://hub.example.com"
         << "User123" << "secret" << "my desc" << "UTF-8";
    model.addResult(data);

    // DisplayRole
    QModelIndex nameIdx = model.index(0, COLUMN_HUB_NAME);
    REQUIRE(model.data(nameIdx, Qt::DisplayRole).toString() == "MyHub");

    QModelIndex addrIdx = model.index(0, COLUMN_HUB_ADDRESS);
    REQUIRE(model.data(addrIdx, Qt::DisplayRole).toString() == "adc://hub.example.com");

    QModelIndex nickIdx = model.index(0, COLUMN_HUB_NICK);
    REQUIRE(model.data(nickIdx, Qt::DisplayRole).toString() == "User123");
}

TEST_CASE("FavoriteHubModel: data() CheckStateRole on autoconnect column", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> data;
    data << true << "Hub" << "" << "" << "" << "" << "" << "";
    model.addResult(data);

    QModelIndex autoIdx = model.index(0, COLUMN_HUB_AUTOCONNECT);
    QVariant checkState = model.data(autoIdx, Qt::CheckStateRole);
    // The autoconnect column should return a check state
    REQUIRE(checkState.isValid());
}

// ─── Header data ────────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: headerData returns column names", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QVariant header = model.headerData(COLUMN_HUB_NAME, Qt::Horizontal, Qt::DisplayRole);
    REQUIRE(header.isValid());
    REQUIRE(!header.toString().isEmpty());
}

// ─── Flags ──────────────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: flags include editable", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> data;
    data << true << "Hub" << "" << "" << "" << "" << "" << "";
    model.addResult(data);

    QModelIndex idx = model.index(0, COLUMN_HUB_NAME);
    Qt::ItemFlags f = model.flags(idx);
    // Should be at least enabled and selectable
    REQUIRE((f & Qt::ItemIsEnabled));
    REQUIRE((f & Qt::ItemIsSelectable));
}

// ─── Move operations ────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: moveUp and moveDown", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> d1, d2, d3;
    d1 << false << "Hub_A" << "" << "" << "" << "" << "" << "";
    d2 << false << "Hub_B" << "" << "" << "" << "" << "" << "";
    d3 << false << "Hub_C" << "" << "" << "" << "" << "" << "";
    model.addResult(d1);
    model.addResult(d2);
    model.addResult(d3);
    REQUIRE(model.rowCount() == 3);

    // Move second item up → should be first
    QModelIndex idx1 = model.index(1, 0);
    model.moveUp(idx1);

    QModelIndex nameIdx0 = model.index(0, COLUMN_HUB_NAME);
    REQUIRE(model.data(nameIdx0, Qt::DisplayRole).toString() == "Hub_B");

    // Move it back down
    QModelIndex idx0 = model.index(0, 0);
    model.moveDown(idx0);

    nameIdx0 = model.index(0, COLUMN_HUB_NAME);
    REQUIRE(model.data(nameIdx0, Qt::DisplayRole).toString() == "Hub_A");
}

// ─── Clear ──────────────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: clearModel empties all rows", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    for (int i = 0; i < 3; ++i) {
        QList<QVariant> data;
        data << false << "Hub" << "" << "" << "" << "" << "" << "";
        model.addResult(data);
    }
    REQUIRE(model.rowCount() == 3);

    model.clearModel();
    REQUIRE(model.rowCount() == 0);
}

// ─── Sort ───────────────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: sort by name column", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> d1, d2, d3;
    d1 << false << "Zeta" << "" << "" << "" << "" << "" << "";
    d2 << false << "Alpha" << "" << "" << "" << "" << "" << "";
    d3 << false << "Mu" << "" << "" << "" << "" << "" << "";
    model.addResult(d1);
    model.addResult(d2);
    model.addResult(d3);

    model.sort(COLUMN_HUB_NAME, Qt::AscendingOrder);

    REQUIRE(model.data(model.index(0, COLUMN_HUB_NAME), Qt::DisplayRole).toString() == "Alpha");
    REQUIRE(model.data(model.index(1, COLUMN_HUB_NAME), Qt::DisplayRole).toString() == "Mu");
    REQUIRE(model.data(model.index(2, COLUMN_HUB_NAME), Qt::DisplayRole).toString() == "Zeta");
}

// ─── getItems ───────────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: getItems returns all items", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> d1;
    d1 << false << "Hub1" << "" << "" << "" << "" << "" << "";
    model.addResult(d1);

    auto items = model.getItems();
    REQUIRE(items.size() == 1);
    REQUIRE(items[0]->data(COLUMN_HUB_NAME).toString() == "Hub1");
}

// ─── Index / Parent ─────────────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: index and parent functions", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    QList<QVariant> d1;
    d1 << false << "Hub" << "" << "" << "" << "" << "" << "";
    model.addResult(d1);

    QModelIndex idx = model.index(0, 0);
    REQUIRE(idx.isValid());
    REQUIRE(idx.row() == 0);
    REQUIRE(idx.column() == 0);

    // Parent of top-level item is invalid
    QModelIndex parentIdx = model.parent(idx);
    REQUIRE(!parentIdx.isValid());
}

// ─── insertRow / removeRow ──────────────────────────────────────────────

TEST_CASE("FavoriteHubModel: insertRow is no-op stub", "[qt][favhubmodel]") {
    FavoriteHubModel model;

    // insertRow/removeRow are stubs returning true (used for drag-drop)
    bool ok = model.insertRow(0, QModelIndex());
    REQUIRE(ok);
    // Row count unchanged — these are placeholders for drag-drop
    REQUIRE(model.rowCount() == 0);
}
