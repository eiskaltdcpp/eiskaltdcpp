/*
 * Tests for Phase 5 — GtkSettingsModel XML I/O, useDefault parameter,
 * defaults population, and WulforSettingsManager migration.
 */
#include <catch2/catch_test_macros.hpp>
#include "GtkSettingsModel.h"
#include "GtkSettingsDefaults.h"

#include <dcpp/stdinc.h>
#include <dcpp/Util.h>
#include "TestContext.h"

#include <fstream>
#include <cstdio>

using namespace gtk_settings;
using dcpp::test::TestContext;

// ── Helper: create a temp file path ──
static std::string tmpXmlPath()
{
    // Use dcpp config dir which TestContext guarantees exists
    return dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG) + "test_settings.xml";
}

static void removeFile(const std::string &path)
{
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());
}

// ════════════════════════════════════════════════════════════════════
// useDefault parameter
// ════════════════════════════════════════════════════════════════════

TEST_CASE("GtkSettingsModel: getInt useDefault=true returns default even if overridden",
          "[gtk][settings][phase5]")
{
    GtkSettingsModel model;
    model.setDefault("width", 800);
    model.set("width", 1024);

    REQUIRE(model.getInt("width") == 1024);           // normal
    REQUIRE(model.getInt("width", false) == 1024);     // explicit false
    REQUIRE(model.getInt("width", true) == 800);       // useDefault
}

TEST_CASE("GtkSettingsModel: getString useDefault=true returns default even if overridden",
          "[gtk][settings][phase5]")
{
    GtkSettingsModel model;
    model.setDefault("color", std::string("#000000"));
    model.set("color", std::string("#FF0000"));

    REQUIRE(model.getString("color") == "#FF0000");
    REQUIRE(model.getString("color", false) == "#FF0000");
    REQUIRE(model.getString("color", true) == "#000000");
}

TEST_CASE("GtkSettingsModel: getBool useDefault=true",
          "[gtk][settings][phase5]")
{
    GtkSettingsModel model;
    model.setDefault("enabled", 1);
    model.set("enabled", 0);

    REQUIRE(model.getBool("enabled") == false);
    REQUIRE(model.getBool("enabled", true) == true);
}

// ════════════════════════════════════════════════════════════════════
// set(key, bool) overload
// ════════════════════════════════════════════════════════════════════

TEST_CASE("GtkSettingsModel: set bool via int stores 0/1",
          "[gtk][settings][phase5]")
{
    GtkSettingsModel model;
    model.setDefault("flag", 0);

    model.set("flag", 1);  // true
    REQUIRE(model.getInt("flag") == 1);

    model.set("flag", 0);  // false
    REQUIRE(model.getInt("flag") == 0);
}

// ════════════════════════════════════════════════════════════════════
// Bulk operations
// ════════════════════════════════════════════════════════════════════

TEST_CASE("GtkSettingsModel: clearOverrides keeps defaults",
          "[gtk][settings][phase5]")
{
    GtkSettingsModel model;
    model.setDefault("width", 800);
    model.set("width", 1024);
    model.setDefault("color", std::string("#000"));
    model.set("color", std::string("#FFF"));

    model.clearOverrides();

    REQUIRE(model.getInt("width") == 800);        // falls back to default
    REQUIRE(model.getString("color") == "#000");   // falls back to default
    REQUIRE(model.hasDefault("width"));            // defaults still present
}

TEST_CASE("GtkSettingsModel: clear removes everything",
          "[gtk][settings][phase5]")
{
    GtkSettingsModel model;
    model.setDefault("width", 800);
    model.set("width", 1024);
    model.addPreviewApp("test", "app", "ext");

    model.clear();

    REQUIRE(model.getInt("width") == 0);       // no default, no override
    REQUIRE_FALSE(model.hasDefault("width"));
    REQUIRE(model.previewAppCount() == 0);
}

// ════════════════════════════════════════════════════════════════════
// XML round-trip
// ════════════════════════════════════════════════════════════════════

