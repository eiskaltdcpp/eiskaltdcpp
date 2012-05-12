/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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

#include <string>
#include <map>
#include <dcpp/stdinc.h>
#include <dcpp/Singleton.h>

#define WSET(key, value) WulforSettingsManager::getInstance()->set(key, value)
#define WGETI(key) WulforSettingsManager::getInstance()->getInt(key)
#define WGETS(key) WulforSettingsManager::getInstance()->getString(key)
#define WGETB(key) WulforSettingsManager::getInstance()->getBool(key)
#define WSCMD(cmd) WulforSettingsManager::getInstance()->parseCmd(cmd);

/* default font theme */
#define TEXT_WEIGHT_NORMAL PANGO_WEIGHT_NORMAL
#define TEXT_WEIGHT_BOLD   PANGO_WEIGHT_BOLD
#define TEXT_STYLE_NORMAL  PANGO_STYLE_NORMAL
#define TEXT_STYLE_ITALIC  PANGO_STYLE_ITALIC

class PreviewApp
{
    public:

    typedef std::vector<PreviewApp*> List;
    typedef List::size_type size;
    typedef List::const_iterator Iter;

    PreviewApp(std::string name, std::string app, std::string ext) : name(name), app(app), ext(ext) {}
    ~PreviewApp() {}

    std::string name;
    std::string app;
    std::string ext;
};

class WulforSettingsManager : public dcpp::Singleton<WulforSettingsManager>
{
    public:
        WulforSettingsManager();
        virtual ~WulforSettingsManager();

        int getInt(const std::string &key, bool useDefault = false);
        bool getBool(const std::string &key, bool useDefault = false);
        std::string getString(const std::string &key, bool useDefault = false);
        void set(const std::string &key, int value);
        void set(const std::string &key, bool value);
        void set(const std::string &key, const std::string &value);
        const std::string parseCmd(const std::string cmd);
        void load();
        void save();

        PreviewApp* applyPreviewApp(std::string &oldName, std::string &newName, std::string &app, std::string &ext);
        PreviewApp* addPreviewApp(std::string name, std::string app, std::string ext);
        bool getPreviewApp(std::string &name, PreviewApp::size &index);
        bool getPreviewApp(std::string &name);
        bool removePreviewApp(std::string &name);

        const PreviewApp::List& getPreviewApps() const {return previewApps;}

    private:
        typedef std::map<std::string, int> IntMap;
        typedef std::map<std::string, std::string> StringMap;

        IntMap intMap;
        StringMap stringMap;
        IntMap defaultInt;
        StringMap defaultString;
        std::string configFile;

        PreviewApp::List previewApps;
};
