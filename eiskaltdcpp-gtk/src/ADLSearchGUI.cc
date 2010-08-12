/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
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

#include "ADLSearchGUI.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

#include <dcpp/ADLSearch.h>


using namespace std;
using namespace dcpp;

ADLSearchGUI::ADLSearchGUI():
        BookEntry(Entry::ADL, _("ADLSearch"), "ADLSearch.glade")
{
        // Configure the dialog
        gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("favoriteHubsDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

        // Fill the charset drop-down list in edit fav hub dialog.
        vector<std::string> &charsets = ADLSearchGUI::getCharsets();

        for (vector<std::string>::const_iterator it = charsets.begin(); it != charsets.end(); ++it)
                gtk_combo_box_append_text(GTK_COMBO_BOX(getWidget("comboboxCharset")), it->c_str());


        // Initialize favorite hub list treeview
        favoriteView.setView(GTK_TREE_VIEW(getWidget("favoriteView")), TRUE, "favoritehubs");
        favoriteView.insertColumn("Active", G_TYPE_BOOLEAN, TreeView::BOOL, 100);
        favoriteView.insertColumn("Search String", G_TYPE_STRING, TreeView::STRING, 150);
        favoriteView.insertColumn("Dest Dir", G_TYPE_STRING, TreeView::STRING, 250);
        favoriteView.insertColumn("Source Type", G_TYPE_STRING, TreeView::STRING, 175);
        favoriteView.insertColumn("min Size", G_TYPE_STRING, TreeView::STRING, 100);
        favoriteView.insertColumn("max Size", G_TYPE_STRING, TreeView::STRING, 100);
        favoriteView.insertHiddenColumn("isQueue", G_TYPE_BOOLEAN);

        favoriteView.finalize();
        favoriteStore = gtk_list_store_newv(favoriteView.getColCount(), favoriteView.getGTypes());
        gtk_tree_view_set_model(favoriteView.get(), GTK_TREE_MODEL(favoriteStore));
        g_object_unref(favoriteStore);
        gtk_tree_view_set_fixed_height_mode(favoriteView.get(), TRUE);
        favoriteSelection = gtk_tree_view_get_selection(favoriteView.get());
        GList *list = gtk_tree_view_column_get_cell_renderers(gtk_tree_view_get_column(favoriteView.get(), favoriteView.col("Active")));
        GtkCellRenderer *renderer = (GtkCellRenderer *)g_list_nth_data(list, 0);
        g_list_free(list);

        // Treat "String" as the default col instead of "Active"
        gtk_tree_view_set_search_column(favoriteView.get(), favoriteView.col("Search String"));
        GtkTreeViewColumn *column = gtk_tree_view_get_column(favoriteView.get(), favoriteView.col("Search String"));
        gtk_widget_grab_focus(column->button);

        // Connect the signals to their callback functions.
        g_signal_connect(getWidget("buttonNew"), "clicked", G_CALLBACK(onAddEntry_gui), (gpointer)this);
        g_signal_connect(getWidget("buttonProperties"), "clicked", G_CALLBACK(onEditEntry_gui), (gpointer)this);
        g_signal_connect(getWidget("buttonRemove"), "clicked", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
        g_signal_connect(getWidget("addMenuItem"), "activate", G_CALLBACK(onAddEntry_gui), (gpointer)this);
        g_signal_connect(getWidget("propertiesMenuItem"), "activate", G_CALLBACK(onEditEntry_gui), (gpointer)this);
        g_signal_connect(getWidget("removeMenuItem"), "activate", G_CALLBACK(onRemoveEntry_gui), (gpointer)this);
        g_signal_connect(renderer, "toggled", G_CALLBACK(onToggledClicked_gui), (gpointer)this);
        g_signal_connect(favoriteView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
        g_signal_connect(favoriteView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);

}

ADLSearchGUI::~ADLSearchGUI()
{
        ADLSearchManager::getInstance()->Save();
}

void ADLSearchGUI::show()
{
        initializeList_client();
        ADLSearchManager::getInstance()->Load();
}

void ADLSearchGUI::addEntry_gui(StringMap params)
{
        GtkTreeIter iter;
        gtk_list_store_append(favoriteStore, &iter);
        editEntry_gui(params, &iter);
}

void ADLSearchGUI::editEntry_gui(StringMap &params, GtkTreeIter *iter)
{
        gtk_list_store_set(favoriteStore, iter,
                favoriteView.col("Active"), (Util::toInt(params["Active"])),
                favoriteView.col("Search String"), params["SearchString"].c_str(),
                favoriteView.col("Source Type"), params["SourceType"].c_str(),
                favoriteView.col("Dest Dir"), params["DestDir"].c_str(),
                favoriteView.col("min Size"), params["minSize"].c_str(),
                favoriteView.col("max Size"), params["maxSize"].c_str(),
                favoriteView.col("isQueue"), (Util::toInt(params["AutoQueue"])),
                -1);
}

void ADLSearchGUI::removeEntry_gui(string address)
{
        GtkTreeIter iter;
        GtkTreeModel *m = GTK_TREE_MODEL(favoriteStore);
        bool valid = gtk_tree_model_get_iter_first(m, &iter);

        while (valid)
        {
                if (favoriteView.getString(&iter, "Search String") == address)
                {
                        gtk_list_store_remove(favoriteStore, &iter);
                        break;
                }
                valid = gtk_tree_model_iter_next(m, &iter);
        }
}

void ADLSearchGUI::showErrorDialog_gui(const string &description)
{
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(getWidget("favoriteHubsDialog")),
                GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", description.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
}

void ADLSearchGUI::popupMenu_gui()
{
        if (!gtk_tree_selection_get_selected(favoriteSelection, NULL, NULL))
        {
                gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), FALSE);
                gtk_widget_set_sensitive(getWidget("removeMenuItem"), FALSE);

        }
        else
        {
                gtk_widget_set_sensitive(getWidget("propertiesMenuItem"), TRUE);
                gtk_widget_set_sensitive(getWidget("removeMenuItem"), TRUE);

        }
        gtk_menu_popup(GTK_MENU(getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean ADLSearchGUI::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        ADLSearchGUI *fh = (ADLSearchGUI *)data;
        fh->previous = event->type;
        return FALSE;
}

gboolean ADLSearchGUI::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        ADLSearchGUI *fh = (ADLSearchGUI *)data;
        GtkTreeIter iter;

        if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
        {
                gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
                gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);

        }
        else
        {
                gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), TRUE);
                gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), TRUE);


                if (fh->previous == GDK_BUTTON_PRESS && event->button == 3)
                {
                        fh->popupMenu_gui();
                }
                //else if (fh->previous == GDK_2BUTTON_PRESS && event->button == 1)
                //{
                //      WulforManager::get()->getMainWindow()->showHub_gui(
                //              fh->favoriteView.getString(&iter, "Address"),
                //              fh->favoriteView.getString(&iter, "Encoding"));
                //}
        }

        return FALSE;
}

