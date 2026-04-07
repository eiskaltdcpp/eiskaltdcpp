/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <catch2/catch_test_macros.hpp>
#include "GtkWulforUtil.h"

TEST_CASE("GtkWulforUtil: splitString", "[gtk][wulforutil]")
{
    SECTION("empty string") {
        auto v = gtk_util::splitString("", ",");
        REQUIRE(v.empty());
    }

    SECTION("empty delimiter") {
        auto v = gtk_util::splitString("1,2,3", "");
        REQUIRE(v.empty());
    }

    SECTION("both empty") {
        auto v = gtk_util::splitString("", "");
        REQUIRE(v.empty());
    }

    SECTION("single value") {
        auto v = gtk_util::splitString("42", ",");
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 42);
    }

    SECTION("multiple values") {
        auto v = gtk_util::splitString("1,2,3,4,5", ",");
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
        REQUIRE(v[3] == 4);
        REQUIRE(v[4] == 5);
    }

    SECTION("multi-char delimiter") {
        auto v = gtk_util::splitString("10::20::30", "::");
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 10);
        REQUIRE(v[1] == 20);
        REQUIRE(v[2] == 30);
    }

    SECTION("non-numeric values become 0") {
        auto v = gtk_util::splitString("abc,def", ",");
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == 0);
        REQUIRE(v[1] == 0);
    }

    SECTION("negative values") {
        auto v = gtk_util::splitString("-1,-2", ",");
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == -1);
        REQUIRE(v[1] == -2);
    }
}

TEST_CASE("GtkWulforUtil: linuxSeparator", "[gtk][wulforutil]")
{
    SECTION("no backslashes") {
        REQUIRE(gtk_util::linuxSeparator("/home/user/file") == "/home/user/file");
    }

    SECTION("backslashes") {
        REQUIRE(gtk_util::linuxSeparator("C:\\Users\\file") == "C:/Users/file");
    }

    SECTION("mixed") {
        REQUIRE(gtk_util::linuxSeparator("C:\\Users/file\\test") == "C:/Users/file/test");
    }

    SECTION("empty") {
        REQUIRE(gtk_util::linuxSeparator("").empty());
    }
}

TEST_CASE("GtkWulforUtil: windowsSeparator", "[gtk][wulforutil]")
{
    SECTION("no forward slashes") {
        REQUIRE(gtk_util::windowsSeparator("C:\\Users\\file") == "C:\\Users\\file");
    }

    SECTION("forward slashes") {
        REQUIRE(gtk_util::windowsSeparator("/home/user/file") == "\\home\\user\\file");
    }

    SECTION("mixed") {
        REQUIRE(gtk_util::windowsSeparator("/home\\user/file") == "\\home\\user\\file");
    }

    SECTION("empty") {
        REQUIRE(gtk_util::windowsSeparator("").empty());
    }
}

TEST_CASE("GtkWulforUtil: isMagnet", "[gtk][wulforutil]")
{
    SECTION("valid magnet") {
        REQUIRE(gtk_util::isMagnet("magnet:?xt=urn:tree:tiger:ABC123"));
    }

    SECTION("case insensitive") {
        REQUIRE(gtk_util::isMagnet("MAGNET:?xt=urn:tree:tiger:ABC123"));
        REQUIRE(gtk_util::isMagnet("Magnet:?xt=urn:tree:tiger:ABC123"));
    }

    SECTION("not a magnet") {
        REQUIRE_FALSE(gtk_util::isMagnet("http://example.com"));
        REQUIRE_FALSE(gtk_util::isMagnet(""));
        REQUIRE_FALSE(gtk_util::isMagnet("magnet"));
    }
}

