/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2019 EiskaltDC++ developers
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

/*
 * Automatic Directory Listing Search
 * Henrik Engström, henrikengstrom at home se
 */

#include "stdinc.h"

#include "ADLSearch.h"

#include "ClientManager.h"
#include "File.h"
#include "QueueManager.h"
#include "SimpleXML.h"
#include "StringTokenizer.h"

#ifdef USE_PCRE
#include "pcrecpp.h"
#endif

namespace dcpp {
ADLSearch::ADLSearch() :
    searchString(_("<Enter string>")),
    isActive(true),
    isAutoQueue(false),
    sourceType(OnlyFile),
    minFileSize(-1),
    maxFileSize(-1),
    typeFileSize(SizeBytes),
    destDir("ADLSearch"),
    ddIndex(0),
    bUseRegexp(false)
{}

void ADLSearch::prepare(ParamMap& params) {
    // Prepare quick search of substrings
    stringSearchList.clear();
#ifdef USE_PCRE
    if(searchString.find("$Re:") == 0){
        regexpstring.clear();
        regexpstring=searchString.substr(4);
        bUseRegexp = true;
    } else {
#endif
        // Replace parameters such as %[nick]
        string stringParams = Util::formatParams(searchString, params, false);

        // Split into substrings
        StringTokenizer<string> st(stringParams, ' ');
        for(auto &t : st.getTokens()) {
            if(!t.empty()) {
                // Add substring search
                stringSearchList.push_back(StringSearch(t));
            }
        }
#ifdef USE_PCRE
    }
#endif
}

ADLSearch::SourceType ADLSearch::StringToSourceType(const string& s) {
    if(Util::stricmp(s.c_str(), "Filename") == 0) {
        return OnlyFile;
    } else if(Util::stricmp(s.c_str(), "Directory") == 0) {
        return OnlyDirectory;
    } else if(Util::stricmp(s.c_str(), "Full Path") == 0) {
        return FullPath;
    } else {
        return OnlyFile;
    }
}

string ADLSearch::SourceTypeToString(SourceType t) {
    switch(t) {
    default:
    case OnlyFile:      return "Filename";
    case OnlyDirectory: return "Directory";
    case FullPath:      return "Full Path";
    }
}

ADLSearch::SizeType ADLSearch::StringToSizeType(const string& s) {
    if(Util::stricmp(s.c_str(), "B") == 0) {
        return SizeBytes;
    } else if(Util::stricmp(s.c_str(), "KiB") == 0) {
        return SizeKibiBytes;
    } else if(Util::stricmp(s.c_str(), "MiB") == 0) {
        return SizeMebiBytes;
    } else if(Util::stricmp(s.c_str(), "GiB") == 0) {
        return SizeGibiBytes;
    } else {
        return SizeBytes;
    }
}

string ADLSearch::SizeTypeToString(SizeType t) {
    switch(t) {
    default:
    case SizeBytes:     return "B";
    case SizeKibiBytes: return "KiB";
    case SizeMebiBytes: return "MiB";
    case SizeGibiBytes: return "GiB";
    }
}
int64_t ADLSearch::GetSizeBase() {
    switch(typeFileSize) {
    default:
    case SizeBytes:     return (int64_t)1;
    case SizeKibiBytes: return (int64_t)1024;
    case SizeMebiBytes: return (int64_t)1024 * (int64_t)1024;
    case SizeGibiBytes: return (int64_t)1024 * (int64_t)1024 * (int64_t)1024;
    }
}

bool ADLSearch::matchesFile(const string& f, const string& fp, int64_t size) {
    // Check status
    if(!isActive) {
        return false;
    }

    // Check size for files
    if(size >= 0 && (sourceType == OnlyFile || sourceType == FullPath)) {
        if(minFileSize >= 0 && size < minFileSize * GetSizeBase()) {
            // Too small
            return false;
        }
        if(maxFileSize >= 0 && size > maxFileSize * GetSizeBase()) {
            // Too large
            return false;
        }
    }

    // Do search
    switch(sourceType) {
    default:
    case OnlyDirectory: return false;
    case OnlyFile:      return searchAll(f);
    case FullPath:      return searchAll(fp);
    }
}

bool ADLSearch::matchesDirectory(const string& d) {
    // Check status
    if(!isActive) {
        return false;
    }
    if(sourceType != OnlyDirectory) {
        return false;
    }

    // Do search
    return searchAll(d);
}

bool ADLSearch::searchAll(const string& s) {
#ifdef USE_PCRE
    if(bUseRegexp){
        pcrecpp::RE_Options options;
        options.set_utf8(true);
        options.set_caseless(true);
        pcrecpp::RE regexp(regexpstring, options);
        if(regexp.FullMatch(s))
            return true;
        else
            return false;
    } else {
#endif
        // Match all substrings
        for(auto& i : stringSearchList) {
            if(!i.match(s)) {
                return false;
            }
        }
        return !stringSearchList.empty();
#ifdef USE_PCRE
    }
#endif
}

ADLSearchManager::ADLSearchManager() : breakOnFirst(false), user(UserPtr(), Util::emptyString) {
    load();
}

ADLSearchManager::~ADLSearchManager() {
    save();
}

void ADLSearchManager::load() {
    // Clear current
    collection.clear();

    // Load file as a string
    try {
        SimpleXML xml;
        Util::migrate(getConfigFile());
        xml.fromXML(File(getConfigFile(), File::READ, File::OPEN).read());

        if(xml.findChild("ADLSearch")) {
            xml.stepIn();

            // Predicted several groups of searches to be differentiated
            // in multiple categories. Not implemented yet.
            if(xml.findChild("SearchGroup")) {
                xml.stepIn();

                // Loop until no more searches found
                while(xml.findChild("Search")) {
                    xml.stepIn();

                    // Found another search, load it
                    ADLSearch search;

                    if(xml.findChild("SearchString")) {
                        search.searchString = xml.getChildData();
                    }
                    if(xml.findChild("SourceType")) {
                        search.sourceType = search.StringToSourceType(xml.getChildData());
                    }
                    if(xml.findChild("DestDirectory")) {
                        search.destDir = xml.getChildData();
                    }
                    if(xml.findChild("IsActive")) {
                        search.isActive = (Util::toInt(xml.getChildData()) != 0);
                    }
                    if(xml.findChild("MaxSize")) {
                        search.maxFileSize = Util::toInt64(xml.getChildData());
                    }
                    if(xml.findChild("MinSize")) {
                        search.minFileSize = Util::toInt64(xml.getChildData());
                    }
                    if(xml.findChild("SizeType")) {
                        search.typeFileSize = search.StringToSizeType(xml.getChildData());
                    }
                    if(xml.findChild("IsAutoQueue")) {
                        search.isAutoQueue = (Util::toInt(xml.getChildData()) != 0);
                    }

                    // Add search to collection
                    if(!search.searchString.empty()) {
                        collection.push_back(search);
                    }

                    // Go to next search
                    xml.stepOut();
                }
            }
        }
    }
    catch(const SimpleXMLException&) { }
    catch(const FileException&) { }
}

void ADLSearchManager::save() {
    // Prepare xml string for saving
    try {
        SimpleXML xml;

        xml.addTag("ADLSearch");
        xml.stepIn();

        // Predicted several groups of searches to be differentiated
        // in multiple categories. Not implemented yet.
        xml.addTag("SearchGroup");
        xml.stepIn();

        // Save all searches
        for(auto& search: collection) {
            if(search.searchString.empty()) {
                continue;
            }
            xml.addTag("Search");
            xml.stepIn();

            xml.addTag("SearchString", search.searchString);
            xml.addTag("SourceType", search.SourceTypeToString(search.sourceType));
            xml.addTag("DestDirectory", search.destDir);
            xml.addTag("IsActive", search.isActive);
            xml.addTag("MaxSize", search.maxFileSize);
            xml.addTag("MinSize", search.minFileSize);
            xml.addTag("SizeType", search.SizeTypeToString(search.typeFileSize));
            xml.addTag("IsAutoQueue", search.isAutoQueue);
            xml.stepOut();
        }

        xml.stepOut();

        xml.stepOut();

        // Save string to file
        try {
            File fout(getConfigFile(), File::WRITE, File::CREATE | File::TRUNCATE);
            fout.write(SimpleXML::utf8Header);
            fout.write(xml.toXML());
            fout.close();
        } catch(const FileException&) { }
    } catch(const SimpleXMLException&) { }
}

void ADLSearchManager::matchesFile(DestDirList& destDirVector, DirectoryListing::File *currentFile, string& fullPath) {
    // Add to any substructure being stored
    for(auto& id: destDirVector) {
        if(id.subdir != NULL) {
            DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile, true);
            dcassert(id.subdir->getAdls());

            id.subdir->files.insert(copyFile);
        }
        id.fileAdded = false;  // Prepare for next stage
    }

