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

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <dcpp/Text.h>
#include <dcpp/SimpleXML.h>//NOTE: core 0.762
#include "emoticons.hh"

using namespace std;
using namespace dcpp;

Emoticons *Emoticons::emoticons = NULL;

void Emoticons::start()
{
	dcassert(!emoticons);
	emoticons = new Emoticons();
}

void Emoticons::stop()
{
	dcassert(emoticons);
	delete emoticons;
	emoticons = NULL;
}

Emoticons* Emoticons::get()
{
	dcassert(emoticons);
	return emoticons;
}

Emoticons::Emoticons()
{
	currPackName = WGETS("emoticons-pack");
	create();
}

Emoticons::~Emoticons()
{
	clean();
	WSET("emoticons-pack", currPackName);
}

void Emoticons::create()
{
	clean();

	if (!WGETB("emoticons-use"))
		return;

	string file = currPackName;
	string path = string(_DATADIR) + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	string packName = file;

	/* load current pack */
	if (load(path + packName + ".xml"))
	{
		useEmotions = TRUE;
		return;
	}

	/* load next packs */
	StringList files = File::findFiles(path, "*.xml");

	for(StringIter it = files.begin(); it != files.end(); ++it)
	{
		file = Util::getFileName(*it);
		string::size_type pos = file.rfind('.');
		file = file.substr(0, pos);

		if (file != packName)
		{
			if (load(path + file + ".xml"))
			{
				useEmotions = TRUE;
				currPackName = file;
				return;
			}
		}
	}

	currPackName = "";
}

void Emoticons::clean()
{
	if (!pack.empty())
	{
		for (Emot::Iter it = pack.begin(); it != pack.end(); ++it)
		{
			GList *list = (*it)->getNames();
			g_list_foreach(list, (GFunc)g_free, NULL);
			g_list_free(list);
			g_object_unref(G_OBJECT((*it)->getPixbuf()));

			delete *it;
		}

		pack.clear();
		filter.clear();
	}

	useEmotions = FALSE;
}

bool Emoticons::load(const string &file)
{
	if (file.empty())
		return FALSE;

	string path = string(_DATADIR) + G_DIR_SEPARATOR_S + "emoticons" + G_DIR_SEPARATOR_S;
	countfile = 0;

	try
	{
		SimpleXML xml;
		xml.fromXML(File(file, File::READ, File::OPEN).read());

		if (xml.findChild("emoticons-map"))
		{
			string map = xml.getChildAttrib("name");
			path += map + G_DIR_SEPARATOR_S;
			string emotName, emotPath, emotFile;

			xml.stepIn();

			while (xml.findChild("emoticon"))
			{
				GList *list = NULL;
				emotFile = xml.getChildAttrib("file");
				emotPath = Text::fromUtf8(path + emotFile);

				if (!g_file_test(emotPath.c_str(), G_FILE_TEST_EXISTS))
					continue;

				xml.stepIn();

				while (xml.findChild("name"))
				{
					emotName = xml.getChildAttrib("text");

					if (emotName.empty() || g_utf8_strlen(emotName.c_str(), -1) > Emot::SIZE_NAME || filter.count(emotName))
						continue;

//					FIXME limit emotions
//					if (pack.size() > Emot::SIZE_LIST)
//						continue;

					list = g_list_append(list, g_strdup(emotName.c_str()));
					filter.insert(emotName);
				}

				if (list != NULL)
				{
					Emot *emot = new Emot(list, emotFile, gdk_pixbuf_new_from_file(emotPath.c_str(), NULL));
					pack.push_back(emot);
					countfile++;
				}
					xml.stepOut();
			}
				xml.stepOut();
		}
	}
	catch (const Exception &e)
	{
		dcdebug("eiskaltdcpp-gtk: %s...\n", e.getError().c_str());
		return FALSE;
	}

	return !pack.empty();
}
