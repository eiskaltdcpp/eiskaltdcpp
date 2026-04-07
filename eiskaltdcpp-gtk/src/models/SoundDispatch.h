/*
 * Copyright © 2009-2010 freedcpp, https://github.com/eiskaltdcpp/freedcpp
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#pragma once

#include <string>

/**
 * Sound dispatch type → settings key mapping.
 * No GTK dependency — pure lookup tables.
 *
 * Extracted from sound.cc's playSound(TypeSound) switch statement.
 * The actual sound playback remains in the UI layer.
 */
namespace gtk_sound {

/// Sound event types (mirrors Sound::TypeSound)
enum TypeSound
{
    DOWNLOAD_BEGINS,
    DOWNLOAD_FINISHED,
    DOWNLOAD_FINISHED_USER_LIST,
    UPLOAD_FINISHED,
    PRIVATE_MESSAGE,
    HUB_CONNECT,
    HUB_DISCONNECT,
    FAVORITE_USER_JOIN,
    FAVORITE_USER_QUIT,
    SOUND_NONE,

    SOUND_TYPE_COUNT  ///< Sentinel
};

/// Settings key info for a sound event
struct SoundKeyInfo {
    const char *useKey;     ///< Settings key for enable/disable (e.g. "sound-download-finished-use")
    const char *fileKey;    ///< Settings key for sound file path (e.g. "sound-download-finished")
    bool valid;             ///< Whether this type has associated settings
};

/// Get the settings key names for a given sound type.
/// Returns a SoundKeyInfo with valid=false for unrecognized/NONE types.
inline SoundKeyInfo soundSettingsKeys(TypeSound type)
{
    switch (type) {
    case DOWNLOAD_BEGINS:
        // Not yet implemented in the original code
        return {"sound-download-begins-use", "sound-download-begins", false};
    case DOWNLOAD_FINISHED:
        return {"sound-download-finished-use", "sound-download-finished", true};
    case DOWNLOAD_FINISHED_USER_LIST:
        return {"sound-download-finished-ul-use", "sound-download-finished-ul", true};
    case UPLOAD_FINISHED:
        return {"sound-upload-finished-use", "sound-upload-finished", true};
    case PRIVATE_MESSAGE:
        return {"sound-private-message-use", "sound-private-message", true};
    case HUB_CONNECT:
        return {"sound-hub-connect-use", "sound-hub-connect", true};
    case HUB_DISCONNECT:
        return {"sound-hub-disconnect-use", "sound-hub-disconnect", true};
    case FAVORITE_USER_JOIN:
        return {"sound-fuser-join-use", "sound-fuser-join", true};
    case FAVORITE_USER_QUIT:
        return {"sound-fuser-quit-use", "sound-fuser-quit", true};
    default:
        return {nullptr, nullptr, false};
    }
}

} // namespace gtk_sound
