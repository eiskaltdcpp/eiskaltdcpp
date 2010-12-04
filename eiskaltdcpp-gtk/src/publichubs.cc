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

#include "publichubs.hh"
#include "wulformanager.hh"

using namespace std;
using namespace dcpp;

PublicHubs::PublicHubs():
	BookEntry(Entry::PUBLIC_HUBS, _("Public Hubs"), "publichubs.glade"),
	hubs(0),
	filter("")
{
	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("configureDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// menu
	g_object_ref_sink(getWidget("menu"));

	// Initialize public hub list treeview
	hubView.setView(GTK_TREE_VIEW(getWidget("hubView")), true, "publichubs");
	hubView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 200);
	hubView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 350);
	hubView.insertColumn(_("Users"), G_TYPE_INT, TreeView::INT, 75);
	hubView.insertColumn(_("Address"), G_TYPE_STRING, TreeView::STRING, 110);
	hubView.insertColumn(_("Country"), G_TYPE_STRING, TreeView::STRING, 100);
	hubView.insertColumn(_("Shared"), G_TYPE_INT64, TreeView::SIZE, 70);
	hubView.insertColumn(_("Min Share"), G_TYPE_INT64, TreeView::SIZE, 80);
	hubView.insertColumn(_("Min Slots"), G_TYPE_INT, TreeView::INT, 70);
	hubView.insertColumn(_("Max Hubs"), G_TYPE_INT, TreeView::INT, 80);
	hubView.insertColumn(_("Max Users"), G_TYPE_INT, TreeView::INT, 80);
	hubView.insertColumn(_("Rating"), G_TYPE_STRING, TreeView::STRING, 70);
	hubView.finalize();
	hubStore = gtk_list_store_newv(hubView.getColCount(), hubView.getGTypes());
	gtk_tree_view_set_model(hubView.get(), GTK_TREE_MODEL(hubStore));
	g_object_unref(hubStore);
	hubSelection = gtk_tree_view_get_selection(hubView.get());
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(hubStore), hubView.col(_("Users")), GTK_SORT_DESCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(hubView.get(), hubView.col(_("Users"))), TRUE);
	gtk_tree_view_set_fixed_height_mode(hubView.get(), TRUE);

	// Initialize list of public hub lists treeview
	listsView.setView(GTK_TREE_VIEW(getWidget("listsView")));
	listsView.insertColumn(_("List"), G_TYPE_STRING, TreeView::EDIT_STRING, -1);
	listsView.finalize();
	listsStore = gtk_list_store_newv(listsView.getColCount(), listsView.getGTypes());
	gtk_tree_view_set_model(listsView.get(), GTK_TREE_MODEL(listsStore));
	g_object_unref(listsStore);
	gtk_tree_view_set_headers_visible(listsView.get(), FALSE);
	listsSelection = gtk_tree_view_get_selection(listsView.get());
	GtkTreeViewColumn *c = gtk_tree_view_get_column(listsView.get(), 0);
	GList *l = gtk_tree_view_column_get_cell_renderers(c);
	GObject *editRenderer = G_OBJECT(g_list_nth_data(l, 0));
	g_list_free(l);

	// Initialize the hub lists combo box
	gtk_combo_box_set_model(GTK_COMBO_BOX(getWidget("hubListBox")), GTK_TREE_MODEL(listsStore));
	GtkCellRenderer *cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(GTK_COMBO_BOX(getWidget("hubListBox"))), cell, FALSE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(GTK_COMBO_BOX(getWidget("hubListBox"))), cell, "text", 0);

	// Connect the signals to their callback functions.
	g_signal_connect(getContainer(), "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
	g_signal_connect(getWidget("filterEntry"), "key-release-event", G_CALLBACK(onFilterHubs_gui), (gpointer)this);
	g_signal_connect(getWidget("connectButton"), "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("connectMenuItem"), "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("refreshButton"), "clicked", G_CALLBACK(onRefresh_gui), (gpointer)this);
	g_signal_connect(getWidget("hubListBox"), "changed", G_CALLBACK(onRefresh_gui), (gpointer)this);
	g_signal_connect(getWidget("configureButton"), "clicked", G_CALLBACK(onConfigure_gui), (gpointer)this);
	g_signal_connect(getWidget("upButton"), "clicked", G_CALLBACK(onMoveUp_gui), (gpointer)this);
	g_signal_connect(getWidget("downButton"), "clicked", G_CALLBACK(onMoveDown_gui), (gpointer)this);
	g_signal_connect(getWidget("addButton"), "clicked", G_CALLBACK(onAdd_gui), (gpointer)this);
	g_signal_connect(getWidget("removeButton"), "clicked", G_CALLBACK(onRemove_gui), (gpointer)this);
	g_signal_connect(editRenderer, "edited", G_CALLBACK(onCellEdited_gui), (gpointer)this);
	g_signal_connect(hubView.get(), "button-press-event", G_CALLBACK(onButtonPress_gui), (gpointer)this);
	g_signal_connect(hubView.get(), "button-release-event", G_CALLBACK(onButtonRelease_gui), (gpointer)this);
	g_signal_connect(hubView.get(), "key-release-event", G_CALLBACK(onKeyRelease_gui), (gpointer)this);
	g_signal_connect(getWidget("favMenuItem"), "activate", G_CALLBACK(onAddFav_gui), (gpointer)this);
}

