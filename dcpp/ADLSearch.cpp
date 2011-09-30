/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
 * Henrik Engström, henrikengstrom at home se
 */

#include "stdinc.h"

#include "ADLSearch.h"
#include "QueueManager.h"
#include "ClientManager.h"

#include "File.h"
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

void ADLSearch::Prepare(StringMap& params) {
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
        for(StringList::iterator i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
            if(i->size() > 0) {
                // Add substring search
                stringSearchList.push_back(StringSearch(*i));
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
string ADLSearch::SourceTypeToString(ADLSearch::SourceType t) {
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

string ADLSearch::SizeTypeToString(ADLSearch::SizeType t) {
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

bool ADLSearch::MatchesFile(const string& f, const string& fp, int64_t size) {
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
    case OnlyFile:      return SearchAll(f);
    case FullPath:      return SearchAll(fp);
    }
}
bool ADLSearch::MatchesDirectory(const string& d) {
    // Check status
    if(!isActive) {
        return false;
    }
    if(sourceType != OnlyDirectory) {
        return false;
    }

    // Do search
    return SearchAll(d);
}

bool ADLSearch::SearchAll(const string& s) {
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
        for(StringSearch::List::iterator i = stringSearchList.begin(); i != stringSearchList.end(); ++i) {
            if(!i->match(s)) {
                return false;
            }
        }
        return (stringSearchList.size() != 0);
    #ifdef USE_PCRE
    }
    #endif
}

///  Load old searches from disk
void ADLSearchManager::Load()
{
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
                    if(search.searchString.size() > 0) {
                        collection.push_back(search);
                    }

                    // Go to next search
                    xml.stepOut();
                }
            }
        }
    } catch(const SimpleXMLException&) {
        return;
    } catch(const FileException&) {
        return;
    }
}

///  Save current searches to disk
void ADLSearchManager::Save()
{
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
        for(SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i) {
            ADLSearch& search = *i;
            if(search.searchString.size() == 0) {
                continue;
            }
            string type = "type";
            xml.addTag("Search");
            xml.stepIn();

            xml.addTag("SearchString", search.searchString);;

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
        } catch(const FileException&) {
            return;
        }
    } catch(const SimpleXMLException&) {
        return;
    }
}

