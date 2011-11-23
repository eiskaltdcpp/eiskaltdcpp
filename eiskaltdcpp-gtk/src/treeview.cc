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

#include "treeview.hh"
#include "settingsmanager.hh"
#include "WulforUtil.hh"

#include <glib/gi18n.h>

using namespace std;

TreeView::TreeView()
{
    view = NULL;
    count = 0;
    padding = false;
    gtypes = NULL;
}

TreeView::~TreeView()
{
    if (!name.empty())
        saveSettings();
    if (gtypes)
        delete [] gtypes;
    g_object_unref(menu);
}

void TreeView::setView(GtkTreeView *view)
{
    this->view = view;
    gtk_tree_view_set_headers_clickable(view, TRUE);
    gtk_tree_view_set_rubber_banding(view, TRUE);
}

void TreeView::setView(GtkTreeView *view, bool padding, const string &name)
{
    this->view = view;
    this->padding = padding;
    this->name = name;
    gtk_tree_view_set_headers_clickable(view, TRUE);
    gtk_tree_view_set_rubber_banding(view, TRUE);
}

GtkTreeView *TreeView::get()
{
    return view;
}

/*
 * We can't use getValue() for strings since it would cause a memory leak.
 */
string TreeView::getString(GtkTreeIter *i, const string &column, GtkTreeModel *m)
{
    if (m == NULL)
        m = gtk_tree_view_get_model(view);
    string value;
    gchar* temp;
    dcassert(gtk_tree_model_get_column_type(m, col(column)) == G_TYPE_STRING);
    gtk_tree_model_get(m, i, col(column), &temp, -1);

    if (temp != NULL)
    {
        value = string(temp);
        g_free(temp);
    }

    return value;
}

void TreeView::insertColumn(const string &title, const GType &gtype, const columnType type, const int width, const string &linkedCol)
{
    // All insertColumn's have to be called before any insertHiddenColumn's.
    dcassert(hiddenColumns.size() == 0);

    // Title must be unique.
    dcassert(!title.empty() && columns.find(title) == columns.end());

    columns[title] = Column(title, count, gtype, type, width, linkedCol);
    sortedColumns[count] = title;
    ++count;
}

void TreeView::insertColumn(const string &title, const GType &gtype, const columnType type, const int width,
    const string &linkedCol, const string &linkedTextColor)
{
    // All insertColumn's have to be called before any insertHiddenColumn's.
    dcassert(hiddenColumns.size() == 0);

    // Title must be unique.
    dcassert(!title.empty() && columns.find(title) == columns.end());

    columns[title] = Column(title, count, gtype, type, width, linkedCol, linkedTextColor);
    sortedColumns[count] = title;
    ++count;
}

void TreeView::insertHiddenColumn(const string &title, const GType &gtype)
{
    // Title must be unique.
    dcassert(!title.empty());
    dcassert(hiddenColumns.find(title) == hiddenColumns.end());
    dcassert(columns.find(title) == columns.end());

    hiddenColumns[title] = Column(title, count, gtype);
    sortedHiddenColumns[count] = title;
    ++count;
}

void TreeView::finalize()
{
    dcassert(count > 0);

    menu = GTK_MENU(gtk_menu_new());
    g_object_ref_sink(menu);
    visibleColumns = columns.size();

    if (!name.empty())
        restoreSettings();

    for (SortedColIter iter = sortedColumns.begin(); iter != sortedColumns.end(); iter++)
    {
        Column& col = columns[iter->second];
        addColumn_gui(col);

        colMenuItems[col.title] = gtk_check_menu_item_new_with_label(col.title.c_str());
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(colMenuItems[col.title]), col.visible);
        g_signal_connect(colMenuItems[col.title], "activate", G_CALLBACK(toggleColumnVisibility), (gpointer)this);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), colMenuItems[col.title]);

        if (!col.visible)
            visibleColumns--;
    }

    if (padding)
    {
        GtkTreeViewColumn *col = gtk_tree_view_column_new();
        // Set to fixed so that gtk_tree_view_set_fixed_height() doesn't complain.
        gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_insert_column(view, col, count);
    }
}

/*
 * This is the total number of columns, including hidden columns.
 */