TEST_CASE("GtkWulforUtil: isLink", "[gtk][wulforutil]")
{
    SECTION("http") {
        REQUIRE(gtk_util::isLink("http://example.com"));
        REQUIRE(gtk_util::isLink("HTTP://EXAMPLE.COM"));
    }

    SECTION("https") {
        REQUIRE(gtk_util::isLink("https://example.com"));
    }

    SECTION("www") {
        REQUIRE(gtk_util::isLink("www.example.com"));
    }

    SECTION("ftp") {
        REQUIRE(gtk_util::isLink("ftp://files.example.com"));
    }

    SECTION("sftp") {
        REQUIRE(gtk_util::isLink("sftp://files.example.com"));
    }

    SECTION("irc") {
        REQUIRE(gtk_util::isLink("irc://irc.freenode.net"));
    }

    SECTION("ircs") {
        REQUIRE(gtk_util::isLink("ircs://irc.freenode.net"));
    }

    SECTION("im") {
        REQUIRE(gtk_util::isLink("im:user@example.com"));
    }

    SECTION("mailto") {
        REQUIRE(gtk_util::isLink("mailto:user@example.com"));
    }

    SECTION("news") {
        REQUIRE(gtk_util::isLink("news:comp.lang.c++"));
    }

    SECTION("not a link") {
        REQUIRE_FALSE(gtk_util::isLink(""));
        REQUIRE_FALSE(gtk_util::isLink("hello world"));
        REQUIRE_FALSE(gtk_util::isLink("magnet:?xt=urn:tree:tiger:ABC"));
        REQUIRE_FALSE(gtk_util::isLink("dchub://hub.example.com"));
    }
}

TEST_CASE("GtkWulforUtil: isHubURL", "[gtk][wulforutil]")
{
    SECTION("dchub") {
        REQUIRE(gtk_util::isHubURL("dchub://hub.example.com"));
    }

    SECTION("nmdcs") {
        REQUIRE(gtk_util::isHubURL("nmdcs://hub.example.com"));
    }

    SECTION("adc") {
        REQUIRE(gtk_util::isHubURL("adc://hub.example.com"));
    }

    SECTION("adcs") {
        REQUIRE(gtk_util::isHubURL("adcs://hub.example.com"));
    }

    SECTION("case insensitive") {
        REQUIRE(gtk_util::isHubURL("DCHUB://hub.example.com"));
        REQUIRE(gtk_util::isHubURL("ADC://hub.example.com"));
    }

    SECTION("not a hub URL") {
        REQUIRE_FALSE(gtk_util::isHubURL(""));
        REQUIRE_FALSE(gtk_util::isHubURL("http://example.com"));
        REQUIRE_FALSE(gtk_util::isHubURL("magnet:?xt=urn:tree:tiger:ABC"));
    }
}

TEST_CASE("GtkWulforUtil: makeMagnet", "[gtk][wulforutil]")
{
    SECTION("empty name") {
        REQUIRE(gtk_util::makeMagnet("", 100, "ABCDEF123456").empty());
    }

    SECTION("empty tth") {
        REQUIRE(gtk_util::makeMagnet("file.txt", 100, "").empty());
    }

    SECTION("simple file") {
        auto m = gtk_util::makeMagnet("file.txt", 1024, "ABCDEF123456");
        REQUIRE(m.find("magnet:?") == 0);
        REQUIRE(m.find("urn:tree:tiger:ABCDEF123456") != std::string::npos);
        REQUIRE(m.find("xl=1024") != std::string::npos);
        REQUIRE(m.find("dn=file.txt") != std::string::npos);
    }

    SECTION("strips path with forward slash") {
        auto m = gtk_util::makeMagnet("/home/user/file.txt", 500, "TTH123");
        REQUIRE(m.find("dn=file.txt") != std::string::npos);
    }

    SECTION("strips path with backslash") {
        auto m = gtk_util::makeMagnet("C:\\Users\\file.txt", 500, "TTH123");
        REQUIRE(m.find("dn=file.txt") != std::string::npos);
    }

    SECTION("zero size") {
        auto m = gtk_util::makeMagnet("test.bin", 0, "TTH456");
        REQUIRE(m.find("xl=0") != std::string::npos);
    }
}

TEST_CASE("GtkWulforUtil: separator round-trip", "[gtk][wulforutil]")
{
    std::string path = "C:\\Users\\test/mixed\\path/file.txt";
    auto linux_path = gtk_util::linuxSeparator(path);
    auto win_path = gtk_util::windowsSeparator(path);

    // All separators should be consistent
    REQUIRE(linux_path.find('\\') == std::string::npos);
    REQUIRE(win_path.find('/') == std::string::npos);

    // Round-trip: linux → windows → linux should give same result
    REQUIRE(gtk_util::linuxSeparator(gtk_util::windowsSeparator(linux_path)) == linux_path);
}
