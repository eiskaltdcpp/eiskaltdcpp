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

#pragma once

#include <gtk/gtk.h>
#include <cassert>
#include <map>
#include <string>

class TreeView
{
    public:
        typedef enum
        {
            STRING,
            STRINGR,
            INT,
            BOOL,
            PIXBUF,
            PIXBUF_STRING,
            ICON_STRING,
            ICON_STRING_TEXT_COLOR,
            EDIT_STRING,
            PROGRESS,
            SPEED,
            SIZE,
            TIME_LEFT
        } columnType;

        TreeView();
        ~TreeView();
        void setView(GtkTreeView *view);
        void setView(GtkTreeView *view, bool padding, const std::string &name = "");
        GtkTreeView *get();
        void insertColumn(const std::string &title, const GType &gtype, const columnType type, const int width, const std::string &linkedCol = "");
        void insertColumn(const std::string &title, const GType &gtype, const columnType type, const int width,
            const std::string &linkedCol, const std::string &linkedTextColor);
        void insertHiddenColumn(const std::string &title, const GType &gtype);
        void finalize();
        int getColCount();
        int getRowCount();
        GType* getGTypes();
        void setSortColumn_gui(const std::string &column, const std::string &sortColumn);
        int col(const std::string &title);
        void saveSettings();
        std::string getString(GtkTreeIter *i, const std::string &column, GtkTreeModel *m = NULL);
        template<class T>
        T getValue(GtkTreeIter *i, const std::string &column, GtkTreeModel *m = NULL)
        {
            if (m == NULL)
                m = gtk_tree_view_get_model(view);
            T value;
            assert(gtk_tree_model_get_column_type(m, col(column)) != G_TYPE_STRING);
            gtk_tree_model_get(m, i, col(column), &value, -1);
            return value;
        }
        template<class T, class C>
        C getValue(GtkTreeIter *i, const std::string &column, GtkTreeModel *m = NULL)
        {
            return static_cast<C>(getValue<T>(i, column, m));
        }

    private:
        class Column
        {
            public:
                Column() {};
                Column(const std::string &title, int id, GType gtype, TreeView::columnType type, int width, const std::string &linkedCol = "") :
                    title(title), id(id), gtype(gtype), type(type), width(width), pos(id), linkedCol(linkedCol), visible(true) {};
                Column(const std::string &title, int id, GType gtype, TreeView::columnType type, int width,
                    const std::string &linkedCol, const std::string &linkedTextColor) :
                    title(title),
                    id(id),
                    gtype(gtype),
                    type(type),
                    width(width),
                    pos(id),
                    linkedCol(linkedCol),
                    linkedTextColor(linkedTextColor),
                    visible(true) {};
                Column(const std::string &title, int id, GType gtype) :
                    title(title), id(id), gtype(gtype), pos(id) {};
                std::string title;
                int id;
                GType gtype;
                TreeView::columnType type;
                int width;
                int pos;
                std::string linkedCol;
                std::string linkedTextColor;
                bool visible;
                bool operator<(const Column &right) const
                {
                    return pos < right.pos;
                }
        };

        void addColumn_gui(Column& column);
        void restoreSettings();
        static gboolean popupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static void toggleColumnVisibility(GtkMenuItem *item, gpointer data);
        static void speedDataFunc(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
        static void sizeDataFunc(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
        static void timeLeftDataFunc(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);

        GtkTreeView *view;
        std::string name; // Used to save settings
        bool padding;
        int count;
        int visibleColumns;
        GtkMenu *menu;
        GType *gtypes;
        std::map<std::string, GtkWidget*> colMenuItems;

        typedef std::map<std::string, Column> ColMap;
        typedef std::map<int, std::string> SortedColMap;
        typedef ColMap::iterator ColIter;
        typedef SortedColMap::iterator SortedColIter;
        ColMap columns;
        SortedColMap sortedColumns;
        ColMap hiddenColumns;
        SortedColMap sortedHiddenColumns;
};
