/*
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

#include <memory>

namespace dcpp { class DCContext; }

// Forward declarations — no heavy includes in this header.
class WulforSettings;
class SearchBlacklist;
class AntiSpam;

#ifndef QT_CONTEXT_MINIMAL
class GlobalTimer;
class WulforUtil;
class ArenaWidgetManager;
class MainWindow;
class HubManager;
class EmoticonFactory;
class Notification;
class ShortcutManager;
class TransferView;
class FavoriteHubs;
class PublicHubs;
class FavoriteUsers;
class DownloadQueue;
class SpyFrame;
class ADLS;
class CmdDebug;
class Secretary;
class QueuedUsers;
template <bool isUpload> class FinishedTransfers;
using FinishedUploads   = FinishedTransfers<true>;
using FinishedDownloads = FinishedTransfers<false>;
#ifdef USE_ASPELL
class SpellCheck;
#endif
#ifdef USE_JS
class ScriptEngine;
class ClientManagerScript;
class HashManagerScript;
class LogManagerScript;
#endif
#endif // QT_CONTEXT_MINIMAL

/**
 * Qt front-end application context — owns UI-side services.
 *
 * Mirrors the dcpp::DCContext pattern: one context, created by main()
 * (or by unit tests), that owns the services as unique_ptr members.
 * Destruction order is the reverse of declaration order.
 *
 * The constructor registers the context via QtContextAware::setCurrent()
 * so that any code can reach it through the qtCtx() free function or
 * through the QtContextAware mixin.
 *
 * Usage (application):
 *   QtContext ctx;           // auto-registers
 *   ctx.createSettings();
 *   // ... run application — qtCtx()->settings() works everywhere ...
 *   // ctx destructor auto-deregisters and cleans up
 *
 * Usage (tests):
 *   QtContext ctx;           // auto-registers
 *   ctx.createSettings();
 *   // ... test code calls qtCtx()->settings() normally ...
 *   // ctx destructor auto-deregisters
 */
class QtContext {
public:
    explicit QtContext(dcpp::DCContext& dcCtx);
    ~QtContext();

    // Non-copyable, non-movable
    QtContext(const QtContext&) = delete;
    QtContext& operator=(const QtContext&) = delete;
    QtContext(QtContext&&) = delete;
    QtContext& operator=(QtContext&&) = delete;

    /** Access the core library context. */
    [[nodiscard]] dcpp::DCContext& dcCtx() const noexcept { return dcCtx_; }

    // ── Service creation (call in dependency order) ────────────────────
    void createSettings();
    void createSearchBlacklist();
    void createAntiSpam();
    void destroyAntiSpam();          // runtime toggle support

#ifndef QT_CONTEXT_MINIMAL
    void createGlobalTimer();
    void createWulforUtil();
    void createArenaWidgetManager();
    void createMainWindow();
    void createHubManager();
    void createEmoticonFactory();
    void destroyEmoticonFactory();   // runtime toggle support
    void createNotification();
    void destroyNotification();      // destroyed in closeEvent path
    void createShortcutManager();
    void createTransferView();
    // ── Lazy widget singletons (created on demand by UI) ───────────────
    void createFavoriteHubs();
    void createPublicHubs();
    void createFavoriteUsers();
    void createDownloadQueue();
    void createSpyFrame();
    void createADLS();
    void createCmdDebug();
    void createSecretary();
    void createQueuedUsers();
    void createFinishedUploads();
    void createFinishedDownloads();
#ifdef USE_ASPELL
    void createSpellCheck();
    void destroySpellCheck();        // runtime toggle support
#endif
#ifdef USE_JS
    void createScriptEngine();
    void createClientManagerScript();
    void destroyClientManagerScript();
    void createHashManagerScript();
    void destroyHashManagerScript();
    void createLogManagerScript();
    void destroyLogManagerScript();
#endif
#endif // QT_CONTEXT_MINIMAL

