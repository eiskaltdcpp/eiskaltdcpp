/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::intrusive_ptr (custom smart pointer with ref counting)
 */

#include <catch2/catch_test_macros.hpp>

#include "intrusive_ptr.h"

#include <atomic>
#include <functional>
#include <utility>

using namespace dcpp;

// ── Test reference-counted object ───────────────────────────────────────

namespace {

struct RefCounted {
    std::atomic<int> refCount{0};
    int value;

    explicit RefCounted(int v) : value(v) {}
};

void intrusive_ptr_add_ref(RefCounted* p) {
    p->refCount.fetch_add(1, std::memory_order_relaxed);
}

void intrusive_ptr_release(RefCounted* p) {
    if (p->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete p;
    }
}

} // anonymous namespace

// ── Construction ────────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: default construct is null", "[intrusive_ptr]") {
    intrusive_ptr<RefCounted> p;
    REQUIRE(p.get() == nullptr);
    REQUIRE_FALSE(p);
    REQUIRE(p == nullptr);
}

TEST_CASE("intrusive_ptr: construct from raw pointer adds ref", "[intrusive_ptr]") {
    auto* raw = new RefCounted(42);
    {
        intrusive_ptr<RefCounted> p(raw);
        REQUIRE(p.get() == raw);
        REQUIRE(raw->refCount == 1);
        REQUIRE(p->value == 42);
        REQUIRE((*p).value == 42);
    }
    // destructor should have released and deleted
}

TEST_CASE("intrusive_ptr: construct with add_ref=false", "[intrusive_ptr]") {
    auto* raw = new RefCounted(10);
    raw->refCount = 1; // manually set
    {
        intrusive_ptr<RefCounted> p(raw, false); // don't add ref
        REQUIRE(raw->refCount == 1);
    }
    // destructor calls release, refCount goes to 0 → deleted
}

// ── Copy ────────────────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: copy constructor increments refcount", "[intrusive_ptr]") {
    auto* raw = new RefCounted(1);
    intrusive_ptr<RefCounted> p1(raw);
    REQUIRE(raw->refCount == 1);

    intrusive_ptr<RefCounted> p2(p1);
    REQUIRE(raw->refCount == 2);
    REQUIRE(p1.get() == p2.get());
}

TEST_CASE("intrusive_ptr: copy assignment", "[intrusive_ptr]") {
    auto* a = new RefCounted(1);
    auto* b = new RefCounted(2);

    intrusive_ptr<RefCounted> pa(a);
    intrusive_ptr<RefCounted> pb(b);

    pa = pb;
    REQUIRE(pa.get() == b);
    REQUIRE(b->refCount == 2);
    // a should have been released (deleted since refcount was 1)
}

// ── Move ────────────────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: move constructor", "[intrusive_ptr]") {
    auto* raw = new RefCounted(5);
    intrusive_ptr<RefCounted> p1(raw);
    REQUIRE(raw->refCount == 1);

    intrusive_ptr<RefCounted> p2(std::move(p1));
    REQUIRE(p1.get() == nullptr);  // NOLINT: testing moved-from state
    REQUIRE(p2.get() == raw);
    REQUIRE(raw->refCount == 1); // no extra ref
}

TEST_CASE("intrusive_ptr: move assignment", "[intrusive_ptr]") {
    auto* raw = new RefCounted(7);
    intrusive_ptr<RefCounted> p1(raw);
    intrusive_ptr<RefCounted> p2;

    p2 = std::move(p1);
    REQUIRE(p1.get() == nullptr);  // NOLINT
    REQUIRE(p2.get() == raw);
    REQUIRE(raw->refCount == 1);
}

// ── Assignment from raw pointer ─────────────────────────────────────────

TEST_CASE("intrusive_ptr: assign from raw pointer", "[intrusive_ptr]") {
    auto* raw = new RefCounted(99);
    intrusive_ptr<RefCounted> p;

    p = raw;
    REQUIRE(p.get() == raw);
    REQUIRE(raw->refCount == 1);
}

