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

#include "wulformanager.hh"
#include "WulforUtil.hh"

#include "adlsearch.hh"

using namespace std;
using namespace dcpp;

SearchADL::SearchADL():
	BookEntry(Entry::SEARCH_ADL, _("ADL Search"), "adlsearch.ui")
{
	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("ADLSearchDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
	g_object_ref_sink(getWidget("menu"));

	// Initialize search list treeview
	searchADLView.setView(GTK_TREE_VIEW(getWidget("searchADLView")), TRUE, "searchadl");
	searchADLView.insertColumn(_("Enabled"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	searchADLView.insertColumn(_("Search String"), G_TYPE_STRING, TreeView::STRING, 100);
	searchADLView.insertColumn(_("Source Type"), G_TYPE_STRING, TreeView::STRING, 90);
	searchADLView.insertColumn(_("Destination Directory"), G_TYPE_STRING, TreeView::STRING, 90);
	searchADLView.insertColumn(_("Min Size"), G_TYPE_STRING, TreeView::STRING, 100);
	searchADLView.insertColumn(_("Max Size"), G_TYPE_STRING, TreeView::STRING, 100);
	searchADLView.insertHiddenColumn("Download Matches", G_TYPE_BOOLEAN);
	searchADLView.insertHiddenColumn("MinSize", G_TYPE_INT64);
	searchADLView.insertHiddenColumn("MaxSize", G_TYPE_INT64);
	searchADLView.insertHiddenColumn("SourceType", G_TYPE_INT);
	searchADLView.insertHiddenColumn("SizeType", G_TYPE_INT);
	searchADLView.finalize();

	searchADLStore = gtk_list_store_newv(searchADLView.getColCount(), searchADLView.getGTypes());
	gtk_tree_view_set_model(searchADLView.get(), GTK_TREE_MODEL(searchADLStore));
	g_object_unref(searchADLStore);

	searchADLSelection = gtk_tree_view_get_selection(searchADLView.get());
	gtk_tree_selection_set_mode(searchADLSelection, GTK_SELECTION_SINGLE);

	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Enabled"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Search String"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Source Type"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Destination Directory"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Min Size"))), FALSE);
	gtk_tree_view_column_set_clickable(gtk_tree_view_get_column(searchADLView.get(), searchADLView.col(_("Max Size"))), FALSE);

	GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(searchADLView.get(),
		searchADLView.col(_("Enabled"))));
	GObject *renderer = (GObject *)g_list_nth_data(list, 0);
	g_signal_connect(renderer, "toggled", G_CALLBACK(onActiveToggled_gui), (gpointer)this);
	g_list_free(list);

	g_signal_connect(getWidget("addItem"), "activate", G_CALLBACK(onAddClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesItem"), "activate", G_CALLBACK(onPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveClicked_gui), (gpointer)this);

	g_signal_connect(getWidget("addButton"), "clicked", G_CALLBACK(onAddClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("propertiesButton"), "clicked", G_CALLBACK(onPropertiesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("moveUpButton"), "clicked", G_CALLBACK(onMoveUpClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("moveDownButton"), "clicked", G_CALLBACK(onMoveDownClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeButton"), "clicked", G_CALLBACK(onRemoveClicked_gui), (gpointer)this);

	g_signal_connect(searchADLView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(searchADLView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(searchADLView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
}

SearchADL::~SearchADL()
{
	ADLSearchManager::getInstance()->Save();
	gtk_widget_destroy(getWidget("ADLSearchDialog"));
	g_object_unref(getWidget("menu"));
}

void SearchADL::show()
{
	// initialize searches list
	string minSize, maxSize;
	ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
	for (ADLSearchManager::SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i)
	{
		GtkTreeIter iter;
		ADLSearch &search = *i;
		minSize = search.minFileSize >= 0 ? Util::toString(search.minFileSize) + " " +
			search.SizeTypeToString(search.typeFileSize) : "";
		maxSize = search.maxFileSize >= 0 ? Util::toString(search.maxFileSize) + " " +
			search.SizeTypeToString(search.typeFileSize) : "";

		gtk_list_store_append(searchADLStore, &iter);
		gtk_list_store_set(searchADLStore, &iter,
			searchADLView.col(_("Enabled")), search.isActive,
			searchADLView.col(_("Search String")), search.searchString.c_str(),
			searchADLView.col(_("Source Type")), search.SourceTypeToString(search.sourceType).c_str(),
			searchADLView.col(_("Destination Directory")), search.destDir.c_str(),
			searchADLView.col(_("Min Size")), minSize.c_str(),
			searchADLView.col(_("Max Size")), maxSize.c_str(),
			searchADLView.col("Download Matches"), search.isAutoQueue,
			searchADLView.col("MinSize"), search.minFileSize,
			searchADLView.col("MaxSize"), search.maxFileSize,
			searchADLView.col("SourceType"), search.sourceType,
			searchADLView.col("SizeType"), search.typeFileSize,
			-1);
	}
}

void SearchADL::onRemoveClicked_gui(GtkWidget *widget, gpointer data)
{
	SearchADL *s = (SearchADL *)data;

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (i < collection.size())
		{
			collection.erase(collection.begin() + i);
			gtk_list_store_remove(s->searchADLStore, &iter);
		}
	}
}

void SearchADL::onAddClicked_gui(GtkWidget *widget, gpointer data)
{
	SearchADL *s = (SearchADL *)data;

	ADLSearch search;
	if (showPropertiesDialog_gui(search, false, s))
	{
		GtkTreeIter iter;
		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;

		if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
		{
			gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
			SearchType i = (SearchType)Util::toInt(p);
			g_free(p);

			if (i < collection.size())
			{
				GtkTreeIter it;
				collection.insert(collection.begin() + i, search);
				gtk_list_store_insert_before(s->searchADLStore, &it, &iter);
				s->setSearch_gui(search, &it);
			}
		}
		else
		{
			collection.push_back(search);
			gtk_list_store_append(s->searchADLStore, &iter);
			s->setSearch_gui(search, &iter);
		}
	}
}

void SearchADL::onPropertiesClicked_gui(GtkWidget *widget, gpointer data)
{
	SearchADL *s = (SearchADL *)data;
	
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
	{
		ADLSearch search;
		if (showPropertiesDialog_gui(search, true, s))
		{
			gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
			SearchType i = (SearchType)Util::toInt(p);
			g_free(p);

			ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
			if (i < collection.size())
			{
				collection[i] = search;
				s->setSearch_gui(search, &iter);
			}
		}
	}
}

void SearchADL::setSearch_gui(ADLSearch &search, GtkTreeIter *iter)
{
	string minSize = search.minFileSize >= 0 ? Util::toString(search.minFileSize) + " " +
		search.SizeTypeToString(search.typeFileSize) : "";
	string maxSize = search.maxFileSize >= 0 ? Util::toString(search.maxFileSize) + " " +
		search.SizeTypeToString(search.typeFileSize) : "";

	gtk_list_store_set(searchADLStore, iter,
		searchADLView.col(_("Enabled")), search.isActive,
		searchADLView.col(_("Search String")), search.searchString.c_str(),
		searchADLView.col(_("Source Type")), search.SourceTypeToString(search.sourceType).c_str(),
		searchADLView.col(_("Destination Directory")), search.destDir.c_str(),
		searchADLView.col(_("Min Size")), minSize.c_str(),
		searchADLView.col(_("Max Size")), maxSize.c_str(),
		searchADLView.col("Download Matches"), search.isAutoQueue,
		searchADLView.col("MinSize"), search.minFileSize,
		searchADLView.col("MaxSize"), search.maxFileSize,
		searchADLView.col("SourceType"), search.sourceType,
		searchADLView.col("SizeType"), search.typeFileSize,
		-1);
}

bool SearchADL::showPropertiesDialog_gui(ADLSearch &search, bool edit, SearchADL *s)
{
	GtkTreeIter iter;
	if (edit && !gtk_tree_selection_get_selected(s->searchADLSelection, NULL, &iter))
		return false;

	string searchString, sourceTypeString, destDir;
	gboolean enabledCheck, matchesCheck;
	gint sizeType, sourceType;
	gdouble minSize, maxSize;

	if (edit)
	{
		searchString = s->searchADLView.getString(&iter, _("Search String"));
		destDir = s->searchADLView.getString(&iter, _("Destination Directory"));
		minSize = s->searchADLView.getValue<int64_t>(&iter, "MinSize");
		maxSize = s->searchADLView.getValue<int64_t>(&iter, "MaxSize");
		sourceType = s->searchADLView.getValue<gint>(&iter, "SourceType");
		sizeType = s->searchADLView.getValue<gint>(&iter, "SizeType");
		enabledCheck = s->searchADLView.getValue<gboolean>(&iter, _("Enabled"));
		matchesCheck = s->searchADLView.getValue<gboolean>(&iter, "Download Matches");

		// set text
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("searchStringEntry")), searchString.c_str());
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), destDir.c_str());

		// set size
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("minFileSizeSpinButton")), minSize);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("maxFileSizeSpinButton")), maxSize);

		// set types
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sizeTypeComboBox")), sizeType);
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")), sourceType);

		// set checks
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("enabledCheckButton")), enabledCheck);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("downloadMatchesCheckButton")), matchesCheck);
	}
	else
	{
		// set text default
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("searchStringEntry")), search.searchString.c_str());
		gtk_entry_set_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")), search.destDir.c_str());

		// set size default
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("minFileSizeSpinButton")), search.minFileSize);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(s->getWidget("maxFileSizeSpinButton")), search.maxFileSize);

		// set type default
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sizeTypeComboBox")), search.typeFileSize);
		gtk_combo_box_set_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")), search.sourceType);

		// set checks default
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("enabledCheckButton")), search.isActive);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s->getWidget("downloadMatchesCheckButton")), search.isAutoQueue);
	}

	GtkWidget *dialog = s->getWidget("ADLSearchDialog");
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	// Fix crash, if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return false;

	gtk_widget_hide(dialog);

	if (response != GTK_RESPONSE_OK)
		return false;

	// set search
	enabledCheck = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("enabledCheckButton")));
	matchesCheck = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->getWidget("downloadMatchesCheckButton")));
	searchString = gtk_entry_get_text(GTK_ENTRY(s->getWidget("searchStringEntry")));
	destDir = gtk_entry_get_text(GTK_ENTRY(s->getWidget("destinationDirectoryEntry")));
	minSize = gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("minFileSizeSpinButton")));
	maxSize = gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("maxFileSizeSpinButton")));
	sizeType = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("sizeTypeComboBox")));
	sourceType = gtk_combo_box_get_active(GTK_COMBO_BOX(s->getWidget("sourceTypeComboBox")));

	search.isActive = enabledCheck;
	search.isAutoQueue = matchesCheck;
	search.searchString = searchString;
	search.destDir = destDir;
	search.minFileSize = minSize;
	search.maxFileSize = maxSize;
	search.sourceType = (ADLSearch::SourceType)sourceType;
	search.typeFileSize = (ADLSearch::SizeType)sizeType;

	return true;
}

