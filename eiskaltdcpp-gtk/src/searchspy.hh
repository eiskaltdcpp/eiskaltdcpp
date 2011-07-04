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

#ifndef SEARCH_SPY_HH
#define SEARCH_SPY_HH

#include <dcpp/stdinc.h>
#include <dcpp/ClientManager.h>
#include <dcpp/TimerManager.h>

#include "bookentry.hh"
#include "treeview.hh"

class SearchSpy:
	public BookEntry,
	public dcpp::ClientManagerListener,
	public dcpp::TimerManagerListener
{
	public:
		SearchSpy();
		virtual ~SearchSpy();
		virtual void show();
		void preferences_gui();

	private:
		typedef std::tr1::unordered_map<std::string, GtkTreeIter> SearchIters;
		typedef SearchIters::size_type SearchType;

		// GUI functions
		bool updateFrameStatus_gui(GtkTreeIter *iter, uint64_t tick);
		void updateFrameStatus_gui();
		bool findIter_gui(const std::string &search, GtkTreeIter *iter);
		void updateFrameSearch_gui(const std::string search, const std::string type);
		void setStatus_gui(const std::string text);
		void addTop_gui(const std::string &search, const std::string &type);
		void resetFrame();
		void resetCount();

		// GUI callbacks
		static void onSearchItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data);
		static void onClearFrameClicked_gui(GtkWidget *widget, gpointer data);
		static void onUpdateFrameClicked_gui(GtkWidget *widget, gpointer data);
		static void onShowTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onSearchTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onClearTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onRemoveTopClicked_gui(GtkWidget *widget, gpointer data);
		static void onIgnoreTTHSearchToggled_gui(GtkWidget *widget, gpointer data);
		static void onOKButtonClicked_gui(GtkWidget *widget, gpointer data);
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);

		// Client callbacks
		virtual void on(dcpp::ClientManagerListener::IncomingSearch, const std::string& s) noexcept;
		virtual void on(dcpp::TimerManagerListener::Minute, uint64_t tick) noexcept;

		SearchType FrameSize;
		guint Waiting;
		guint Top;
		GdkEventType previous;
		TreeView searchView;
		GtkListStore *searchStore;
		GtkTreeSelection *searchSelection;
		SearchIters searchIters;
		TreeView topView;
		GtkListStore *topStore;
		std::string aSearchColor, cSearchColor, rSearchColor, tSearchColor, qSearchColor;
};

#else
class SearchSpy;
#endif
