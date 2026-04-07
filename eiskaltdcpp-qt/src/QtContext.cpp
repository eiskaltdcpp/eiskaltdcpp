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

#include "QtContext.h"
#include "QtContextAware.h"
#include "WulforSettings.h"
#include "SearchBlacklist.h"
#include "Antispam.h"

#ifndef QT_CONTEXT_MINIMAL
#include "GlobalTimer.h"
#include "WulforUtil.h"
#include "ArenaWidgetManager.h"
#include "MainWindow.h"
#include "HubManager.h"
#include "EmoticonFactory.h"
#include "Notification.h"
#include "ShortcutManager.h"
#include "TransferView.h"
#include "FavoriteHubs.h"
#include "PublicHubs.h"
#include "FavoriteUsers.h"
#include "DownloadQueue.h"
#include "SpyFrame.h"
#include "ADLS.h"
#include "CmdDebug.h"
#include "Secretary.h"
#include "QueuedUsers.h"
#include "FinishedTransfers.h"
#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif
#ifdef USE_JS
#include "ScriptEngine.h"
#include "scriptengine/ClientManagerScript.h"
#include "scriptengine/HashManagerScript.h"
#include "scriptengine/LogManagerScript.h"
#endif
#endif // QT_CONTEXT_MINIMAL

// ── QtContext implementation ────────────────────────────────────────────

QtContext::QtContext(dcpp::DCContext& dcCtx)
    : dcCtx_(dcCtx)
{
    QtContextAware::setCurrent(this);
}

QtContext::~QtContext() {
#if !defined(QT_CONTEXT_MINIMAL) && defined(USE_JS)
    // ScriptEngine must be destroyed before the script manager objects
    // it references (ClientManagerScript, HashManagerScript, LogManagerScript).
    // Explicit reset here instead of relying on member declaration order.
    scriptEngine_.reset();
    clientManagerScript_.reset();
    hashManagerScript_.reset();
    logManagerScript_.reset();
#endif
#ifndef QT_CONTEXT_MINIMAL
    // ArenaWidgetManager holds raw pointers to arena widgets (HubFrame,
    // FavoriteHubs, etc.) that are owned either as unique_ptr members below
    // or as MainWindow children.  Destroy it first so its destructor runs
    // while the tracked widgets are still alive.
    arenaWidgetManager_.reset();
#endif
    // Remaining members destroyed in reverse declaration order by the
    // compiler-generated sequence.  Deregister after all services are gone.
    QtContextAware::setCurrent(nullptr);
}

void QtContext::createSettings()           { settings_           = std::make_unique<WulforSettings>(dcCtx_); settings_->setQtContext(this); }
void QtContext::createSearchBlacklist()    { searchBlacklist_    = std::make_unique<SearchBlacklist>(dcCtx_); searchBlacklist_->setQtContext(this); }
void QtContext::createAntiSpam()           { antiSpam_           = std::make_unique<AntiSpam>(dcCtx_); antiSpam_->setQtContext(this); }
void QtContext::destroyAntiSpam()          { antiSpam_.reset(); }