TEST_CASE("GtkSettingsModel: save and load XML round-trip",
          "[gtk][settings][phase5][xml]")
{
    TestContext ctx;
    std::string path = tmpXmlPath();
    removeFile(path);

    // Create model with defaults and overrides
    GtkSettingsModel original;
    original.setDefault("width", 800);
    original.setDefault("height", 600);
    original.setDefault("color", std::string("#000000"));
    original.setDefault("name", std::string("default"));

    original.set("width", 1024);
    original.set("color", std::string("#FF0000"));
    original.addPreviewApp("VLC", "vlc %f", "avi;mkv");
    original.addPreviewApp("MPV", "mpv %f", "mp4;webm");

    REQUIRE(original.saveToXml(path));

    // Load into a new model with the same defaults
    GtkSettingsModel loaded;
    loaded.setDefault("width", 800);
    loaded.setDefault("height", 600);
    loaded.setDefault("color", std::string("#000000"));
    loaded.setDefault("name", std::string("default"));

    REQUIRE(loaded.loadFromXml(path));

    // Overridden values loaded
    REQUIRE(loaded.getInt("width") == 1024);
    REQUIRE(loaded.getString("color") == "#FF0000");

    // Non-overridden values fall back to default
    REQUIRE(loaded.getInt("height") == 600);
    REQUIRE(loaded.getString("name") == "default");

    // Preview apps loaded
    REQUIRE(loaded.previewAppCount() == 2);
    auto *vlc = loaded.findPreviewApp("VLC");
    REQUIRE(vlc != nullptr);
    REQUIRE(vlc->application == "vlc %f");
    REQUIRE(vlc->extension == "avi;mkv");

    auto *mpv = loaded.findPreviewApp("MPV");
    REQUIRE(mpv != nullptr);
    REQUIRE(mpv->application == "mpv %f");

    removeFile(path);
}

TEST_CASE("GtkSettingsModel: loadFromXml ignores keys without defaults",
          "[gtk][settings][phase5][xml]")
{
    TestContext ctx;
    std::string path = tmpXmlPath();
    removeFile(path);

    // Save with extra keys
    GtkSettingsModel writer;
    writer.setDefault("known-int", 0);
    writer.setDefault("extra-int", 0);
    writer.setDefault("known-str", std::string(""));
    writer.setDefault("extra-str", std::string(""));
    writer.set("known-int", 42);
    writer.set("extra-int", 99);
    writer.set("known-str", std::string("hello"));
    writer.set("extra-str", std::string("world"));
    REQUIRE(writer.saveToXml(path));

    // Load with only some defaults defined
    GtkSettingsModel reader;
    reader.setDefault("known-int", 0);
    reader.setDefault("known-str", std::string(""));
    // "extra-int" and "extra-str" have no defaults — should be skipped

    REQUIRE(reader.loadFromXml(path));
    REQUIRE(reader.getInt("known-int") == 42);
    REQUIRE(reader.getString("known-str") == "hello");
    REQUIRE(reader.getInt("extra-int") == 0);      // no default, returns 0
    REQUIRE(reader.getString("extra-str").empty()); // no default, returns empty

    removeFile(path);
}

TEST_CASE("GtkSettingsModel: loadFromXml returns false for missing file",
          "[gtk][settings][phase5][xml]")
{
    GtkSettingsModel model;
    REQUIRE_FALSE(model.loadFromXml("/tmp/nonexistent_settings_file.xml"));
}

TEST_CASE("GtkSettingsModel: saveToXml atomic write (tmp file cleaned up)",
          "[gtk][settings][phase5][xml]")
{
    TestContext ctx;
    std::string path = tmpXmlPath();
    removeFile(path);

    GtkSettingsModel model;
    model.setDefault("x", 1);
    model.set("x", 2);
    REQUIRE(model.saveToXml(path));

    // The .tmp file should not remain
    std::ifstream tmpCheck(path + ".tmp");
    REQUIRE_FALSE(tmpCheck.good());

    // The main file should exist
    std::ifstream mainCheck(path);
    REQUIRE(mainCheck.good());

    removeFile(path);
}

// ════════════════════════════════════════════════════════════════════
// Defaults population
// ════════════════════════════════════════════════════════════════════

TEST_CASE("populateDefaults: correct number of int defaults",
          "[gtk][settings][phase5][defaults]")
{
    GtkSettingsModel model;
    populateDefaults(model);

    auto keys = model.intKeys();
    REQUIRE(keys.size() == EXPECTED_INT_DEFAULT_COUNT);
}

TEST_CASE("populateDefaults: correct number of string defaults",
          "[gtk][settings][phase5][defaults]")
{
    GtkSettingsModel model;
    populateDefaults(model);

    auto keys = model.stringKeys();
    REQUIRE(keys.size() == EXPECTED_STRING_DEFAULT_COUNT);
}

TEST_CASE("populateDefaults: spot-check int values",
          "[gtk][settings][phase5][defaults]")
{
    GtkSettingsModel model;
    populateDefaults(model);

    REQUIRE(model.getInt("main-window-size-x") == 875);
    REQUIRE(model.getInt("main-window-size-y") == 685);
    REQUIRE(model.getInt("main-window-pos-x") == 100);
    REQUIRE(model.getInt("transfer-pane-position") == 204);
    REQUIRE(model.getInt("nick-pane-position") == 255);
    REQUIRE(model.getInt("sound-pm") == 1);
    REQUIRE(model.getInt("text-bold-autors") == 1);
    REQUIRE(model.getInt("magnet-action") == -1);
    REQUIRE(model.getInt("notify-pm-length") == 50);
    REQUIRE(model.getInt("search-spy-frame") == 50);
    REQUIRE(model.getInt("emoticons-use") == 1);
}