void SearchADL::onMoveUpClicked_gui(GtkWidget *widget, gpointer data)
{
	SearchADL *s = (SearchADL *)data;

	GtkTreeIter prev, current;
	GtkTreeModel *m = GTK_TREE_MODEL(s->searchADLStore);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->searchADLView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &current);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (i == 0 || !(i < collection.size()))
			return;

		bool swap = false;
		GtkTreePath *path = gtk_tree_model_get_path(m, &current);
		if (gtk_tree_path_prev(path) && gtk_tree_model_get_iter(m, &prev, path))
		{
			swap = true;
			gtk_list_store_swap(s->searchADLStore, &current, &prev);
		}
		gtk_tree_path_free(path);

		if (swap)
			std::swap(collection[i], collection[i - 1]);
	}
}

void SearchADL::onMoveDownClicked_gui(GtkWidget *widget, gpointer data)
{
	SearchADL *s = (SearchADL *)data;

	GtkTreeIter current, next;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(s->searchADLView.get());

	if (gtk_tree_selection_get_selected(sel, NULL, &current))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &current);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (collection.empty() || !(i < collection.size() - 1))
			return;

		bool swap = false;
		next = current;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(s->searchADLStore), &next))
		{
			swap = true;
			gtk_list_store_swap(s->searchADLStore, &current, &next);
		}

		if (swap)
			std::swap(collection[i], collection[i + 1]);
	}
}

