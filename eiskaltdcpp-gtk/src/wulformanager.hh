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

#pragma once

#include <gtk/gtk.h>
#include <glib.h>
#include <string>
#include "dialogentry.hh"
#include "func.hh"
#include "mainwindow.hh"

namespace dcpp { class DCContext; }

class WulforManager
{
public:
    WulforManager(dcpp::DCContext& dcCtx, int argc, char **argv);
    ~WulforManager();

    WulforManager(const WulforManager&) = delete;
    WulforManager& operator=(const WulforManager&) = delete;

    std::string getURL();
    std::string getPath() const;
    MainWindow *getMainWindow();
    void deleteMainWindow();
    void dispatchGuiFunc(FuncBase *func);
    void dispatchClientFunc(FuncBase *func);

    void insertEntry_gui(Entry *entry);
    void deleteEntry_gui(Entry *entry);
    bool isEntry_gui(Entry *entry);

    // DialogEntry functions
    gint openHashDialog_gui();
    gint openSettingsDialog_gui();
    DialogEntry *getHashDialog_gui();
    DialogEntry *getSettingsDialog_gui();

    void onReceived_gui(const std::string &link);

private:
    // argv[1] from main
    static std::string argv1;

    // MainWindow-related functions
    void createMainWindow();

    // Entry functions
    DialogEntry *getDialogEntry_gui(const std::string &id);

    // Thread-related functions
    static gboolean idleCallback_gui(gpointer data);
    static gpointer threadFunc_client(gpointer data);
    void processClientQueue();

    MainWindow *mainWin;
    std::string path;
    std::deque<FuncBase *> clientFuncs;
    std::unordered_map<std::string, Entry *> entries;
    gint clientCondValue;
    GCond *clientCond;
    GMutex *clientCondMutex;
    GMutex *clientCallMutex;
    GMutex *clientQueueMutex;
#if !GLIB_CHECK_VERSION(2,32,0)
    GStaticRWLock entryMutex;
#else
    GRWLock entryMutex;
#endif
    dcpp::DCContext& dcCtx_;
    GThread *clientThread;
    bool abort;
};

/// Access the active WulforManager instance.
/// The pointer is set in wulfor.cc during the RAII scope block.
WulforManager *wulforManagerInstance();

/// Set/clear the active WulforManager instance.
void setWulforManagerInstance(WulforManager *instance);
