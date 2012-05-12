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

#ifndef DCPLUSPLUS_DCPP_DIRECTORY_LISTING_H
#define DCPLUSPLUS_DCPP_DIRECTORY_LISTING_H

//#include "forward.h"
#include "noexcept.h"
#include "User.h"
#include "FastAlloc.h"

#include "MerkleTree.h"
#include "Streams.h"
#include "MediaInfo.h"

namespace dcpp {

class ListLoader;

class DirectoryListing : boost::noncopyable
{
public:
    class Directory;

    class File : public FastAlloc<File> {
    public:
        typedef File* Ptr;
        struct FileSort {
            bool operator()(const Ptr& a, const Ptr& b) const {
                return Util::stricmp(a->getName().c_str(), b->getName().c_str()) < 0;
            }
        };
        typedef vector<Ptr> List;
        typedef List::iterator Iter;

        File(Directory* aDir, const string& aName, int64_t aSize, const TTHValue& aTTH) noexcept :
            name(aName), size(aSize), parent(aDir), tthRoot(aTTH), adls(false)
        {
        }

        File(const File& rhs, bool _adls = false) : name(rhs.name), size(rhs.size), parent(rhs.parent), tthRoot(rhs.tthRoot), adls(_adls)
        {
        }

        File& operator=(const File& rhs) {
            name = rhs.name; size = rhs.size; parent = rhs.parent; tthRoot = rhs.tthRoot;
            return *this;
        }

        ~File() { }

        GETSET(string, name, Name);
        GETSET(int64_t, size, Size);
        GETSET(Directory*, parent, Parent);
        GETSET(TTHValue, tthRoot, TTH);
        GETSET(bool, adls, Adls);
        MediaInfo mediaInfo;
    };

    class Directory : public FastAlloc<Directory>, boost::noncopyable {
    public:
        typedef Directory* Ptr;
        struct DirSort {
            bool operator()(const Ptr& a, const Ptr& b) const {
                return Util::stricmp(a->getName().c_str(), b->getName().c_str()) < 0;
            }
        };
        typedef vector<Ptr> List;
        typedef List::iterator Iter;

        typedef unordered_set<TTHValue> TTHSet;

        List directories;
        File::List files;

        Directory(Directory* aParent, const string& aName, bool _adls, bool aComplete)
            : name(aName), parent(aParent), adls(_adls), complete(aComplete) { }

        virtual ~Directory() {
            for_each(directories.begin(), directories.end(), DeleteFunction());
            for_each(files.begin(), files.end(), DeleteFunction());
        }

        size_t getTotalFileCount(bool adls = false);
        int64_t getTotalSize(bool adls = false);
        void filterList(DirectoryListing& dirList);
        void filterList(TTHSet& l);
        void getHashList(TTHSet& l);

        size_t getFileCount() { return files.size(); }

        int64_t getSize() {
            int64_t x = 0;
            for(auto i = files.begin(); i != files.end(); ++i) {
                x+=(*i)->getSize();
            }
            return x;
        }

        GETSET(string, name, Name);
        GETSET(Directory*, parent, Parent);
        GETSET(bool, adls, Adls);
        GETSET(bool, complete, Complete);

    };

    class AdlDirectory : public Directory {
    public:
        AdlDirectory(const string& aFullPath, Directory* aParent, const string& aName) : Directory(aParent, aName, true, true), fullPath(aFullPath) { }

        GETSET(string, fullPath, FullPath);
    };

    DirectoryListing(const HintedUser& aUser);
    ~DirectoryListing();

    void loadFile(const string& name);

    string updateXML(const std::string&);
    string loadXML(InputStream& xml, bool updating);

    void download(const string& aDir, const string& aTarget, bool highPrio);
    void download(Directory* aDir, const string& aTarget, bool highPrio);
    void download(File* aFile, const string& aTarget, bool view, bool highPrio);

    string getPath(const Directory* d) const;
    string getPath(const File* f) const { return getPath(f->getParent()); }

    /** returns the local path of the file when browsing own file list */
    StringList getLocalPaths(const File* f) const;
    /** returns the local paths of the directory when browsing own file list */
    StringList getLocalPaths(const Directory* d) const;

    int64_t getTotalSize(bool adls = false) { return root->getTotalSize(adls); }
    size_t getTotalFileCount(bool adls = false) { return root->getTotalFileCount(adls); }

    const Directory* getRoot() const { return root; }
    Directory* getRoot() { return root; }

    static UserPtr getUserFromFilename(const string& fileName);

    GETSET(HintedUser, user, User);

private:
    friend class ListLoader;

    Directory* root;

    Directory* find(const string& aName, Directory* current);

};

inline bool operator==(DirectoryListing::Directory::Ptr a, const string& b) { return Util::stricmp(a->getName(), b) == 0; }
inline bool operator==(DirectoryListing::File::Ptr a, const string& b) { return Util::stricmp(a->getName(), b) == 0; }

} // namespace dcpp

#endif // !defined(DIRECTORY_LISTING_H)
