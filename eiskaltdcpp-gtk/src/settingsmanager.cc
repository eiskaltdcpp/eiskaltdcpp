/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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

#include "settingsmanager.hh"
#include "GtkSettingsDefaults.h"
#include <dcpp/SettingsManager.h>
#include <dcpp/Util.h>
#include <dcpp/StringTokenizer.h>
#include "WulforUtil.hh"
#include <glib/gi18n.h>
#include <glib/gstdio.h>

using namespace std;
using namespace dcpp;

// ── Module-level active instance pointer (set/cleared in wulfor.cc) ──
static WulforSettingsManager *s_settingsInstance = nullptr;

WulforSettingsManager *wulforSettingsInstance()
{
    dcassert(s_settingsInstance);
    return s_settingsInstance;
}

void setWulforSettingsInstance(WulforSettingsManager *instance)
{
    s_settingsInstance = instance;
}

// ── Constructor / Destructor ──

WulforSettingsManager::WulforSettingsManager():
    configFile(Util::getPath(Util::PATH_USER_CONFIG) + "EiskaltDC++_Gtk.xml")
{
    // Populate all defaults via the widget-independent function.
    // Pass GTK-specific values for the few defaults that depend on them.
    gtk_settings::populateDefaults(model_,
                                    dcpp::getContext()->getSettingsManager()->get(SettingsManager::DOWNLOAD_DIRECTORY, true),
                                    WulforUtil::ENCODING_LOCALE);

    // Override notification titles with translated strings
    model_.set("notify-download-finished-title", string(_("Download finished")));
    model_.set("notify-download-finished-ul-title", string(_("Download finished file list")));
    model_.set("notify-private-message-title", string(_("Private message")));
    model_.set("notify-hub-disconnect-title", string(_("Hub disconnected")));
    model_.set("notify-hub-connect-title", string(_("Hub connected")));
    model_.set("notify-fuser-join-title", string(_("Favorite user joined")));
    model_.set("notify-fuser-quit-title", string(_("Favorite user quit")));

    load();

    string path_image = Util::getPath(Util::PATH_USER_LOCAL) + "Images/";
    g_mkdir_with_parents(path_image.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
}

WulforSettingsManager::~WulforSettingsManager()
{
    save();
    freePreviewApps();
}

// ── Delegated get/set ──

int WulforSettingsManager::getInt(const string &key, bool useDefault)
{
    return model_.getInt(key, useDefault);
}

string WulforSettingsManager::getString(const string &key, bool useDefault)
{
    return model_.getString(key, useDefault);
}

bool WulforSettingsManager::getBool(const string &key, bool useDefault)
{
    return model_.getBool(key, useDefault);
}

void WulforSettingsManager::set(const string &key, int value)
{
    model_.set(key, value);
}

void WulforSettingsManager::set(const string &key, bool value)
{
    model_.set(key, value);
}

void WulforSettingsManager::set(const string &key, const string &value)
{
    model_.set(key, value);
}

// ── XML I/O (delegates to model) ──

void WulforSettingsManager::load()
{
    model_.loadFromXml(configFile);
    syncPreviewAppsFromModel();
}

void WulforSettingsManager::save()
{
    model_.saveToXml(configFile);
}

// ── Preview app management ──

PreviewApp* WulforSettingsManager::addPreviewApp(string name, string app, string ext)
{
    model_.addPreviewApp(name, app, ext);

    PreviewApp* pa = new PreviewApp(name, app, ext);
    previewApps.push_back(pa);
    return pa;
}

bool WulforSettingsManager::removePreviewApp(string &name)
{
    PreviewApp::size index;

    if (getPreviewApp(name, index))
    {
        delete previewApps[index];
        previewApps.erase(previewApps.begin() + index);
        model_.removePreviewApp(name);
        return true;
    }

    return false;
}

PreviewApp* WulforSettingsManager::applyPreviewApp(string &oldName, string &newName, string &app, string &ext)
{
    PreviewApp::size index;
    PreviewApp *pa = NULL;

    if(getPreviewApp(oldName, index))
    {
        delete previewApps[index];
        pa = new PreviewApp(newName, app, ext);
        previewApps[index] = pa;
        model_.updatePreviewApp(oldName, newName, app, ext);
    }

    return pa;
}

bool WulforSettingsManager::getPreviewApp(string &name)
{
    PreviewApp::size index;
    return getPreviewApp(name, index);
}

bool WulforSettingsManager::getPreviewApp(string &name, PreviewApp::size &index)
{
    index = 0;

    for (PreviewApp::Iter item = previewApps.begin(); item != previewApps.end(); ++item, ++index)
        if((*item)->name == name) return true;

    return false;
}

// ── Parse command ──

const std::string WulforSettingsManager::parseCmd(const std::string &cmd)
{
    StringTokenizer<string> sl(cmd, ' ');
    if (sl.getTokens().size() == 1 || sl.getTokens().size() == 2) {
        string tmp;
        if (model_.hasDefault(sl.getTokens().at(0))) {
            // Check if it's an int key
            auto intKeys = model_.intKeys();
            bool isInt = false;
            for (const auto &k : intKeys) {
                if (k == sl.getTokens().at(0)) { isInt = true; break; }
            }

            if (isInt) {
                if (sl.getTokens().size() == 2) {
                    int i = atoi(sl.getTokens().at(1).c_str());
                    WSET(sl.getTokens().at(0), i);
                } else {
                    tmp = Util::toString(WGETI(sl.getTokens().at(0)));
                }
            } else {
                if (sl.getTokens().size() == 2)
                    WSET(sl.getTokens().at(0), sl.getTokens().at(1));
                else
                    tmp = WGETS(sl.getTokens().at(0));
            }
        } else {
            return _("Error: setting not found!");
        }
        return !tmp.empty() ?
                    _("Gui setting ") + string(sl.getTokens().at(0)) + ": " + tmp :
                    _("Change gui setting ") + string(sl.getTokens().at(0)) + _(" to ") + string(sl.getTokens().at(1));
    }
    return _("Error: segv parser :D");
}

// ── Internal helpers ──

void WulforSettingsManager::syncPreviewAppsFromModel()
{
    freePreviewApps();
    for (const auto &app : model_.previewApps()) {
        previewApps.push_back(new PreviewApp(app.name, app.application, app.extension));
    }
}

void WulforSettingsManager::freePreviewApps()
{
    for (auto *pa : previewApps)
        delete pa;
    previewApps.clear();
}