void ADLSearchGUI::onAddEntry_gui(GtkWidget *widget, gpointer data)
{
        ADLSearchGUI *fh = (ADLSearchGUI *)data;
        const string emptyString = "";


        StringMap params;
        params["SearchString"] = emptyString;
        params["SourceType"] = emptyString;
        params["maxSize"] = "0";
        params["minSize"] = "0";
        params["Active"] = "0";
        params["DestDir"] = emptyString;
        params["AutoQueue"] = "0";


        bool updatedEntry = fh->showFavoriteHubDialog_gui(params);

        if (updatedEntry)
        {
                typedef Func1<ADLSearchGUI, StringMap> F1;
                F1 *func = new F1(fh, &ADLSearchGUI::addEntry_client, params);
                WulforManager::get()->dispatchClientFunc(func);
        }
}

void ADLSearchGUI::onEditEntry_gui(GtkWidget *widget, gpointer data)
{
        ADLSearchGUI *fh = (ADLSearchGUI *)data;
        GtkTreeIter iter;

        if (!gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
                return;

        StringMap params;
        params["SearchString"] = fh->favoriteView.getString(&iter, "Search String");
        params["maxSize"] = fh->favoriteView.getString(&iter, "max Size");
        params["minSize"] = fh->favoriteView.getString(&iter, "min Size");
        params["DestDir"] = fh->favoriteView.getString(&iter, "Dest Dir");
        params["SourceType"] = fh->favoriteView.getString(&iter, "Source Type");
        params["Active"] = fh->favoriteView.getValue<gboolean>(&iter, "Active") ? "1" : "0";
        params["AutoQueue"] = fh->favoriteView.getValue<gboolean>(&iter, "isQueue") ? "1" : "0";


        bool entryUpdated = fh->showFavoriteHubDialog_gui(params);

        if (entryUpdated)
        {
                string address = fh->favoriteView.getString(&iter, "Search String");
                fh->editEntry_gui(params, &iter);

                typedef Func2<ADLSearchGUI, string, StringMap> F2;
                F2 *func = new F2(fh, &ADLSearchGUI::editEntry_client, address, params);
                WulforManager::get()->dispatchClientFunc(func);
        }
}

bool ADLSearchGUI::showFavoriteHubDialog_gui(StringMap &params)
{
        // Populate the dialog with initial values
        gtk_entry_set_text(GTK_ENTRY(getWidget("entrySearch")), params["SearchString"].c_str());
        gtk_entry_set_text(GTK_ENTRY(getWidget("entryMaxSize")), params["maxSize"].c_str());
        gtk_entry_set_text(GTK_ENTRY(getWidget("entryMinSize")), params["minSize"].c_str());
        gtk_entry_set_text(GTK_ENTRY(getWidget("entryDestDir")), params["DestDir"].c_str());
        gtk_entry_set_text(GTK_ENTRY(getWidget("comboboxCharset")), params["SourceType"].c_str());

        // Set the auto quene checkbox
        gboolean isQuene = params["AutoQueue"] == "1" ? TRUE : FALSE;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("isQueue")), isQuene);

        // Set the Active checkbox
        gboolean isActive = params["Active"] == "1" ? TRUE : FALSE;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("getActive")), isActive);

        // Show the dialog
        gint response = gtk_dialog_run(GTK_DIALOG(getWidget("favoriteHubsDialog")));

        while (response == GTK_RESPONSE_OK)
        {
                params.clear();
                params["SearchString"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entrySearch")));
                params["maxSize"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryMaxSize")));
                params["minSize"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryMinSize")));
                params["DestDir"] = gtk_entry_get_text(GTK_ENTRY(getWidget("entryDestDir")));
                params["Active"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("getActive"))) ? "1" : "0";
                params["AutoQueue"] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("isQueue"))) ? "1" : "0";

                gchar *encoding = gtk_combo_box_get_active_text(GTK_COMBO_BOX(getWidget("comboboxCharset")));
                params["SourceType"] = string(encoding);
                g_free(encoding);

                if (params["SearchString"].empty() )
                {
                        showErrorDialog_gui(_("The search string fields are required"));
                        response = gtk_dialog_run(GTK_DIALOG(getWidget("favoriteHubsDialog")));
                }
                else
                {
                        gtk_widget_hide(getWidget("favoriteHubsDialog"));
                        return TRUE;
                }
        }

        gtk_widget_hide(getWidget("favoriteHubsDialog"));
        return FALSE;
}