TEST_CASE("populateDefaults: spot-check string values",
          "[gtk][settings][phase5][defaults]")
{
    GtkSettingsModel model;
    populateDefaults(model, "/downloads/", "UTF-8");

    REQUIRE(model.getString("magnet-choose-dir") == "/downloads/");
    REQUIRE(model.getString("default-charset") == "UTF-8");
    REQUIRE(model.getString("sound-command") == "aplay -q");
    REQUIRE(model.getString("text-general-fore-color") == "#4D4D4D");
    REQUIRE(model.getString("text-myown-fore-color") == "#207505");
    REQUIRE(model.getString("emoticons-icon-size") == "24x24");
    REQUIRE(model.getString("theme-name") == "default");
    REQUIRE(model.getString("icon-file") == "gtk-file");
    REQUIRE(model.getString("icon-quit") == "eiskaltdcpp-application-exit");
}

TEST_CASE("populateDefaults: default encodingLocale is 'System default'",
          "[gtk][settings][phase5][defaults]")
{
    GtkSettingsModel model;
    populateDefaults(model);  // no args — uses defaults

    REQUIRE(model.getString("default-charset") == "System default");
    REQUIRE(model.getString("magnet-choose-dir").empty());
}

TEST_CASE("populateDefaults: notification titles are English",
          "[gtk][settings][phase5][defaults]")
{
    GtkSettingsModel model;
    populateDefaults(model);

    REQUIRE(model.getString("notify-download-finished-title") == "Download finished");
    REQUIRE(model.getString("notify-private-message-title") == "Private message");
    REQUIRE(model.getString("notify-hub-disconnect-title") == "Hub disconnected");
    REQUIRE(model.getString("notify-hub-connect-title") == "Hub connected");
    REQUIRE(model.getString("notify-fuser-join-title") == "Favorite user joined");
    REQUIRE(model.getString("notify-fuser-quit-title") == "Favorite user quit");
}

// ════════════════════════════════════════════════════════════════════
// Full round-trip with populated defaults
// ════════════════════════════════════════════════════════════════════

TEST_CASE("Full round-trip: populate defaults, override, save, load",
          "[gtk][settings][phase5][xml][defaults]")
{
    TestContext ctx;
    std::string path = tmpXmlPath();
    removeFile(path);

    // Simulate real startup: populate defaults, then override some
    GtkSettingsModel original;
    populateDefaults(original, "/dl/", "UTF-8");
    original.set("main-window-size-x", 1920);
    original.set("main-window-size-y", 1080);
    original.set("text-general-fore-color", std::string("#123456"));
    original.addPreviewApp("Video", "vlc", "avi;mp4");

    REQUIRE(original.saveToXml(path));

    // Simulate restart: populate defaults fresh, then load
    GtkSettingsModel reloaded;
    populateDefaults(reloaded, "/dl/", "UTF-8");
    REQUIRE(reloaded.loadFromXml(path));

    // Overridden values
    REQUIRE(reloaded.getInt("main-window-size-x") == 1920);
    REQUIRE(reloaded.getInt("main-window-size-y") == 1080);
    REQUIRE(reloaded.getString("text-general-fore-color") == "#123456");

    // Non-overridden: fall back to defaults
    REQUIRE(reloaded.getInt("main-window-pos-x") == 100);
    REQUIRE(reloaded.getString("sound-command") == "aplay -q");

    // useDefault still works
    REQUIRE(reloaded.getInt("main-window-size-x", true) == 875);

    // Preview app
    REQUIRE(reloaded.previewAppCount() == 1);
    REQUIRE(reloaded.findPreviewApp("Video") != nullptr);

    removeFile(path);
}

TEST_CASE("XML compatibility: model can read WulforSettingsManager-format XML",
          "[gtk][settings][phase5][xml]")
{
    TestContext ctx;
    std::string path = tmpXmlPath();
    removeFile(path);

    // Write a minimal XML in WulforSettingsManager format
    {
        std::ofstream f(path);
        f << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n"
          << "<EiskaltDC++_Gtk>\n"
          << "  <Settings>\n"
          << "    <main-window-size-x type=\"int\">1600</main-window-size-x>\n"
          << "    <text-general-fore-color type=\"string\">#AABBCC</text-general-fore-color>\n"
          << "  </Settings>\n"
          << "  <PreviewApps>\n"
          << "    <Application Name=\"Test\" Application=\"test %f\" Extension=\"txt\"/>\n"
          << "  </PreviewApps>\n"
          << "</EiskaltDC++_Gtk>\n";
    }

    GtkSettingsModel model;
    populateDefaults(model);
    REQUIRE(model.loadFromXml(path));

    REQUIRE(model.getInt("main-window-size-x") == 1600);
    REQUIRE(model.getString("text-general-fore-color") == "#AABBCC");
    REQUIRE(model.previewAppCount() == 1);
    REQUIRE(model.findPreviewApp("Test")->application == "test %f");

    removeFile(path);
}
