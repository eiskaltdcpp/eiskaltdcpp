/*
 * Tests for GtkSettingsModel — Phase 3 stateful model.
 */
#include <catch2/catch_test_macros.hpp>
#include "GtkSettingsModel.h"

using namespace gtk_settings;

// ── Int settings ──

TEST_CASE("GtkSettingsModel: set and get int", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("window-width", 800);
    model.set("window-width", 1024);
    REQUIRE(model.getInt("window-width") == 1024);
}

TEST_CASE("GtkSettingsModel: int falls back to default", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("window-width", 800);
    REQUIRE(model.getInt("window-width") == 800);
}

TEST_CASE("GtkSettingsModel: getBool", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("show-toolbar", 1);
    REQUIRE(model.getBool("show-toolbar") == true);

    model.set("show-toolbar", 0);
    REQUIRE(model.getBool("show-toolbar") == false);
}

TEST_CASE("GtkSettingsModel: getInt unknown key returns 0", "[gtk][settings]")
{
    GtkSettingsModel model;
    REQUIRE(model.getInt("nonexistent") == 0);
}

// ── String settings ──

TEST_CASE("GtkSettingsModel: set and get string", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("chat-color", "#000000");
    model.set("chat-color", "#FF0000");
    REQUIRE(model.getString("chat-color") == "#FF0000");
}

TEST_CASE("GtkSettingsModel: string falls back to default", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("chat-color", "#000000");
    REQUIRE(model.getString("chat-color") == "#000000");
}

TEST_CASE("GtkSettingsModel: getString unknown key returns empty", "[gtk][settings]")
{
    GtkSettingsModel model;
    REQUIRE(model.getString("nonexistent").empty());
}

// ── Defaults management ──

TEST_CASE("GtkSettingsModel: hasDefault", "[gtk][settings]")
{
    GtkSettingsModel model;
    REQUIRE_FALSE(model.hasDefault("width"));
    model.setDefault("width", 800);
    REQUIRE(model.hasDefault("width"));
}

TEST_CASE("GtkSettingsModel: intKeys and stringKeys", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("width", 800);
    model.setDefault("height", 600);
    model.setDefault("color", std::string("#000"));

    REQUIRE(model.intKeys().size() == 2);
    REQUIRE(model.stringKeys().size() == 1);
}

// ── Preview apps ──

TEST_CASE("GtkSettingsModel: add preview app", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.addPreviewApp("Video Player", "vlc", "avi;mkv;mp4");
    REQUIRE(model.previewAppCount() == 1);

    auto app = model.findPreviewApp("Video Player");
    REQUIRE(app != nullptr);
    REQUIRE(app->application == "vlc");
    REQUIRE(app->extension == "avi;mkv;mp4");
}

TEST_CASE("GtkSettingsModel: remove preview app", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.addPreviewApp("VLC", "vlc", "avi");
    model.addPreviewApp("MPV", "mpv", "mkv");
    REQUIRE(model.removePreviewApp("VLC"));
    REQUIRE(model.previewAppCount() == 1);
    REQUIRE(model.findPreviewApp("VLC") == nullptr);
}

TEST_CASE("GtkSettingsModel: remove non-existent preview app", "[gtk][settings]")
{
    GtkSettingsModel model;
    REQUIRE_FALSE(model.removePreviewApp("Nope"));
}

TEST_CASE("GtkSettingsModel: update preview app", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.addPreviewApp("VLC", "vlc", "avi");
    REQUIRE(model.updatePreviewApp("VLC", "VLC Media", "cvlc", "avi;mkv"));

    auto app = model.findPreviewApp("VLC Media");
    REQUIRE(app != nullptr);
    REQUIRE(app->application == "cvlc");
    REQUIRE(app->extension == "avi;mkv");
    REQUIRE(model.findPreviewApp("VLC") == nullptr);
}

TEST_CASE("GtkSettingsModel: previewApps returns all", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.addPreviewApp("VLC", "vlc", "avi");
    model.addPreviewApp("MPV", "mpv", "mkv");

    const auto &apps = model.previewApps();
    REQUIRE(apps.size() == 2);
}

// ── parseCmd ──

TEST_CASE("GtkSettingsModel: parseCmd get int", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("width", 800);
    REQUIRE(model.parseCmd("width") == "800");
}

TEST_CASE("GtkSettingsModel: parseCmd set int", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("width", 800);
    model.parseCmd("width 1024");
    REQUIRE(model.getInt("width") == 1024);
}

TEST_CASE("GtkSettingsModel: parseCmd get string", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("color", std::string("#000"));
    REQUIRE(model.parseCmd("color") == "#000");
}

TEST_CASE("GtkSettingsModel: parseCmd set string", "[gtk][settings]")
{
    GtkSettingsModel model;
    model.setDefault("color", std::string("#000"));
    model.parseCmd("color #FF0000");
    REQUIRE(model.getString("color") == "#FF0000");
}

TEST_CASE("GtkSettingsModel: parseCmd unknown key", "[gtk][settings]")
{
    GtkSettingsModel model;
    std::string result = model.parseCmd("nonexistent");
    REQUIRE(result.find("Unknown") != std::string::npos);
}

// ── Constants ──

TEST_CASE("GtkSettingsModel: text weight constants", "[gtk][settings]")
{
    REQUIRE(TEXT_WEIGHT_NORMAL == 400);
    REQUIRE(TEXT_WEIGHT_BOLD == 700);
    REQUIRE(TEXT_STYLE_NORMAL == 0);
    REQUIRE(TEXT_STYLE_ITALIC == 2);
}