void ADLSearchGUI::onRemoveEntry_gui(GtkWidget *widget, gpointer data)
{
        ADLSearchGUI *fh = (ADLSearchGUI *)data;
        GtkTreeIter iter;

        if (gtk_tree_selection_get_selected(fh->favoriteSelection, NULL, &iter))
        {
                if (BOOLSETTING(CONFIRM_HUB_REMOVAL))
                {
                        string name = fh->favoriteView.getString(&iter, "Search String").c_str();
                        GtkWindow* parent = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
                        GtkWidget* dialog = gtk_message_dialog_new(parent,
                                GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                _("Are you sure you want to delete ADL search \"%s\"?"), name.c_str());
                        gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
                        gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
                        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
                        gtk_widget_destroy(dialog);

                        if (response != GTK_RESPONSE_YES)
                                return;
                }

                gtk_widget_set_sensitive(fh->getWidget("buttonProperties"), FALSE);
                gtk_widget_set_sensitive(fh->getWidget("buttonRemove"), FALSE);


                string address = fh->favoriteView.getString(&iter, "Search String");

                typedef Func1<ADLSearchGUI, string> F1;
                F1 *func = new F1(fh, &ADLSearchGUI::removeEntry_client, address);
                WulforManager::get()->dispatchClientFunc(func);
        }
}



void ADLSearchGUI::onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
        ADLSearchGUI *fh = (ADLSearchGUI *)data;
        GtkTreeIter iter;

        if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fh->favoriteStore), &iter, path))
        {
                string address = fh->favoriteView.getString(&iter, "Search String");
                bool fixed = fh->favoriteView.getValue<gboolean>(&iter, "Active");
                fixed = !fixed;
                gtk_list_store_set(fh->favoriteStore, &iter, fh->favoriteView.col("Active"), fixed, -1);

                typedef Func2<ADLSearchGUI, string, bool> F2;
                F2 *func = new F2(fh, &ADLSearchGUI::setConnect_client, address, fixed);
                WulforManager::get()->dispatchClientFunc(func);
        }
}



