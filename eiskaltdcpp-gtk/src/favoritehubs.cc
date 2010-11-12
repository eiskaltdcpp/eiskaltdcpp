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

#include "favoritehubs.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

FavoriteHubs::FavoriteHubs():
	BookEntry(Entry::FAVORITE_HUBS, _("Favorite Hubs"), "favoritehubs.glade")
{
	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("favoriteHubsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	gtk_widget_set_sensitive(getWidget("comboboxCharset"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryNick"), FALSE);
	gtk_widget_set_sensitive(getWidget("entryUserDescription"), FALSE);

	// menu
	g_object_ref_sink(getWidget("menu"));

	gtk_window_set_transient_for(GTK_WINDOW(getWidget("favoriteHubsDialog")), GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(getWidget("favoriteHubsDialog")), TRUE);

	// Fill the charset drop-down list in edit fav hub dialog.
	vector<string> &charsets = WulforUtil::getCharsets();
	for (vector<string>::const_iterator it = charsets.begin(); it != charsets.end(); ++it)
		gtk_combo_box_append_text(GTK_COMBO_BOX(getWidget("comboboxCharset")), it->c_str());

	// Initialize favorite hub list treeview
	favoriteView.setView(GTK_TREE_VIEW(getWidget("favoriteView")), TRUE, "favoritehubs");
	favoriteView.insertColumn(_("Auto Connect"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	favoriteView.insertColumn(_("Name"), G_TYPE_STRING, TreeView::STRING, 150);
	favoriteView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 250);
	favoriteView.insertColumn(_("Address"), G_TYPE_STRING, TreeView::STRING, 175);
	favoriteView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn(_("Password"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteView.insertColumn(_("User Description"), G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertColumn(_("Encoding"), G_TYPE_STRING, TreeView::STRING, 125);
	favoriteView.insertHiddenColumn("Hidden Password", G_TYPE_STRING);
	favoriteView.finalize();
	favoriteStore = gtk_list_store_newv(favoriteView.getColCount(), favoriteView.getGTypes());
	gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));
	g_object_unref(favoriteStore);
	gtk_tree_view_set_fixed_height_mode(favoriteView.get(), TRUE);
	favoriteSelection = gtk_tree_view_get_selection(favoriteView.get());
	GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(favoriteView.get(), favoriteView.col(_("Auto Connect"))));
	GtkCellRenderer *renderer = (GtkCellRenderer *)g_list_nth_data(list, 0);
	g_list_free(list);

	// Treat "Name" as the default col instead of "Auto Connect"
	gtk_tree_view_set_search_column(favoriteView.get(), favoriteView.col(_("Name")));
	GtkTreeViewColumn *column = gtk_tree_view_get_column(favoriteView.get(), favoriteView.col(_("Name")));
	gtk_widget_grab_focus(column->button);

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("buttonNew"), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonConnect"), "clicked", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonProperties"), "clicked", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("buttonRemove"), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("addMenuItem"), "activate", G_CALLBACK(onAddEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("connectMenuItem"), "activate", G_CALLBACK(onConnect_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesMenuItem"), "activate", G_CALLBACK(onEditEntry_gui), (gpointer)this);
	g_signal_connect(getWidget("removeMenuItem"), "activate", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
	g_signal_connect(renderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(favoriteView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(getWidget("checkbuttonEncoding"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("comboboxCharset"));
	g_signal_connect(getWidget("checkbuttonNick"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryNick"));
	g_signal_connect(getWidget("checkbuttonUserDescription"), "toggled", G_CALLBACK(onCheckButtonToggled_gui), getWidget("entryUserDescription"));
}

FavoriteHubs::~FavoriteHubs()
{
	FavoriteManager::getInstance()->removeListener(this);

	gtk_widget_destroy(getWidget("favoriteHubsDialog"));
	g_object_unref(getWidget("menu"));
}

void FavoriteHubs::show()
{
	initializeList_client();
	//WulforManager::get()->dispatchClientFunc(new Func0<FavoriteHubs>(this, &FavoriteHubs::initializeList_client));
	FavoriteManager::getInstance()->addListener(this);
}

void FavoriteHubs::addEntry_gui(StringMap params)
{
	GtkTreeIter iter;
	gtk_list_store_append(favoriteStore, &iter);
	editEntry_gui(params, &iter);
}

void FavoriteHubs::editEntry_gui(StringMap &params, GtkTreeIter *iter)
{
	string password = params["Password"].empty() ? "" : string(8, '*');

	gtk_list_store_set(favoriteStore, iter,
		favoriteView.col(_("Auto Connect")), Util::toInt(params["Auto Connect"]),
		favoriteView.col(_("Name")), params["Name"].c_str(),
		favoriteView.col(_("Description")), params["Description"].c_str(),
		favoriteView.col(_("Nick")), params["Nick"].c_str(),
		favoriteView.col(_("Password")), password.c_str(),
		favoriteView.col("Hidden Password"), params["Password"].c_str(),
		favoriteView.col(_("Address")), params["Address"].c_str(),
		favoriteView.col(_("User Description")), params["User Description"].c_str(),
		favoriteView.col(_("Encoding")), params["Encoding"].c_str(),
		-1);
}

void FavoriteHubs::removeEntry_gui(string address)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (favoriteView.getString(&iter, _("Address")) == address)
		{
			gtk_list_store_remove(favoriteStore, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

bool FavoriteHubs::showErrorDialog_gui(const string &description, FavoriteHubs *fh)
{
	GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(fh->getWidget("favoriteHubsDialog")),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;

	gtk_widget_destroy(dialog);

	return TRUE;
}

void FavoriteHubs::popupMenu_gui()
{
	if (!gtk_tree_selection_get_selected(favoriteSelection, NULL, NULL))
	{
		gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("removeMenuItem"), FALSE);
		gtk_widget_set_sensitive(getWidget("connectMenuItem"), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("removeMenuItem"), TRUE);
		gtk_widget_set_sensitive(getWidget("connectMenuItem"), TRUE);
	}
	gtk_menu_popup(GTK_MENU(getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean FavoriteHubs::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	fh->previous = event->type;
	return FALSE;
}

gboolean FavoriteHubs::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);

		if (fh->previous == GDK_BUTTON_PRESS && event->button == 3)
		{
			fh->popupMenu_gui();
		}
		else if (fh->previous == GDK_2BUTTON_PRESS && event->button == 1)
		{
			WulforManager::get()->getMainWindow()->showHub_gui(
				fh->favoriteView.getString(&iter, _("Address")),
				fh->favoriteView.getString(&iter, _("Encoding")));
		}
	}

	return FALSE;
}

gboolean FavoriteHubs::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), TRUE);

		if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			GtkTreeViewColumn *column;
			gtk_tree_view_get_cursor(fh->favoriteView.get(), NULL, &column);
			if (column && column != gtk_tree_view_get_column(fh->favoriteView.get(), fh->favoriteView.col(_("Auto Connect"))))
			{
				WulforManager::get()->getMainWindow()->showHub_gui(
					fh->favoriteView.getString(&iter, _("Address")),
					fh->favoriteView.getString(&iter, _("Encoding")));
			}
		}
		else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			fh->onRemoveEntry_gui(widget, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			fh->popupMenu_gui();
		}
	}

	return FALSE;
}

void FavoriteHubs::onAddEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	const string emptyString = "";

	StringMap params;
	params["Name"] = emptyString;
	params["Address"] = emptyString;
	params["Description"] = emptyString;
	params["Nick"] = emptyString;
	params["Password"] = emptyString;
	params["User Description"] = emptyString;
	params["Encoding"] = emptyString;
	params["Auto Connect"] = "0";

	bool updatedEntry = fh->showFavoriteHubDialog_gui(params, fh);

	if (updatedEntry)
	{
		typedef Func1<FavoriteHubs, StringMap> F1;
		F1 *func = new F1(fh, &FavoriteHubs::addEntry_client, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onEditEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		return;

	StringMap params;
	params["Name"] = fh->favoriteView.getString(&iter, _("Name"));
	params["Address"] = fh->favoriteView.getString(&iter, _("Address"));
	params["Description"] = fh->favoriteView.getString(&iter, _("Description"));
	params["Nick"] = fh->favoriteView.getString(&iter, _("Nick"));
	params["Password"] = fh->favoriteView.getString(&iter, "Hidden Password");
	params["User Description"] = fh->favoriteView.getString(&iter, _("User Description"));
	params["Encoding"] = fh->favoriteView.getString(&iter, _("Encoding"));
	params["Auto Connect"] = fh->favoriteView.getValue<gboolean>(&iter, _("Auto Connect")) ? "1" : "0";

	bool entryUpdated = showFavoriteHubDialog_gui(params, fh);

	if (entryUpdated)
	{
		string address = fh->favoriteView.getString(&iter, _("Address"));
		fh->editEntry_gui(params, &iter);

		typedef Func2<FavoriteHubs, string, StringMap> F2;
		F2 *func = new F2(fh, &FavoriteHubs::editEntry_client, address, params);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

bool FavoriteHubs::showFavoriteHubDialog_gui(StringMap &params, FavoriteHubs *fh)
{
	// Populate the dialog with initial values
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryName")), params["Name"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryAddress")), params["Address"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryDescription")), params["Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryNick")), params["Nick"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryPassword")), params["Password"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("entryUserDescription")), params["User Description"].c_str());
	gtk_entry_set_text(GTK_ENTRY(fh->getWidget("comboboxentryCharset")), params["Encoding"].c_str());

	// Set the auto connect checkbox
	gboolean autoConnect = params["Auto Connect"] == "1" ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonAutoConnect")), autoConnect);

	// Set the override default encoding checkbox. Check for "Global hub default"
	// for backwards compatability w/ 1.0.3. Should be removed at some point.
	gboolean overrideEncoding = !(params["Encoding"].empty() || params["Encoding"] == "Global hub default");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonEncoding")), overrideEncoding);

	// Set the override default nick checkbox
	gboolean overrideNick = !(params["Nick"].empty() || params["Nick"] == SETTING(NICK));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonNick")), overrideNick);

	// Set the override default user description checkbox
	gboolean overrideUserDescription = !(params["User Description"].empty() || params["User Description"] == SETTING(DESCRIPTION));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonUserDescription")), overrideUserDescription);

	// Show the dialog
	gint response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return FALSE;

	while (response == GTK_RESPONSE_OK)
	{
		params.clear();
		params["Name"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryName")));
		params["Address"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryAddress")));
		params["Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryDescription")));
		params["Password"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryPassword")));
		params["Auto Connect"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkButtonAutoConnect"))) ? "1" : "0";

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonEncoding"))))
		{
			gchar *encoding = gtk_combo_box_get_active_text(GTK_COMBO_BOX(fh->getWidget("comboboxCharset")));
			params["Encoding"] = string(encoding);
			g_free(encoding);
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonNick"))))
		{
			params["Nick"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryNick")));
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fh->getWidget("checkbuttonUserDescription"))))
		{
			params["User Description"] = gtk_entry_get_text(GTK_ENTRY(fh->getWidget("entryUserDescription")));
		}

		if (params["Name"].empty() || params["Address"].empty())
		{
			if (showErrorDialog_gui(_("The name and address fields are required"), fh))
			{
				response = gtk_dialog_run(GTK_DIALOG(fh->getWidget("favoriteHubsDialog")));

				// Fix crash, if the dialog gets programmatically destroyed.
				if (response == GTK_RESPONSE_NONE)
					return FALSE;
			}
			else
				return FALSE;
		}
		else
		{
			gtk_widget_hide(fh->getWidget("favoriteHubsDialog"));
			return TRUE;
		}
	}

	gtk_widget_hide(fh->getWidget("favoriteHubsDialog"));
	return FALSE;
}

void FavoriteHubs::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
	{
		if (WGETB("confirm-hub-removal"))
		{
			string name = fh->favoriteView.getString(&iter, _("Name")).c_str();
			GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
			GtkWidget* dialog = gtk_message_dialog_new(parent,
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				_("Are you sure you want to delete favorite hub \"%s\"?"), name.c_str());
			gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
			gint response = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			if (response != GTK_RESPONSE_YES)
				return;
		}

		gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);
		gtk_widget_set_sensitive(fh->getWidget("buttonConnect"), FALSE);

		string address = fh->favoriteView.getString(&iter, _("Address"));

		typedef Func1<FavoriteHubs, string> F1;
		F1 *func = new F1(fh, &FavoriteHubs::removeEntry_client, address);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onConnect_gui(GtkButton *widget, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
		WulforManager::get()->getMainWindow()->showHub_gui(
			fh->favoriteView.getString(&iter, _("Address")),
			fh->favoriteView.getString(&iter, _("Encoding")));
}

void FavoriteHubs::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	FavoriteHubs *fh = (FavoriteHubs *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fh->favoriteStore), &iter, path))
	{
		string address = fh->favoriteView.getString(&iter, _("Address"));
		bool fixed = fh->favoriteView.getValue<gboolean>(&iter, _("Auto Connect"));
		fixed = !fixed;
		gtk_list_store_set(fh->favoriteStore, &iter, fh->favoriteView.col(_("Auto Connect")), fixed, -1);

		typedef Func2<FavoriteHubs, string, bool> F2;
		F2 *func = new F2(fh, &FavoriteHubs::setConnect_client, address, fixed);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteHubs::onCheckButtonToggled_gui(GtkToggleButton *button, gpointer data)
{
	GtkWidget *widget = (GtkWidget*)data;
	bool override = gtk_toggle_button_get_active(button);

	gtk_widget_set_sensitive(widget, override);

	if (override)
	{
		gtk_widget_grab_focus(widget);
	}
}

void FavoriteHubs::initializeList_client()
{
	StringMap params;
	typedef Func1<FavoriteHubs, StringMap> F1;
	//F1 *func;
	const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

	for (FavoriteHubEntryList::const_iterator it = fl.begin(); it != fl.end(); ++it)
	{
		getFavHubParams_client(*it, params);
		addEntry_gui(params);
		//func = new F1(this, &FavoriteHubs::addEntry_gui, params);
		//WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FavoriteHubs::getFavHubParams_client(const FavoriteHubEntry *entry, StringMap &params)
{
	params["Auto Connect"] = entry->getConnect() ? "1" : "0";
	params["Name"] = entry->getName();
	params["Description"] = entry->getDescription();
	params["Nick"] = entry->getNick(FALSE); // Don't display default nick to avoid accidentally saving it
	params["Password"] = entry->getPassword();
	params["Address"] = entry->getServer();
	params["User Description"] = entry->getUserDescription();
	params["Encoding"] = entry->getEncoding();
}

void FavoriteHubs::addEntry_client(StringMap params)
{
    FavoriteHubEntry entry;
    entry.setConnect(Util::toInt(params["Auto Connect"]));
    entry.setName(params["Name"]);
    entry.setServer(params["Address"]);
    entry.setDescription(params["Description"]);
    entry.setNick(params["Nick"]);
    entry.setPassword(params["Password"]);
    entry.setUserDescription(params["User Description"]);
    entry.setEncoding(params["Encoding"]);
    FavoriteManager::getInstance()->addFavorite(entry);

    const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
    WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);
}

void FavoriteHubs::editEntry_client(string address, StringMap params)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		entry->setConnect(Util::toInt(params["Auto Connect"]));
		entry->setName(params["Name"]);
		entry->setServer(params["Address"]);
		entry->setDescription(params["Description"]);
		entry->setNick(params["Nick"]);
		entry->setPassword(params["Password"]);
		entry->setUserDescription(params["User Description"]);
		entry->setEncoding(params["Encoding"]);
		FavoriteManager::getInstance()->save();

        const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
        WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);
	}
}

void FavoriteHubs::removeEntry_client(string address)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

    if (entry)
    {
        FavoriteManager::getInstance()->removeFavorite(entry);

        const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
        WulforManager::get()->getMainWindow()->updateFavoriteHubMenu_client(fh);
    }
}

void FavoriteHubs::setConnect_client(string address, bool active)
{
	FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address);

	if (entry)
	{
		entry->setConnect(active);
		FavoriteManager::getInstance()->save();
	}
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteAdded, const FavoriteHubEntryPtr entry) throw()
{
	StringMap params;
	getFavHubParams_client(entry, params);

	typedef Func1<FavoriteHubs, StringMap> F1;
	F1 *func = new F1(this, &FavoriteHubs::addEntry_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteHubs::on(FavoriteManagerListener::FavoriteRemoved, const FavoriteHubEntryPtr entry) throw()
{
	typedef Func1<FavoriteHubs, string> F1;
	F1 *func = new F1(this, &FavoriteHubs::removeEntry_gui, entry->getServer());
	WulforManager::get()->dispatchGuiFunc(func);
}