void SearchADL::onActiveToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	SearchADL *s = (SearchADL *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(s->searchADLStore), &iter, path))
	{
		gchar *p = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(s->searchADLStore), &iter);
		SearchType i = (SearchType)Util::toInt(p);
		g_free(p);

		ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
		if (i < collection.size())
		{
			gboolean active = s->searchADLView.getValue<gboolean>(&iter, _("Enabled"));
			active = !active;
			gtk_list_store_set(s->searchADLStore, &iter, s->searchADLView.col(_("Enabled")), active, -1);

			ADLSearch search = collection[i];
			search.isActive = active;
			collection[i] = search;
		}
	}
}

gboolean SearchADL::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	SearchADL *s = (SearchADL *)data;
	s->previous = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;

		if (gtk_tree_view_get_path_at_pos(s->searchADLView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(s->searchADLSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean SearchADL::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	SearchADL *s = (SearchADL *)data;

	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, NULL))
	{
		if (event->button == 1 && s->previous == GDK_2BUTTON_PRESS)
		{
			// show dialog
			onPropertiesClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			// show menu
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean SearchADL::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	SearchADL *s = (SearchADL *)data;

	if (gtk_tree_selection_get_selected(s->searchADLSelection, NULL, NULL))
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			s->onRemoveClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}
