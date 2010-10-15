/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_DCPP_SHARE_MANAGER_H
#define DCPLUSPLUS_DCPP_SHARE_MANAGER_H

#include "TimerManager.h"
#include "SearchManager.h"
#include "SettingsManager.h"
#include "HashManagerListener.h"
#include "QueueManagerListener.h"

#include "Exception.h"
#include "CriticalSection.h"
#include "StringSearch.h"
#include "Singleton.h"
#include "BloomFilter.h"
#include "FastAlloc.h"
#include "MerkleTree.h"
#include "Pointer.h"

namespace dht {
    class IndexManager;
}
namespace dcpp {

STANDARD_EXCEPTION(ShareException);

class SimpleXML;
class Client;
class File;
class OutputStream;
class MemoryInputStream;

struct ShareLoader;
class ShareManager : public Singleton<ShareManager>, private SettingsManagerListener, private Thread, private TimerManagerListener,
    private HashManagerListener, private QueueManagerListener
{
public:
    /**
     * @param aDirectory Physical directory location
     * @param aName Virtual name
     */
    void addDirectory(const string& realPath, const string &virtualName) throw(ShareException);
    void removeDirectory(const string& realPath);
    void renameDirectory(const string& realPath, const string& virtualName) throw(ShareException);

    bool isRefreshing() { return refreshing; }

    string toVirtual(const TTHValue& tth) const throw(ShareException);
    string toReal(const string& virtualFile) throw(ShareException);
    StringList getRealPaths(const string& virtualPath) throw(ShareException);
    TTHValue getTTH(const string& virtualFile) const throw(ShareException);

    void refresh(bool dirs = false, bool aUpdate = true, bool block = false) throw();
    void setDirty() { xmlDirty = true; }

    void search(SearchResultList& l, const string& aString, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) throw();
    void search(SearchResultList& l, const StringList& params, StringList::size_type maxResults) throw();

    StringPairList getDirectories() const throw();

    MemoryInputStream* generatePartialList(const string& dir, bool recurse) const;
    MemoryInputStream* getTree(const string& virtualFile) const;

    AdcCommand getFileInfo(const string& aFile) throw(ShareException);

    int64_t getShareSize() const throw();
    int64_t getShareSize(const string& realPath) const throw();

    size_t getSharedFiles() const throw();

    string getShareSizeString() const { return Util::toString(getShareSize()); }
    string getShareSizeString(const string& aDir) const { return Util::toString(getShareSize(aDir)); }

    void getBloom(ByteVector& v, size_t k, size_t m, size_t h) const;

    SearchManager::TypeModes getType(const string& fileName) const throw();

    string validateVirtual(const string& /*aVirt*/) const throw();
    bool hasVirtual(const string& name) const throw();

    void addHits(uint32_t aHits) {
        hits += aHits;
    }

    string getOwnListFile() {
        generateXmlList();
        return getBZXmlFile();
    }

    bool isTTHShared(const TTHValue& tth){
        Lock l(cs);
        return tthIndex.find(tth) != tthIndex.end();
    }
    void publish();
    GETSET(uint32_t, hits, Hits);
    GETSET(string, bzXmlFile, BZXmlFile);
private:
    struct AdcSearch;
    class Directory : public FastAlloc<Directory>, public intrusive_ptr_base<Directory>, boost::noncopyable {
    public:
        typedef boost::intrusive_ptr<Directory> Ptr;
        typedef unordered_map<string, Ptr, noCaseStringHash, noCaseStringEq> Map;
        typedef Map::iterator MapIter;

        struct File {
            struct StringComp {
                StringComp(const string& s) : a(s) { }
                bool operator()(const File& b) const {
                if (BOOLSETTING(CASESENSITIVE_FILELIST))
                    return strcmp(a.c_str(), b.getName().c_str()) == 0;
                else
                    return Util::stricmp(a, b.getName()) == 0;
                }

                const string& a;
            private:
                StringComp& operator=(const StringComp&);
            };
            struct FileLess {
                bool operator()(const File& a, const File& b) const {
                    if (BOOLSETTING(CASESENSITIVE_FILELIST))
                        return (strcmp(a.getName().c_str(), b.getName().c_str()) < 0);
                    else
                        return (Util::stricmp(a.getName(), b.getName()) < 0);
                }
            };
            typedef set<File, FileLess> Set;

            File() : size(0), parent(0) { }
            File(const string& aName, int64_t aSize, const Directory::Ptr& aParent, const TTHValue& aRoot) :
            name(aName), tth(aRoot), size(aSize), parent(aParent.get()) { }
            File(const File& rhs) :
            name(rhs.getName()), tth(rhs.getTTH()), size(rhs.getSize()), parent(rhs.getParent()) { }

            ~File() { }

            File& operator=(const File& rhs) {
                name = rhs.name; size = rhs.size; parent = rhs.parent; tth = rhs.tth;
                return *this;
            }

            bool operator==(const File& rhs) const {
                if (BOOLSETTING(CASESENSITIVE_FILELIST))
                    return getParent() == rhs.getParent() && (strcmp(getName().c_str(), rhs.getName().c_str()) == 0);
                else
                    return getParent() == rhs.getParent() && (Util::stricmp(getName(), rhs.getName()) == 0);
            }

            string getADCPath() const { return parent->getADCPath() + name; }
            string getFullName() const { return parent->getFullName() + name; }
            string getRealPath() const { return parent->getRealPath(name); }

            GETSET(string, name, Name);
            GETSET(TTHValue, tth, TTH);
            GETSET(int64_t, size, Size);
            GETSET(Directory*, parent, Parent);
        };

        int64_t size;
        Map directories;
        File::Set files;

        static Ptr create(const string& aName, const Ptr& aParent = Ptr()) { return Ptr(new Directory(aName, aParent)); }

        bool hasType(uint32_t type) const throw() {
            return ( (type == SearchManager::TYPE_ANY) || (fileTypes & (1 << type)) );
        }
        void addType(uint32_t type) throw();

        string getADCPath() const throw();
        string getFullName() const throw();
        string getRealPath(const std::string& path) const throw(ShareException);

        int64_t getSize() const throw();

        void search(SearchResultList& aResults, StringSearch::List& aStrings, int aSearchType, int64_t aSize, int aFileType, Client* aClient, StringList::size_type maxResults) const throw();
        void search(SearchResultList& aResults, AdcSearch& aStrings, StringList::size_type maxResults) const throw();

        void toXml(OutputStream& xmlFile, string& indent, string& tmp2, bool fullList) const;
        void filesToXml(OutputStream& xmlFile, string& indent, string& tmp2) const;

        File::Set::const_iterator findFile(const string& aFile) const { return find_if(files.begin(), files.end(), Directory::File::StringComp(aFile)); }

        void merge(const Ptr& source);

        GETSET(string, name, Name);
        GETSET(Directory*, parent, Parent);
    private:
        friend void intrusive_ptr_release(intrusive_ptr_base<Directory>*);

        Directory(const string& aName, const Ptr& aParent);
        ~Directory() { }

        /** Set of flags that say which SearchManager::TYPE_* a directory contains */
        uint32_t fileTypes;

    };

    friend class Directory;
    friend struct ShareLoader;

    friend class Singleton<ShareManager>;
    ShareManager();

    virtual ~ShareManager();

    struct AdcSearch {
        AdcSearch(const StringList& params);

        bool isExcluded(const string& str) {
            for(StringSearch::List::iterator i = exclude.begin(); i != exclude.end(); ++i) {
                if(i->match(str))
                    return true;
            }
            return false;
        }

        bool hasExt(const string& name) {
            if(ext.empty())
                return true;
            for(StringIter i = ext.begin(); i != ext.end(); ++i) {
                if(name.length() >= i->length() && Util::stricmp(name.c_str() + name.length() - i->length(), i->c_str()) == 0)
                    return true;
            }
            return false;
        }

        StringSearch::List* include;
        StringSearch::List includeX;
        StringSearch::List exclude;
        StringList ext;

        int64_t gt;
        int64_t lt;

        TTHValue root;
        bool hasRoot;

        bool isDirectory;
    };

    int64_t xmlListLen;
    TTHValue xmlRoot;
    int64_t bzXmlListLen;
    TTHValue bzXmlRoot;
    auto_ptr<File> bzXmlRef;

    bool xmlDirty;
    bool forceXmlRefresh; /// bypass the 15-minutes guard
    bool refreshDirs;
    bool update;
    bool initial;

    int listN;

    volatile long refreshing;

    uint64_t lastXmlUpdate;
    uint64_t lastFullUpdate;

    mutable CriticalSection cs;

    // List of root directory items
    typedef std::list<Directory::Ptr> DirList;
    DirList directories;

    /** Map real name to virtual name - multiple real names may be mapped to a single virtual one */
    StringMap shares;
    friend class ::dht::IndexManager;
    typedef unordered_map<TTHValue, Directory::File::Set::const_iterator> HashFileMap;
    typedef HashFileMap::iterator HashFileIter;

    HashFileMap tthIndex;

    BloomFilter<5> bloom;

    Directory::File::Set::const_iterator findFile(const string& virtualFile) const throw(ShareException);

    Directory::Ptr buildTree(const string& aName, const Directory::Ptr& aParent);
    bool checkHidden(const string& aName) const;

    void rebuildIndices();

    void updateIndices(Directory& aDirectory);
    void updateIndices(Directory& dir, const Directory::File::Set::iterator& i);

    Directory::Ptr merge(const Directory::Ptr& directory);

    void generateXmlList();
    bool loadCache() throw();
    DirList::const_iterator getByVirtual(const string& virtualName) const throw();
    pair<Directory::Ptr, string> splitVirtual(const string& virtualPath) const throw(ShareException);

    string findRealRoot(const string& virtualRoot, const string& virtualLeaf) const throw(ShareException);

    Directory::Ptr getDirectory(const string& fname);

    virtual int run();

    // QueueManagerListener
    virtual void on(QueueManagerListener::Finished, QueueItem* qi, const string& dir, int64_t speed) throw();

    // HashManagerListener
    virtual void on(HashManagerListener::TTHDone, const string& fname, const TTHValue& root) throw();

    // SettingsManagerListener
    virtual void on(SettingsManagerListener::Save, SimpleXML& xml) throw() {
        save(xml);
    }
    virtual void on(SettingsManagerListener::Load, SimpleXML& xml) throw() {
        load(xml);
    }

    // TimerManagerListener
    virtual void on(TimerManagerListener::Minute, uint32_t tick) throw();
    void load(SimpleXML& aXml);
    void save(SimpleXML& aXml);

};

} // namespace dcpp

#endif // !defined(SHARE_MANAGER_H)
