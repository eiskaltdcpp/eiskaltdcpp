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

#pragma once

#include <gtk/gtk.h>

#define EMOTICONS_MAX 48

class Emot
{
	public:
//		enum {SIZE_LIST = 255}; FIXME limit emotions
		enum {SIZE_NAME = 24};
		typedef std::vector<Emot *> List;
		typedef List::const_iterator Iter;

		Emot(GList *names, std::string file, GdkPixbuf *pixbuf = NULL) :
			names(names), file(file), pixbuf(pixbuf) {}
		~Emot() {}

		GList* getNames() {return names;}
		std::string getFile() {return file;}
		GdkPixbuf* getPixbuf() {return pixbuf;}

	private:
		GList *names;
		std::string file;
		GdkPixbuf *pixbuf;
};

class Emoticons
{
	public:
		static void start();
		static void stop();
		static Emoticons* get();

		Emoticons();
		~Emoticons();

		// GUI functions
		Emot::List& getPack_gui() {return pack;}
		int getCountFile_gui() {return countfile;}
		bool useEmoticons_gui() {return useEmotions;}
		std::string getCurrPackName_gui() {return currPackName;}
		void setCurrPackName_gui(const std::string &name) {currPackName = name;}
		void reloadPack_gui() {create();}

	private:
		static Emoticons *emoticons;

		bool load(const std::string &file);
		void create();
		void clean();

		bool useEmotions;
		int countfile;
		Emot::List pack;
		std::set<std::string> filter;
		std::string currPackName;
};
