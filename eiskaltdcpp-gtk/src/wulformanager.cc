/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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

#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "GtkContextAware.hh"
#include <iostream>
#include <glib/gi18n.h>
#include "hashdialog.hh"
#include "settingsdialog.hh"
#include "settingsmanager.hh"

using namespace std;
using namespace dcpp;

// ── Module-level active instance pointer (set/cleared in wulfor.cc) ──
static WulforManager *s_managerInstance = nullptr;
string WulforManager::argv1;

WulforManager *wulforManagerInstance()
{
    dcassert(s_managerInstance);
    return s_managerInstance;
}

void setWulforManagerInstance(WulforManager *instance)
{
    s_managerInstance = instance;
}

WulforManager::WulforManager(dcpp::DCContext& dcCtx, int argc, char **argv)
    : dcCtx_(dcCtx)
    , clientCondValue(0)
    , abort(false)
{
    if (argc > 1)
    {
        argv1 = argv[1];
    }
#if !GLIB_CHECK_VERSION(2,32,0)
    clientCond = g_cond_new();
    clientCondMutex = g_mutex_new();

    clientCallMutex = g_mutex_new();
    clientQueueMutex = g_mutex_new();

    g_static_rw_lock_init(&entryMutex);
#else
    clientCond = new GCond();
    clientCondMutex = new GMutex();

    clientCallMutex = new GMutex();
    clientQueueMutex = new GMutex();

    g_cond_init(clientCond);
    g_mutex_init(clientCondMutex);

    g_mutex_init(clientCallMutex);
    g_mutex_init(clientQueueMutex);

    g_rw_lock_init(&entryMutex);
#endif

#if !GLIB_CHECK_VERSION(2,32,0)
    GError *error = NULL;
    clientThread = g_thread_create(threadFunc_client, (gpointer)this, true, &error);
    if (error)
    {
        cerr << "Unable to create client thread: " << error->message << endl;
        g_error_free(error);
        exit(EXIT_FAILURE);
    }
#else
    clientThread = g_thread_new("gtkclient",threadFunc_client, (gpointer)this);
#endif

    mainWin = NULL;

    // Determine path to data files
    path = string(_DATADIR) + G_DIR_SEPARATOR_S + string("gtk");
    if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
    {
        cerr << path << " is inaccessible, falling back to current directory instead.\n";
        path = ".";
    }

    // Set the custom icon search path so GTK+ can find our icons
    const string iconPath = path + G_DIR_SEPARATOR_S + "icons";
    const string themes = path + G_DIR_SEPARATOR_S + "themes";
    GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
    gtk_icon_theme_append_search_path(iconTheme, iconPath.c_str());
    gtk_icon_theme_append_search_path(iconTheme, themes.c_str());

    // Post-init: apply settings and create main window
    std::string lang = WGETS("translation-lang");
    if (!lang.empty())
        dcpp::Util::setLang(dcCtx_, lang);

    createMainWindow();
    dcpp::Text::hubDefaultCharset = WGETS("default-charset");
}

WulforManager::~WulforManager()
{
    abort = true;

    g_mutex_lock(clientCondMutex);
    clientCondValue++;
    g_cond_signal(clientCond);
    g_mutex_unlock(clientCondMutex);

    g_thread_join(clientThread);

#if !GLIB_CHECK_VERSION(2,32,0)
    g_cond_free(clientCond);
    g_mutex_free(clientCondMutex);
    g_mutex_free(clientCallMutex);
    g_mutex_free(clientQueueMutex);

    g_static_rw_lock_free(&entryMutex);
#else
    g_cond_clear(clientCond);
    g_mutex_clear(clientCondMutex);
    g_mutex_clear(clientCallMutex);
    g_mutex_clear(clientQueueMutex);

    delete clientCond;
    delete clientCondMutex;
    delete clientCallMutex;
    delete clientQueueMutex;

    g_rw_lock_clear(&entryMutex);
#endif
}

