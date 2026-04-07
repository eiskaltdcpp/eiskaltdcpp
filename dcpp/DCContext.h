/*
 * Copyright (C) 2001-2012 Jacek Sieka, arber@dez.org
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>

class IPFilter;  // defined in extra/ipfilter.h (global scope)

namespace dht { class DHT; }  // DHT lives outside dcpp::

namespace dcpp {

// Forward declarations — avoids pulling every manager header into every TU.
class ResourceManager;
class SettingsManager;
class LogManager;
class TimerManager;
class HashManager;
class CryptoManager;
class SearchManager;
class ClientManager;
class ConnectionManager;
class DownloadManager;
class UploadManager;
class ThrottleManager;
class QueueManager;
class ShareManager;
class FavoriteManager;
class FinishedManager;
class ADLSearchManager;
class ConnectivityManager;
class MappingManager;
class DebugManager;
class DynDNS;
#ifdef LUA_SCRIPT
class ScriptManager;
#endif

class DCContext;  // forward for ContextAware

/**
 * Mixin base for classes that hold a back-reference to their owning DCContext.
 *
 * All manager classes inherit from ContextAware.  The context is set once
 * at construction time and is guaranteed non-null for the lifetime of the
 * object — no two-phase init, no global state.
 */
class ContextAware {
public:
    [[nodiscard]] DCContext& ctx() const { return ctx_; }

protected:
    explicit ContextAware(DCContext& ctx) : ctx_(ctx) {}
    ~ContextAware() = default;

private:
    DCContext& ctx_;
};

/**
 * Application context that owns all core manager instances.
 *
 * Construction order mirrors DCPlusPlus::startup(); destruction is the
 * exact reverse.  Each manager is held in a unique_ptr so destruction
 * happens in reverse declaration order automatically.
 *
 * Usage:
 *   auto ctx = std::make_unique<DCContext>();
 *   ctx.startup(progressCallback);
 *   // … run application …
 *   ctx.shutdown();
 *   ctx.reset();  // or let scope handle it
 *
 * Every owned manager's ctx() returns a reference back here for
 * cross-manager access without going through global singletons.
 */
class DCContext {
public:
    /// Progress callback: receives (userData, statusMessage).
    using ProgressFn = std::function<void(const std::string&)>;

    DCContext();
    ~DCContext();

    // Non-copyable, non-movable
    DCContext(const DCContext&) = delete;
    DCContext& operator=(const DCContext&) = delete;
    DCContext(DCContext&&) = delete;
    DCContext& operator=(DCContext&&) = delete;

    /// Create all managers in dependency order and load persistent state.
    void startup(ProgressFn progress = {});

    /// Lightweight startup for testing: only creates ResourceManager,
    /// SettingsManager, and LogManager.  Does NOT load settings from disk,
    /// start threads, or initialize networking.
    /// Call Util::initialize() with appropriate path overrides BEFORE this
    /// if you need to redirect config/data to a temp directory.
    void startupMinimal();

    /// Flush state, stop threads, tear down in reverse order.
    void shutdown();

    [[nodiscard]] bool isRunning() const { return running_; }

    // ── Typed accessors (non-owning raw pointers) ──────────────────────
    [[nodiscard]] ResourceManager*     getResourceManager()     const { return resourceManager_.get(); }
    [[nodiscard]] SettingsManager*     getSettingsManager()     const { return settingsManager_.get(); }
    [[nodiscard]] LogManager*          getLogManager()          const { return logManager_.get(); }
    [[nodiscard]] TimerManager*        getTimerManager()        const { return timerManager_.get(); }
    [[nodiscard]] HashManager*         getHashManager()         const { return hashManager_.get(); }
    [[nodiscard]] CryptoManager*       getCryptoManager()       const { return cryptoManager_.get(); }
    [[nodiscard]] SearchManager*       getSearchManager()       const { return searchManager_.get(); }
    [[nodiscard]] ClientManager*       getClientManager()       const { return clientManager_.get(); }
    [[nodiscard]] ConnectionManager*   getConnectionManager()   const { return connectionManager_.get(); }
    [[nodiscard]] DownloadManager*     getDownloadManager()     const { return downloadManager_.get(); }
    [[nodiscard]] UploadManager*       getUploadManager()       const { return uploadManager_.get(); }
    [[nodiscard]] ThrottleManager*     getThrottleManager()     const { return throttleManager_.get(); }
    [[nodiscard]] QueueManager*        getQueueManager()        const { return queueManager_.get(); }
    [[nodiscard]] ShareManager*        getShareManager()        const { return shareManager_.get(); }
    [[nodiscard]] FavoriteManager*     getFavoriteManager()     const { return favoriteManager_.get(); }
    [[nodiscard]] FinishedManager*     getFinishedManager()     const { return finishedManager_.get(); }
    [[nodiscard]] ADLSearchManager*    getADLSearchManager()    const { return adlSearchManager_.get(); }
    [[nodiscard]] ConnectivityManager* getConnectivityManager() const { return connectivityManager_.get(); }
    [[nodiscard]] MappingManager*      getMappingManager()      const { return mappingManager_.get(); }
    [[nodiscard]] DebugManager*        getDebugManager()        const { return debugManager_.get(); }
    [[nodiscard]] DynDNS*               getDynDNS()              const { return dynDNS_.get(); }
    [[nodiscard]] ::IPFilter*            getIPFilter()            const { return ipFilter_.get(); }
#ifdef WITH_DHT
    [[nodiscard]] ::dht::DHT*             getDHT()                 const { return dht_.get(); }
#endif
#ifdef LUA_SCRIPT
    [[nodiscard]] ScriptManager*        getScriptManager()       const { return scriptManager_.get(); }
#endif

private:
    bool running_ = false;
    bool minimalMode_ = false;  ///< true when started via startupMinimal()

    // ── Owned managers (destruction is reverse of declaration order) ────
    // Order must match DCPlusPlus::startup() construction order.
    std::unique_ptr<ResourceManager>     resourceManager_;
    std::unique_ptr<SettingsManager>     settingsManager_;
    std::unique_ptr<LogManager>          logManager_;
    std::unique_ptr<TimerManager>        timerManager_;
    std::unique_ptr<HashManager>         hashManager_;
    std::unique_ptr<CryptoManager>       cryptoManager_;
    std::unique_ptr<SearchManager>       searchManager_;
    std::unique_ptr<ClientManager>       clientManager_;
    std::unique_ptr<ConnectionManager>   connectionManager_;
    std::unique_ptr<DownloadManager>     downloadManager_;
    std::unique_ptr<UploadManager>       uploadManager_;
    std::unique_ptr<ThrottleManager>     throttleManager_;
    std::unique_ptr<QueueManager>        queueManager_;
    std::unique_ptr<ShareManager>        shareManager_;
    std::unique_ptr<FavoriteManager>     favoriteManager_;
    std::unique_ptr<FinishedManager>     finishedManager_;
    std::unique_ptr<ADLSearchManager>    adlSearchManager_;
    std::unique_ptr<ConnectivityManager> connectivityManager_;
    std::unique_ptr<MappingManager>      mappingManager_;
    std::unique_ptr<DebugManager>        debugManager_;
    std::unique_ptr<DynDNS>              dynDNS_;
    std::unique_ptr<::IPFilter>          ipFilter_;
#ifdef WITH_DHT
    std::unique_ptr<::dht::DHT>           dht_;
#endif
#ifdef LUA_SCRIPT
    std::unique_ptr<ScriptManager>       scriptManager_;
#endif
};

} // namespace dcpp
