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

#ifndef FAVORITE_USERS_HH
#define FAVORITE_USERS_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FavoriteManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class FavoriteUsers:
	public BookEntry,
	public dcpp::FavoriteManagerListener
{
	public:
		FavoriteUsers();
		virtual ~FavoriteUsers();
		virtual void show();

	private:
		typedef std::map<std::string, std::string> ParamMap;
		typedef std::tr1::unordered_map<std::string, GtkTreeIter> UserIters;

		// GUI functions
		bool findUser_gui(const std::string &cid, GtkTreeIter *iter);
		void updateFavoriteUser_gui(ParamMap params);
		void removeFavoriteUser_gui(const std::string cid);
		void setStatus_gui(const std::string text);

		// GUI callbacks
		static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchQueueItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantSlotItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onConnectItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveFromQueueItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onDescriptionItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onAutoGrantSlotToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

		// Client functions
		void getFileList_client(const std::string cid, const std::string hubUrl, bool match);
		void grantSlot_client(const std::string cid, const std::string hubUrl);
		void removeUserFromQueue_client(const std::string cid);
		void removeFavoriteUser_client(const std::string cid);
		void setAutoGrantSlot_client(const std::string cid, bool grant);
		void setUserDescription_client(const std::string cid, const std::string description);

		// Favorite callbacks
		virtual void on(dcpp::FavoriteManagerListener::UserAdded, const dcpp::FavoriteUser &user) throw();
		virtual void on(dcpp::FavoriteManagerListener::UserRemoved, const dcpp::FavoriteUser &user) throw();
		virtual void on(dcpp::FavoriteManagerListener::StatusChanged, const dcpp::FavoriteUser &user) throw();

		UserIters userIters;
		GdkEventType previous;
		TreeView favoriteUserView;
		GtkListStore *favoriteUserStore;
		GtkTreeSelection *favoriteUserSelection;
};

#else
class FavoriteUsers;
#endif
