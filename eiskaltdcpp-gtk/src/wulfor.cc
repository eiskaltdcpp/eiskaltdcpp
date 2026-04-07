/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "bacon-message-connection.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#else
#include <signal.h>
#endif

#define GUI_LOCALE_DIR LOCALE_DIR

#define GUI_PACKAGE "eiskaltdcpp-gtk"

#include "VersionGlobal.h"

#ifdef ENABLE_STACKTRACE
#include "extra/stacktrace.h"
#endif // ENABLE_STACKTRACE

void printHelp()
{
    printf(_(
               "Usage:\n"
               "  eiskaltdcpp-gtk <magnet link> <dchub://link> <adc(s)://link>\n"
               "  eiskaltdcpp-gtk <Key>\n"
               "EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocols.\n"
               "\n"
               "Keys:\n"
               "  -h, --help\t Show this message\n"
               "  -V, --version\t Show version string\n"
               )
           );
}

void printVersion()
{
    printf("%s version: %s\n", eiskaltdcppAppNameString.c_str(), eiskaltdcppVersionString.c_str());
    printf("GTK+ version: %d.%d.%d\n", gtk_major_version, gtk_minor_version, gtk_micro_version);
    printf("Glib version: %d.%d.%d\n", glib_major_version, glib_minor_version, glib_micro_version);
}

BaconMessageConnection *connection = NULL;

void receiver(const char *link, gpointer data)
{
    (void)data;
    g_return_if_fail(link != NULL);
    wulforManagerInstance()->onReceived_gui(link);
}

void callBack(void *, const std::string &a)
{
    std::cout << _("Loading: ") << a << std::endl;
}

#ifdef _WIN32
/**
 * Set GTK3 runtime environment variables relative to the executable's
 * directory so that double-clicking the .exe works without a launcher script.
 *
 * Also rewrites the gdk-pixbuf loaders.cache so module paths are absolute.
 * The shipped cache has relative paths like "loaders/foo.dll" which fail on
 * Windows because LoadLibrary() resolves them relative to CWD, not the
 * cache file's directory.
 *
 * Must be called before gtk_init().
 */
static std::string getExeDirectory()
{
    char modulePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return {};

    std::string dir(modulePath);
    auto pos = dir.find_last_of("\\/");
    if (pos != std::string::npos)
        dir.erase(pos);
    else
        return {};
    return dir;
}

/**
 * Read loaders.cache from the bundled location, replace relative module
 * paths with absolute ones, and write the fixed cache to %TEMP%.
 * Returns the path to the fixed cache, or empty on failure.
 */
