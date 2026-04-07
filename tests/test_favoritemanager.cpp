/*
 * Tests for FavoriteManager — hub CRUD, directory CRUD, user commands, save/load.
 * Requires TestContext. FavoriteManager is constructed directly since the
 * constructor now null-guards the ClientManager listener.
 */

#include <catch2/catch_test_macros.hpp>

#include "stdinc.h"
#include "FavoriteManager.h"
#include "FavoriteManagerListener.h"
#include "SettingsManager.h"
#include "DCPlusPlus.h"
#include "Util.h"
#include "TestContext.h"

#include <cstdlib>
#include <filesystem>

using namespace dcpp;

// Shared test context
static std::unique_ptr<dcpp::test::TestContext> g_tc;
static std::unique_ptr<FavoriteManager> g_fm;

// atexit callback: destroy FavoriteManager while DCContext is still alive.
// Registered AFTER TestContext's atexit, so it runs FIRST (LIFO order).
static void cleanupFavoriteManager() {
    g_fm.reset();
}

static void ensureContext() {
    if (!g_tc) {
        g_tc = std::make_unique<dcpp::test::TestContext>();
        // Construct FavoriteManager with context reference (minimal context has no ClientManager,
        // but the constructor null-guards the listener registration).
        // Saves go to temp dir automatically via TestContext path overrides.
        g_fm = std::make_unique<FavoriteManager>(*getContext());
        // Register cleanup AFTER TestContext's atexit → runs FIRST (LIFO).
        std::atexit(cleanupFavoriteManager);
    }
}

// ─── Favorite Hubs ──────────────────────────────────────────────────────

TEST_CASE("FavoriteManager: addFavorite hub", "[favoritemanager]") {
    ensureContext();

    FavoriteHubEntry entry(*getContext());
    entry.setServer("adc://test-hub.example.com");
    entry.setName("Test Hub");
    entry.setHubDescription("A test hub");

    g_fm->addFavorite(entry);
    REQUIRE(g_fm->isFavoriteHub("adc://test-hub.example.com") == true);
}

TEST_CASE("FavoriteManager: isFavoriteHub false for unknown", "[favoritemanager]") {
    ensureContext();

    REQUIRE(g_fm->isFavoriteHub("adc://nonexistent.example.com") == false);
}

TEST_CASE("FavoriteManager: addFavorite no duplicate", "[favoritemanager]") {
    ensureContext();

    FavoriteHubEntry entry(*getContext());
    entry.setServer("adc://dup-hub.example.com");
    entry.setName("Dup Hub");
    g_fm->addFavorite(entry);

    auto sizeBefore = g_fm->getFavoriteHubs().size();

    // Try adding again with same server
    g_fm->addFavorite(entry);
    REQUIRE(g_fm->getFavoriteHubs().size() == sizeBefore); // no duplicate
}

TEST_CASE("FavoriteManager: removeFavorite hub", "[favoritemanager]") {
    ensureContext();

    FavoriteHubEntry entry(*getContext());
    entry.setServer("adc://remove-me.example.com");
    entry.setName("Remove Me");
    g_fm->addFavorite(entry);

    REQUIRE(g_fm->isFavoriteHub("adc://remove-me.example.com") == true);

    // Find and remove
    auto* found = g_fm->getFavoriteHubEntry("adc://remove-me.example.com");
    REQUIRE(found != nullptr);
    g_fm->removeFavorite(found);

    REQUIRE(g_fm->isFavoriteHub("adc://remove-me.example.com") == false);
}

TEST_CASE("FavoriteManager: getFavoriteHubEntry", "[favoritemanager]") {
    ensureContext();

    FavoriteHubEntry entry(*getContext());
    entry.setServer("adc://entry-test.example.com");
    entry.setName("Entry Test");
    entry.setNick("myNick");
    g_fm->addFavorite(entry);

    auto* found = g_fm->getFavoriteHubEntry("adc://entry-test.example.com");
    REQUIRE(found != nullptr);
    REQUIRE(found->getName() == "Entry Test");
}

// ─── Favorite Hub Groups ────────────────────────────────────────────────

TEST_CASE("FavoriteManager: hub groups CRUD", "[favoritemanager]") {
    ensureContext();

    FavHubGroups groups;
    FavHubGroupProperties props;
    props.priv = true;
    props.connect = false;
    groups["TestGroup"] = props;
    g_fm->setFavHubGroups(groups);

    auto retrieved = g_fm->getFavHubGroups();
    REQUIRE(retrieved.count("TestGroup") == 1);
    REQUIRE(retrieved["TestGroup"].priv == true);
    REQUIRE(retrieved["TestGroup"].connect == false);
}

// Helper: build a directory path string with native separators + trailing separator
static std::string nativeDirPath(const std::string& name) {
    auto p = std::filesystem::temp_directory_path() / name;
    auto s = p.make_preferred().string();
    if (!s.empty() && s.back() != PATH_SEPARATOR)
        s += PATH_SEPARATOR;
    return s;
}