#ifndef QT_CONTEXT_MINIMAL
void QtContext::createGlobalTimer()        { globalTimer_        = std::make_unique<GlobalTimer>(dcCtx_); globalTimer_->setQtContext(this); }
void QtContext::createWulforUtil()         { wulforUtil_         = std::make_unique<WulforUtil>(dcCtx_); wulforUtil_->setQtContext(this); }
void QtContext::createArenaWidgetManager() { arenaWidgetManager_ = std::make_unique<ArenaWidgetManager>(dcCtx_); arenaWidgetManager_->setQtContext(this); }
void QtContext::createMainWindow()         { mainWindow_         = std::make_unique<MainWindow>(dcCtx_); mainWindow_->setQtContext(this); }
void QtContext::createHubManager()         { hubManager_         = std::make_unique<HubManager>(dcCtx_); hubManager_->setQtContext(this); }
void QtContext::createEmoticonFactory()    { emoticonFactory_    = std::make_unique<EmoticonFactory>(dcCtx_); emoticonFactory_->setQtContext(this); }
void QtContext::destroyEmoticonFactory()   { emoticonFactory_.reset(); }
void QtContext::createNotification()       { notification_       = std::make_unique<Notification>(dcCtx_); notification_->setQtContext(this); }
void QtContext::destroyNotification()      { notification_.reset(); }
void QtContext::createShortcutManager()    { shortcutManager_    = std::make_unique<ShortcutManager>(dcCtx_); shortcutManager_->setQtContext(this); }
void QtContext::createTransferView()       { transferView_       = std::make_unique<TransferView>(dcCtx_); transferView_->setQtContext(this); }
void QtContext::createFavoriteHubs()       { favoriteHubs_       = std::make_unique<FavoriteHubs>(dcCtx_); favoriteHubs_->setQtContext(this); }
void QtContext::createPublicHubs()         { publicHubs_         = std::make_unique<PublicHubs>(dcCtx_); publicHubs_->setQtContext(this); }
void QtContext::createFavoriteUsers()      { favoriteUsers_      = std::make_unique<FavoriteUsers>(dcCtx_); favoriteUsers_->setQtContext(this); }
void QtContext::createDownloadQueue()      { downloadQueue_      = std::make_unique<DownloadQueue>(dcCtx_); downloadQueue_->setQtContext(this); }
void QtContext::createSpyFrame()           { spyFrame_           = std::make_unique<SpyFrame>(dcCtx_); spyFrame_->setQtContext(this); }
void QtContext::createADLS()               { adls_               = std::make_unique<ADLS>(dcCtx_); adls_->setQtContext(this); }
void QtContext::createCmdDebug()           { cmdDebug_           = std::make_unique<CmdDebug>(dcCtx_); cmdDebug_->setQtContext(this); }
void QtContext::createSecretary()          { secretary_          = std::make_unique<Secretary>(dcCtx_); secretary_->setQtContext(this); }
void QtContext::createQueuedUsers()        { queuedUsers_        = std::make_unique<QueuedUsers>(dcCtx_); queuedUsers_->setQtContext(this); }
void QtContext::createFinishedUploads()    { finishedUploads_    = std::make_unique<FinishedUploads>(dcCtx_); finishedUploads_->setQtContext(this); }
void QtContext::createFinishedDownloads()  { finishedDownloads_  = std::make_unique<FinishedDownloads>(dcCtx_); finishedDownloads_->setQtContext(this); }
FinishedUploads*   QtContext::finishedUploads()   const noexcept { return finishedUploads_.get(); }
FinishedDownloads* QtContext::finishedDownloads() const noexcept { return finishedDownloads_.get(); }
#ifdef USE_ASPELL
void QtContext::createSpellCheck()         { spellCheck_         = std::make_unique<SpellCheck>(dcCtx_); spellCheck_->setQtContext(this); }
void QtContext::destroySpellCheck()        { spellCheck_.reset(); }
#endif
#ifdef USE_JS
void QtContext::createScriptEngine()       { scriptEngine_       = std::make_unique<ScriptEngine>(dcCtx_); scriptEngine_->setQtContext(this); }
void QtContext::createClientManagerScript() { clientManagerScript_ = std::make_unique<ClientManagerScript>(dcCtx_); clientManagerScript_->setQtContext(this); }
void QtContext::destroyClientManagerScript() { clientManagerScript_.reset(); }
void QtContext::createHashManagerScript()  { hashManagerScript_  = std::make_unique<HashManagerScript>(dcCtx_); hashManagerScript_->setQtContext(this); }
void QtContext::destroyHashManagerScript() { hashManagerScript_.reset(); }
void QtContext::createLogManagerScript()   { logManagerScript_   = std::make_unique<LogManagerScript>(dcCtx_); logManagerScript_->setQtContext(this); }
void QtContext::destroyLogManagerScript()  { logManagerScript_.reset(); }
#endif
#endif // QT_CONTEXT_MINIMAL
