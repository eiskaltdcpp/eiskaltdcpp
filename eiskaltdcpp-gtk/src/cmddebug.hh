/*
 * Copyright © 2010 Mank <mank@besthub.eu>
 * Copyright © 2010 Eugene Petrov <dhamp@ya.ru>
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

#ifndef WULFOR_CMDDEBUG_HH
#define WULFOR_CMDDEBUG_HH

#include <dcpp/stdinc.h>

#include "bookentry.hh"
#include "treeview.hh"
#include "wulformanager.hh"


class cmddebug:
    public BookEntry,
    private dcpp::DebugManagerListener
{
    public:
        cmddebug();
        virtual ~cmddebug();
        virtual void show();
        //GUI FCE
        void add_gui(std::string file);

    private:
        // Client functions
        void ini_client();
        void addCmd(const std::string& cmd,const std::string& ip);

        //DebugManager
        void on(dcpp::DebugManagerListener::DebugDetection, const std::string& com) throw();
        void on(dcpp::DebugManagerListener::DebugCommand, const std::string& mess, int typedir, const std::string& ip) throw();

        static void onScroll_gui(GtkAdjustment *adjustment, gpointer data);
        static void onResize_gui(GtkAdjustment *adjustment, gpointer data);

        GtkTextBuffer *buffer;
        static const int maxLines = 1000;
        GtkTextIter iter;
        bool scrollToBottom;
        GtkTextMark *cmdMark;

};

#else
class cmddebug;
#endif
