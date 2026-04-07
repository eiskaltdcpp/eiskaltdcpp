/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Unit tests for dcpp::ScopedFunctor (RAII scope guard)
 */

#include <catch2/catch_test_macros.hpp>

#include "ScopedFunctor.h"

using namespace dcpp;

TEST_CASE("ScopedFunctor: runs functor on scope exit", "[ScopedFunctor]") {
    int counter = 0;
    {
        auto guard = makeScopedFunctor([&counter]() { counter++; });
        REQUIRE(counter == 0);
    }
    REQUIRE(counter == 1);
}

TEST_CASE("ScopedFunctor: multiple guards in same scope", "[ScopedFunctor]") {
    std::string order;
    {
        auto g1 = makeScopedFunctor([&order]() { order += "1"; });
        auto g2 = makeScopedFunctor([&order]() { order += "2"; });
        auto g3 = makeScopedFunctor([&order]() { order += "3"; });
    }
    // Destructors run in reverse order of construction
    REQUIRE(order == "321");
}

TEST_CASE("ScopedFunctor: modifies external state", "[ScopedFunctor]") {
    bool cleaned = false;
    {
        auto guard = makeScopedFunctor([&cleaned]() { cleaned = true; });
        REQUIRE_FALSE(cleaned);
    }
    REQUIRE(cleaned);
}

TEST_CASE("ScopedFunctor: macro generates unique names", "[ScopedFunctor]") {
    int count = 0;
    {
        ScopedFunctor([&count]() { count++; });
        ScopedFunctor([&count]() { count += 10; });
    }
    REQUIRE(count == 11);
}

TEST_CASE("ScopedFunctor: works with function pointer", "[ScopedFunctor]") {
    static int staticCounter = 0;
    staticCounter = 0;

    struct Helper {
        static void increment() { staticCounter++; }
    };

    {
        auto guard = makeScopedFunctor(&Helper::increment);
    }
    REQUIRE(staticCounter == 1);
}