int TreeView::getColCount()
{
    return count;
}

/*
 * Slow method. Shouldn't be used unless necessary.
 */
int TreeView::getRowCount()
{
    GtkTreeIter iter;
    GtkTreeModel *m = gtk_tree_view_get_model(view);
    int numRows = 0;
    gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

    while (valid)
    {
        ++numRows;
        valid = gtk_tree_model_iter_next(m, &iter);
    }

    return numRows;
}

GType* TreeView::getGTypes()
{
    int i = 0;
    if (gtypes != NULL)
        return gtypes;
    gtypes = new GType[count];

    for (SortedColIter iter = sortedColumns.begin(); iter != sortedColumns.end(); iter++)
        gtypes[i++] = columns[iter->second].gtype;
    for (SortedColIter iter = sortedHiddenColumns.begin(); iter != sortedHiddenColumns.end(); iter++)
        gtypes[i++] = hiddenColumns[iter->second].gtype;

    return gtypes;
}

void TreeView::speedDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer column)
{
    string speedString;
    int64_t speed;
    gtk_tree_model_get(model, iter, static_cast<Column*>(column)->pos, &speed, -1);

    if (speed >= 0)
    {
        speedString = dcpp::Util::formatBytes(speed) + "/" + _("s");
    }

    g_object_set(renderer, "text", speedString.c_str(), NULL);
}

void TreeView::sizeDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer column)
{
    string sizeString;
    int64_t size;
    gtk_tree_model_get(model, iter, static_cast<Column*>(column)->pos, &size, -1);

    if (size >= 0)
    {
        sizeString = dcpp::Util::formatBytes(size);
    }

    g_object_set(renderer, "text", sizeString.c_str(), NULL);
}

void TreeView::timeLeftDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer column)
{
    string timeLeftString;
    int64_t seconds;
    gtk_tree_model_get(model, iter, static_cast<Column*>(column)->pos, &seconds, -1);

    if (seconds >= 0)
    {
        timeLeftString = dcpp::Util::formatSeconds(seconds);
    }

    g_object_set(renderer, "text", timeLeftString.c_str(), NULL);
}

