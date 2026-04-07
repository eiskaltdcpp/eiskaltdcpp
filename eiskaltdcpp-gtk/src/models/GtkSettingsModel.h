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

#include <map>
#include <string>
#include <vector>

/**
 * Widget-independent settings key-value store.
 *
 * Mirrors the logic of WulforSettingsManager (settingsmanager.cc) without
 * GTK/Pango dependencies. Provides get/set for int and string settings
 * with defaults, preview application management, and XML serialization.
 *
 * Text weight and style constants are defined as plain ints rather than
 * Pango macros so tests can run without GTK.
 *
 * No GTK dependency.
 */
namespace gtk_settings {

/// Text weight constants (matching Pango values without the Pango dep).
constexpr int TEXT_WEIGHT_NORMAL = 400;
constexpr int TEXT_WEIGHT_BOLD   = 700;

/// Text style constants.
constexpr int TEXT_STYLE_NORMAL  = 0;
constexpr int TEXT_STYLE_ITALIC  = 2;

/// A preview application entry.
struct PreviewApp {
    std::string name;
    std::string application;
    std::string extension;
};

class GtkSettingsModel {
public:
    // ── Int settings ──

    /// Define a default int value for a key.
    void setDefault(const std::string &key, int value);

    /// Set an int value.
    void set(const std::string &key, int value);

    /// Get an int value. If useDefault is true, always returns the default.
    /// Otherwise returns the explicitly-set value, falling back to default.
    int getInt(const std::string &key, bool useDefault = false) const;

    /// Get as bool (int != 0). If useDefault is true, returns default value.
    bool getBool(const std::string &key, bool useDefault = false) const;

    // ── String settings ──

    /// Define a default string value for a key.
    void setDefault(const std::string &key, const std::string &value);

    /// Set a string value.
    void set(const std::string &key, const std::string &value);

    /// Get a string value. If useDefault is true, always returns the default.
    /// Otherwise returns the explicitly-set value, falling back to default.
    std::string getString(const std::string &key, bool useDefault = false) const;

    // ── Defaults management ──

    /// Check if a key has a default (int or string).
    bool hasDefault(const std::string &key) const;

    /// Get all default int keys.
    std::vector<std::string> intKeys() const;

    /// Get all default string keys.
    std::vector<std::string> stringKeys() const;

    // ── Preview apps ──

    /// Add a preview application.
    void addPreviewApp(const std::string &name,
                       const std::string &application,
                       const std::string &extension);

    /// Remove a preview application by name. Returns true if found and removed.
    bool removePreviewApp(const std::string &name);

    /// Update an existing preview app. Returns true if found.
    bool updatePreviewApp(const std::string &oldName,
                          const std::string &newName,
                          const std::string &application,
                          const std::string &extension);

    /// Find a preview app by name. Returns nullptr if not found.
    const PreviewApp *findPreviewApp(const std::string &name) const;

    /// All preview apps.
    const std::vector<PreviewApp> &previewApps() const { return previewApps_; }

    /// Number of preview apps.
    size_t previewAppCount() const { return previewApps_.size(); }

    // ── XML serialization ──

    /// Load settings and preview apps from an XML file (EiskaltDC++_Gtk.xml format).
    /// Only loads keys that have defaults defined. Preview apps are appended.
    /// Returns true on success, false on file/parse error (model is unchanged on error).
    bool loadFromXml(const std::string &path);

    /// Save all explicitly-set settings and preview apps to an XML file.
    /// Uses atomic write (write to .tmp, then rename).
    /// Returns true on success, false on file error.
    bool saveToXml(const std::string &path) const;

    // ── Serialization support (parse command) ──

    /// Parse a "key [value]" command string.
    /// With value: sets the setting. Without value: returns current as string.
    /// Returns the value as string (whether retrieved or set).
    std::string parseCmd(const std::string &cmd);

    // ── Bulk operations ──

    /// Clear all explicitly-set values (defaults and preview apps are kept).
    void clearOverrides();

    /// Clear everything (defaults, overrides, preview apps).
    void clear();

private:
    std::map<std::string, int> intMap_;
    std::map<std::string, std::string> stringMap_;
    std::map<std::string, int> defaultInt_;
    std::map<std::string, std::string> defaultString_;
    std::vector<PreviewApp> previewApps_;
};

} // namespace gtk_settings
