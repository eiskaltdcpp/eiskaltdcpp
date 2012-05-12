/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
 */

/*
 * Automatic Directory Listing Search
 * Henrik Engstr�m, henrikengstrom at home se
 */

#pragma once

#include "Util.h"

#include "SettingsManager.h"

#include "StringSearch.h"
#include "Singleton.h"
#include "DirectoryListing.h"

namespace dcpp {
class AdlSearchManager;

///  Class that represent an ADL search
class ADLSearch
{
public:

    // Constructor
    ADLSearch();
    // Prepare search
    void Prepare(StringMap& params);
    // The search string
    string searchString;
    // Active search
    bool isActive;
    // Auto Queue Results
    bool isAutoQueue;
    // Search source type
    enum SourceType {
        TypeFirst = 0,
        OnlyFile = TypeFirst,
        OnlyDirectory,
        FullPath,
        TypeLast
    } sourceType;

    SourceType StringToSourceType(const string& s);
    string SourceTypeToString(SourceType t);

    // Maximum & minimum file sizes (in bytes).
    // Negative values means do not check.
    int64_t minFileSize;
    int64_t maxFileSize;

    enum SizeType {
        SizeBytes   = TypeFirst,
        SizeKibiBytes,
        SizeMebiBytes,
        SizeGibiBytes
    };

    SizeType typeFileSize;
    SizeType StringToSizeType(const string& s);
    string SizeTypeToString(SizeType t);
    int64_t GetSizeBase();

    // Name of the destination directory (empty = 'ADLSearch') and its index
    string destDir;
    unsigned long ddIndex;
    // Search for file match
    bool MatchesFile(const string& f, const string& fp, int64_t size);
    // Search for directory match
    bool MatchesDirectory(const string& d);

private:
    friend class ADLSearchManager;
    //decide if regexp should be used
    bool bUseRegexp;
    string regexpstring;
    // Substring searches
    StringSearch::List stringSearchList;
    bool SearchAll(const string& s);
};

///  Class that holds all active searches
class ADLSearchManager : public Singleton<ADLSearchManager>
{
public:
    // Destination directory indexing
    struct DestDir {
        string name;
        DirectoryListing::Directory* dir;
        DirectoryListing::Directory* subdir;
        bool fileAdded;
        DestDir() : name(""), dir(NULL), subdir(NULL) {}
    };
    typedef vector<DestDir> DestDirList;

    // Constructor/destructor
    ADLSearchManager() : user(UserPtr(), Util::emptyString) { Load(); }
    virtual ~ADLSearchManager() { Save(); }

    // Search collection
    typedef vector<ADLSearch> SearchCollection;
    SearchCollection collection;

    // Load/save search collection to XML file
    void Load();
    void Save();

    // Settings
    GETSET(bool, breakOnFirst, BreakOnFirst)
    GETSET(HintedUser, user, User)

    // @remarks Used to add ADLSearch directories to an existing DirectoryListing
    void matchListing(DirectoryListing& /*aDirList*/) noexcept;

private:
    // @internal
    void matchRecurse(DestDirList& /*aDestList*/, DirectoryListing::Directory* /*aDir*/, string& /*aPath*/);
    // Search for file match
    void MatchesFile(DestDirList& destDirVector, DirectoryListing::File *currentFile, string& fullPath);
    // Search for directory match
    void MatchesDirectory(DestDirList& destDirVector, DirectoryListing::Directory* currentDir, string& fullPath);
    // Step up directory
    void StepUpDirectory(DestDirList& destDirVector);
    // Prepare destination directory indexing
    void PrepareDestinationDirectories(DestDirList& destDirVector, DirectoryListing::Directory* root, StringMap& params);
    // Finalize destination directories
    void FinalizeDestinationDirectories(DestDirList& destDirVector, DirectoryListing::Directory* root);

    string getConfigFile();
};

} // namespace dcpp
