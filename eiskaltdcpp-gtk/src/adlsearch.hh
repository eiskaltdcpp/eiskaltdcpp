/*
 * Copyright © 2009-2010 freedcpp, http://code.google.com/p/freedcpp
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

#pragma once

#include <dcpp/stdinc.h>
#include <dcpp/ADLSearch.h>
#include "bookentry.hh"
#include "treeview.hh"

class SearchADL:
    public BookEntry
{
    public:
        SearchADL();
        virtual ~SearchADL();
        virtual void show();
    private:
        typedef dcpp::ADLSearchManager::SearchCollection::size_type SearchType;

        // GUI functions
        void setSearch_gui(dcpp::ADLSearch &search, GtkTreeIter *iter);

        // GUI callbacks
        static void onAddClicked_gui(GtkWidget *widget, gpointer data);
        static void onPropertiesClicked_gui(GtkWidget *widget, gpointer data);
        static void onMoveUpClicked_gui(GtkWidget *widget, gpointer data);
        static void onMoveDownClicked_gui(GtkWidget *widget, gpointer data);
        static void onRemoveClicked_gui(GtkWidget *widget, gpointer data);
        static void onActiveToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
        static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
        static bool showPropertiesDialog_gui(dcpp::ADLSearch &search, bool edit, SearchADL *s);

        GdkEventType previous;
        TreeView searchADLView;
        GtkListStore *searchADLStore;
        GtkTreeSelection *searchADLSelection;
};
