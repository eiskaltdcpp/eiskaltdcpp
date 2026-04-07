/*
 * TestContext — shared fixture for tests that need a DCContext.
 *
 * Creates a minimal DCContext with ResourceManager, SettingsManager, and
 * LogManager.  No network, no hashing, no file I/O beyond a temp directory.
 * The process-wide getContext() is wired so settings access works.
 *
 * Usage (Catch2):
 *   #include "TestContext.h"
 *   TEST_CASE("my test") {
 *       TestContext tc;   // creates temp dir, starts minimal context
 *       // ... SETTING(SLOTS) etc. now work with defaults ...
 *   }  // shutdown + cleanup on scope exit
 *
 * IMPORTANT — static destruction order:
 * On MSVC the linker may destroy static objects from SettingsManager.cpp
 * (e.g. settingTags[]) before static unique_ptrs.  If a static unique_ptr's
 * destructor then calls shutdown() → save(), it dereferences destroyed
 * std::strings → heap corruption.
 *
 * We guard against this with an atexit() handler that shuts down and
 * clears the context while all statics are still alive.
 */

#pragma once

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "DCContext.h"
#include "Util.h"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

namespace dcpp {
namespace test {

/// atexit callback: shuts down and releases the process-wide DCContext
/// *before* static destruction begins, avoiding use-after-destroy of
/// SettingsManager::settingTags[] and similar statics.
inline void atexitShutdownContext() {
    if (dcpp::getContext()) {
        dcpp::getContext()->shutdown();
        dcpp::setContext(nullptr);
    }
}

struct TestContext {
    std::filesystem::path tmpDir;
    std::unique_ptr<DCContext> ownedCtx;  ///< this fixture owns the context

    TestContext() {
        // Create a unique temp directory for this test run
        tmpDir = std::filesystem::temp_directory_path() / "eiskalt_test";
        // Append a unique suffix using the address of this object
        char suffix[32];
        snprintf(suffix, sizeof(suffix), "_%p", static_cast<void*>(this));
        tmpDir += suffix;
        std::filesystem::create_directories(tmpDir);

        // Use make_preferred() to ensure native separators (\ on Windows)
        tmpDir = tmpDir.make_preferred();
        auto dirStr = tmpDir.string() + std::string(1, PATH_SEPARATOR);

        // Initialize Util with paths redirected to our temp dir
        Util::PathsMap overrides;
        overrides[Util::PATH_USER_CONFIG] = dirStr;
        overrides[Util::PATH_USER_LOCAL]  = dirStr;
        overrides[Util::PATH_DOWNLOADS]   = dirStr + "Downloads" + std::string(1, PATH_SEPARATOR);
        Util::initialize(overrides);

        // Create minimal context (ResourceManager + SettingsManager + LogManager)
        ownedCtx = std::make_unique<DCContext>();
        ownedCtx->startupMinimal();
        dcpp::setContext(ownedCtx.get());

        // Register atexit handler AFTER static init of settingTags etc.
        // guarantees it runs BEFORE those statics are destroyed.
        static bool registered = false;
        if (!registered) {
            std::atexit(atexitShutdownContext);
            registered = true;
        }
    }

    ~TestContext() {
        if (dcpp::getContext()) {
            dcpp::getContext()->shutdown();
            dcpp::setContext(nullptr);
        }
        ownedCtx.reset();
        std::error_code ec;
        std::filesystem::remove_all(tmpDir, ec);
    }

    // Non-copyable
    TestContext(const TestContext&) = delete;
    TestContext& operator=(const TestContext&) = delete;
};

} // namespace test
} // namespace dcpp