void TreeView::addColumn_gui(Column& column)
{
    GtkTreeViewColumn *col = NULL;
    GtkCellRenderer *renderer;

    switch (column.type)
    {
        case INT:
        case STRING:
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(),
                gtk_cell_renderer_text_new(), "text", column.pos, NULL);
            break;
        case SIZE:
            renderer = gtk_cell_renderer_text_new();
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(),
                    renderer, "text", column.pos, NULL);
            gtk_tree_view_column_set_cell_data_func(col, renderer, TreeView::sizeDataFunc, &column, NULL);
            break;
        case SPEED:
            renderer = gtk_cell_renderer_text_new();
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(),
                    renderer, "text", column.pos, NULL);
            gtk_tree_view_column_set_cell_data_func(col, renderer, TreeView::speedDataFunc, &column, NULL);
            break;
        case TIME_LEFT:
            renderer = gtk_cell_renderer_text_new();
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(),
                    renderer, "text", column.pos, NULL);
            gtk_tree_view_column_set_cell_data_func(col, renderer, TreeView::timeLeftDataFunc, &column, NULL);
            break;
        case STRINGR:
            renderer = gtk_cell_renderer_text_new();
            g_object_set(renderer, "xalign", 1.0, NULL);
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(), renderer, "text", column.pos, NULL);
            gtk_tree_view_column_set_alignment(col, 1.0);
            break;
        case BOOL:
            renderer = gtk_cell_renderer_toggle_new();
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(), renderer, "active", column.pos, NULL);
            break;
        case PIXBUF:
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(),
                gtk_cell_renderer_pixbuf_new(), "pixbuf", column.pos, NULL);
            break;
        case PIXBUF_STRING:
            renderer = gtk_cell_renderer_pixbuf_new();
            col = gtk_tree_view_column_new();
            gtk_tree_view_column_set_title(col, column.title.c_str());
            gtk_tree_view_column_pack_start(col, renderer, false);
            gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", TreeView::col(column.linkedCol));
            renderer = gtk_cell_renderer_text_new();
            gtk_tree_view_column_pack_start(col, renderer, true);
            gtk_tree_view_column_add_attribute(col, renderer, "text", column.pos);
            break;
        case ICON_STRING:
            renderer = gtk_cell_renderer_pixbuf_new();
            col = gtk_tree_view_column_new();
            gtk_tree_view_column_set_title(col, column.title.c_str());
            gtk_tree_view_column_pack_start(col, renderer, false);
            gtk_tree_view_column_add_attribute(col, renderer, "stock-id", TreeView::col(column.linkedCol));
            renderer = gtk_cell_renderer_text_new();
            gtk_tree_view_column_pack_start(col, renderer, true);
            gtk_tree_view_column_add_attribute(col, renderer, "text", column.pos);
            break;
        case ICON_STRING_TEXT_COLOR:
            // icon
            renderer = gtk_cell_renderer_pixbuf_new();
            col = gtk_tree_view_column_new();
            gtk_tree_view_column_set_title(col, column.title.c_str());
            gtk_tree_view_column_pack_start(col, renderer, false);
            gtk_tree_view_column_add_attribute(col, renderer, "stock-id", TreeView::col(column.linkedCol));
            // text
            renderer = gtk_cell_renderer_text_new();
            gtk_tree_view_column_pack_start(col, renderer, true);
            gtk_tree_view_column_add_attribute(col, renderer, "text", column.pos);
            //gtk_tree_view_column_add_attribute(col, renderer, "foreground", TreeView::col(column.linkedTextColor));
            break;
        case EDIT_STRING:
            renderer = gtk_cell_renderer_text_new();
            g_object_set(renderer, "editable", TRUE, NULL);
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(), renderer, "text", column.pos, NULL);
            break;
        case PROGRESS:
            renderer = gtk_cell_renderer_progress_new();
            g_object_set(renderer, "xalign", 0.0, NULL); // Doesn't work yet. See: http://bugzilla.gnome.org/show_bug.cgi?id=334576
            col = gtk_tree_view_column_new_with_attributes(column.title.c_str(),
                renderer, "text", column.pos, "value", TreeView::col(column.linkedCol), NULL);
            break;
    }

    if (!col)
        return;

    // If columns are too small, they can't be manipulated
    if (column.width >= 20)
    {
        gtk_tree_view_column_set_fixed_width(col, column.width);
        gtk_tree_view_column_set_resizable(col, TRUE);
    }
    if (column.width != -1)
        gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);

    //make columns sortable
    if (column.type != BOOL && column.type != PIXBUF && column.type != EDIT_STRING)
    {
        gtk_tree_view_column_set_sort_column_id(col, column.pos);
        gtk_tree_view_column_set_sort_indicator(col, TRUE);
    }

    gtk_tree_view_column_set_clickable(col, TRUE);
    gtk_tree_view_column_set_reorderable(col, true);
    gtk_tree_view_column_set_visible(col, column.visible);

    gtk_tree_view_insert_column(view, col, column.pos);

    /*
     * Breaks GTK+ API, but is the only way to attach a signal to a gtktreeview column header. See GTK bug #141937.
     * @todo: Replace when GTK adds a way to add a signal to the entire header (remove visibleColumns var, too).
     */
#if GTK_CHECK_VERSION(3, 0, 0)
    g_signal_connect(gtk_tree_view_column_get_button(col) , "button-release-event", G_CALLBACK(popupMenu_gui), (gpointer)this);
#else
    g_signal_connect(col->button, "button-release-event", G_CALLBACK(popupMenu_gui), (gpointer)this);
#endif
}

void TreeView::setSortColumn_gui(const string &column, const string &sortColumn)
{
    GtkTreeViewColumn *gtkColumn = gtk_tree_view_get_column(view, col(column));
    gtk_tree_view_column_set_sort_column_id(gtkColumn, col(sortColumn));
}

int TreeView::col(const string &title)
{
    dcassert(!title.empty());
    dcassert(columns.find(title) != columns.end() || hiddenColumns.find(title) != hiddenColumns.end());
    int retval = -1;

    if (columns.find(title) != columns.end())
        retval = columns[title].pos;
    else
        retval = hiddenColumns[title].id;

    dcassert(retval >= 0 && (retval < count));
    return retval;
}