PublicHubs::~PublicHubs()
{
	FavoriteManager::getInstance()->removeListener(this);
	gtk_widget_destroy(getWidget("configureDialog"));
	g_object_unref(getWidget("menu"));
}

void PublicHubs::show()
{
	buildHubList_gui();

	FavoriteManager::getInstance()->addListener(this);
	Func0<PublicHubs> *func = new Func0<PublicHubs>(this, &PublicHubs::downloadList_client);
	WulforManager::get()->dispatchClientFunc(func);
}

// Populate the public hubs list
void PublicHubs::buildHubList_gui()
{
 	StringList list = FavoriteManager::getInstance()->getHubLists();
	int selected = FavoriteManager::getInstance()->getSelectedHubList();

	for (StringList::iterator it = list.begin(); it != list.end(); ++it)
	{
		GtkTreeIter iter;
		gtk_list_store_append(listsStore, &iter);
		gtk_list_store_set(listsStore, &iter, 0, it->c_str(), -1);
	}

	if (static_cast<size_t>(selected) >= list.size())
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("hubListBox")), list.size() - 1);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(getWidget("hubListBox")), selected);
}

void PublicHubs::updateList_gui()
{
	HubEntryList::const_iterator i;
	GtkTreeIter iter;
	int numHubs = 0;
	int numUsers = 0;
	gint sortColumn;
	GtkSortType sortType;

	gtk_list_store_clear(hubStore);

	gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(hubStore), &sortColumn, &sortType);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(hubStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, sortType);

	for (i = hubs.begin(); i != hubs.end(); i++)
	{
		if (filter.getPattern().empty() || filter.match(i->getName()) ||
			filter.match(i->getDescription()) || filter.match(i->getServer()))
		{
			gtk_list_store_append(hubStore, &iter);
			gtk_list_store_set(hubStore, &iter,
				hubView.col(_("Name")), i->getName().c_str(),
				hubView.col(_("Description")), i->getDescription().c_str(),
				hubView.col(_("Users")), i->getUsers(),
				hubView.col(_("Address")), i->getServer().c_str(),
				hubView.col(_("Country")), i->getCountry().c_str(),
				hubView.col(_("Shared")), (int64_t)i->getShared(),
				hubView.col(_("Min Share")), (int64_t)i->getMinShare(),
				hubView.col(_("Min Slots")), i->getMinSlots(),
				hubView.col(_("Max Hubs")), i->getMaxHubs(),
				hubView.col(_("Max Users")), i->getMaxUsers(),
				hubView.col(_("Rating")), i->getRating().c_str(),
				-1);

			numUsers += i->getUsers();
			numHubs++;
		}
	}

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(hubStore), sortColumn, sortType);

	setStatus_gui("statusHubs", _("Hubs: ") + Util::toString(numHubs));
	setStatus_gui("statusUsers", _("Users: ") + Util::toString(numUsers));
}