    // Prepare to match searches
    if(currentFile->getName().empty()) {
        return;
    }

    string filePath = fullPath + "\\" + currentFile->getName();
    // Match searches
    for(auto& is: collection) {
        if(destDirVector[is.ddIndex].fileAdded) {
            continue;
        }
        if(is.matchesFile(currentFile->getName(), filePath, currentFile->getSize())) {
            DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile, true);
            destDirVector[is.ddIndex].dir->files.insert(copyFile);
            destDirVector[is.ddIndex].fileAdded = true;

            if(is.isAutoQueue){
                try {
                    QueueManager::getInstance()->add(SETTING(DOWNLOAD_DIRECTORY) + currentFile->getName(),
                                                     currentFile->getSize(), currentFile->getTTH(), getUser());
                } catch(const Exception&) { }
            }

            if(breakOnFirst) {
                // Found a match, search no more
                break;
            }
        }
    }
}

void ADLSearchManager::matchesDirectory(DestDirList& destDirVector, DirectoryListing::Directory* currentDir, string& fullPath) {
    // Add to any substructure being stored
    for(auto& id: destDirVector) {
        if(id.subdir != NULL) {
            DirectoryListing::Directory* newDir =
                    new DirectoryListing::AdlDirectory(fullPath, id.subdir, currentDir->getName());
            id.subdir->directories.insert(newDir);
            id.subdir = newDir;
        }
    }

    // Prepare to match searches
    if(currentDir->getName().empty()) {
        return;
    }

    // Match searches
    for(auto& is: collection) {
        if(destDirVector[is.ddIndex].subdir != NULL) {
            continue;
        }
        if(is.matchesDirectory(currentDir->getName())) {
            destDirVector[is.ddIndex].subdir =
                    new DirectoryListing::AdlDirectory(fullPath, destDirVector[is.ddIndex].dir, currentDir->getName());
            destDirVector[is.ddIndex].dir->directories.insert(destDirVector[is.ddIndex].subdir);
            if(breakOnFirst) {
                // Found a match, search no more
                break;
            }
        }
    }
}

