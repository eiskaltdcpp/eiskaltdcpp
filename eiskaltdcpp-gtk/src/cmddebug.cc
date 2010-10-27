/*
 * Copyright © 2010 Mank <mank@besthub.eu>
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

#include "cmddebug.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <dcpp/DebugManager.h>

using namespace std;
using namespace dcpp;


cmddebug::cmddebug():
BookEntry(Entry::CMD,_("CMD"),"cmddebug.glade"),stop(false)
{
buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget("textview1")));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("checkbutton1")) ,TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("checkbutton2")) ,TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("checkbutton3")) ,TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("checkbutton4")) ,TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(getWidget("checkbutton5")) ,TRUE);

}


cmddebug::~cmddebug()
{
	DebugManager::getInstance()->removeListener(this);
}

void cmddebug::add_gui(time_t t,string file)
{
	string line;
	line="";

	gtk_text_buffer_get_end_iter(buffer, &iter);

	line +=Text::toUtf8("["+Util::getShortTimeString(t)+"]"+file+"\n\0");

	gtk_text_buffer_insert(buffer, &iter, line.c_str(), line.size());
	gtk_text_buffer_get_end_iter(buffer, &iter);

	// Limit size of chat text
	if (gtk_text_buffer_get_line_count(buffer) > maxLines + 1)
	{
		GtkTextIter next;
		gtk_text_buffer_get_start_iter(buffer, &iter);
		gtk_text_buffer_get_iter_at_line(buffer, &next, 1);
		gtk_text_buffer_delete(buffer, &iter, &next);
	}
	gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(getWidget("textview1")));

}

void cmddebug::ini_client()
{
start();
DebugManager::getInstance()->addListener(this);

}

void cmddebug::show()
{
ini_client();
}
