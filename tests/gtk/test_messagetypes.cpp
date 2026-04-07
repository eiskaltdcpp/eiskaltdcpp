/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <catch2/catch_test_macros.hpp>
#include "GtkMessageTypes.h"

TEST_CASE("GtkMessageTypes: TypeMsg values", "[gtk][messagetypes]")
{
    SECTION("enum values are sequential") {
        REQUIRE(gtk_msg::MSG_GENERAL == 0);
        REQUIRE(gtk_msg::MSG_PRIVATE == 1);
        REQUIRE(gtk_msg::MSG_MYOWN == 2);
        REQUIRE(gtk_msg::MSG_SYSTEM == 3);
        REQUIRE(gtk_msg::MSG_STATUS == 4);
        REQUIRE(gtk_msg::MSG_FAVORITE == 5);
        REQUIRE(gtk_msg::MSG_OPERATOR == 6);
        REQUIRE(gtk_msg::MSG_UNKNOWN == 7);
        REQUIRE(gtk_msg::MSG_COUNT == 8);
    }

    SECTION("msgTypeCount returns correct count") {
        REQUIRE(gtk_msg::msgTypeCount() == 8);
    }
}

TEST_CASE("GtkMessageTypes: TypeTag values", "[gtk][messagetypes]")
{
    SECTION("TAG_FIRST is 0") {
        REQUIRE(gtk_msg::TAG_FIRST == 0);
    }

    SECTION("TAG_GENERAL equals TAG_FIRST") {
        REQUIRE(gtk_msg::TAG_GENERAL == gtk_msg::TAG_FIRST);
    }

    SECTION("TAG_LAST is the sentinel") {
        REQUIRE(gtk_msg::TAG_LAST > gtk_msg::TAG_FIRST);
    }

    SECTION("tagTypeCount returns TAG_LAST") {
        REQUIRE(gtk_msg::tagTypeCount() == static_cast<int>(gtk_msg::TAG_LAST));
    }

    SECTION("all tag values are unique") {
        // Collect all tag values
        int tags[] = {
            gtk_msg::TAG_GENERAL, gtk_msg::TAG_PRIVATE, gtk_msg::TAG_MYOWN,
            gtk_msg::TAG_SYSTEM, gtk_msg::TAG_STATUS, gtk_msg::TAG_TIMESTAMP,
            gtk_msg::TAG_MYNICK, gtk_msg::TAG_NICK, gtk_msg::TAG_OPERATOR,
            gtk_msg::TAG_FAVORITE, gtk_msg::TAG_URL
        };
        constexpr int n = sizeof(tags) / sizeof(tags[0]);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                REQUIRE(tags[i] != tags[j]);
            }
        }
    }
}
