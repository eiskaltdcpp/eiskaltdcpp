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
#include <dcpp/File.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/Util.h>

#include "GtkSettingsModel.h"

#include <algorithm>
#include <sstream>

using std::string;
using std::map;
using std::vector;

namespace gtk_settings {

// ── Int settings ──

void GtkSettingsModel::setDefault(const string &key, int value)
{
    defaultInt_[key] = value;
}

void GtkSettingsModel::set(const string &key, int value)
{
    intMap_[key] = value;
}

int GtkSettingsModel::getInt(const string &key, bool useDefault) const
{
    if (useDefault) {
        auto dit = defaultInt_.find(key);
        return (dit != defaultInt_.end()) ? dit->second : 0;
    }

    auto it = intMap_.find(key);
    if (it != intMap_.end())
        return it->second;
    auto dit = defaultInt_.find(key);
    if (dit != defaultInt_.end())
        return dit->second;
    return 0;
}

bool GtkSettingsModel::getBool(const string &key, bool useDefault) const
{
    return getInt(key, useDefault) != 0;
}

// ── String settings ──

void GtkSettingsModel::setDefault(const string &key, const string &value)
{
    defaultString_[key] = value;
}

void GtkSettingsModel::set(const string &key, const string &value)
{
    stringMap_[key] = value;
}

string GtkSettingsModel::getString(const string &key, bool useDefault) const
{
    if (useDefault) {
        auto dit = defaultString_.find(key);
        return (dit != defaultString_.end()) ? dit->second : string();
    }

    auto it = stringMap_.find(key);
    if (it != stringMap_.end())
        return it->second;
    auto dit = defaultString_.find(key);
    if (dit != defaultString_.end())
        return dit->second;
    return string();
}

// ── Defaults management ──

bool GtkSettingsModel::hasDefault(const string &key) const
{
    return defaultInt_.count(key) > 0 || defaultString_.count(key) > 0;
}

vector<string> GtkSettingsModel::intKeys() const
{
    vector<string> keys;
    keys.reserve(defaultInt_.size());
    for (const auto &p : defaultInt_)
        keys.push_back(p.first);
    return keys;
}

vector<string> GtkSettingsModel::stringKeys() const
{
    vector<string> keys;
    keys.reserve(defaultString_.size());
    for (const auto &p : defaultString_)
        keys.push_back(p.first);
    return keys;
}

// ── Preview apps ──

void GtkSettingsModel::addPreviewApp(const string &name,
                                      const string &application,
                                      const string &extension)
{
    PreviewApp app;
    app.name = name;
    app.application = application;
    app.extension = extension;
    previewApps_.push_back(std::move(app));
}

bool GtkSettingsModel::removePreviewApp(const string &name)
{
    auto it = std::remove_if(previewApps_.begin(), previewApps_.end(),
        [&name](const PreviewApp &a) { return a.name == name; });
    if (it == previewApps_.end())
        return false;
    previewApps_.erase(it, previewApps_.end());
    return true;
}

bool GtkSettingsModel::updatePreviewApp(const string &oldName,
                                         const string &newName,
                                         const string &application,
                                         const string &extension)
{
    for (auto &app : previewApps_) {
        if (app.name == oldName) {
            app.name = newName;
            app.application = application;
            app.extension = extension;
            return true;
        }
    }
    return false;
}

const PreviewApp *GtkSettingsModel::findPreviewApp(const string &name) const
{
    for (const auto &app : previewApps_) {
        if (app.name == name)
            return &app;
    }
    return nullptr;
}

// ── XML serialization ──

bool GtkSettingsModel::loadFromXml(const string &path)
{
    try
    {
        dcpp::SimpleXML xml;
        xml.fromXML(dcpp::File(path, dcpp::File::READ, dcpp::File::OPEN).read());
        xml.resetCurrentChild();
        xml.stepIn();

        if (xml.findChild("Settings"))
        {
            xml.stepIn();

            for (auto iit = defaultInt_.begin(); iit != defaultInt_.end(); ++iit)
            {
                if (xml.findChild(iit->first))
                    intMap_.insert(map<string, int>::value_type(
                        iit->first, dcpp::Util::toInt(xml.getChildData())));
                xml.resetCurrentChild();
            }

            for (auto sit = defaultString_.begin(); sit != defaultString_.end(); ++sit)
            {
                if (xml.findChild(sit->first))
                    stringMap_.insert(map<string, string>::value_type(
                        sit->first, xml.getChildData()));
                xml.resetCurrentChild();
            }

            xml.stepOut();
        }

        if (xml.findChild("PreviewApps"))
        {
            xml.stepIn();

            for (; xml.findChild("Application");)
                addPreviewApp(xml.getChildAttrib("Name"),
                              xml.getChildAttrib("Application"),
                              xml.getChildAttrib("Extension"));

            xml.stepOut();
        }

        return true;
    }
    catch (const dcpp::Exception &)
    {
        return false;
    }
}

bool GtkSettingsModel::saveToXml(const string &path) const
{
    dcpp::SimpleXML xml;
    xml.addTag("EiskaltDC++_Gtk");
    xml.stepIn();
    xml.addTag("Settings");
    xml.stepIn();

    for (auto iit = intMap_.begin(); iit != intMap_.end(); ++iit)
    {
        xml.addTag(iit->first, iit->second);
        xml.addChildAttrib(string("type"), string("int"));
    }

    for (auto sit = stringMap_.begin(); sit != stringMap_.end(); ++sit)
    {
        xml.addTag(sit->first, sit->second);
        xml.addChildAttrib(string("type"), string("string"));
    }

    xml.stepOut();

    xml.addTag("PreviewApps");
    xml.stepIn();

    for (const auto &app : previewApps_)
    {
        xml.addTag("Application");
        xml.addChildAttrib("Name", app.name);
        xml.addChildAttrib("Application", app.application);
        xml.addChildAttrib("Extension", app.extension);
    }

    xml.stepOut();

    try
    {
        dcpp::File out(path + ".tmp", dcpp::File::WRITE,
                       dcpp::File::CREATE | dcpp::File::TRUNCATE);
        dcpp::BufferedOutputStream<false> f(&out);
        f.write(dcpp::SimpleXML::utf8Header);
        xml.toXML(&f);
        f.flush();
        out.close();
        dcpp::File::deleteFile(path);
        dcpp::File::renameFile(path + ".tmp", path);
        return true;
    }
    catch (const dcpp::FileException &)
    {
        return false;
    }
}

// ── Parse command ──

string GtkSettingsModel::parseCmd(const string &cmd)
{
    // Split: first word is key, rest is value
    string::size_type sp = cmd.find(' ');
    string key = (sp == string::npos) ? cmd : cmd.substr(0, sp);

    if (sp == string::npos) {
        // Get mode — return current value
        if (defaultInt_.count(key))
            return dcpp::Util::toString(getInt(key));
        if (defaultString_.count(key))
            return getString(key);
        return "Unknown setting: " + key;
    }

    // Set mode
    string value = cmd.substr(sp + 1);
    if (defaultInt_.count(key)) {
        set(key, dcpp::Util::toInt(value));
        return value;
    }
    if (defaultString_.count(key)) {
        set(key, value);
        return value;
    }
    return "Unknown setting: " + key;
}

// ── Bulk operations ──

void GtkSettingsModel::clearOverrides()
{
    intMap_.clear();
    stringMap_.clear();
}

void GtkSettingsModel::clear()
{
    intMap_.clear();
    stringMap_.clear();
    defaultInt_.clear();
    defaultString_.clear();
    previewApps_.clear();
}

} // namespace gtk_settings