void PublicHubs::setStatus_gui(string statusBar, string text)
{
	gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
	gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
}

gboolean PublicHubs::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	gtk_widget_grab_focus(ph->getWidget("filterEntry"));

	return TRUE;
}

gboolean PublicHubs::onButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS)
	{
		ph->oldType = event->type;
		ph->oldButton = event->button;
	}

	return FALSE;
}

gboolean PublicHubs::onButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	if (ph->oldButton == event->button && gtk_tree_selection_get_selected(ph->hubSelection, NULL, NULL))
	{
		if (event->button == 3 && ph->oldType == GDK_BUTTON_PRESS)
		{
			gtk_menu_popup(GTK_MENU(ph->getWidget("menu")), NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(ph->getWidget("menu"));
		}
		else if (event->button == 1 && ph->oldType == GDK_2BUTTON_PRESS)
		{
			ph->onConnect_gui(NULL, data);
		}
	}

	return FALSE;
}

gboolean PublicHubs::onKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	if (gtk_tree_selection_get_selected(ph->hubSelection, NULL, NULL))
	{
		if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(ph->getWidget("menu")), NULL, NULL, NULL, NULL, 0, event->time);
			gtk_widget_show_all(ph->getWidget("menu"));
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			ph->onConnect_gui(NULL, data);
		}
	}

	return FALSE;
}

gboolean PublicHubs::onFilterHubs_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	StringSearch pattern(gtk_entry_get_text(GTK_ENTRY(ph->getWidget("filterEntry"))));

	if (!(pattern == ph->filter))
	{
		ph->filter = pattern;
		ph->updateList_gui();
	}

	return FALSE;
}

void PublicHubs::onConnect_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(ph->hubSelection, NULL, &iter))
	{
		string address = ph->hubView.getString(&iter, _("Address"));
		WulforManager::get()->getMainWindow()->showHub_gui(address);
	}
}

void PublicHubs::onRefresh_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	int pos = gtk_combo_box_get_active(GTK_COMBO_BOX(ph->getWidget("hubListBox")));

	typedef Func1<PublicHubs, int> F1;
	F1 *func = new F1(ph, &PublicHubs::refresh_client, pos);
	WulforManager::get()->dispatchClientFunc(func);
}

void PublicHubs::onAddFav_gui(GtkMenuItem *item, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(ph->hubSelection, NULL, &iter))
	{
		FavoriteHubEntry entry;
		string name = ph->hubView.getString(&iter, _("Name"));
		string description = ph->hubView.getString(&iter, _("Description"));
		string address = ph->hubView.getString(&iter, _("Address"));

		entry.setName(name);
		entry.setServer(address);
		entry.setDescription(description);
		entry.setNick(SETTING(NICK));
		entry.setPassword("");
		entry.setUserDescription(SETTING(DESCRIPTION));

		typedef Func1<PublicHubs, FavoriteHubEntry> F1;
		F1 *func = new F1(ph, &PublicHubs::addFav_client, entry);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void PublicHubs::onConfigure_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;

	// Have to get active here since temp could be NULL after dialog is closed
	gchar *temp = gtk_combo_box_get_active_text(GTK_COMBO_BOX(ph->getWidget("hubListBox")));
	string active = string(temp);
	g_free(temp);

	gint response = gtk_dialog_run(GTK_DIALOG(ph->getWidget("configureDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(ph->getWidget("configureDialog"));

	if (response == GTK_RESPONSE_OK)
	{
		string lists, url;
		GtkTreeIter iter;

		GtkTreeModel *m = GTK_TREE_MODEL(ph->listsStore);
		gboolean valid = gtk_tree_model_get_iter_first(m, &iter);
		while (valid)
		{
			url = ph->listsView.getString(&iter, _("List"));
			lists += url + ";";
			if (url == active)
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ph->getWidget("hubListBox")), &iter);
			valid = gtk_tree_model_iter_next(m, &iter);
		}

		if (gtk_combo_box_get_active(GTK_COMBO_BOX(ph->getWidget("hubListBox"))) < 0)
			gtk_combo_box_set_active(GTK_COMBO_BOX(ph->getWidget("hubListBox")), 0);

		if (!lists.empty())
			lists.erase(lists.size() - 1);

		SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, lists);
	}
}

