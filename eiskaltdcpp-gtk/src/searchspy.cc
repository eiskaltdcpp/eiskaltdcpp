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

#include "searchspy.hh"
#include <dcpp/SearchManager.h>
#include <dcpp/TimerManager.h>
#include <dcpp/Util.h>
#include "settingsmanager.hh"
#include "search.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

SearchSpy::SearchSpy():
    BookEntry(Entry::SEARCH_SPY, _("Search Spy"), "searchspy.ui")
{
#if !GTK_CHECK_VERSION(3,0,0)
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("statusbar")),FALSE);
#endif

    FrameSize = (SearchType)WGETI("search-spy-frame");
    Waiting = (guint)WGETI("search-spy-waiting");
    Top = (guint)WGETI("search-spy-top");

    // Configure the dialog
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("ignoreTTHSearchCheckButton")), WGETB("spyframe-ignore-tth-searches"));
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("TopSearchDialog")), GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("frameSpinButton")), (double)FrameSize);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("waitingSpinButton")), (double)Waiting);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("topSpinButton")), (double)Top);

    // menu
    g_object_ref_sink(getWidget("menu"));

    // Initialize search list treeview
    searchView.setView(GTK_TREE_VIEW(getWidget("searchSpyView")), TRUE, "searchspy");
    searchView.insertColumn(_("Search String"), G_TYPE_STRING, TreeView::ICON_STRING_TEXT_COLOR, 305, "icon", "color");
    searchView.insertColumn(_("Count"), G_TYPE_STRING, TreeView::STRING, 70);
    searchView.insertColumn(_("Time"), G_TYPE_STRING, TreeView::STRING, 90);
    searchView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::STRING, 90);
    searchView.insertHiddenColumn("type", G_TYPE_STRING);
    searchView.insertHiddenColumn("count", G_TYPE_UINT);
    searchView.insertHiddenColumn("tick", G_TYPE_UINT64);
    searchView.insertHiddenColumn("icon", G_TYPE_STRING);
    searchView.insertHiddenColumn("order", G_TYPE_STRING);
    searchView.insertHiddenColumn("color", G_TYPE_STRING);
    searchView.finalize();

    searchStore = gtk_list_store_newv(searchView.getColCount(), searchView.getGTypes());
    gtk_tree_view_set_model(searchView.get(), GTK_TREE_MODEL(searchStore));
    g_object_unref(searchStore);

    searchSelection = gtk_tree_view_get_selection(searchView.get());
    gtk_tree_selection_set_mode(searchSelection, GTK_SELECTION_MULTIPLE);
    searchView.setSortColumn_gui(_("Search String"), "count");
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(searchStore), searchView.col("count"), GTK_SORT_DESCENDING);
    gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(searchView.get(), searchView.col(_("Search String"))), TRUE);
    gtk_tree_view_set_fixed_height_mode(searchView.get(), TRUE);

    topView.setView(GTK_TREE_VIEW(getWidget("topView")));
    topView.insertColumn(_("Search String"), G_TYPE_STRING, TreeView::STRING, -1);
    topView.insertHiddenColumn("type", G_TYPE_STRING);
    topView.finalize();

    topStore = gtk_list_store_newv(topView.getColCount(), topView.getGTypes());
    gtk_tree_view_set_model(topView.get(), GTK_TREE_MODEL(topStore));
    g_object_unref(topStore);

    g_signal_connect(getWidget("searchItem"), "activate", G_CALLBACK(onSearchItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("clearFrameButton"), "clicked", G_CALLBACK(onClearFrameClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("updateFrameButton"), "clicked", G_CALLBACK(onUpdateFrameClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("showTopButton"), "clicked", G_CALLBACK(onShowTopClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("clearTopButton"), "clicked", G_CALLBACK(onClearTopClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchTopButton"), "clicked", G_CALLBACK(onSearchTopClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeTopButton"), "clicked", G_CALLBACK(onRemoveTopClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("ignoreTTHSearchCheckButton"), "toggled", G_CALLBACK(onIgnoreTTHSearchToggled_gui), (gpointer)this);
    g_signal_connect(searchView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
    g_signal_connect(searchView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
    g_signal_connect(searchView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
    g_signal_connect(getWidget("okButton"), "clicked", G_CALLBACK(onOKButtonClicked_gui), (gpointer)this);

    aSearchColor = WGETS("search-spy-a-color");
    tSearchColor = WGETS("search-spy-t-color");
    qSearchColor = WGETS("search-spy-q-color");
    cSearchColor = WGETS("search-spy-c-color");
    rSearchColor = WGETS("search-spy-r-color");
}

SearchSpy::~SearchSpy()
{
    WSET("search-spy-frame", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("frameSpinButton"))));
    WSET("search-spy-waiting", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("waitingSpinButton"))));
    WSET("search-spy-top", (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(getWidget("topSpinButton"))));

    gtk_widget_destroy(getWidget("TopSearchDialog"));
    g_object_unref(getWidget("menu"));

    TimerManager::getInstance()->removeListener(this);
    ClientManager::getInstance()->removeListener(this);
}

void SearchSpy::show()
{
    ClientManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
}

void SearchSpy::preferences_gui()
{
    FrameSize = (SearchType)WGETI("search-spy-frame");
    Waiting = (guint)WGETI("search-spy-waiting");
    Top = (guint)WGETI("search-spy-top");

    resetFrame();

    // reset colors
    aSearchColor = WGETS("search-spy-a-color");
    tSearchColor = WGETS("search-spy-t-color");
    qSearchColor = WGETS("search-spy-q-color");
    cSearchColor = WGETS("search-spy-c-color");
    rSearchColor = WGETS("search-spy-r-color");

    string color, order;
    GtkTreeIter iter;

    for (SearchIters::const_iterator it = searchIters.begin(); it != searchIters.end(); ++it)
    {
        iter = it->second;
        order = searchView.getString(&iter, "order");
        guint count = searchView.getValue<guint>(&iter, "count");

        // reset count
        if (count > Top)
        {
            gtk_list_store_set(searchStore, &iter,
                searchView.col(_("Count")), Util::toString(Top).c_str(),
                searchView.col("count"), Top,
                -1);
        }

        switch (order[0])
        {
            case 'a': color = aSearchColor; break;
            case 'c': color = cSearchColor; break;
            case 'r': color = rSearchColor; break;
            case 't': color = tSearchColor; break;
            case 'q': color = qSearchColor; break;
        }
        gtk_list_store_set(searchStore, &iter, searchView.col("color"), color.c_str(), -1);
    }

    // reset spin button
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("frameSpinButton")), (double)FrameSize);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("waitingSpinButton")), (double)Waiting);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(getWidget("topSpinButton")), (double)Top);
}

