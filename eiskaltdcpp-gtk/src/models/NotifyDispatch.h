/*
 * Copyright © 2009 Leliksan Floyd <leliksan@Quadrafon2>
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
 * Notification dispatch type → settings key mapping.
 * No GTK dependency — pure lookup tables.
 *
 * Extracted from notify.cc's showNotify(TypeNotify) switch statement.
 * The actual notification display remains in the UI layer.
 */
namespace gtk_notify {

/// Notification event types (mirrors Notify::TypeNotify)
enum TypeNotify
{
    DOWNLOAD_FINISHED,
    DOWNLOAD_FINISHED_USER_LIST,
    PRIVATE_MESSAGE,
    HUB_CONNECT,
    HUB_DISCONNECT,
    FAVORITE_USER_JOIN,
    FAVORITE_USER_QUIT,
    NOTIFY_NONE,

    NOTIFY_TYPE_COUNT  ///< Sentinel
};

/// Settings key info for a notification event
struct NotifyKeyInfo {
    const char *useKey;      ///< Settings key for enable/disable (e.g. "notify-download-finished-use")
    const char *titleKey;    ///< Settings key for notification title
    const char *iconKey;     ///< Settings key for notification icon path
    bool valid;              ///< Whether this type has associated settings
};

/// Get the settings key names for a given notification type.
/// Returns a NotifyKeyInfo with valid=false for unrecognized/NONE types.
inline NotifyKeyInfo notifySettingsKeys(TypeNotify type)
{
    switch (type) {
    case DOWNLOAD_FINISHED:
        return {"notify-download-finished-use", "notify-download-finished-title",
                "notify-download-finished-icon", true};
    case DOWNLOAD_FINISHED_USER_LIST:
        return {"notify-download-finished-ul-use", "notify-download-finished-ul-title",
                "notify-download-finished-ul-icon", true};
    case PRIVATE_MESSAGE:
        return {"notify-private-message-use", "notify-private-message-title",
                "notify-private-message-icon", true};
    case HUB_CONNECT:
        return {"notify-hub-connect-use", "notify-hub-connect-title",
                "notify-hub-connect-icon", true};
    case HUB_DISCONNECT:
        return {"notify-hub-disconnect-use", "notify-hub-disconnect-title",
                "notify-hub-disconnect-icon", true};
    case FAVORITE_USER_JOIN:
        return {"notify-fuser-join", "notify-fuser-join-title",
                "notify-fuser-join-icon", true};
    case FAVORITE_USER_QUIT:
        return {"notify-fuser-quit", "notify-fuser-quit-title",
                "notify-fuser-quit-icon", true};
    default:
        return {nullptr, nullptr, nullptr, false};
    }
}

/// Icon size presets matching the Notify class enum
enum IconSizePreset {
    ICON_16 = 0,
    ICON_22,
    ICON_24,
    ICON_32,
    ICON_36,
    ICON_48,
    ICON_64,
    ICON_DEFAULT
};

/// Convert an icon size preset to actual pixel dimensions.
/// Returns the pixel size, or 0 for ICON_DEFAULT (meaning use original size).
inline int iconSizePixels(IconSizePreset preset)
{
    switch (preset) {
    case ICON_16:  return 16;
    case ICON_22:  return 22;
    case ICON_24:  return 24;
    case ICON_32:  return 32;
    case ICON_36:  return 36;
    case ICON_48:  return 48;
    case ICON_64:  return 64;
    case ICON_DEFAULT: return 0;
    default: return 0;
    }
}

} // namespace gtk_notify
