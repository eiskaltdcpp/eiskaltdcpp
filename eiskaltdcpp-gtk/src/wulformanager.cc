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
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include "hashdialog.hh"
#include "settingsdialog.hh"

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

	gdk_threads_enter();
	manager->createMainWindow();
	gdk_threads_leave();
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
	guiCond = g_cond_new();
	clientCond = g_cond_new();
	guiCondMutex = g_mutex_new();
	clientCondMutex = g_mutex_new();

	clientCallMutex = g_mutex_new();
	guiQueueMutex = g_mutex_new();
	clientQueueMutex = g_mutex_new();
	g_static_rw_lock_init(&entryMutex);

	GError *error = NULL;
	guiThread = g_thread_create(threadFunc_gui, (gpointer)this, TRUE, &error);
	if (error != NULL)
	{
		cerr << "Unable to create gui thread: " << error->message << endl;
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	g_clear_error(&error);

	clientThread = g_thread_create(threadFunc_client, (gpointer)this, TRUE, &error);
	if (error != NULL)
	{
		cerr << "Unable to create client thread: " << error->message << endl;
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

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

	g_cond_free(guiCond);
	g_cond_free(clientCond);
	g_mutex_free(clientCondMutex);
	g_mutex_free(guiCondMutex);
	g_mutex_free(clientCallMutex);
	g_mutex_free(guiQueueMutex);
	g_mutex_free(clientQueueMutex);
	g_static_rw_lock_free(&entryMutex);
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

	if (hashDialogEntry != NULL)
	{
		gtk_dialog_response(GTK_DIALOG(hashDialogEntry->getContainer()), GTK_RESPONSE_OK);
	}
	if (settingsDialogEntry != NULL)
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
		while (guiFuncs.size() > 0)
		{
			func = guiFuncs.front();
			guiFuncs.erase(guiFuncs.begin());
			g_mutex_unlock(guiQueueMutex);

			func->call();
			delete func;

			g_mutex_lock(guiQueueMutex);
		}
		g_mutex_unlock(guiQueueMutex);

		// Don't call gdk_flush() since it actually calls XSync, which can
		// block waiting on events
		XFlush(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()));
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
		while (clientFuncs.size() > 0)
		{
			func = clientFuncs.front();
			clientFuncs.erase(clientFuncs.begin());
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
	g_static_rw_lock_reader_lock(&entryMutex);

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

	g_static_rw_lock_reader_unlock(&entryMutex);
}

void WulforManager::dispatchClientFunc(FuncBase *func)
{
	g_static_rw_lock_reader_lock(&entryMutex);

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

	g_static_rw_lock_reader_unlock(&entryMutex);
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
	g_static_rw_lock_writer_lock(&entryMutex);
	entries[entry->getID()] = entry;
	g_static_rw_lock_writer_unlock(&entryMutex);
}

// Should be called from a callback, so gdk_threads_enter/leave is called automatically.
void WulforManager::deleteEntry_gui(Entry *entry)
{
	const string &id = entry->getID();
	vector<FuncBase *>::iterator fIt;

	g_mutex_lock(clientCallMutex);

	// Erase any pending calls to this bookentry.
	g_mutex_lock(clientQueueMutex);
	fIt = clientFuncs.begin();
	while (fIt != clientFuncs.end())
	{
		if ((*fIt)->getID() == id)
		{
			delete *fIt;
			clientFuncs.erase(fIt);
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
			guiFuncs.erase(fIt);
		}
		else
			++fIt;
	}
	g_mutex_unlock(guiQueueMutex);

	// Remove the bookentry from the list.
	g_static_rw_lock_writer_lock(&entryMutex);
	if (entries.find(id) != entries.end())
		entries.erase(id);
	g_static_rw_lock_writer_unlock(&entryMutex);

	g_mutex_unlock(clientCallMutex);

	delete entry;
	entry = NULL;
}

DialogEntry* WulforManager::getDialogEntry_gui(const string &id)
{
	DialogEntry *ret = NULL;

	g_static_rw_lock_reader_lock(&entryMutex);
	if (entries.find(id) != entries.end())
		ret = dynamic_cast<DialogEntry *>(entries[id]);
	g_static_rw_lock_reader_unlock(&entryMutex);

	return ret;
}

void WulforManager::onReceived_gui(const string link)
{
	dcassert(mainWin);

	if (WulforUtil::isHubURL(link) && BOOLSETTING(URL_HANDLER))
		mainWin->showHub_gui(link);

	else if (WulforUtil::isMagnet(link) && BOOLSETTING(MAGNET_REGISTER))
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
