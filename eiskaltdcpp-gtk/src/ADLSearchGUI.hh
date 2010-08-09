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

#ifndef WULFOR_ADL_HH
#define WULFOR_ADL_HH



#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/ADLSearch.h>


#include "bookentry.hh"
#include "treeview.hh"

class ADLSearchGUI:
        public BookEntry,
        public dcpp::FavoriteManagerListener
{
        public:
                ADLSearchGUI();
                virtual ~ADLSearchGUI();
                virtual void show();

        private:
                // GUI functions
                void addEntry_gui(dcpp::StringMap params);
                void editEntry_gui(dcpp::StringMap &params, GtkTreeIter *iter);
                void removeEntry_gui(std::string address);
                void showErrorDialog_gui(const std::string &description);
                void popupMenu_gui();
                bool showFavoriteHubDialog_gui(dcpp::StringMap &params);

                // GUI callbacks
                static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
                static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
                //static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
                static void onAddEntry_gui(GtkWidget *widget, gpointer data);
                static void onEditEntry_gui(GtkWidget *widget, gpointer data);
                static void onRemoveEntry_gui(GtkWidget *widget, gpointer data);

                static void onToggledClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);


                // Client functions
                void initializeList_client();
                void getFavHubParams_client(dcpp::ADLSearch& search, dcpp::StringMap &params);

                void addEntry_client(dcpp::StringMap params);
                void editEntry_client(std::string address, dcpp::StringMap params);
                void removeEntry_client(std::string address);
                void setConnect_client(std::string address, bool active);

                std::vector<std::string> charsets;
                std::vector<std::string>& getCharsets()
                {
                        if (charsets.size() == 0)
                        {
                        charsets.push_back(_("Filename"));
                        charsets.push_back(_("Full Path"));
                        charsets.push_back(_("Directory"));
                        }
                return charsets;
        }



                TreeView favoriteView;
                GtkListStore *favoriteStore;
                GtkTreeSelection *favoriteSelection;
                GdkEventType previous;
};

#else
class ADLSearchGUI;
#endif
