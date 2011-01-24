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

#include "hashdialog.hh"
#include <dcpp/HashManager.h>
#include "wulformanager.hh"

using namespace std;
using namespace dcpp;

Hash::Hash(GtkWindow* parent):
    DialogEntry(Entry::HASH_DIALOG, "hash.glade", parent)
{
    string tmp;
    startTime = GET_TICK();
    HashManager::getInstance()->getStats(tmp, startBytes, startFiles);
    HashManager::getInstance()->setPriority(Thread::NORMAL);
    updateStats_gui("", 0, 0, 0);
//NOTE: [core 0.762
        bool paused = HashManager::getInstance()->isHashingPaused();
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("pauseHashingToggleButton")), paused);

        if (paused)
                gtk_window_set_title(GTK_WINDOW(getContainer()), _("Paused..."));
        else
                gtk_window_set_title(GTK_WINDOW(getContainer()), _("Indexing files..."));

        g_signal_connect(getWidget("pauseHashingToggleButton"), "toggled", G_CALLBACK(onPauseHashing_gui), (gpointer)this);
//NOTE: core 0.762]
    TimerManager::getInstance()->addListener(this);
}

Hash::~Hash()
{
    HashManager::getInstance()->setPriority(Thread::IDLE);
    TimerManager::getInstance()->removeListener(this);
}

void Hash::updateStats_gui(string file, int64_t bytes, size_t files, uint64_t tick)
{
    if (bytes > startBytes)
        startBytes = bytes;

    if (files > startFiles)
        startFiles = files;

    double diff = tick - startTime;
    bool paused = HashManager::getInstance()->isHashingPaused();//NOTE: core 0.762

    if (diff < 1000 || files == 0 || bytes == 0 || paused)
    {
        gtk_label_set_text(GTK_LABEL(getWidget("labelSpeed")), string(string("-.-- ") + _("B/s") + ", " + Util::formatBytes(bytes) + _(" left")).c_str());
        gtk_label_set_text(GTK_LABEL(getWidget("labelTime")), _("-:--:-- left"));
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR(getWidget("progressbar")), "0%");
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbar")), 0.0);
    }
    else
    {
        double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

        gtk_label_set_text(GTK_LABEL(getWidget("labelSpeed")), string(Util::formatBytes((int64_t)speedStat) + "/" + _("s") + ", " + Util::formatBytes(bytes) + _(" left")).c_str());

        if (speedStat == 0)
        {
            gtk_label_set_text(GTK_LABEL(getWidget("labelTime")), _("-:--:-- left"));
        }
        else
        {
            double ss = (double)bytes / speedStat;
            gtk_label_set_text(GTK_LABEL(getWidget("labelTime")), string(Util::formatSeconds((int64_t)ss) + _(" left")).c_str());
        }
    }

    if (startFiles == 0 || startBytes == 0)
    {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")), "100%");
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbar")), 1.0);
    }
    else
    {
        double progress = ((0.5 * (double)(startFiles - files)/(double)startFiles) + (0.5 * (double)(startBytes - bytes)/(double)startBytes));
        char buf[24];
        snprintf(buf, sizeof(buf), "%.0lf%%", progress * 100);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")), buf);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbar")), progress);
    }

    if (files == 0)
    {
        gtk_label_set_text(GTK_LABEL(getWidget("labelFile")), _("Done"));
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(getWidget("progressbar")), "100%");
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(getWidget("progressbar")), 1.0);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(getWidget("labelFile")), file.c_str());
    }
}

void Hash::onPauseHashing_gui(GtkWidget *widget, gpointer data)
{
        Hash *h = (Hash *)data;
        bool paused = HashManager::getInstance()->isHashingPaused();

        if (paused)
        {
                gtk_window_set_title(GTK_WINDOW(h->getContainer()), _("Indexing files..."));
                HashManager::getInstance()->resumeHashing();
        }
        else
        {
                gtk_window_set_title(GTK_WINDOW(h->getContainer()), _("Paused..."));
                HashManager::getInstance()->pauseHashing();
        }
}

void Hash::on(TimerManagerListener::Second, uint64_t tics) throw()
{
    string file;
    int64_t bytes = 0;
    size_t files = 0;

    HashManager::getInstance()->getStats(file, bytes, files);

    typedef Func4<Hash, string, int64_t, size_t, uint64_t> F4;
    F4 *func = new F4(this, &Hash::updateStats_gui, file, bytes, files, GET_TICK());
    WulforManager::get()->dispatchGuiFunc(func);
}
