/*
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
 */

#include <dcpp/stdinc.h>

#include "GtkSettingsDefaults.h"
#include "GtkSettingsModel.h"

using std::string;

namespace gtk_settings {

void populateDefaults(GtkSettingsModel &m,
                      const string &downloadDir,
                      const string &encodingLocale)
{
    // ────────────────────────── Int defaults ──────────────────────────
    // Extracted from WulforSettingsManager::WulforSettingsManager()
    // in eiskaltdcpp-gtk/src/settingsmanager.cc
    //
    // Note: "urlhandler" appeared twice in the original; we set it once.

    // Window geometry
    m.setDefault("main-window-maximized", 0);
    m.setDefault("main-window-size-x", 875);
    m.setDefault("main-window-size-y", 685);
    m.setDefault("main-window-pos-x", 100);
    m.setDefault("main-window-pos-y", 100);
    m.setDefault("main-window-no-close", 0);

    // Pane positions
    m.setDefault("transfer-pane-position", 204);
    m.setDefault("nick-pane-position", 255);
    m.setDefault("downloadqueue-pane-position", 200);
    m.setDefault("sharebrowser-pane-position", 200);

    // Tabs / toolbar
    m.setDefault("tab-position", 0);
    m.setDefault("toolbar-style", 5);

    // Sound toggles
    m.setDefault("sound-pm-open", 0);
    m.setDefault("sound-pm", 1);
    m.setDefault("sound-download-finished-use", 0);
    m.setDefault("sound-download-finished-ul-use", 0);
    m.setDefault("sound-upload-finished-use", 0);
    m.setDefault("sound-private-message-use", 0);
    m.setDefault("sound-hub-connect-use", 0);
    m.setDefault("sound-hub-disconnect-use", 0);
    m.setDefault("sound-fuser-join-use", 0);
    m.setDefault("sound-fuser-quit-use", 0);

    // Magnet
    m.setDefault("use-magnet-split", 1);

    // Text style booleans (bold / italic per category)
    m.setDefault("text-bold-autors", 1);
    m.setDefault("text-general-bold", 0);
    m.setDefault("text-general-italic", 0);
    m.setDefault("text-myown-bold", 1);
    m.setDefault("text-myown-italic", 0);
    m.setDefault("text-private-bold", 0);
    m.setDefault("text-private-italic", 0);
    m.setDefault("text-system-bold", 1);
    m.setDefault("text-system-italic", 0);
    m.setDefault("text-status-bold", 1);
    m.setDefault("text-status-italic", 0);
    m.setDefault("text-timestamp-bold", 1);
    m.setDefault("text-timestamp-italic", 0);
    m.setDefault("text-mynick-bold", 1);
    m.setDefault("text-mynick-italic", 0);
    m.setDefault("text-fav-bold", 1);
    m.setDefault("text-fav-italic", 0);
    m.setDefault("text-op-bold", 1);
    m.setDefault("text-op-italic", 0);
    m.setDefault("text-url-bold", 0);
    m.setDefault("text-url-italic", 0);

    // Toolbar buttons
    m.setDefault("toolbar-button-separators", 0);
    m.setDefault("toolbar-button-reconnect", 1);
    m.setDefault("toolbar-button-connect", 1);
    m.setDefault("toolbar-button-fav-hubs", 1);
    m.setDefault("toolbar-button-fav-users", 1);
    m.setDefault("toolbar-button-public-hubs", 1);
    m.setDefault("toolbar-button-settings", 1);
    m.setDefault("toolbar-button-own-filelist", 1);
    m.setDefault("toolbar-button-refresh", 1);
    m.setDefault("toolbar-button-hash", 1);
    m.setDefault("toolbar-button-search", 1);
    m.setDefault("toolbar-button-search-spy", 1);
    m.setDefault("toolbar-button-queue", 1);
    m.setDefault("toolbar-button-quit", 1);
    m.setDefault("toolbar-button-finished-downloads", 1);
    m.setDefault("toolbar-button-search-adl", 1);
    m.setDefault("toolbar-button-finished-uploads", 1);
    m.setDefault("toolbar-button-add", 0);

    // Notifications
    m.setDefault("notify-download-finished-use", 0);
    m.setDefault("notify-download-finished-ul-use", 0);
    m.setDefault("notify-private-message-use", 0);
    m.setDefault("notify-hub-disconnect-use", 0);
    m.setDefault("notify-hub-connect-use", 0);
    m.setDefault("notify-fuser-join", 0);
    m.setDefault("notify-fuser-quit", 0);
    m.setDefault("notify-pm-length", 50);
    m.setDefault("notify-icon-size", 3);
    m.setDefault("notify-only-not-active", 0);

    // Status icon
    m.setDefault("status-icon-blink-use", 1);

    // Emoticons
    m.setDefault("emoticons-use", 1);

    // PM behavior
    m.setDefault("pm", 0);

    // Search spy
    m.setDefault("search-spy-frame", 50);
    m.setDefault("search-spy-waiting", 40);
    m.setDefault("search-spy-top", 4);

    // Toolbar position
    m.setDefault("toolbar-position", 1);
    m.setDefault("toolbar-small", 0);

    // Open-on-startup
    m.setDefault("open-public", 0);
    m.setDefault("open-favorite-hubs", 0);
    m.setDefault("open-queue", 0);
    m.setDefault("open-finished-downloads", 0);
    m.setDefault("open-finished-uploads", 0);
    m.setDefault("open-favorite-users", 0);
    m.setDefault("open-search-spy", 0);

    // Confirmation dialogs
    m.setDefault("confirm-exit", 0);
    m.setDefault("confirm-hub-removal", 0);
    m.setDefault("confirm-user-removal", 0);
    m.setDefault("confirm-hub-closing", 0);
    m.setDefault("confirm-item-removal", 0);
    m.setDefault("confirm-adls-removal", 0);

    // Bold tab indicators
    m.setDefault("bold-pm", 0);
    m.setDefault("bold-queue", 0);
    m.setDefault("bold-hub", 0);
    m.setDefault("bold-search", 0);
    m.setDefault("bold-search-spy", 0);
    m.setDefault("bold-finished-uploads", 0);
    m.setDefault("bold-finished-downloads", 0);

    // Magnet / misc
    m.setDefault("magnet-action", -1);
    m.setDefault("magnet-register", 1);
    m.setDefault("urlhandler", 0);
    m.setDefault("join-open-new-window", 0);
    m.setDefault("always-tray", 0);
    m.setDefault("minimize-tray", 0);
    m.setDefault("popunder-pm", 0);
    m.setDefault("popunder-filelist", 0);
    m.setDefault("use-oen-monofont", 0);
    m.setDefault("clearsearch", 0);
    m.setDefault("use-system-icons", 1);
    m.setDefault("sort-favusers-first", 0);
    m.setDefault("status-in-chat", 1);
    m.setDefault("spyframe-ignore-tth-searches", 0);
    m.setDefault("fav-show-joins", 0);
    m.setDefault("show-joins", 0);
    m.setDefault("show-preferences-on-startup", 1);
    m.setDefault("use-native-back-color-for-text", 1);
    m.setDefault("show-free-space-bar", 1);
    m.setDefault("show-transfers", 1);
    m.setDefault("last-search-type", 0);
    m.setDefault("hub-nickview-left", 0);

    // ────────────────────────── String defaults ──────────────────────────

    // Magnet / download directory
    m.setDefault("magnet-choose-dir", downloadDir);

    // Column order/width/visibility for all views
    m.setDefault("downloadqueue-order", string(""));
    m.setDefault("downloadqueue-width", string(""));
    m.setDefault("downloadqueue-visibility", string(""));
    m.setDefault("favoritehubs-order", string(""));
    m.setDefault("favoritehubs-width", string(""));
    m.setDefault("favoritehubs-visibility", string(""));
    m.setDefault("favoriteusers-order", string(""));
    m.setDefault("favoriteusers-width", string(""));
    m.setDefault("favoriteusers-visibility", string(""));
    m.setDefault("finished-order", string(""));
    m.setDefault("finished-width", string(""));
    m.setDefault("finished-visibility", string(""));
    m.setDefault("hub-order", string(""));
    m.setDefault("hub-width", string(""));
    m.setDefault("hub-visibility", string(""));
    m.setDefault("transfers-order", string(""));
    m.setDefault("transfers-width", string(""));
    m.setDefault("transfers-visibility", string(""));
    m.setDefault("publichubs-order", string(""));
    m.setDefault("publichubs-width", string(""));
    m.setDefault("publichubs-visibility", string(""));
    m.setDefault("search-order", string(""));
    m.setDefault("search-width", string(""));
    m.setDefault("search-visibility", string(""));
    m.setDefault("searchadl-order", string(""));
    m.setDefault("searchadl-width", string(""));
    m.setDefault("searchadl-visibility", string(""));
    m.setDefault("searchspy-order", string(""));
    m.setDefault("searchspy-width", string(""));
    m.setDefault("searchspy-visibility", string(""));
    m.setDefault("sharebrowser-order", string(""));
    m.setDefault("sharebrowser-width", string(""));
    m.setDefault("sharebrowser-visibility", string(""));

    // Charset
    m.setDefault("default-charset", encodingLocale);

    // Sound paths
    m.setDefault("sound-download-finished", string(""));
    m.setDefault("sound-download-finished-ul", string(""));
    m.setDefault("sound-upload-finished", string(""));
    m.setDefault("sound-private-message", string(""));
    m.setDefault("sound-hub-connect", string(""));
    m.setDefault("sound-hub-disconnect", string(""));
    m.setDefault("sound-fuser-join", string(""));
    m.setDefault("sound-fuser-quit", string(""));
    m.setDefault("sound-command", string("aplay -q"));

    // Text colors (fore/back per category)
    m.setDefault("text-general-back-color", string("#FFFFFF"));
    m.setDefault("text-general-fore-color", string("#4D4D4D"));
    m.setDefault("text-myown-back-color", string("#FFFFFF"));
    m.setDefault("text-myown-fore-color", string("#207505"));
    m.setDefault("text-private-back-color", string("#FFFFFF"));
    m.setDefault("text-private-fore-color", string("#2763CE"));
    m.setDefault("text-system-back-color", string("#FFFFFF"));
    m.setDefault("text-system-fore-color", string("#1A1A1A"));
    m.setDefault("text-status-back-color", string("#FFFFFF"));
    m.setDefault("text-status-fore-color", string("#7F7F7F"));
    m.setDefault("text-timestamp-back-color", string("#FFFFFF"));
    m.setDefault("text-timestamp-fore-color", string("#43629A"));
    m.setDefault("text-mynick-back-color", string("#FFFFFF"));
    m.setDefault("text-mynick-fore-color", string("#A52A2A"));
    m.setDefault("text-fav-back-color", string("#FFFFFF"));
    m.setDefault("text-fav-fore-color", string("#FFA500"));
    m.setDefault("text-op-back-color", string("#FFFFFF"));
    m.setDefault("text-op-fore-color", string("#0000FF"));
    m.setDefault("text-url-back-color", string("#FFFFFF"));
    m.setDefault("text-url-fore-color", string("#0000FF"));

    // Search spy colors
    m.setDefault("search-spy-a-color", string("#339900"));
    m.setDefault("search-spy-t-color", string("#ff0000"));
    m.setDefault("search-spy-q-color", string("#b0b0b0"));
    m.setDefault("search-spy-c-color", string("#b28600"));
    m.setDefault("search-spy-r-color", string("#6c85ca"));

    // Emoticons
    m.setDefault("emoticons-pack", string(""));
    m.setDefault("emoticons-icon-size", string("24x24"));

    // Notification titles (untranslated — GTK layer can apply _() at runtime)
    m.setDefault("notify-download-finished-title", string("Download finished"));
    m.setDefault("notify-download-finished-icon", string(""));
    m.setDefault("notify-download-finished-ul-title", string("Download finished file list"));
    m.setDefault("notify-download-finished-ul-icon", string(""));
    m.setDefault("notify-private-message-title", string("Private message"));
    m.setDefault("notify-private-message-icon", string(""));
    m.setDefault("notify-hub-disconnect-title", string("Hub disconnected"));
    m.setDefault("notify-hub-disconnect-icon", string(""));
    m.setDefault("notify-hub-connect-title", string("Hub connected"));
    m.setDefault("notify-hub-connect-icon", string(""));
    m.setDefault("notify-fuser-join-title", string("Favorite user joined"));
    m.setDefault("notify-fuser-join-icon", string(""));
    m.setDefault("notify-fuser-quit-title", string("Favorite user quit"));
    m.setDefault("notify-fuser-quit-icon", string(""));

    // Theme / icons
    m.setDefault("theme-name", string("default"));
    m.setDefault("icon-dc++", string("eiskaltdcpp-dc++"));
    m.setDefault("icon-dc++-fw", string("eiskaltdcpp-dc++-fw"));
    m.setDefault("icon-dc++-fw-op", string("eiskaltdcpp-dc++-fw-op"));
    m.setDefault("icon-dc++-op", string("eiskaltdcpp-dc++-op"));
    m.setDefault("icon-normal", string("eiskaltdcpp-normal"));
    m.setDefault("icon-normal-fw", string("eiskaltdcpp-normal-fw"));
    m.setDefault("icon-normal-fw-op", string("eiskaltdcpp-normal-fw-op"));
    m.setDefault("icon-normal-op", string("eiskaltdcpp-normal-op"));
    m.setDefault("icon-smile", string("face-smile"));
    m.setDefault("icon-download", string("eiskaltdcpp-go-down"));
    m.setDefault("icon-favorite-hubs", string("eiskaltdcpp-favserver"));
    m.setDefault("icon-favorite-users", string("eiskaltdcpp-favusers"));
    m.setDefault("icon-finished-downloads", string("eiskaltdcpp-go-down-search"));
    m.setDefault("icon-finished-uploads", string("eiskaltdcpp-go-up-search"));
    m.setDefault("icon-hash", string("eiskaltdcpp-hashing"));
    m.setDefault("icon-refresh", string("eiskaltdcpp-refrlist"));
    m.setDefault("icon-preferences", string("eiskaltdcpp-configure"));
    m.setDefault("icon-public-hubs", string("eiskaltdcpp-server"));
    m.setDefault("icon-queue", string("eiskaltdcpp-download"));
    m.setDefault("icon-search", string("eiskaltdcpp-edit-find"));
    m.setDefault("icon-search-spy", string("eiskaltdcpp-spy"));
    m.setDefault("icon-search-adl", string("eiskaltdcpp-adls"));
    m.setDefault("icon-upload", string("eiskaltdcpp-go-up"));
    m.setDefault("icon-quit", string("eiskaltdcpp-application-exit"));
    m.setDefault("icon-connect", string("eiskaltdcpp-network-connect"));
    m.setDefault("icon-reconnect", string("eiskaltdcpp-reconnect"));
    // GTK_STOCK_FILE is deprecated; its string value is "gtk-file"
    m.setDefault("icon-file", string("gtk-file"));
    m.setDefault("icon-directory", string("eiskaltdcpp-folder-blue"));
    m.setDefault("icon-openlist", string("eiskaltdcpp-openlist"));
    m.setDefault("icon-own-filelist", string("eiskaltdcpp-own_filelist"));
    m.setDefault("icon-magnet", string("eiskaltdcpp-gui"));
    m.setDefault("icon-pm-online", string("eiskaltdcpp-users"));
    m.setDefault("icon-pm-offline", string("eiskaltdcpp-users-red"));
    m.setDefault("icon-hub-online", string("eiskaltdcpp-server"));
    m.setDefault("icon-hub-offline", string("eiskaltdcpp-server-red"));

    // Aliases / translation
    m.setDefault("custom-aliases", string(""));
    m.setDefault("translation-lang", string(""));
}

} // namespace gtk_settings