void PublicHubs::onAdd_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;

	gtk_list_store_append(ph->listsStore, &iter);
	gtk_list_store_set(ph->listsStore, &iter, ph->listsView.col(_("List")), _("New list"), -1);
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(ph->listsStore), &iter);
	col = gtk_tree_view_get_column(ph->listsView.get(), 0);
	gtk_tree_view_set_cursor(ph->listsView.get(), path, col, TRUE);
	gtk_tree_path_free(path);
}

void PublicHubs::onMoveUp_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(ph->listsStore);

	if (gtk_tree_selection_get_selected(ph->listsSelection, NULL, &current))
	{
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
			gtk_list_store_swap(ph->listsStore, &current, &prev);
		gtk_tree_path_free(path);
	}
}

void PublicHubs::onMoveDown_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter current, next;

	if (gtk_tree_selection_get_selected(ph->listsSelection, NULL, &current))
	{
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(ph->listsStore), &next))
			gtk_list_store_swap(ph->listsStore, &current, &next);
	}
}

void PublicHubs::onRemove_gui(GtkWidget *widget, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter current;

	if (gtk_tree_selection_get_selected(ph->listsSelection, NULL, &current))
		gtk_list_store_remove(ph->listsStore, &current);
}

void PublicHubs::onCellEdited_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data)
{
	PublicHubs *ph = (PublicHubs *)data;
	GtkTreeIter it;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ph->listsStore), &it, path))
		gtk_list_store_set(ph->listsStore, &it, ph->listsView.col(_("List")), text, -1);
}

void PublicHubs::downloadList_client()
{
	hubs = FavoriteManager::getInstance()->getPublicHubs();

	if (hubs.empty())
		FavoriteManager::getInstance()->refresh();

	FavoriteManager::getInstance()->save();

	Func0<PublicHubs> *func = new Func0<PublicHubs>(this, &PublicHubs::updateList_gui);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::refresh_client(int pos)
{
	FavoriteManager::getInstance()->setHubList(pos);
	FavoriteManager::getInstance()->refresh();
}

void PublicHubs::addFav_client(FavoriteHubEntry entry)
{
	FavoriteManager::getInstance()->addFavorite(entry);
}

void PublicHubs::on(FavoriteManagerListener::DownloadStarting, const string &file) throw()
{
	string msg = _("Download starting: ") + file;
	typedef Func2<PublicHubs, string, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, "statusMain", msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::on(FavoriteManagerListener::DownloadFailed, const string &file) throw()
{
	string msg = _("Download failed: ") + file;
	typedef Func2<PublicHubs, string, string> Func;
	Func *func = new Func(this, &PublicHubs::setStatus_gui, "statusMain", msg);
	WulforManager::get()->dispatchGuiFunc(func);
}

void PublicHubs::on(FavoriteManagerListener::DownloadFinished, const string &file) throw()
{
	string msg = _("Download finished: ") + file;
	typedef Func2<PublicHubs, string, string> Func;
	Func *f2 = new Func(this, &PublicHubs::setStatus_gui, "statusMain", msg);
	WulforManager::get()->dispatchGuiFunc(f2);

	hubs = FavoriteManager::getInstance()->getPublicHubs();

	Func0<PublicHubs> *f0 = new Func0<PublicHubs>(this, &PublicHubs::updateList_gui);
	WulforManager::get()->dispatchGuiFunc(f0);
}