void WulforManager::createMainWindow()
{
    dcassert(!mainWin);
    mainWin = new MainWindow(dcCtx_);
    WulforManager::insertEntry_gui(mainWin);
    mainWin->show();
}

void WulforManager::deleteMainWindow()
{
    // response dialogs: hash, settings
    DialogEntry *hashDialogEntry = getHashDialog_gui();
    DialogEntry *settingsDialogEntry = getSettingsDialog_gui();

    if (hashDialogEntry)
    {
        gtk_dialog_response(GTK_DIALOG(hashDialogEntry->getContainer()), GTK_RESPONSE_OK);
    }
    if (settingsDialogEntry)
    {
        dynamic_cast<Settings*>(settingsDialogEntry)->response_gui();
    }

    mainWin->remove();
    gtk_main_quit();
}

gboolean WulforManager::idleCallback_gui(gpointer data)
{
    FuncBase *func = static_cast<FuncBase *>(data);

    // Re-check the entry still exists (it may have been deleted between
    // dispatch and this callback firing on the main thread).
    WulforManager *man = wulforManagerInstance();
    if (man && !man->abort)
    {
#if !GLIB_CHECK_VERSION(2,32,0)
        g_static_rw_lock_reader_lock(&man->entryMutex);
#else
        g_rw_lock_reader_lock(&man->entryMutex);
#endif
        bool valid = man->entries.find(func->getID()) != man->entries.end();
#if !GLIB_CHECK_VERSION(2,32,0)
        g_static_rw_lock_reader_unlock(&man->entryMutex);
#else
        g_rw_lock_reader_unlock(&man->entryMutex);
#endif
        if (valid)
            func->call();
    }

    delete func;
    return G_SOURCE_REMOVE; // one-shot
}

gpointer WulforManager::threadFunc_client(gpointer data)
{
    WulforManager *man = (WulforManager *)data;
    man->processClientQueue();
    return NULL;
}

void WulforManager::processClientQueue()
{
    FuncBase *func;

    while (!abort)
    {
        g_mutex_lock(clientCondMutex);
        while (clientCondValue < 1)
            g_cond_wait(clientCond, clientCondMutex);
        clientCondValue--;
        g_mutex_unlock(clientCondMutex);

        g_mutex_lock(clientCallMutex);
        g_mutex_lock(clientQueueMutex);
        while (!clientFuncs.empty())
        {
            func = clientFuncs.front();
            clientFuncs.pop_front();
            g_mutex_unlock(clientQueueMutex);

            func->call();
            delete func;

            g_mutex_lock(clientQueueMutex);
        }
        g_mutex_unlock(clientQueueMutex);
        g_mutex_unlock(clientCallMutex);
    }

    g_thread_exit(NULL);
}

void WulforManager::dispatchGuiFunc(FuncBase *func)
{

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_reader_lock(&entryMutex);
#else
    g_rw_lock_reader_lock(&entryMutex);
#endif

    // Make sure we're not adding functions to deleted objects.
    if (entries.find(func->getID()) != entries.end())
    {
        // Schedule func to run on the GTK main thread via g_idle_add.
        // The callback will re-check entry validity before calling.
        g_idle_add(idleCallback_gui, func);
    }
    else
        delete func;

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_reader_unlock(&entryMutex);
#else
    g_rw_lock_reader_unlock(&entryMutex);
#endif

}

void WulforManager::dispatchClientFunc(FuncBase *func)
{

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_reader_lock(&entryMutex);
#else
    g_rw_lock_reader_lock(&entryMutex);
#endif

    // Make sure we're not adding functions to deleted objects.
    if (entries.find(func->getID()) != entries.end())
    {
        g_mutex_lock(clientQueueMutex);
        clientFuncs.push_back(func);
        g_mutex_unlock(clientQueueMutex);

        g_mutex_lock(clientCondMutex);
        clientCondValue++;
        g_cond_signal(clientCond);
        g_mutex_unlock(clientCondMutex);
    }
    else
        delete func;

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_reader_unlock(&entryMutex);
#else
    g_rw_lock_reader_unlock(&entryMutex);
#endif

}