// ── Reset ───────────────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: reset to null", "[intrusive_ptr]") {
    auto* raw = new RefCounted(3);
    intrusive_ptr<RefCounted> p(raw);
    REQUIRE(raw->refCount == 1);

    p.reset();
    REQUIRE(p.get() == nullptr);
    REQUIRE_FALSE(p);
}

TEST_CASE("intrusive_ptr: reset to new pointer", "[intrusive_ptr]") {
    auto* a = new RefCounted(1);
    auto* b = new RefCounted(2);

    intrusive_ptr<RefCounted> p(a);
    p.reset(b);

    REQUIRE(p.get() == b);
    REQUIRE(b->refCount == 1);
    // a was released (deleted)
}

// ── Swap ────────────────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: swap", "[intrusive_ptr]") {
    auto* a = new RefCounted(1);
    auto* b = new RefCounted(2);

    intrusive_ptr<RefCounted> pa(a);
    intrusive_ptr<RefCounted> pb(b);

    pa.swap(pb);

    REQUIRE(pa.get() == b);
    REQUIRE(pb.get() == a);
    REQUIRE(a->refCount == 1);
    REQUIRE(b->refCount == 1);
}

// ── Comparison operators ────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: equality comparison", "[intrusive_ptr]") {
    auto* raw = new RefCounted(1);
    intrusive_ptr<RefCounted> p1(raw);
    intrusive_ptr<RefCounted> p2(p1);

    REQUIRE(p1 == p2);
    REQUIRE(p1 == raw);
    REQUIRE(raw == p1);
    REQUIRE_FALSE(p1 != p2);
}

TEST_CASE("intrusive_ptr: inequality comparison", "[intrusive_ptr]") {
    auto* a = new RefCounted(1);
    auto* b = new RefCounted(2);

    intrusive_ptr<RefCounted> pa(a);
    intrusive_ptr<RefCounted> pb(b);

    REQUIRE(pa != pb);
    REQUIRE(pa != b);
    REQUIRE(a != pb);
}

TEST_CASE("intrusive_ptr: nullptr comparison", "[intrusive_ptr]") {
    intrusive_ptr<RefCounted> null_ptr;
    intrusive_ptr<RefCounted> valid(new RefCounted(1));

    REQUIRE(null_ptr == nullptr);
    REQUIRE(nullptr == null_ptr);
    REQUIRE_FALSE(valid == nullptr);
    REQUIRE(valid != nullptr);
    REQUIRE(nullptr != valid);
}

// ── Less-than (for ordered containers) ──────────────────────────────────

TEST_CASE("intrusive_ptr: less-than operator", "[intrusive_ptr]") {
    auto* a = new RefCounted(1);
    auto* b = new RefCounted(2);

    intrusive_ptr<RefCounted> pa(a);
    intrusive_ptr<RefCounted> pb(b);

    // One must be less than the other
    REQUIRE((pa < pb) != (pb < pa));
    // Not less than self
    REQUIRE_FALSE(pa < pa);
}

// ── hash_value ──────────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: hash_value", "[intrusive_ptr]") {
    auto* raw = new RefCounted(1);
    intrusive_ptr<RefCounted> p(raw);

    size_t h = hash_value(p);
    REQUIRE(h == std::hash<RefCounted*>()(raw));
}

TEST_CASE("intrusive_ptr: hash_value of null", "[intrusive_ptr]") {
    intrusive_ptr<RefCounted> p;
    REQUIRE(hash_value(p) == std::hash<RefCounted*>()(nullptr));
}

// ── Bool conversion ─────────────────────────────────────────────────────

TEST_CASE("intrusive_ptr: bool conversion", "[intrusive_ptr]") {
    intrusive_ptr<RefCounted> null_ptr;
    intrusive_ptr<RefCounted> valid(new RefCounted(1));

    REQUIRE_FALSE(static_cast<bool>(null_ptr));
    REQUIRE(static_cast<bool>(valid));

    if (valid) {
        SUCCEED("valid pointer is truthy");
    } else {
        FAIL("valid pointer should be truthy");
    }
}