void SearchSpy::resetFrame()
{
    GtkTreeIter iter;

    if (FrameSize > 0 && searchIters.size() > FrameSize)
    {
        SearchType i = 0;
        gtk_tree_selection_select_all(searchSelection);

        for (SearchIters::const_iterator it = searchIters.begin(); it != searchIters.end(); ++it)
        {
            if (++i > FrameSize)
                break;

            iter = it->second;
            gtk_tree_selection_unselect_iter(searchSelection, &iter);
        }
        onRemoveItemClicked_gui(NULL, (gpointer)this);
    }
}

bool SearchSpy::findIter_gui(const string &search, GtkTreeIter *iter)
{
    SearchIters::const_iterator it = searchIters.find(search);

    if (it != searchIters.end())
    {
        if (iter)
            *iter = it->second;

        return TRUE;
    }

    return FALSE;
}

void SearchSpy::addTop_gui(const string &search, const string &type)
{
    GtkTreeIter iter;
    GtkTreeModel *m = GTK_TREE_MODEL(topStore);
    gboolean valid = gtk_tree_model_get_iter_first(m, &iter);

    while (valid)
    {
        string line = topView.getString(&iter, _("Search String"));

        if (search == line)
        {
            return;
        }
        valid = gtk_tree_model_iter_next(m, &iter);
    }

    gtk_list_store_append(topStore, &iter);
    gtk_list_store_set(topStore, &iter,
        topView.col(_("Search String")), search.c_str(),
        topView.col("type"), type.c_str(),
        -1);

    if (WGETB("bold-search-spy"))
        setUrgent_gui();
}