// ─── Favorite Directories ───────────────────────────────────────────────

TEST_CASE("FavoriteManager: addFavoriteDir", "[favoritemanager]") {
    ensureContext();

    bool added = g_fm->addFavoriteDir(nativeDirPath("share1"), "Share One");
    REQUIRE(added == true);

    auto dirs = g_fm->getFavoriteDirs();
    bool found = false;
    for (auto& d : dirs) {
        if (d.second == "Share One") {
            found = true;
            break;
        }
    }
    REQUIRE(found == true);
}

TEST_CASE("FavoriteManager: addFavoriteDir no duplicate name", "[favoritemanager]") {
    ensureContext();

    g_fm->addFavoriteDir(nativeDirPath("unique_dir1"), "UniqueName");
    bool added = g_fm->addFavoriteDir(nativeDirPath("unique_dir2"), "UniqueName");
    REQUIRE(added == false); // same name
}

TEST_CASE("FavoriteManager: removeFavoriteDir", "[favoritemanager]") {
    ensureContext();

    std::string dirPath = nativeDirPath("removable");
    g_fm->addFavoriteDir(dirPath, "Removable");
    bool removed = g_fm->removeFavoriteDir(dirPath);
    REQUIRE(removed == true);

    bool removedAgain = g_fm->removeFavoriteDir(dirPath);
    REQUIRE(removedAgain == false); // already removed
}

TEST_CASE("FavoriteManager: renameFavoriteDir", "[favoritemanager]") {
    ensureContext();

    g_fm->addFavoriteDir(nativeDirPath("renamedir"), "OldName");
    bool renamed = g_fm->renameFavoriteDir("OldName", "NewName");
    REQUIRE(renamed == true);

    auto dirs = g_fm->getFavoriteDirs();
    bool foundNew = false;
    for (auto& d : dirs) {
        if (d.second == "NewName") {
            foundNew = true;
            break;
        }
    }
    REQUIRE(foundNew == true);
}

// ─── User Commands ──────────────────────────────────────────────────────

TEST_CASE("FavoriteManager: addUserCommand", "[favoritemanager]") {
    ensureContext();

    UserCommand uc = g_fm->addUserCommand(
        UserCommand::TYPE_RAW, UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE,
        "TestCmd", "/raw command", "", "adc://hub.com"
    );

    REQUIRE(uc.getName() == "TestCmd");
    REQUIRE(uc.getCommand() == "/raw command");
}

TEST_CASE("FavoriteManager: getUserCommand", "[favoritemanager]") {
    ensureContext();

    UserCommand added = g_fm->addUserCommand(
        UserCommand::TYPE_RAW, UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE,
        "FindMe", "/find", "", ""
    );

    UserCommand found;
    REQUIRE(g_fm->getUserCommand(added.getId(), found) == true);
    REQUIRE(found.getName() == "FindMe");
}

TEST_CASE("FavoriteManager: getUserCommand not found", "[favoritemanager]") {
    ensureContext();

    UserCommand uc;
    REQUIRE(g_fm->getUserCommand(99999, uc) == false);
}

TEST_CASE("FavoriteManager: findUserCommand", "[favoritemanager]") {
    ensureContext();

    g_fm->addUserCommand(
        UserCommand::TYPE_RAW, UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE,
        "Findable", "/cmd", "", "adc://find.hub"
    );

    int id = g_fm->findUserCommand("Findable", "adc://find.hub");
    REQUIRE(id >= 0);
}

TEST_CASE("FavoriteManager: removeUserCommand", "[favoritemanager]") {
    ensureContext();

    UserCommand uc = g_fm->addUserCommand(
        UserCommand::TYPE_RAW, UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE,
        "RemoveMe", "/rm", "", ""
    );

    g_fm->removeUserCommand(uc.getId());

    UserCommand found;
    REQUIRE(g_fm->getUserCommand(uc.getId(), found) == false);
}

TEST_CASE("FavoriteManager: updateUserCommand", "[favoritemanager]") {
    ensureContext();

    UserCommand uc = g_fm->addUserCommand(
        UserCommand::TYPE_RAW, UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE,
        "UpdateMe", "/old", "", ""
    );

    uc.setCommand("/new");
    g_fm->updateUserCommand(uc);

    UserCommand found;
    REQUIRE(g_fm->getUserCommand(uc.getId(), found) == true);
    REQUIRE(found.getCommand() == "/new");
}

TEST_CASE("FavoriteManager: save writes file", "[favoritemanager]") {
    ensureContext();

    FavoriteHubEntry entry(*getContext());
    entry.setServer("adc://save-test.example.com");
    entry.setName("Save Test Hub");
    g_fm->addFavorite(entry);

    // addFavorite calls save() internally, which writes Favorites.xml to the
    // temp config dir. Verify the file exists.
    std::string configPath = Util::getPath(Util::PATH_USER_CONFIG) + "Favorites.xml";
    namespace fs = std::filesystem;
    REQUIRE(fs::exists(configPath));
    REQUIRE(fs::file_size(configPath) > 0);
}