gboolean TreeView::popupMenu_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    TreeView *tv = (TreeView*)data;

    if (event->button == 3)
    {
        gtk_menu_popup(tv->menu, NULL, NULL, NULL, NULL, (event != NULL) ? event->button : 0, gdk_event_get_time((GdkEvent*)event));
        gtk_widget_show_all(GTK_WIDGET(tv->menu));
        return true;
    }
    else
        return false;
}

void TreeView::toggleColumnVisibility(GtkMenuItem *item, gpointer data)
{
    TreeView *tv = (TreeView*)data;
    GtkTreeViewColumn *column = NULL;
    gboolean visible;
    SortedColIter iter;
    string title = string(gtk_label_get_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(item)))));

    // Function col(title) doesn't work here, so we have to find column manually.
    for (iter = tv->sortedColumns.begin(); iter != tv->sortedColumns.end(); iter++)
    {
        column = gtk_tree_view_get_column(tv->view, iter->first);
        if (string(gtk_tree_view_column_get_title(column)) == title)
            break;
    }

    if (!column)
        return;

    visible = !gtk_tree_view_column_get_visible(column);

    // Can't let number of visible columns fall below 1, otherwise there's no way to unhide columns.
    if (!visible && tv->visibleColumns <= 1)
    {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(tv->colMenuItems[title]), true);
        return;
    }

    gtk_tree_view_column_set_visible(column, visible);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(tv->colMenuItems[title]), visible);

    if (visible)
    {
        tv->visibleColumns++;
        // Seems to be a bug in gtk where sometimes columns are unresizable after being made visible
        gtk_tree_view_column_set_resizable(column, TRUE);
    }
    else
        tv->visibleColumns--;
}

void TreeView::restoreSettings()
{
    vector<int> columnOrder, columnWidth, columnVisibility;
    columnOrder = WulforUtil::splitString(WGETS(name + "-order"), ",");
    columnWidth = WulforUtil::splitString(WGETS(name + "-width"), ",");
    columnVisibility = WulforUtil::splitString(WGETS(name + "-visibility"), ",");

    if (columns.size() == columnOrder.size() &&
        columnOrder.size() == columnWidth.size() &&
        columnWidth.size() == columnVisibility.size())
    {
        for (ColIter iter = columns.begin(); iter != columns.end(); ++iter)
        {
            for (size_t i = 0; i < columns.size(); i++)
            {
                if (iter->second.id == columnOrder.at(i))
                {
                    iter->second.pos = i;
                    sortedColumns[i] = iter->second.title;
                    if (columnWidth.at(i) >= 20)
                        iter->second.width = columnWidth.at(i);
                    if (columnVisibility.at(i) == 0 || columnVisibility.at(i) == 1)
                        iter->second.visible = columnVisibility.at(i);
                    break;
                }
            }
        }
    }
}

void TreeView::saveSettings()
{
    string columnOrder, columnWidth, columnVisibility, title;
    GtkTreeViewColumn *col;
    gint width;

    dcassert(columns.size() > 0);

    for (size_t i = 0; i < columns.size(); i++)
    {
        col = gtk_tree_view_get_column(view, i);
        if (col == NULL)
            continue;

        title = string(gtk_tree_view_column_get_title(col));
        width = gtk_tree_view_column_get_width(col);

        // A col was moved to the right of the padding col
        if (title.empty())
            return;

        columnOrder += dcpp::Util::toString(columns[title].id) + ",";
        if (width >= 20)
            columnWidth += dcpp::Util::toString(width) + ",";
        else
            columnWidth += dcpp::Util::toString(columns[title].width) + ",";
        columnVisibility += dcpp::Util::toString(gtk_tree_view_column_get_visible(col)) + ",";
    }

    if (columnOrder.size() > 0)
    {
        columnOrder.erase(columnOrder.size() - 1, 1);
        columnWidth.erase(columnWidth.size() - 1, 1);
        columnVisibility.erase(columnVisibility.size() - 1, 1);

        WSET(name + "-order", columnOrder);
        WSET(name + "-width", columnWidth);
        WSET(name + "-visibility", columnVisibility);
    }
}