void SearchSpy::updateFrameSearch_gui(const string search, const string type)
{
    g_return_if_fail(FrameSize > 0 && FrameSize <= 256);

    GtkTreeIter iter;

    if (findIter_gui(search, &iter))
    {
        uint64_t tick = GET_TICK();

        if (searchView.getString(&iter, "order")[0] == 't')
        {
            updateFrameStatus_gui(NULL, tick);
            return;
        }

        string time = Util::formatTime("%H:%M:%S", GET_TIME());
        guint count = searchView.getValue<guint>(&iter, "count");
        string order = "c";

        if (count >= Top)
        {
            addTop_gui(search, type);

            count = 0;
            order = "t";
        }
        count++;
        gtk_list_store_set(searchStore, &iter,
            searchView.col(_("Count")), Util::toString(count).c_str(),
            searchView.col(_("Time")), time.c_str(),
            searchView.col("count"), count,
            searchView.col("tick"), tick,
            searchView.col("order"), order.c_str(),
            -1);
        updateFrameStatus_gui(NULL, tick);
    }
    else
    {
        string time = Util::formatTime("%H:%M:%S", GET_TIME());
        uint64_t tick = GET_TICK();

        if (searchIters.size() >= FrameSize)
        {
            if (updateFrameStatus_gui(&iter, tick))
            {
                string oldstring = searchView.getString(&iter, _("Search String"));
                searchIters.erase(oldstring);
                searchIters.insert(SearchIters::value_type(search, iter));

                tick = GET_TICK();

                gtk_list_store_set(searchStore, &iter,
                    searchView.col(_("Search String")), search.c_str(),
                    searchView.col(_("Count")), "1",
                    searchView.col(_("Time")), time.c_str(),
                    searchView.col(_("Status")), _("waiting..."),
                    searchView.col("type"), type.c_str(),
                    searchView.col("count"), 1,
                    searchView.col("tick"), tick,
                    searchView.col("icon"), GTK_STOCK_FIND,
                    searchView.col("order"), "r",
                    searchView.col("color"), rSearchColor.c_str(),
                    -1);
            }
            return;
        }

        gtk_list_store_insert_with_values(searchStore, &iter, searchIters.size(),
            searchView.col(_("Search String")), search.c_str(),
            searchView.col(_("Count")), "1",
            searchView.col(_("Time")), time.c_str(),
            searchView.col("type"), type.c_str(),
            searchView.col("count"), 1,
            searchView.col("tick"), tick,
            searchView.col("order"), "a",
            -1);

        searchIters.insert(SearchIters::value_type(search, iter));
        updateFrameStatus_gui(NULL, tick);
    }
}

