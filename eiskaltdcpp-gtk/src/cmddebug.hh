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

#ifndef WULFOR_CMDDEBUG_HH
#define WULFOR_CMDDEBUG_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/DebugManager.h>
#include <dcpp/Util.h>

#include "bookentry.hh"
#include "treeview.hh"

#include "wulformanager.hh"


class cmddebug:
	public BookEntry, private dcpp::DebugManagerListener, public dcpp::Thread
{
	public:
		cmddebug();
		virtual ~cmddebug();
		virtual void show();
		//GUI FCE
		void add_gui(time_t t,std::string file);

	private:
		// Client functions
		void ini_client();
		bool stop;

		dcpp::CriticalSection cs;
		dcpp::Semaphore s;
		std::deque<std::string> cmdList;

		int run() {
			setThreadPriority(dcpp::Thread::LOW);
			std::string x ;//= dcpp::Util::emptyString;
			stop = false;

			while(true) {
				s.wait();

				if(stop)
					break;

				{
					dcpp::Lock l(cs);

					//if(cmdList.empty()) continue;

					x = cmdList.front();
					cmdList.pop_front();

				}
			typedef Func2<cmddebug,time_t,std::string> F2;
			time_t tt = time(NULL);
			F2 *func = new F2(this, &cmddebug::add_gui, tt, x);
			WulforManager::get()->dispatchGuiFunc(func);
			}

		stop = false;
		return 0;
	}

	void addCmd(const std::string& cmd) {
		{
			dcpp::Lock l(cs);
		//	g_print("CMD %s",cmd.c_str());
			cmdList.push_back(cmd);
		}

		s.signal();
	}

	//DebugManager
	void on(dcpp::DebugManagerListener::DebugDetection, const std::string& com) throw()
	{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton5"))) == TRUE)
		addCmd(com);
	}
	void on(dcpp::DebugManagerListener::DebugCommand, const std::string& mess, int typedir, const std::string& ip) throw()
	{
	switch(typedir) {
			case dcpp::DebugManager::HUB_IN :
					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton1"))) == TRUE)
					{
						if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton6"))) == TRUE)
						{
							if(gtk_entry_get_text(GTK_ENTRY(getWidget("entrybyip"))) == ip.c_str())
								addCmd("HUB_IN: "+ip+": "+mess);
						}
						else
								addCmd("HUB_IN: "+ip+": "+mess);
					}
					break;
			case dcpp::DebugManager::HUB_OUT :
					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton2"))) == TRUE)
					{

						if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton6"))) == TRUE)
						{
							if(gtk_entry_get_text(GTK_ENTRY(getWidget("entrybyip"))) == ip.c_str())
								addCmd("HUB_IN: "+ip+": "+mess);
						}
						else
						{	addCmd("HUB_OUT: "+ip+": "+mess); }
					}
					break;
			case dcpp::DebugManager::CLIENT_IN:
					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton3"))) == TRUE)
					{
						if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton6"))) == TRUE)
						{
							if(gtk_entry_get_text(GTK_ENTRY(getWidget("entrybyip"))) == ip.c_str())
								addCmd("HUB_IN: "+ip+": "+mess);
						}
						else
						{	addCmd("CL_IN: "+ip+": "+mess); }
					}
					break;
			case dcpp::DebugManager::CLIENT_OUT:
					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton4"))) == TRUE)
					{
						if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("checkbutton6"))) == TRUE)
						{
							if(gtk_entry_get_text(GTK_ENTRY(getWidget("entrybyip"))) == ip.c_str())
								addCmd("HUB_IN: "+ip+": "+mess);
						}else
						{		addCmd("CL_OUT: "+ip+": "+mess); }
					}
					break;
			default: dcassert(0);
		}
	}
	GtkTextBuffer *buffer;
	static const int maxLines = 1000;
	GtkTextIter iter;

};

#else
class cmddebug;
#endif