// GDK's loaders.cache parser interprets C-style escape sequences inside
// quoted strings (\t → tab, \n → newline, etc.).  Windows paths contain
// backslashes that collide with this (e.g. C:\temp → C:<TAB>emp), so we
// must use forward slashes in all paths written to the cache.
static std::string toForwardSlash(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

static std::string fixLoadersCacheWin32(const std::string& exeDir)
{
    std::string srcCache = exeDir + "\\lib\\gdk-pixbuf-2.0\\2.10.0\\loaders.cache";
    std::string loadersDir = toForwardSlash(exeDir + "/lib/gdk-pixbuf-2.0/2.10.0/loaders/");

    std::ifstream in(srcCache);
    if (!in.is_open())
        return {};

    // Build fixed cache content: for every module-path line (starts with '"'),
    // extract just the DLL filename and reconstruct an absolute path under our
    // local loadersDir.  This handles any path format the CI might produce:
    //   - relative "loaders/foo.dll"
    //   - absolute CI paths "D:/a/.../loaders/foo.dll"
    //   - Windows backslash paths "D:\a\...\loaders\foo.dll"
    std::ostringstream fixed;
    std::string line;
    while (std::getline(in, line)) {
        // Module path lines start with a quoted string containing a DLL path.
        // Data lines also start with '"' but contain short names like "svg",
        // "image/svg+xml" — we detect module paths by the .dll extension.
        if (!line.empty() && line[0] == '"') {
            auto endQuote = line.find('"', 1);
            if (endQuote != std::string::npos) {
                std::string modPath = line.substr(1, endQuote - 1);
                // Check if this looks like a DLL path (module path)
                if (modPath.size() > 4 &&
                    (modPath.compare(modPath.size() - 4, 4, ".dll") == 0 ||
                     modPath.compare(modPath.size() - 4, 4, ".DLL") == 0))
                {
                    // Extract just the filename from any path format
                    auto lastSlash = modPath.find_last_of("\\/");
                    std::string dllName = (lastSlash != std::string::npos)
                        ? modPath.substr(lastSlash + 1)
                        : modPath;
                    // Construct absolute path under our loadersDir
                    fixed << '"' << loadersDir << dllName << '"'
                          << line.substr(endQuote + 1) << '\n';
                    continue;
                }
            }
        }
        fixed << line << '\n';
    }
    in.close();

    // Write fixed cache to %TEMP%
    const char* tmpDir = getenv("TEMP");
    if (!tmpDir) tmpDir = getenv("TMP");
    if (!tmpDir) tmpDir = ".";

    std::string dstDir = std::string(tmpDir) + "\\eiskaltdcpp-gtk";
    CreateDirectoryA(dstDir.c_str(), NULL);
    std::string dstCache = dstDir + "\\loaders.cache";

    std::ofstream out(dstCache, std::ios::trunc);
    if (!out.is_open())
        return {};

    out << fixed.str();
    out.close();
    return dstCache;
}

/**
 * Fallback when %TEMP% writing fails: overwrite the bundled loaders.cache
 * in-place with absolute paths.  Returns the path on success, empty on failure.
 */
static std::string fixLoadersCacheInPlace(const std::string& exeDir,
                                          const std::string& cacheFile)
{
    std::string loadersDir = toForwardSlash(exeDir + "/lib/gdk-pixbuf-2.0/2.10.0/loaders/");

    std::ifstream in(cacheFile);
    if (!in.is_open())
        return {};

    std::ostringstream buf;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line[0] == '"') {
            auto endQuote = line.find('"', 1);
            if (endQuote != std::string::npos) {
                std::string modPath = line.substr(1, endQuote - 1);
                if (modPath.size() > 4 &&
                    (modPath.compare(modPath.size() - 4, 4, ".dll") == 0 ||
                     modPath.compare(modPath.size() - 4, 4, ".DLL") == 0))
                {
                    auto lastSlash = modPath.find_last_of("\\/");
                    std::string dllName = (lastSlash != std::string::npos)
                        ? modPath.substr(lastSlash + 1) : modPath;
                    buf << '"' << loadersDir << dllName << '"'
                        << line.substr(endQuote + 1) << '\n';
                    continue;
                }
            }
        }
        buf << line << '\n';
    }
    in.close();

    std::ofstream out(cacheFile, std::ios::trunc);
    if (!out.is_open())
        return {};

    out << buf.str();
    out.close();
    return cacheFile;
}

void setupGtkEnvironmentWin32()
{
    std::string exeDir = getExeDirectory();
    if (exeDir.empty())
        return;

    // Ensure DLLs in the exe directory are findable by loader modules
    // (e.g. pixbufloader_svg.dll depends on librsvg, libxml2, etc.)
    SetDllDirectoryA(exeDir.c_str());

    // Helper: set env var if not already set (allows manual overrides)
    auto setIfEmpty = [](const char *var, const std::string &value) {
        if (!g_getenv(var))
            g_setenv(var, value.c_str(), FALSE);
    };

    // Fix loaders.cache to have absolute paths, write to temp location.
    // MUST force-set GDK_PIXBUF_MODULE_FILE even if already set (e.g. by
    // the launcher .cmd script) because the bundled cache has relative
    // paths that resolve against CWD, not the cache file's directory.
    std::string fixedCache = fixLoadersCacheWin32(exeDir);
    if (!fixedCache.empty()) {
        g_setenv("GDK_PIXBUF_MODULE_FILE", fixedCache.c_str(), TRUE);
    } else {
        // fixLoadersCacheWin32 failed (can't read source or write temp).
        // Try to overwrite the bundled cache in-place as a last resort.
        std::string bundledCache = exeDir + "\\lib\\gdk-pixbuf-2.0\\2.10.0\\loaders.cache";
        std::string retryCache = fixLoadersCacheInPlace(exeDir, bundledCache);
        if (!retryCache.empty()) {
            g_setenv("GDK_PIXBUF_MODULE_FILE", retryCache.c_str(), TRUE);
        } else {
            // All attempts failed.  Set to bundled cache and hope for the best.
            setIfEmpty("GDK_PIXBUF_MODULE_FILE", bundledCache);
            fprintf(stderr, "eiskaltdcpp-gtk: WARNING: could not fix loaders.cache paths\n");
        }
    }

    setIfEmpty("GDK_PIXBUF_MODULEDIR",
        exeDir + "\\lib\\gdk-pixbuf-2.0\\2.10.0\\loaders");
    setIfEmpty("GSETTINGS_SCHEMA_DIR",
        exeDir + "\\share\\glib-2.0\\schemas");
    setIfEmpty("GTK_EXE_PREFIX", exeDir);
    setIfEmpty("XDG_DATA_DIRS", exeDir + "\\share");
}
#endif /* _WIN32 */

