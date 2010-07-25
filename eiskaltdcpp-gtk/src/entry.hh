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

#ifndef WULFOR_ENTRY_HH
#define WULFOR_ENTRY_HH

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <string>
#include <map>

class Entry
{
	public:
		typedef enum
		{
			DOWNLOAD_QUEUE,
			FAVORITE_HUBS,
			FAVORITE_USERS,
			FINISHED_DOWNLOADS,
			FINISHED_UPLOADS,
			HASH_DIALOG,
			HUB,
			MAIN_WINDOW,
			PRIVATE_MESSAGE,
			PUBLIC_HUBS,
			SEARCH,
			SETTINGS_DIALOG,
			SHARE_BROWSER,
			TRANSFERS,
			USER_COMMAND_MENU,
			SEARCH_SPY
		} EntryType;

		Entry() : xml(NULL) {}
		Entry(const EntryType type, const std::string &glade, const std::string &id = "");
		virtual ~Entry();

		const EntryType getType();
		const std::string& getID();
		virtual GtkWidget *getContainer() = 0;
		void remove();
		virtual void show() {};

	protected:
		std::string generateID();
		GtkWidget *getWidget(const std::string &name);
		void addChild(Entry *entry);
		Entry *getChild(const EntryType childType, const std::string &childId);
		void removeChild(const EntryType childType, const std::string &childId);
		void removeChild(Entry *entry);
		void removeChildren();

	private:
		GladeXML *xml;
		EntryType type;
		std::string id;
		std::map<std::string, Entry *> children;
};

#else
class Entry;
#endif
