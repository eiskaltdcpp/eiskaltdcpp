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

#ifdef USE_LIBGNOME2
#include <libgnome/gnome-sound.h>
#endif

#include "settingsmanager.hh"
#include <dcpp/Text.h>
#include "sound.hh"
#include "wulformanager.hh"

using namespace std;
using namespace dcpp;

Sound *Sound::pSound = NULL;

void Sound::start()
{
	dcassert(!pSound);
	pSound = new Sound();
}

void Sound::stop()
{
	dcassert(pSound);
	delete pSound;
	pSound = NULL;
}

Sound* Sound::get()
{
	dcassert(pSound);
	return pSound;
}

void Sound::sound_init()
{
#ifdef USE_LIBGNOME2
	gnome_sound_init(NULL);
	dcdebug("Sound::sound_init: Esound connection %d...\n", gnome_sound_connection_get());
#endif
}

void Sound::playSound(TypeSound sound)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	switch (sound)
	{
//		TODO: download begins, uncomment when implemented
//		case DOWNLOAD_BEGINS:
//
//			if (wsm->getInt("sound-download-begins-use"))
//				playSound(wsm->getString("sound-download-begins"));
//		break;

		case DOWNLOAD_FINISHED:

			if (wsm->getInt("sound-download-finished-use"))
				playSound(wsm->getString("sound-download-finished"));
		break;

		case DOWNLOAD_FINISHED_USER_LIST:

			if (wsm->getInt("sound-download-finished-ul-use"))
				playSound(wsm->getString("sound-download-finished-ul"));
		break;

		case UPLOAD_FINISHED:

			if (wsm->getInt("sound-upload-finished-use"))
				playSound(wsm->getString("sound-upload-finished"));
		break;

		case PRIVATE_MESSAGE:

			if (wsm->getInt("sound-private-message-use"))
				playSound(wsm->getString("sound-private-message"));
		break;

		case HUB_CONNECT:

			if (wsm->getInt("sound-hub-connect-use"))
				playSound(wsm->getString("sound-hub-connect"));
		break;

		case HUB_DISCONNECT:

			if (wsm->getInt("sound-hub-disconnect-use"))
				playSound(wsm->getString("sound-hub-disconnect"));
		break;

		case FAVORITE_USER_JOIN:

			if (wsm->getInt("sound-fuser-join-use"))
				playSound(wsm->getString("sound-fuser-join"));
		break;

		case FAVORITE_USER_QUIT:

			if (wsm->getInt("sound-fuser-quit-use"))
				playSound(wsm->getString("sound-fuser-quit"));
		break;

		default: break;
	}
}

void Sound::playSound(const string &target)
{
#ifdef USE_LIBGNOME2
	gnome_sound_play(Text::fromUtf8(target).c_str());
#else
	FILE *pipe = popen((WulforSettingsManager::getInstance()->getString("sound-command") + " \"" +target+"\" &" ).c_str(), "w" );
	pclose( pipe );
#endif
}

void Sound::sound_finalize()
{
#ifdef USE_LIBGNOME2
	gnome_sound_shutdown();
#endif
}