    // ── Typed accessors (non-owning raw pointers) ──────────────────────
    [[nodiscard]] WulforSettings*     settings()           const noexcept { return settings_.get(); }
    [[nodiscard]] SearchBlacklist*    searchBlacklist()    const noexcept { return searchBlacklist_.get(); }
    [[nodiscard]] AntiSpam*           antiSpam()           const noexcept { return antiSpam_.get(); }

#ifndef QT_CONTEXT_MINIMAL
    [[nodiscard]] GlobalTimer*        globalTimer()        const noexcept { return globalTimer_.get(); }
    [[nodiscard]] WulforUtil*         wulforUtil()         const noexcept { return wulforUtil_.get(); }
    [[nodiscard]] ArenaWidgetManager* arenaWidgetManager() const noexcept { return arenaWidgetManager_.get(); }
    [[nodiscard]] MainWindow*         mainWindow()         const noexcept { return mainWindow_.get(); }
    [[nodiscard]] HubManager*         hubManager()         const noexcept { return hubManager_.get(); }
    [[nodiscard]] EmoticonFactory*    emoticonFactory()    const noexcept { return emoticonFactory_.get(); }
    [[nodiscard]] Notification*       notification()       const noexcept { return notification_.get(); }
    [[nodiscard]] ShortcutManager*    shortcutManager()    const noexcept { return shortcutManager_.get(); }
    [[nodiscard]] TransferView*       transferView()       const noexcept { return transferView_.get(); }
    [[nodiscard]] FavoriteHubs*       favoriteHubs()       const noexcept { return favoriteHubs_.get(); }
    [[nodiscard]] PublicHubs*         publicHubs()         const noexcept { return publicHubs_.get(); }
    [[nodiscard]] FavoriteUsers*      favoriteUsers()      const noexcept { return favoriteUsers_.get(); }
    [[nodiscard]] DownloadQueue*      downloadQueue()      const noexcept { return downloadQueue_.get(); }
    [[nodiscard]] SpyFrame*           spyFrame()           const noexcept { return spyFrame_.get(); }
    [[nodiscard]] ADLS*               adls()               const noexcept { return adls_.get(); }
    [[nodiscard]] CmdDebug*           cmdDebug()           const noexcept { return cmdDebug_.get(); }
    [[nodiscard]] Secretary*          secretary()           const noexcept { return secretary_.get(); }
    [[nodiscard]] QueuedUsers*        queuedUsers()        const noexcept { return queuedUsers_.get(); }
    [[nodiscard]] FinishedUploads*    finishedUploads()    const noexcept;
    [[nodiscard]] FinishedDownloads*  finishedDownloads()  const noexcept;
#ifdef USE_ASPELL
    [[nodiscard]] SpellCheck*         spellCheck()         const noexcept { return spellCheck_.get(); }
#endif
#ifdef USE_JS
    [[nodiscard]] ScriptEngine*       scriptEngine()       const noexcept { return scriptEngine_.get(); }
    [[nodiscard]] ClientManagerScript* clientManagerScript() const noexcept { return clientManagerScript_.get(); }
    [[nodiscard]] HashManagerScript*   hashManagerScript()  const noexcept { return hashManagerScript_.get(); }
    [[nodiscard]] LogManagerScript*    logManagerScript()   const noexcept { return logManagerScript_.get(); }
#endif
#endif // QT_CONTEXT_MINIMAL

private:
    dcpp::DCContext& dcCtx_;

    // Destruction is reverse of declaration order.
    // Declare in creation-dependency order (earliest first).
    std::unique_ptr<WulforSettings>     settings_;
    std::unique_ptr<SearchBlacklist>    searchBlacklist_;
    std::unique_ptr<AntiSpam>           antiSpam_;

#ifndef QT_CONTEXT_MINIMAL
    std::unique_ptr<GlobalTimer>        globalTimer_;
    std::unique_ptr<WulforUtil>         wulforUtil_;
    std::unique_ptr<ArenaWidgetManager> arenaWidgetManager_;
    std::unique_ptr<MainWindow>         mainWindow_;
    std::unique_ptr<HubManager>         hubManager_;
    std::unique_ptr<EmoticonFactory>    emoticonFactory_;
    std::unique_ptr<Notification>       notification_;
    std::unique_ptr<ShortcutManager>    shortcutManager_;
    std::unique_ptr<TransferView>       transferView_;
    std::unique_ptr<FavoriteHubs>       favoriteHubs_;
    std::unique_ptr<PublicHubs>         publicHubs_;
    std::unique_ptr<FavoriteUsers>      favoriteUsers_;
    std::unique_ptr<DownloadQueue>      downloadQueue_;
    std::unique_ptr<SpyFrame>           spyFrame_;
    std::unique_ptr<ADLS>               adls_;
    std::unique_ptr<CmdDebug>           cmdDebug_;
    std::unique_ptr<Secretary>          secretary_;
    std::unique_ptr<QueuedUsers>        queuedUsers_;
    std::unique_ptr<FinishedUploads>    finishedUploads_;
    std::unique_ptr<FinishedDownloads>  finishedDownloads_;
#ifdef USE_ASPELL
    std::unique_ptr<SpellCheck>         spellCheck_;
#endif
#ifdef USE_JS
    std::unique_ptr<ScriptEngine>        scriptEngine_;
    std::unique_ptr<ClientManagerScript>  clientManagerScript_;
    std::unique_ptr<HashManagerScript>    hashManagerScript_;
    std::unique_ptr<LogManagerScript>     logManagerScript_;
#endif
#endif // QT_CONTEXT_MINIMAL
};
