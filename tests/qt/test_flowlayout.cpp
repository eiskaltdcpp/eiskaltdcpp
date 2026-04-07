/*
 * Tests for FlowLayout — custom QLayout for flowing widget arrangement.
 * Pure Qt, zero dcpp dependencies.
 */

#include <catch2/catch_test_macros.hpp>
#include <QWidget>
#include <QPushButton>
#include <QSize>

#include "FlowLayout.h"

// ─── Construction ───────────────────────────────────────────────────────

TEST_CASE("FlowLayout: default construction", "[qt][flowlayout]") {
    FlowLayout layout;
    REQUIRE(layout.count() == 0);
    REQUIRE(layout.horizontalSpacing() == 0); // default ctor parameter = 0
    REQUIRE(layout.verticalSpacing() == 0);
}

TEST_CASE("FlowLayout: construction with spacing", "[qt][flowlayout]") {
    FlowLayout layout(5, 10, 15);
    REQUIRE(layout.horizontalSpacing() == 10);
    REQUIRE(layout.verticalSpacing() == 15);
}

TEST_CASE("FlowLayout: construction with parent widget", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 3, 6, 9);
    REQUIRE(layout->count() == 0);
    REQUIRE(layout->horizontalSpacing() == 6);
    REQUIRE(layout->verticalSpacing() == 9);
}

// ─── Add / Count / ItemAt ───────────────────────────────────────────────

TEST_CASE("FlowLayout: addWidget increases count", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);

    layout->addWidget(new QPushButton("A"));
    REQUIRE(layout->count() == 1);

    layout->addWidget(new QPushButton("B"));
    layout->addWidget(new QPushButton("C"));
    REQUIRE(layout->count() == 3);
}

TEST_CASE("FlowLayout: itemAt retrieves correct item", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *btn = new QPushButton("Test");
    layout->addWidget(btn);

    QLayoutItem *item = layout->itemAt(0);
    REQUIRE(item != nullptr);
    REQUIRE(item->widget() == btn);
}

TEST_CASE("FlowLayout: itemAt out-of-range returns nullptr", "[qt][flowlayout]") {
    FlowLayout layout;
    REQUIRE(layout.itemAt(0) == nullptr);
    REQUIRE(layout.itemAt(-1) == nullptr);
    REQUIRE(layout.itemAt(100) == nullptr);
}

// ─── TakeAt ─────────────────────────────────────────────────────────────

TEST_CASE("FlowLayout: takeAt removes and returns item", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *btn = new QPushButton("Remove me");
    layout->addWidget(btn);
    REQUIRE(layout->count() == 1);

    QLayoutItem *taken = layout->takeAt(0);
    REQUIRE(taken != nullptr);
    REQUIRE(taken->widget() == btn);
    REQUIRE(layout->count() == 0);
    delete taken;
}

TEST_CASE("FlowLayout: takeAt out-of-range returns nullptr", "[qt][flowlayout]") {
    FlowLayout layout;
    REQUIRE(layout.takeAt(0) == nullptr);
    REQUIRE(layout.takeAt(-1) == nullptr);
}

// ─── MoveLeft / MoveRight ───────────────────────────────────────────────

TEST_CASE("FlowLayout: moveRight swaps adjacent items", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *a = new QPushButton("A");
    auto *b = new QPushButton("B");
    layout->addWidget(a);
    layout->addWidget(b);

    // Move A right → [B, A]
    QLayoutItem *itemA = layout->itemAt(0);
    REQUIRE(layout->moveRight(itemA));

    REQUIRE(layout->itemAt(0)->widget() == b);
    REQUIRE(layout->itemAt(1)->widget() == a);
}

TEST_CASE("FlowLayout: moveRight at end returns false", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *a = new QPushButton("A");
    layout->addWidget(a);

    QLayoutItem *item = layout->itemAt(0);
    REQUIRE_FALSE(layout->moveRight(item)); // already at end
}

TEST_CASE("FlowLayout: moveLeft swaps adjacent items", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *a = new QPushButton("A");
    auto *b = new QPushButton("B");
    layout->addWidget(a);
    layout->addWidget(b);

    // Move B left → [B, A]
    QLayoutItem *itemB = layout->itemAt(1);
    REQUIRE(layout->moveLeft(itemB));

    REQUIRE(layout->itemAt(0)->widget() == b);
    REQUIRE(layout->itemAt(1)->widget() == a);
}

TEST_CASE("FlowLayout: moveLeft at beginning returns false", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *a = new QPushButton("A");
    layout->addWidget(a);

    QLayoutItem *item = layout->itemAt(0);
    REQUIRE_FALSE(layout->moveLeft(item)); // already at start
}

// ─── Layout properties ─────────────────────────────────────────────────

TEST_CASE("FlowLayout: expandingDirections is empty", "[qt][flowlayout]") {
    FlowLayout layout;
    REQUIRE(layout.expandingDirections() == Qt::Orientations());
}

TEST_CASE("FlowLayout: hasHeightForWidth is true", "[qt][flowlayout]") {
    FlowLayout layout;
    REQUIRE(layout.hasHeightForWidth());
}

TEST_CASE("FlowLayout: sizeHint equals minimumSize", "[qt][flowlayout]") {
    FlowLayout layout;
    REQUIRE(layout.sizeHint() == layout.minimumSize());
}

TEST_CASE("FlowLayout: heightForWidth with items", "[qt][flowlayout]") {
    QWidget parent;
    parent.resize(200, 200);
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *btn = new QPushButton("Hello");
    btn->setFixedSize(50, 30);
    layout->addWidget(btn);

    int h = layout->heightForWidth(200);
    REQUIRE(h > 0);
}

// ─── Place ──────────────────────────────────────────────────────────────

TEST_CASE("FlowLayout: place moves widget before target", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *a = new QPushButton("A");
    auto *b = new QPushButton("B");
    auto *c = new QPushButton("C");
    layout->addWidget(a);
    layout->addWidget(b);
    layout->addWidget(c);

    // Place C onto A's position → [C, A, B]
    layout->place(a, c);
    REQUIRE(layout->itemAt(0)->widget() == c);
    REQUIRE(layout->itemAt(1)->widget() == a);
    REQUIRE(layout->itemAt(2)->widget() == b);
}

TEST_CASE("FlowLayout: place with null does nothing", "[qt][flowlayout]") {
    QWidget parent;
    auto *layout = new FlowLayout(&parent, 0, 5, 5);
    auto *a = new QPushButton("A");
    layout->addWidget(a);

    layout->place(nullptr, a); // should not crash
    layout->place(a, nullptr);
    REQUIRE(layout->count() == 1);
}