#ifndef _WIN32
void catchSIG(int sigNum) {
    psignal(sigNum, _("Catching signal "));

#ifdef ENABLE_STACKTRACE
    printBacktrace(sigNum);
#endif // ENABLE_STACKTRACE

    raise(SIGINT);

    std::abort();
}

template <int sigNum = 0, int ... Params>
void catchSignals() {
    if (!sigNum)
        return;

    psignal(sigNum, _("Installing handler for"));

    signal(sigNum, catchSIG);

    catchSignals<Params ... >();
}

void installHandlers(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
        printf(_("Cannot handle SIGPIPE\n"));
    else {
        sigset_t set;
        sigemptyset (&set);
        sigaddset (&set, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
    }

    catchSignals<SIGSEGV, SIGABRT, SIGBUS, SIGTERM>();

    printf(_("Signal handlers installed.\n"));
}
#endif /* !_WIN32 */

int main(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){
            printHelp();
            exit(0);
        }
        else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-V")){
            printVersion();
            exit(0);
        }
    }

    // Initialize i18n support
    bindtextdomain(GUI_PACKAGE, GUI_LOCALE_DIR);
    textdomain(GUI_PACKAGE);
    bind_textdomain_codeset(GUI_PACKAGE, "UTF-8");

    connection = bacon_message_connection_new(GUI_PACKAGE);

    dcdebug(connection ? "eiskaltdcpp-gtk: connection yes...\n" : "eiskaltdcpp-gtk: connection no...\n");

    // Check if profile is locked
    if (WulforUtil::profileIsLocked())
    {
        if (!bacon_message_connection_get_is_server(connection))
        {
            dcdebug("eiskaltdcpp-gtk: is client...\n");

            if (argc > 1)
            {
                dcdebug("eiskaltdcpp-gtk: send %s\n", argv[1]);
                bacon_message_connection_send(connection, argv[1]);
            }
        }

        bacon_message_connection_free(connection);

        return 0;
    }

    if (bacon_message_connection_get_is_server(connection))
    {
        dcdebug("eiskaltdcpp-gtk: is server...\n");
        bacon_message_connection_set_callback(connection, receiver, NULL);
    }

    // Start the DC++ client core — caller owns the returned context
    auto dcContext = dcpp::startup(callBack, NULL);

    dcContext->getTimerManager()->start();

#if !GLIB_CHECK_VERSION(2,32,0)
    g_thread_init(NULL);
#endif

#ifdef _WIN32
    setupGtkEnvironmentWin32();
#endif

    gtk_init(&argc, &argv);
    g_set_application_name("EiskaltDC++ Gtk");

#ifndef _WIN32
    installHandlers();
#endif

    {
        WulforSettingsManager settingsMgr;
        setWulforSettingsInstance(&settingsMgr);
        WulforManager wulforMgr(*dcContext, argc, argv);
        setWulforManagerInstance(&wulforMgr);
        gtk_main();
        bacon_message_connection_free(connection);
        setWulforManagerInstance(nullptr);
        setWulforSettingsInstance(nullptr);
        // ~WulforManager then ~WulforSettingsManager run here (RAII)
    }

    std::cout << _("Shutting down libeiskaltdcpp...") << std::endl;
    dcContext->shutdown();
    dcContext.reset();
    dcpp::setContext(nullptr);
    std::cout << _("Quit...") << std::endl;
    return 0;
}