void ADLSearchManager::stepUpDirectory(DestDirList& destDirVector) {
    for(auto& id: destDirVector) {
        if(id.subdir != NULL) {
            id.subdir = id.subdir->getParent();
            if(id.subdir == id.dir) {
                id.subdir = NULL;
            }
        }
    }
}

void ADLSearchManager::prepareDestinationDirectories(DestDirList& destDirs, DirectoryListing::Directory* root, ParamMap& params) {
    // Load default destination directory (index = 0)
    destDirs.clear();
    DestDir dir = { "ADLSearch", new DirectoryListing::Directory(root, "<<<ADLSearch>>>", true, true), nullptr, false };
    destDirs.push_back(std::move(dir));

    // Scan all loaded searches
    for(auto& is: collection) {
        // Check empty destination directory
        if(is.destDir.empty()) {
            // Set to default
            is.ddIndex = 0;
            continue;
        }

        // Check if exists
        bool isNew = true;
        long ddIndex = 0;
        for(auto id = destDirs.cbegin(); id != destDirs.cend(); ++id, ++ddIndex) {
            if(Util::stricmp(is.destDir.c_str(), id->name.c_str()) == 0) {
                // Already exists, reuse index
                is.ddIndex = ddIndex;
                isNew = false;
                break;
            }
        }

        if(isNew) {
            // Add new destination directory
            DestDir dir = { is.destDir, new DirectoryListing::Directory(root, "<<<" + is.destDir + ">>>", true, true), nullptr, false };
            destDirs.push_back(std::move(dir));
            is.ddIndex = ddIndex;
        }
    }
    // Prepare all searches
    for(auto& ip: collection) {
        ip.prepare(params);
    }
}

void ADLSearchManager::finalizeDestinationDirectories(DestDirList& destDirs, DirectoryListing::Directory* root) {
    string szDiscard("<<<" + string(_("Discard")) + ">>>");

    // Add non-empty destination directories to the top level
    for(auto& i: destDirs) {
        if(i.dir->files.empty() && i.dir->directories.empty()) {
            delete i.dir;
        } else if(Util::stricmp(i.dir->getName(), szDiscard) == 0) {
            delete i.dir;
        } else {
            root->directories.insert(i.dir);
        }
    }
}

void ADLSearchManager::matchListing(DirectoryListing& aDirList) {
    StringMap params;
    params["userNI"] = ClientManager::getInstance()->getNicks(aDirList.getUser())[0];
    params["userCID"] = aDirList.getUser().user->getCID().toBase32();

    if (BOOLSETTING(USE_ADL_ONLY_OWN_LIST) && params["userCID"] != ClientManager::getInstance()->getMe()->getCID().toBase32())
        return;

    setUser(aDirList.getUser());

    const auto root = aDirList.getRoot();

    DestDirList destDirs;
    prepareDestinationDirectories(destDirs, root, params);
    setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));

    string path(root->getName());
    matchRecurse(destDirs, root, path);

    finalizeDestinationDirectories(destDirs, root);
}

void ADLSearchManager::matchRecurse(DestDirList &aDestList, DirectoryListing::Directory* aDir, string &aPath) {
    for(auto& dirIt: aDir->directories) {
        string tmpPath = aPath + "\\" + dirIt->getName();
        matchesDirectory(aDestList, dirIt, tmpPath);
        matchRecurse(aDestList, dirIt, tmpPath);
    }

    for(auto& fileIt: aDir->files) {
        matchesFile(aDestList, fileIt, aPath);
    }

    stepUpDirectory(aDestList);
}

string ADLSearchManager::getConfigFile() {
    return Util::getPath(Util::PATH_USER_CONFIG) + "ADLSearch.xml";
}

} // namespace dcpp