void ADLSearchManager::MatchesFile(DestDirList& destDirVector, DirectoryListing::File *currentFile, string& fullPath) {
    // Add to any substructure being stored
    for(DestDirList::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id) {
        if(id->subdir != NULL) {
            DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile, true);
            dcassert(id->subdir->getAdls());

            id->subdir->files.push_back(copyFile);
        }
        id->fileAdded = false;  // Prepare for next stage
    }

    // Prepare to match searches
    if(currentFile->getName().size() < 1) {
        return;
    }

    string filePath = fullPath + "\\" + currentFile->getName();
    // Match searches
    for(SearchCollection::iterator is = collection.begin(); is != collection.end(); ++is) {
        if(destDirVector[is->ddIndex].fileAdded) {
            continue;
        }
        if(is->MatchesFile(currentFile->getName(), filePath, currentFile->getSize())) {
            DirectoryListing::File *copyFile = new DirectoryListing::File(*currentFile, true);
            destDirVector[is->ddIndex].dir->files.push_back(copyFile);
            destDirVector[is->ddIndex].fileAdded = true;

            if(is->isAutoQueue){
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

void ADLSearchManager::MatchesDirectory(DestDirList& destDirVector, DirectoryListing::Directory* currentDir, string& fullPath) {
    // Add to any substructure being stored
    for(DestDirList::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id) {
        if(id->subdir != NULL) {
            DirectoryListing::Directory* newDir =
                new DirectoryListing::AdlDirectory(fullPath, id->subdir, currentDir->getName());
            id->subdir->directories.push_back(newDir);
            id->subdir = newDir;
        }
    }

    // Prepare to match searches
    if(currentDir->getName().size() < 1) {
        return;
    }

    // Match searches
    for(SearchCollection::iterator is = collection.begin(); is != collection.end(); ++is) {
        if(destDirVector[is->ddIndex].subdir != NULL) {
            continue;
        }
        if(is->MatchesDirectory(currentDir->getName())) {
            destDirVector[is->ddIndex].subdir =
                new DirectoryListing::AdlDirectory(fullPath, destDirVector[is->ddIndex].dir, currentDir->getName());
            destDirVector[is->ddIndex].dir->directories.push_back(destDirVector[is->ddIndex].subdir);
            if(breakOnFirst) {
                // Found a match, search no more
                break;
            }
        }
    }
}

void ADLSearchManager::PrepareDestinationDirectories(DestDirList& destDirVector, DirectoryListing::Directory* root, StringMap& params) {
    // Load default destination directory (index = 0)
    destDirVector.clear();
    vector<DestDir>::iterator id = destDirVector.insert(destDirVector.end(), DestDir());
    id->name = "ADLSearch";
    id->dir  = new DirectoryListing::Directory(root, "<<<" + id->name + ">>>", true, true);

    // Scan all loaded searches
    for(SearchCollection::iterator is = collection.begin(); is != collection.end(); ++is) {
        // Check empty destination directory
        if(is->destDir.size() == 0) {
            // Set to default
            is->ddIndex = 0;
            continue;
        }

        // Check if exists
        bool isNew = true;
        long ddIndex = 0;
        for(id = destDirVector.begin(); id != destDirVector.end(); ++id, ++ddIndex) {
            if(Util::stricmp(is->destDir.c_str(), id->name.c_str()) == 0) {
                // Already exists, reuse index
                is->ddIndex = ddIndex;
                isNew = false;
                break;
            }
        }

        if(isNew) {
            // Add new destination directory
            id = destDirVector.insert(destDirVector.end(), DestDir());
            id->name = is->destDir;
            id->dir  = new DirectoryListing::Directory(root, "<<<" + id->name + ">>>", true, true);
            is->ddIndex = ddIndex;
        }
    }
    // Prepare all searches
    for(SearchCollection::iterator ip = collection.begin(); ip != collection.end(); ++ip) {
        ip->Prepare(params);
    }
}

void ADLSearchManager::matchListing(DirectoryListing& aDirList) noexcept {
    StringMap params;
    params["userNI"] = ClientManager::getInstance()->getNicks(aDirList.getUser())[0];
    params["userCID"] = aDirList.getUser().user->getCID().toBase32();

    setUser(aDirList.getUser());

    DestDirList destDirs;
    PrepareDestinationDirectories(destDirs, aDirList.getRoot(), params);
    setBreakOnFirst(BOOLSETTING(ADLS_BREAK_ON_FIRST));

    string path(aDirList.getRoot()->getName());
    matchRecurse(destDirs, aDirList.getRoot(), path);

    FinalizeDestinationDirectories(destDirs, aDirList.getRoot());
}

void ADLSearchManager::matchRecurse(DestDirList &aDestList, DirectoryListing::Directory* aDir, string &aPath) {
    for(DirectoryListing::Directory::Iter dirIt = aDir->directories.begin(); dirIt != aDir->directories.end(); ++dirIt) {
        string tmpPath = aPath + "\\" + (*dirIt)->getName();
        MatchesDirectory(aDestList, *dirIt, tmpPath);
        matchRecurse(aDestList, *dirIt, tmpPath);
    }
    for(DirectoryListing::File::Iter fileIt = aDir->files.begin(); fileIt != aDir->files.end(); ++fileIt) {
        MatchesFile(aDestList, *fileIt, aPath);
    }
    StepUpDirectory(aDestList);
}

void ADLSearchManager::StepUpDirectory(DestDirList& destDirVector) {
    for(DestDirList::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id) {
        if(id->subdir != NULL) {
            id->subdir = id->subdir->getParent();
            if(id->subdir == id->dir) {
                id->subdir = NULL;
            }
        }
    }
}
void ADLSearchManager::FinalizeDestinationDirectories(DestDirList& destDirVector, DirectoryListing::Directory* root) {
    string szDiscard("<<<" + string(_("Discard")) + ">>>");

    // Add non-empty destination directories to the top level
    for(vector<DestDir>::iterator id = destDirVector.begin(); id != destDirVector.end(); ++id) {
        if(id->dir->files.size() == 0 && id->dir->directories.size() == 0) {
            delete (id->dir);
        } else if(Util::stricmp(id->dir->getName(), szDiscard) == 0) {
            delete (id->dir);
        } else {
            root->directories.push_back(id->dir);
        }
    }
}

string ADLSearchManager::getConfigFile() {
    return Util::getPath(Util::PATH_USER_CONFIG) + "ADLSearch.xml";
}

} // namespace dcpp