bool SearchSpy::updateFrameStatus_gui(GtkTreeIter *iter, uint64_t tick)
{
    if (tick == 0)
        tick = GET_TICK();

    uint64_t second = (uint64_t)Waiting * 1000;
    bool n = FALSE;
    string status, icon;
    GtkTreeIter itree;
    string color;

    for (SearchIters::const_iterator it = searchIters.begin(); it != searchIters.end(); ++it)
    {
        itree = it->second;
        uint64_t gettick = searchView.getValue<uint64_t>(&itree, "tick");
        string order = searchView.getString(&itree, "order");

        dcassert(tick >= gettick);

        if (tick - gettick > second)
        {
            if (iter)
            {
                *iter = itree;
                n = TRUE;
            }
            status = "?";
            icon = GTK_STOCK_DIALOG_QUESTION;

            color = qSearchColor;
            gtk_list_store_set(searchStore, &itree, searchView.col("order"), "q", -1);
        }
        else
        {
            if (order[0] == 't')
            {
                status = _("top...");
                icon = GTK_STOCK_DIALOG_QUESTION;
            }
            else
            {
                status = _("waiting...");
                icon = GTK_STOCK_FIND;
            }

            switch (order[0])
            {
                case 'a': color = aSearchColor; break;
                case 'c': color = cSearchColor; break;
                case 'r': color = rSearchColor; break;
                case 't': color = tSearchColor; break;
                default:  color = qSearchColor; // fix don't know color
            }
        }
        gtk_list_store_set(searchStore, &itree,
            searchView.col(_("Status")), status.c_str(),
            searchView.col("icon"), icon.c_str(),
            searchView.col("color"), color.c_str(),
            -1);
    }

    return n;
}

void SearchSpy::updateFrameStatus_gui()
{
    updateFrameStatus_gui(NULL, uint64_t(0));
    setStatus_gui(_("Update frame search"));
}

void SearchSpy::setStatus_gui(const string text)
{
    if (!text.empty())
    {
        GtkWidget *status = getWidget("statusbar");
        gtk_statusbar_pop(GTK_STATUSBAR(status), 0);
        gtk_statusbar_push(GTK_STATUSBAR(status), 0, ("[" + Util::getShortTimeString() + "] " + text).c_str());
    }
}

void SearchSpy::onOKButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s =  (SearchSpy *) data;

    s->FrameSize = (SearchType)gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("frameSpinButton")));
    s->Waiting = (guint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("waitingSpinButton")));
    s->Top = (guint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->getWidget("topSpinButton")));
    s->resetFrame();
    s->resetCount();

    s->setStatus_gui(_("top/waiting/frame: ") + Util::toString(s->Top) + "/" + Util::toString(s->Waiting) + "/" +
        Util::toString(s->FrameSize));

    WSET("search-spy-frame", int(s->FrameSize));
    WSET("search-spy-waiting", int(s->Waiting));
    WSET("search-spy-top", int(s->Top));
}

void SearchSpy::resetCount()
{
    GtkTreeIter iter;
    for (SearchIters::const_iterator it = searchIters.begin(); it != searchIters.end(); ++it)
    {
        iter = it->second;
        guint count = searchView.getValue<guint>(&iter, "count");

        if (count > Top)
        {
            gtk_list_store_set(searchStore, &iter,
                searchView.col(_("Count")), Util::toString(Top).c_str(),
                searchView.col("count"), Top,
                -1);
        }
    }
}

void SearchSpy::onShowTopClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    GtkWidget *dialog = s->getWidget("TopSearchDialog");
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    // if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;
    gtk_widget_hide(dialog);
}

void SearchSpy::onClearTopClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;
    gtk_list_store_clear(s->topStore);
}

void SearchSpy::onSearchTopClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    GtkTreeIter iter;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(s->topView.get());

    if (gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
        string type = s->topView.getString(&iter, "type");
        string search = s->topView.getString(&iter, _("Search String"));
        Search *ss = WulforManager::get()->getMainWindow()->addSearch_gui();

        if (type[0] == 't')
        {
            ss->putValue_gui(search, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
        }
        else if (type[0] == 's')
        {
            ss->putValue_gui(search, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_ANY);
        }
    }
}

void SearchSpy::onRemoveTopClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    GtkTreeIter iter;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(s->topView.get());

    if (gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
        gtk_list_store_remove(s->topStore, &iter);
    }
}

void SearchSpy::onClearFrameClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    gtk_list_store_clear(s->searchStore);
    s->searchIters.clear();
    s->setStatus_gui(_("Clear frame search"));
}

void SearchSpy::onUpdateFrameClicked_gui(GtkWidget *widget, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    s->updateFrameStatus_gui();
}