void ADLSearchGUI::initializeList_client()
{
        StringMap params;

        // Load all searches
        ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
        for(ADLSearchManager::SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i)
                {
                getFavHubParams_client(*i,params);
                addEntry_gui(params);
                }

}

void ADLSearchGUI::getFavHubParams_client(ADLSearch& search, StringMap &params)
{
        params["Active"]=search.isActive ? "1" : "0";
        params["SearchString"] =Text::toT(search.searchString);
        params["SourceType"] = Text::toT(search.SourceTypeToString(search.sourceType));
        params["DestDir"] =Text::toT(search.destDir);
        params["minSize"] = (search.minFileSize >= 0) ? Text::toT(Util::toString(search.minFileSize)) + " " + Text::toT(search.SizeTypeToString(search.typeFileSize)) : "";//Util::emptyStringT;
        params["maxSize"] = (search.maxFileSize >= 0) ? Text::toT(Util::toString(search.maxFileSize)) + " " + Text::toT(search.SizeTypeToString(search.typeFileSize)) : "";//Util::emptyStringT;
        params["AutoQueue"] = search.isAutoQueue ? "1" : "0";
}


static void count_foreach_helper (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata)
{
  gint *p_count = (gint*) userdata;
  g_assert (p_count != NULL);
  *p_count = *p_count + 1;
}

gint my_tree_selection_count_selected_rows (GtkTreeSelection *selection)
{
  gint count = 0;
  gtk_tree_selection_selected_foreach(selection, count_foreach_helper, &count);
  return count;
}


void ADLSearchGUI::addEntry_client(StringMap params)
{
        dcpp::ADLSearch * _search = new dcpp::ADLSearch();

        ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;


        int tree=my_tree_selection_count_selected_rows(favoriteSelection);

        int index;
                if(tree>0)
                  index=(int)tree;
                else
                  index=0;

                        _search->searchString = params["SearchString"];
                        _search->isActive = Util::toInt(params["Active"]);
                        _search->maxFileSize = Util::toInt(params["maxSize"]);
                        _search->minFileSize = Util::toInt(params["minSize"]);
                        _search->destDir = params["DestDir"];
                        _search->sourceType = _search->StringToSourceType(params["SourceType"]);
                        _search->isAutoQueue = Util::toInt(params["AutoQueue"]);

                        if(collection.size()>0)
                        collection.insert(collection.begin()+ index, *_search);
                        else
                        collection.push_back(*_search);

                typedef Func1<ADLSearchGUI, StringMap> F1;
                F1 *func = new F1(this, &ADLSearchGUI::addEntry_gui,params);
                WulforManager::get()->dispatchClientFunc(func);

}

void ADLSearchGUI::editEntry_client(string address, StringMap params)
{
        ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
        ADLSearch * _search = new ADLSearch();

        _search->Prepare(params);
        _search->searchString=string(params["SearchString"]);
        _search->isActive= (Util::toInt(params["Active"]) < 1) ? true : false;
        _search->maxFileSize=(Util::toInt(params["maxSize"]) >0) ? Util::toInt(params["maxSize"]) : -1;
        _search->minFileSize=(Util::toInt(params["minSize"])>0) ? Util::toInt(params["minSize"]) : -1;
        _search->destDir=string(params["DestDir"]);
        _search->sourceType=_search->StringToSourceType(params["SourceType"]);
        _search->isAutoQueue = (Util::toInt(params["AutoQueue"]) < 1) ? true : false;

        for(int i=0;i < collection.size(); i++)
        {
                        if(collection[i].searchString==address)
                                {
                                collection.erase(collection.begin()+i);
                                collection.insert(collection.begin()+i,*_search);

                                }
        }
}


void ADLSearchGUI::removeEntry_client(string address)
{
        ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

    for(int i=0;i < collection.size() ;i++)
    {
                if(collection[i].searchString==address)
                collection.erase(collection.begin() + i);

        }

        typedef Func1<ADLSearchGUI, string> F1;
        F1 *func = new F1(this, &ADLSearchGUI::removeEntry_gui,address);
        WulforManager::get()->dispatchClientFunc(func);

}

void ADLSearchGUI::setConnect_client(string address, bool active)
{
        ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
                for(int i=0;i < collection.size() ;i++)
                {
                        if(collection[i].searchString==address)
                        collection[i].isActive=active;
                }
}
