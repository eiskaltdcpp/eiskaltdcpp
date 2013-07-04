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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <iostream>
#include <glib/gi18n.h>
#include "hashdialog.hh"
#include "settingsdialog.hh"
#include "settingsmanager.hh"

using namespace std;
using namespace dcpp;

WulforManager *WulforManager::manager = NULL;
string WulforManager::argv1;

void WulforManager::start(int argc, char **argv)
{
    if (argc > 1)
    {
        argv1 = argv[1];
    }

    // Create WulforManager
    dcassert(!manager);
    manager = new WulforManager();

    std::string lang = WGETS("translation-lang");
    if (!lang.empty())
        dcpp::Util::setLang(lang);

    gdk_threads_enter();
    manager->createMainWindow();
    gdk_threads_leave();
    dcpp::Text::hubDefaultCharset = WGETS("default-charset");
}

void WulforManager::stop()
{
    dcassert(manager);
    delete manager;
    manager = NULL;
}

WulforManager *WulforManager::get()
{
    dcassert(manager);
    return manager;
}

WulforManager::WulforManager()
{
    abort = FALSE;

    // Initialize sempahore variables
    guiCondValue = 0;
    clientCondValue = 0;
#if !GLIB_CHECK_VERSION(2,32,0)
    guiCond = g_cond_new();
    clientCond = g_cond_new();
    guiCondMutex = g_mutex_new();
    clientCondMutex = g_mutex_new();

    clientCallMutex = g_mutex_new();
    guiQueueMutex = g_mutex_new();
    clientQueueMutex = g_mutex_new();

    g_static_rw_lock_init(&entryMutex);
#else
    guiCond = new GCond();
    clientCond = new GCond();
    guiCondMutex = new GMutex();
    clientCondMutex = new GMutex();

    clientCallMutex = new GMutex();
    guiQueueMutex = new GMutex();
    clientQueueMutex = new GMutex();

    g_cond_init(guiCond);
    g_cond_init(clientCond);
    g_mutex_init(guiCondMutex);
    g_mutex_init(clientCondMutex);

    g_mutex_init(clientCallMutex);
    g_mutex_init(guiQueueMutex);
    g_mutex_init(clientQueueMutex);

    g_rw_lock_init(&entryMutex);
#endif

#if !GLIB_CHECK_VERSION(2,32,0)
    GError *error = NULL;
    guiThread = g_thread_create(threadFunc_gui, (gpointer)this, TRUE, &error);
    if (error)
    {
        cerr << "Unable to create gui thread: " << error->message << endl;
        g_error_free(error);
        exit(EXIT_FAILURE);
    }

    g_clear_error(&error);

    clientThread = g_thread_create(threadFunc_client, (gpointer)this, TRUE, &error);
    if (error)
    {
        cerr << "Unable to create client thread: " << error->message << endl;
        g_error_free(error);
        exit(EXIT_FAILURE);
    }
#else
    guiThread = g_thread_new("gtkgui", threadFunc_gui, (gpointer)this);
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
}

WulforManager::~WulforManager()
{
    abort = TRUE;

    g_mutex_lock(guiCondMutex);
    guiCondValue++;
    g_cond_signal(guiCond);
    g_mutex_unlock(guiCondMutex);

    g_mutex_lock(clientCondMutex);
    clientCondValue++;
    g_cond_signal(clientCond);
    g_mutex_unlock(clientCondMutex);

    g_thread_join(guiThread);
    g_thread_join(clientThread);

#if !GLIB_CHECK_VERSION(2,32,0)
    g_cond_free(guiCond);
    g_cond_free(clientCond);
    g_mutex_free(clientCondMutex);
    g_mutex_free(guiCondMutex);
    g_mutex_free(clientCallMutex);
    g_mutex_free(guiQueueMutex);
    g_mutex_free(clientQueueMutex);

    g_static_rw_lock_free(&entryMutex);
#else
    g_cond_clear(guiCond);
    g_cond_clear(clientCond);
    g_mutex_clear(clientCondMutex);
    g_mutex_clear(guiCondMutex);
    g_mutex_clear(clientCallMutex);
    g_mutex_clear(guiQueueMutex);
    g_mutex_clear(clientQueueMutex);

    delete guiCond;
    delete clientCond;
    delete clientCondMutex;
    delete guiCondMutex;
    delete clientCallMutex;
    delete guiQueueMutex;
    delete clientQueueMutex;

    g_rw_lock_clear(&entryMutex);
#endif

}

void WulforManager::createMainWindow()
{
    dcassert(!mainWin);
    mainWin = new MainWindow();
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

gpointer WulforManager::threadFunc_gui(gpointer data)
{
    WulforManager *man = (WulforManager *)data;
    man->processGuiQueue();
    return NULL;
}

gpointer WulforManager::threadFunc_client(gpointer data)
{
    WulforManager *man = (WulforManager *)data;
    man->processClientQueue();
    return NULL;
}

void WulforManager::processGuiQueue()
{
    FuncBase *func;

    while (!abort)
    {
        g_mutex_lock(guiCondMutex);
        while (guiCondValue < 1)
            g_cond_wait(guiCond, guiCondMutex);
        guiCondValue--;
        g_mutex_unlock(guiCondMutex);

        // This must be taken before the queuelock to avoid deadlock.
        gdk_threads_enter();

        g_mutex_lock(guiQueueMutex);
        while (!guiFuncs.empty())
        {
            func = guiFuncs.front();
            guiFuncs.pop_front();
            g_mutex_unlock(guiQueueMutex);

            func->call();
            delete func;

            g_mutex_lock(guiQueueMutex);
        }
        g_mutex_unlock(guiQueueMutex);

        gdk_threads_leave();
    }

    g_thread_exit(NULL);
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
        g_mutex_lock(guiQueueMutex);
        guiFuncs.push_back(func);
        g_mutex_unlock(guiQueueMutex);

        g_mutex_lock(guiCondMutex);
        guiCondValue++;
        g_cond_signal(guiCond);
        g_mutex_unlock(guiCondMutex);
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

string WulforManager::getPath()
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

// Should be called from a callback, so gdk_threads_enter/leave is called automatically.
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

    g_mutex_lock(guiQueueMutex);
    fIt = guiFuncs.begin();
    while (fIt != guiFuncs.end())
    {
        if ((*fIt)->getID() == id)
        {
            delete *fIt;
            fIt = guiFuncs.erase(fIt);
        }
        else
            ++fIt;
    }

    g_mutex_unlock(guiQueueMutex);

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
    Hash *h = new Hash();
    gint response = h->run();

    return response;
}

gint WulforManager::openSettingsDialog_gui()
{
    Settings *s = new Settings();
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