void SearchSpy::onIgnoreTTHSearchToggled_gui(GtkWidget *widget, gpointer data)
{
    gboolean toggle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    WSET("spyframe-ignore-tth-searches",toggle);
}

void SearchSpy::onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    if (gtk_tree_selection_count_selected_rows(s->searchSelection) > 0)
    {
        StringList search;
        GtkTreeIter iter;
        GtkTreePath *path;
        GList *list = gtk_tree_selection_get_selected_rows(s->searchSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *) i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(s->searchStore), &iter, path))
            {
                search.push_back(s->searchView.getString(&iter, _("Search String")));
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);

        for (StringIterC it = search.begin(); it != search.end(); ++it)
        {
            if (s->findIter_gui(*it, &iter))
            {
                gtk_list_store_remove(s->searchStore, &iter);
                s->searchIters.erase(*it);
            }
        }
    }
}

void SearchSpy::onSearchItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    if (gtk_tree_selection_count_selected_rows(s->searchSelection) == 1)
    {
        GList *list = gtk_tree_selection_get_selected_rows(s->searchSelection, NULL);

        if (list != NULL)
        {
            GtkTreeIter iter;
            GtkTreePath *path = (GtkTreePath *) list->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(s->searchStore), &iter, path))
            {
                string type = s->searchView.getString(&iter, "type");
                string search = s->searchView.getString(&iter, _("Search String"));
                Search *ss = WulforManager::get()->getMainWindow()->addSearch_gui();

                if (type[0] == 't')
                {
                    ss->putValue_gui(search, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
                }
                else if (type[0] == 's')
                {
                    ss->putValue_gui(search, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_ANY);
                }
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

gboolean SearchSpy::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;
    s->previous = event->type;

    if (event->button == 3)
    {
        GtkTreePath *path;

        if (gtk_tree_view_get_path_at_pos(s->searchView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
        {
            bool selected = gtk_tree_selection_path_is_selected(s->searchSelection, path);
            gtk_tree_path_free(path);

            if (selected)
                return TRUE;
        }
    }
    return FALSE;
}

gboolean SearchSpy::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    if (gtk_tree_selection_count_selected_rows(s->searchSelection) > 0)
    {
        if (event->button == 1 && s->previous == GDK_2BUTTON_PRESS)
        {
            // search string/tth
            s->onSearchItemClicked_gui(NULL, data);
        }
        else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
        {
            // show menu
            gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
        }
    }

    return FALSE;
}

gboolean SearchSpy::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    SearchSpy *s = (SearchSpy *)data;

    if (gtk_tree_selection_count_selected_rows(s->searchSelection) > 0)
    {
        if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
        {
            s->onRemoveItemClicked_gui(NULL, data);
        }
        else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
        {
            gtk_menu_popup(GTK_MENU(s->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
        }
    }

    return FALSE;
}

void SearchSpy::on(ClientManagerListener::IncomingSearch, const string& s) noexcept
{
    if(WGETB("spyframe-ignore-tth-searches") && s.compare(0, 4, "TTH:") == 0)
        return;

    string search, type;
    if(s.compare(0, 4, "TTH:") == 0)
    {
        type = "t";
        search = s.substr(4);
    }
    else
    {
        type = "s";
        search = s;
        string::size_type i;
        while((i = search.find("$")) != string::npos)
            search[i] = ' ';
    }
    typedef Func2<SearchSpy, string, string> F2;
    F2 *func = new F2(this, &SearchSpy::updateFrameSearch_gui, search, type);
    WulforManager::get()->dispatchGuiFunc(func);
}

void SearchSpy::on(TimerManagerListener::Minute, uint64_t tick) noexcept
{
    typedef Func0<SearchSpy> F0;
    F0 *func = new F0(this, &SearchSpy::updateFrameStatus_gui);
    WulforManager::get()->dispatchGuiFunc(func);
}
