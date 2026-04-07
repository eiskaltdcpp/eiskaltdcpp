/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <catch2/catch_test_macros.hpp>
#include "SoundDispatch.h"
#include "NotifyDispatch.h"

#include <cstring>

// ==========================================================================
// Sound dispatch tests
// ==========================================================================

TEST_CASE("SoundDispatch: all valid types have settings keys", "[gtk][sound]")
{
    // All types that should have valid keys
    gtk_sound::TypeSound validTypes[] = {
        gtk_sound::DOWNLOAD_FINISHED,
        gtk_sound::DOWNLOAD_FINISHED_USER_LIST,
        gtk_sound::UPLOAD_FINISHED,
        gtk_sound::PRIVATE_MESSAGE,
        gtk_sound::HUB_CONNECT,
        gtk_sound::HUB_DISCONNECT,
        gtk_sound::FAVORITE_USER_JOIN,
        gtk_sound::FAVORITE_USER_QUIT,
    };

    for (auto type : validTypes) {
        auto info = gtk_sound::soundSettingsKeys(type);
        REQUIRE(info.valid);
        REQUIRE(info.useKey != nullptr);
        REQUIRE(info.fileKey != nullptr);
        REQUIRE(std::strlen(info.useKey) > 0);
        REQUIRE(std::strlen(info.fileKey) > 0);
    }
}

TEST_CASE("SoundDispatch: DOWNLOAD_BEGINS is not yet valid", "[gtk][sound]")
{
    auto info = gtk_sound::soundSettingsKeys(gtk_sound::DOWNLOAD_BEGINS);
    REQUIRE_FALSE(info.valid);
    // Keys are populated but marked not valid
    REQUIRE(info.useKey != nullptr);
}

TEST_CASE("SoundDispatch: NONE returns invalid", "[gtk][sound]")
{
    auto info = gtk_sound::soundSettingsKeys(gtk_sound::SOUND_NONE);
    REQUIRE_FALSE(info.valid);
    REQUIRE(info.useKey == nullptr);
    REQUIRE(info.fileKey == nullptr);
}

TEST_CASE("SoundDispatch: key names match original code", "[gtk][sound]")
{
    SECTION("download finished") {
        auto info = gtk_sound::soundSettingsKeys(gtk_sound::DOWNLOAD_FINISHED);
        REQUIRE(std::string(info.useKey) == "sound-download-finished-use");
        REQUIRE(std::string(info.fileKey) == "sound-download-finished");
    }

    SECTION("private message") {
        auto info = gtk_sound::soundSettingsKeys(gtk_sound::PRIVATE_MESSAGE);
        REQUIRE(std::string(info.useKey) == "sound-private-message-use");
        REQUIRE(std::string(info.fileKey) == "sound-private-message");
    }

    SECTION("hub connect") {
        auto info = gtk_sound::soundSettingsKeys(gtk_sound::HUB_CONNECT);
        REQUIRE(std::string(info.useKey) == "sound-hub-connect-use");
        REQUIRE(std::string(info.fileKey) == "sound-hub-connect");
    }

    SECTION("favorite user join") {
        auto info = gtk_sound::soundSettingsKeys(gtk_sound::FAVORITE_USER_JOIN);
        REQUIRE(std::string(info.useKey) == "sound-fuser-join-use");
        REQUIRE(std::string(info.fileKey) == "sound-fuser-join");
    }
}

TEST_CASE("SoundDispatch: all useKey values end with -use", "[gtk][sound]")
{
    for (int i = 0; i < static_cast<int>(gtk_sound::SOUND_TYPE_COUNT); ++i) {
        auto info = gtk_sound::soundSettingsKeys(static_cast<gtk_sound::TypeSound>(i));
        if (info.valid && info.useKey) {
            std::string key(info.useKey);
            REQUIRE(key.substr(key.size() - 4) == "-use");
        }
    }
}

// ==========================================================================
// Notify dispatch tests
// ==========================================================================

TEST_CASE("NotifyDispatch: all valid types have settings keys", "[gtk][notify]")
{
    gtk_notify::TypeNotify validTypes[] = {
        gtk_notify::DOWNLOAD_FINISHED,
        gtk_notify::DOWNLOAD_FINISHED_USER_LIST,
        gtk_notify::PRIVATE_MESSAGE,
        gtk_notify::HUB_CONNECT,
        gtk_notify::HUB_DISCONNECT,
        gtk_notify::FAVORITE_USER_JOIN,
        gtk_notify::FAVORITE_USER_QUIT,
    };

    for (auto type : validTypes) {
        auto info = gtk_notify::notifySettingsKeys(type);
        REQUIRE(info.valid);
        REQUIRE(info.useKey != nullptr);
        REQUIRE(info.titleKey != nullptr);
        REQUIRE(info.iconKey != nullptr);
    }
}

TEST_CASE("NotifyDispatch: NONE returns invalid", "[gtk][notify]")
{
    auto info = gtk_notify::notifySettingsKeys(gtk_notify::NOTIFY_NONE);
    REQUIRE_FALSE(info.valid);
}

TEST_CASE("NotifyDispatch: key names match original code", "[gtk][notify]")
{
    SECTION("download finished") {
        auto info = gtk_notify::notifySettingsKeys(gtk_notify::DOWNLOAD_FINISHED);
        REQUIRE(std::string(info.useKey) == "notify-download-finished-use");
        REQUIRE(std::string(info.titleKey) == "notify-download-finished-title");
        REQUIRE(std::string(info.iconKey) == "notify-download-finished-icon");
    }

    SECTION("private message") {
        auto info = gtk_notify::notifySettingsKeys(gtk_notify::PRIVATE_MESSAGE);
        REQUIRE(std::string(info.useKey) == "notify-private-message-use");
        REQUIRE(std::string(info.titleKey) == "notify-private-message-title");
        REQUIRE(std::string(info.iconKey) == "notify-private-message-icon");
    }

    SECTION("favorite user join") {
        auto info = gtk_notify::notifySettingsKeys(gtk_notify::FAVORITE_USER_JOIN);
        REQUIRE(std::string(info.useKey) == "notify-fuser-join");
        REQUIRE(std::string(info.titleKey) == "notify-fuser-join-title");
    }
}

// ==========================================================================
// Notify icon size tests
// ==========================================================================

TEST_CASE("NotifyDispatch: iconSizePixels", "[gtk][notify]")
{
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_16) == 16);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_22) == 22);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_24) == 24);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_32) == 32);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_36) == 36);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_48) == 48);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_64) == 64);
    REQUIRE(gtk_notify::iconSizePixels(gtk_notify::ICON_DEFAULT) == 0);
}