MainWindow *WulforManager::getMainWindow()
{
    dcassert(mainWin);
    return mainWin;
}

string WulforManager::getURL()
{
    return argv1;
}

string WulforManager::getPath() const
{
    return path;
}

void WulforManager::insertEntry_gui(Entry *entry)
{

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_writer_lock(&entryMutex);
#else
    g_rw_lock_writer_lock(&entryMutex);
#endif

    entries[entry->getID()] = entry;

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_writer_unlock(&entryMutex);
#else
    g_rw_lock_writer_unlock(&entryMutex);
#endif

}

// Should be called from the GTK main thread (e.g. from a signal handler or idle callback).
void WulforManager::deleteEntry_gui(Entry *entry)
{
    const string &id = entry->getID();
    deque<FuncBase *>::iterator fIt;

    g_mutex_lock(clientCallMutex);

    // Erase any pending calls to this bookentry.
    g_mutex_lock(clientQueueMutex);
    fIt = clientFuncs.begin();
    while (fIt != clientFuncs.end())
    {
        if ((*fIt)->getID() == id)
        {
            delete *fIt;
            fIt = clientFuncs.erase(fIt);
        }
        else
            ++fIt;
    }
    g_mutex_unlock(clientQueueMutex);

    // NOTE: Pending GUI idle callbacks for this entry cannot be cancelled,
    // but idleCallback_gui() re-checks entry existence before calling, so
    // they will safely no-op once the entry is removed below.

    // Remove the bookentry from the list.

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_writer_lock(&entryMutex);
#else
    g_rw_lock_writer_lock(&entryMutex);
#endif

    if (entries.find(id) != entries.end())
        entries.erase(id);

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_writer_unlock(&entryMutex);
#else
    g_rw_lock_writer_unlock(&entryMutex);
#endif

    g_mutex_unlock(clientCallMutex);

    delete entry;
}

bool WulforManager::isEntry_gui(Entry *entry)
{

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_writer_lock(&entryMutex);
#else
    g_rw_lock_writer_lock(&entryMutex);
#endif

    auto it = find_if(entries.begin(), entries.end(),
                      CompareSecond<string, Entry *>(entry));

    if (it == entries.end())
        entry = NULL;

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_writer_unlock(&entryMutex);
#else
    g_rw_lock_writer_unlock(&entryMutex);
#endif

    return (entry != NULL);
}

DialogEntry* WulforManager::getDialogEntry_gui(const string &id)
{
    DialogEntry *ret = NULL;

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_reader_lock(&entryMutex);
#else
    g_rw_lock_reader_lock(&entryMutex);
#endif

    if (entries.find(id) != entries.end())
        ret = dynamic_cast<DialogEntry *>(entries[id]);

#if !GLIB_CHECK_VERSION(2,32,0)
    g_static_rw_lock_reader_unlock(&entryMutex);
#else
    g_rw_lock_reader_unlock(&entryMutex);
#endif

    return ret;
}

void WulforManager::onReceived_gui(const string &link)
{
    dcassert(mainWin);

    if (WulforUtil::isHubURL(link) && WGETB("urlhandler"))
        mainWin->showHub_gui(link);

    else if (WulforUtil::isMagnet(link) && WGETB("magnet-register"))
        mainWin->actionMagnet_gui(link);
}

gint WulforManager::openHashDialog_gui()
{
    DialogEntry *hash = getHashDialog_gui();
    if (!hash)
        hash = new Hash(dcCtx_);

    gint response = hash->run();

    return response;
}

gint WulforManager::openSettingsDialog_gui()
{
    Settings *s = new Settings(dcCtx_);
    gint response = s->run();

    return response;
}

DialogEntry *WulforManager::getHashDialog_gui()
{
    return getDialogEntry_gui(Util::toString(Entry::HASH_DIALOG) + ":");
}

DialogEntry *WulforManager::getSettingsDialog_gui()
{
    return getDialogEntry_gui(Util::toString(Entry::SETTINGS_DIALOG) + ":");
}
