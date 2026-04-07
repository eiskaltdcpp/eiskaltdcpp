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

#pragma once

#include <string>

namespace gtk_settings {
class GtkSettingsModel;
}

/**
 * Populate a GtkSettingsModel with all default GTK frontend settings.
 *
 * Extracted from WulforSettingsManager's constructor so the defaults
 * can be tested without GTK/Pango dependencies.
 *
 * @param model            The model to populate with defaults.
 * @param downloadDir      Value for "magnet-choose-dir" default
 *                         (usually SETTING(DOWNLOAD_DIRECTORY)).
 * @param encodingLocale   Value for "default-charset" default
 *                         (usually WulforUtil::ENCODING_LOCALE).
 *
 * For GTK-dependent values:
 *  - GTK_STOCK_FILE is replaced by the string "gtk-file"
 *  - Notification title defaults use untranslated English strings
 *    (the GTK layer can override with _() at runtime)
 */
namespace gtk_settings {

void populateDefaults(GtkSettingsModel &model,
                      const std::string &downloadDir = "",
                      const std::string &encodingLocale = "System default");

/// Number of int defaults (for test verification).
constexpr int EXPECTED_INT_DEFAULT_COUNT = 122;

/// Number of string defaults (for test verification).
constexpr int EXPECTED_STRING_DEFAULT_COUNT = 123;

} // namespace gtk_settings
