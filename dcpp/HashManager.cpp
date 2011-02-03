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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "HashManager.h"
#include "SimpleXML.h"
#include "LogManager.h"
#include "File.h"
#include "ZUtils.h"
#include "SFVReader.h"

#ifndef _WIN32
#include <sys/mman.h> // mmap, munmap, madvise
#include <signal.h>  // for handling read errors from previous trio
#include <setjmp.h>
#endif

namespace dcpp {

#define HASH_FILE_VERSION_STRING "2"
static const uint32_t HASH_FILE_VERSION = 2;
const int64_t HashManager::MIN_BLOCK_SIZE = 64 * 1024;

bool HashManager::checkTTH(const string& aFileName, int64_t aSize, uint32_t aTimeStamp) {
    Lock l(cs);
    if (!store.checkTTH(aFileName, aSize, aTimeStamp)) {
        hasher.hashFile(aFileName, aSize);
        return false;
    }
    return true;
}

TTHValue HashManager::getTTH(const string& aFileName, int64_t aSize) throw(HashException) {
    Lock l(cs);
    const TTHValue* tth = store.getTTH(aFileName);
    if (tth == NULL) {
        hasher.hashFile(aFileName, aSize);
        throw HashException();
    }
    return *tth;
}

const TTHValue* HashManager::getFileTTHif(const string& aFileName) {
    Lock l(cs);
    return store.getTTH(aFileName);
}

bool HashManager::getTree(const TTHValue& root, TigerTree& tt) {
    Lock l(cs);
    return store.getTree(root, tt);
}

size_t HashManager::getBlockSize(const TTHValue& root) {
    Lock l(cs);
    return store.getBlockSize(root);
}

void HashManager::hashDone(const string& aFileName, uint32_t aTimeStamp, const TigerTree& tth, int64_t speed, int64_t size) {
    try {
        Lock l(cs);
        store.addFile(aFileName, aTimeStamp, tth, true);
    } catch (const Exception& e) {
        LogManager::getInstance()->message(str(F_("Hashing failed: %1%") % e.getError()));
        return;
    }

    fire(HashManagerListener::TTHDone(), aFileName, tth.getRoot());

    if (speed > 0) {
        LogManager::getInstance()->message(str(F_("Finished hashing: %1% (%2% at %3%/s)") % Util::addBrackets(aFileName) %
            Util::formatBytes(size) % Util::formatBytes(speed)));
    } else if(size >= 0) {
        LogManager::getInstance()->message(str(F_("Finished hashing: %1% (%2%)") % Util::addBrackets(aFileName) %
            Util::formatBytes(size)));
    } else {
        LogManager::getInstance()->message(str(F_("Finished hashing: %1%") % Util::addBrackets(aFileName)));
    }
}

void HashManager::HashStore::addFile(const string& aFileName, uint32_t aTimeStamp, const TigerTree& tth, bool aUsed) {
    addTree(tth);

    string fname = Util::getFileName(aFileName);
    string fpath = Util::getFilePath(aFileName);

    FileInfoList& fileList = fileIndex[fpath];

    FileInfoIter j = find(fileList.begin(), fileList.end(), fname);
    if (j != fileList.end()) {
        fileList.erase(j);
    }

    fileList.push_back(FileInfo(fname, tth.getRoot(), aTimeStamp, aUsed));
    dirty = true;
}

void HashManager::HashStore::addTree(const TigerTree& tt) throw() {
    if (treeIndex.find(tt.getRoot()) == treeIndex.end()) {
        try {
            File f(getDataFile(), File::READ | File::WRITE, File::OPEN);
            int64_t index = saveTree(f, tt);
            treeIndex.insert(make_pair(tt.getRoot(), TreeInfo(tt.getFileSize(), index, tt.getBlockSize())));
            dirty = true;
        } catch (const FileException& e) {
            LogManager::getInstance()->message(str(F_("Error saving hash data: %1%") % e.getError()));
        }
    }
}

int64_t HashManager::HashStore::saveTree(File& f, const TigerTree& tt) throw(FileException) {
    if (tt.getLeaves().size() == 1)
        return SMALL_TREE;

    f.setPos(0);
    int64_t pos = 0;
    size_t n = sizeof(pos);
    if (f.read(&pos, n) != sizeof(pos))
        throw HashException(_("Unable to read hash data file"));

    // Check if we should grow the file, we grow by a meg at a time...
    int64_t datsz = f.getSize();
    if ((pos + (int64_t) (tt.getLeaves().size() * TTHValue::BYTES)) >= datsz) {
        f.setPos(datsz + 1024 * 1024);
        f.setEOF();
    }
    f.setPos(pos);dcassert(tt.getLeaves().size()> 1);
    f.write(tt.getLeaves()[0].data, (tt.getLeaves().size() * TTHValue::BYTES));
    int64_t p2 = f.getPos();
    f.setPos(0);
    f.write(&p2, sizeof(p2));
    return pos;
}

bool HashManager::HashStore::loadTree(File& f, const TreeInfo& ti, const TTHValue& root, TigerTree& tt) {
    if (ti.getIndex() == SMALL_TREE) {
        tt = TigerTree(ti.getSize(), ti.getBlockSize(), root);
        return true;
    }
    try {
        f.setPos(ti.getIndex());
        size_t datalen = TigerTree::calcBlocks(ti.getSize(), ti.getBlockSize()) * TTHValue::BYTES;
        boost::scoped_array<uint8_t> buf(new uint8_t[datalen]);
        f.read(&buf[0], datalen);
        tt = TigerTree(ti.getSize(), ti.getBlockSize(), &buf[0]);
        if (!(tt.getRoot() == root))
            return false;
    } catch (const Exception&) {
        return false;
    }

    return true;
}

bool HashManager::HashStore::getTree(const TTHValue& root, TigerTree& tt) {
    TreeIter i = treeIndex.find(root);
    if (i == treeIndex.end())
        return false;
    try {
        File f(getDataFile(), File::READ, File::OPEN);
        return loadTree(f, i->second, root, tt);
    } catch (const Exception&) {
        return false;
    }
}

size_t HashManager::HashStore::getBlockSize(const TTHValue& root) const {
    TreeMap::const_iterator i = treeIndex.find(root);
    return i == treeIndex.end() ? 0 : i->second.getBlockSize();
}

bool HashManager::HashStore::checkTTH(const string& aFileName, int64_t aSize, uint32_t aTimeStamp) {
    string fname = Util::getFileName(aFileName);
    string fpath = Util::getFilePath(aFileName);
    DirIter i = fileIndex.find(fpath);
    if (i != fileIndex.end()) {
        FileInfoIter j = find(i->second.begin(), i->second.end(), fname);
        if (j != i->second.end()) {
            FileInfo& fi = *j;
            TreeIter ti = treeIndex.find(fi.getRoot());
            if (ti == treeIndex.end() || ti->second.getSize() != aSize || fi.getTimeStamp() != aTimeStamp) {
                i->second.erase(j);
                dirty = true;
                return false;
            }
            return true;
        }
    }
    return false;
}

const TTHValue* HashManager::HashStore::getTTH(const string& aFileName) {
    string fname = Util::getFileName(aFileName);
    string fpath = Util::getFilePath(aFileName);

    DirIter i = fileIndex.find(fpath);
    if (i != fileIndex.end()) {
        FileInfoIter j = find(i->second.begin(), i->second.end(), fname);
        if (j != i->second.end()) {
            j->setUsed(true);
            return &(j->getRoot());
        }
    }
    return NULL;
}

void HashManager::HashStore::rebuild() {
    try {
        DirMap newFileIndex;
        TreeMap newTreeIndex;

        for (DirIter i = fileIndex.begin(); i != fileIndex.end(); ++i) {
            for (FileInfoIter j = i->second.begin(); j != i->second.end(); ++j) {
                if (!j->getUsed())
                    continue;

                TreeIter k = treeIndex.find(j->getRoot());
                if (k != treeIndex.end()) {
                    newTreeIndex[j->getRoot()] = k->second;
                }
            }
        }

        string tmpName = getDataFile() + ".tmp";
        string origName = getDataFile();

        createDataFile(tmpName);

        {
            File in(origName, File::READ, File::OPEN);
            File out(tmpName, File::READ | File::WRITE, File::OPEN);

            for (TreeIter i = newTreeIndex.begin(); i != newTreeIndex.end();) {
                TigerTree tree;
                if (loadTree(in, i->second, i->first, tree)) {
                    i->second.setIndex(saveTree(out, tree));
                    ++i;
                } else {
                    newTreeIndex.erase(i++);
                }
            }
        }

        for (DirIter i = fileIndex.begin(); i != fileIndex.end(); ++i) {
            DirIter fi = newFileIndex.insert(make_pair(i->first, FileInfoList())).first;

            for (FileInfoIter j = i->second.begin(); j != i->second.end(); ++j) {
                if (newTreeIndex.find(j->getRoot()) != newTreeIndex.end()) {
                    fi->second.push_back(*j);
                }
            }

            if (fi->second.empty())
                newFileIndex.erase(fi);
        }

        File::deleteFile(origName);
        File::renameFile(tmpName, origName);
        treeIndex = newTreeIndex;
        fileIndex = newFileIndex;
        dirty = true;
        save();
    } catch (const Exception& e) {
        LogManager::getInstance()->message(str(F_("Hashing failed: %1%") % e.getError()));
    }
}

void HashManager::HashStore::save() {
    if (dirty) {
        try {
            File ff(getIndexFile() + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
            BufferedOutputStream<false> f(&ff);

            string tmp;
            string b32tmp;

            f.write(SimpleXML::utf8Header);
            f.write(LIT("<HashStore Version=\"" HASH_FILE_VERSION_STRING "\">\r\n"));

            f.write(LIT("\t<Trees>\r\n"));

            for (TreeIter i = treeIndex.begin(); i != treeIndex.end(); ++i) {
                const TreeInfo& ti = i->second;
                f.write(LIT("\t\t<Hash Type=\"TTH\" Index=\""));
                f.write(Util::toString(ti.getIndex()));
                f.write(LIT("\" BlockSize=\""));
                f.write(Util::toString(ti.getBlockSize()));
                f.write(LIT("\" Size=\""));
                f.write(Util::toString(ti.getSize()));
                f.write(LIT("\" Root=\""));
                b32tmp.clear();
                f.write(i->first.toBase32(b32tmp));
                f.write(LIT("\"/>\r\n"));
            }

            f.write(LIT("\t</Trees>\r\n\t<Files>\r\n"));

            for (DirIter i = fileIndex.begin(); i != fileIndex.end(); ++i) {
                const string& dir = i->first;
                for (FileInfoIter j = i->second.begin(); j != i->second.end(); ++j) {
                    const FileInfo& fi = *j;
                    f.write(LIT("\t\t<File Name=\""));
                    f.write(SimpleXML::escape(dir + fi.getFileName(), tmp, true));
                    f.write(LIT("\" TimeStamp=\""));
                    f.write(Util::toString(fi.getTimeStamp()));
                    f.write(LIT("\" Root=\""));
                    b32tmp.clear();
                    f.write(fi.getRoot().toBase32(b32tmp));
                    f.write(LIT("\"/>\r\n"));
                }
            }
            f.write(LIT("\t</Files>\r\n</HashStore>"));
            f.flush();
            ff.close();
            File::deleteFile( getIndexFile());
            File::renameFile(getIndexFile() + ".tmp", getIndexFile());

            dirty = false;
        } catch (const FileException& e) {
            LogManager::getInstance()->message(str(F_("Error saving hash data: %1%") % e.getError()));
        }
    }
}

class HashLoader: public SimpleXMLReader::CallBack {
public:
    HashLoader(HashManager::HashStore& s) :
        store(s), size(0), timeStamp(0), version(HASH_FILE_VERSION), inTrees(false), inFiles(false), inHashStore(false) {
    }
    virtual void startTag(const string& name, StringPairList& attribs, bool simple);
    virtual void endTag(const string& name, const string& data);

private:
    HashManager::HashStore& store;

    string file;
    int64_t size;
    uint32_t timeStamp;
    int version;

    bool inTrees;
    bool inFiles;
    bool inHashStore;
};

void HashManager::HashStore::load() {
    try {
        Util::migrate(getIndexFile());

        HashLoader l(*this);
                File f(getIndexFile(), File::READ, File::OPEN);
                SimpleXMLReader(&l).parse(f);
    } catch (const Exception&) {
        // ...
    }
}

static const string sHashStore = "HashStore";
static const string sversion = "version"; // Oops, v1 was like this
static const string sVersion = "Version";
static const string sTrees = "Trees";
static const string sFiles = "Files";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sHash = "Hash";
static const string sType = "Type";
static const string sTTH = "TTH";
static const string sIndex = "Index";
static const string sLeafSize = "LeafSize"; // Residue from v1 as well
static const string sBlockSize = "BlockSize";
static const string sTimeStamp = "TimeStamp";
static const string sRoot = "Root";

void HashLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
    if (!inHashStore && name == sHashStore) {
        version = Util::toInt(getAttrib(attribs, sVersion, 0));
        if (version == 0) {
            version = Util::toInt(getAttrib(attribs, sversion, 0));
        }
        inHashStore = !simple;
    } else if (inHashStore && version == 2) {
        if (inTrees && name == sHash) {
            const string& type = getAttrib(attribs, sType, 0);
            int64_t index = Util::toInt64(getAttrib(attribs, sIndex, 1));
            int64_t blockSize = Util::toInt64(getAttrib(attribs, sBlockSize, 2));
            int64_t size = Util::toInt64(getAttrib(attribs, sSize, 3));
            const string& root = getAttrib(attribs, sRoot, 4);
            if (!root.empty() && type == sTTH && (index >= 8 || index == HashManager::SMALL_TREE) && blockSize >= 1024) {
                store.treeIndex[TTHValue(root)] = HashManager::HashStore::TreeInfo(size, index, blockSize);
            }
        } else if (inFiles && name == sFile) {
            file = getAttrib(attribs, sName, 0);
            timeStamp = Util::toUInt32(getAttrib(attribs, sTimeStamp, 1));
            const string& root = getAttrib(attribs, sRoot, 2);

            if (!file.empty() && size >= 0 && timeStamp > 0 && !root.empty()) {
                string fname = Util::getFileName(file);
                string fpath = Util::getFilePath(file);

                store.fileIndex[fpath].push_back(HashManager::HashStore::FileInfo(fname, TTHValue(root), timeStamp,
                    false));
            }
        } else if (name == sTrees) {
            inTrees = !simple;
        } else if (name == sFiles) {
            inFiles = !simple;
        }
    }
}

void HashLoader::endTag(const string& name, const string&) {
    if (name == sFile) {
        file.clear();
    }
}

HashManager::HashStore::HashStore() :
    dirty(false) {

    Util::migrate(getDataFile());

    if (File::getSize(getDataFile()) <= static_cast<int64_t> (sizeof(int64_t))) {
        try {
            createDataFile( getDataFile());
        } catch (const FileException&) {
            // ?
        }
    }
}

/**
 * Creates the data files for storing hash values.
 * The data file is very simple in its format. The first 8 bytes
 * are filled with an int64_t (little endian) of the next write position
 * in the file counting from the start (so that file can be grown in chunks).
 * We start with a 1 mb file, and then grow it as needed to avoid fragmentation.
 * To find data inside the file, use the corresponding index file.
 * Since file is never deleted, space will eventually be wasted, so a rebuild
 * should occasionally be done.
 */
void HashManager::HashStore::createDataFile(const string& name) {
    try {
        File dat(name, File::WRITE, File::CREATE | File::TRUNCATE);
        dat.setPos(1024 * 1024);
        dat.setEOF();
        dat.setPos(0);
        int64_t start = sizeof(start);
        dat.write(&start, sizeof(start));

    } catch (const FileException& e) {
        LogManager::getInstance()->message(str(F_("Error creating hash data file: %1%") % e.getError()));
    }
}

void HashManager::Hasher::hashFile(const string& fileName, int64_t size) {
    Lock l(cs);
    if (w.insert(make_pair(fileName, size)).second) {
        if(paused > 0)
            paused++;
        else
        s.signal();
    }
}

bool HashManager::Hasher::pause() {
    Lock l(cs);
    return paused++ > 0;
}

void HashManager::Hasher::resume() {
    Lock l(cs);
    while(--paused > 0)
        s.signal();
}

bool HashManager::Hasher::isPaused() const {
    Lock l(cs);
    return paused > 0;
}

void HashManager::Hasher::stopHashing(const string& baseDir) {
    Lock l(cs);
    for (WorkIter i = w.begin(); i != w.end();) {
        if (Util::strnicmp(baseDir, i->first, baseDir.length()) == 0) {
            w.erase(i++);
        } else {
            ++i;
        }
    }
}

void HashManager::Hasher::getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft) {
    Lock l(cs);
    curFile = currentFile;
    filesLeft = w.size();
    if (running)
        filesLeft++;
    bytesLeft = 0;
    for (WorkMap::const_iterator i = w.begin(); i != w.end(); ++i) {
        bytesLeft += i->second;
    }
    bytesLeft += currentSize;
}

void HashManager::Hasher::instantPause() {
    bool wait = false;
    {
        Lock l(cs);
        if(paused > 0) {
            paused++;
            wait = true;
        }
    }
    if(wait)
        s.wait();
}

#ifdef _WIN32
#define BUF_SIZE (256*1024)

bool HashManager::Hasher::fastHash(const string& fname, uint8_t* buf, TigerTree& tth, int64_t size, CRC32Filter* xcrc32) {
    HANDLE h = INVALID_HANDLE_VALUE;
    DWORD x, y;
    if (!GetDiskFreeSpace(Text::toT(Util::getFilePath(fname)).c_str(), &y, &x, &y, &y)) {
        return false;
    } else {
        if ((BUF_SIZE % x) != 0) {
            return false;
        } else {
            h = ::CreateFile(Text::toT(fname).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
            if (h == INVALID_HANDLE_VALUE)
                return false;
        }
    }
    DWORD hn = 0;
    DWORD rn = 0;
    uint8_t* hbuf = buf + BUF_SIZE;
    uint8_t* rbuf = buf;

    OVERLAPPED over = { 0 };
    BOOL res = TRUE;
    over.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

    bool ok = false;

    uint64_t lastRead = GET_TICK();
    if (!::ReadFile(h, hbuf, BUF_SIZE, &hn, &over)) {
        if (GetLastError() == ERROR_HANDLE_EOF) {
            hn = 0;
        } else if (GetLastError() == ERROR_IO_PENDING) {
            if (!GetOverlappedResult(h, &over, &hn, TRUE)) {
                if (GetLastError() == ERROR_HANDLE_EOF) {
                    hn = 0;
                } else {
                    goto cleanup;
                }
            }
        } else {
            goto cleanup;
        }
    }

    over.Offset = hn;
    size -= hn;
    while (!stop) {
        if (size > 0) {
            // Start a new overlapped read
            ResetEvent(over.hEvent);
            if (SETTING(MAX_HASH_SPEED) > 0) {
                uint64_t now = GET_TICK();
                uint64_t minTime = hn * 1000LL / (SETTING(MAX_HASH_SPEED) * 1024LL * 1024LL);
                if (lastRead + minTime > now) {
                    uint64_t diff = now - lastRead;
                    Thread::sleep(minTime - diff);
                }
                lastRead = lastRead + minTime;
            } else {
                lastRead = GET_TICK();
            }
            res = ReadFile(h, rbuf, BUF_SIZE, &rn, &over);
        } else {
            rn = 0;
        }

        tth.update(hbuf, hn);
        if (xcrc32)
            (*xcrc32)(hbuf, hn);

        {
            Lock l(cs);
            currentSize = max(currentSize - hn, _LL(0));
        }

        if (size == 0) {
            ok = true;
            break;
        }

        if (!res) {
            // deal with the error code
            switch (GetLastError()) {
            case ERROR_IO_PENDING:
                if (!GetOverlappedResult(h, &over, &rn, TRUE)) {
                    dcdebug("Error 0x%x: %s\n", GetLastError(), Util::translateError(GetLastError()).c_str());
                    goto cleanup;
                }
                break;
                default:
                dcdebug("Error 0x%x: %s\n", GetLastError(), Util::translateError(GetLastError()).c_str());
                goto cleanup;
            }
        }

        instantPause();

        *((uint64_t*)&over.Offset) += rn;
        size -= rn;

        swap(rbuf, hbuf);
        swap(rn, hn);
    }

    cleanup:
    ::CloseHandle(over.hEvent);
    ::CloseHandle(h);
    return ok;
}

#else // !_WIN32

static sigjmp_buf sb_env;

static void sigbus_handler(int signum, siginfo_t* info, void* context) {
    // Jump back to the fastHash which will return error. Apparently truncating
    // a file in Solaris sets si_code to BUS_OBJERR
    if (signum == SIGBUS && (info->si_code == BUS_ADRERR || info->si_code == BUS_OBJERR))
        siglongjmp(sb_env, 1);
}

bool HashManager::Hasher::fastHash(const string& filename, uint8_t* , TigerTree& tth, int64_t size, CRC32Filter* xcrc32) {
    static const int64_t BUF_BYTES = (SETTING(HASH_BUFFER_SIZE_MB) >= 1)? SETTING(HASH_BUFFER_SIZE_MB)*1024*1024 : 0x800000;
    static const int64_t BUF_SIZE = BUF_BYTES - (BUF_BYTES % getpagesize());

    int fd = open(Text::fromUtf8(filename).c_str(), O_RDONLY);
    if(fd == -1) {
        dcdebug("Error opening file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
        return false;
    }

    int64_t pos = 0;
    int64_t size_read = 0;
        void *buf = NULL;
    bool ok = false;

        // Prepare and setup a signal handler in case of SIGBUS during mmapped file reads.
        // SIGBUS can be sent when the file is truncated or in case of read errors.
        struct sigaction act, oldact;
        sigset_t signalset;

        sigemptyset(&signalset);

        act.sa_handler = NULL;
        act.sa_sigaction = sigbus_handler;
        act.sa_mask = signalset;
#ifdef SA_SIGINFO
        act.sa_flags = SA_SIGINFO | SA_RESETHAND;
#else
        act.sa_flags = NULL;
#endif
        if (sigaction(SIGBUS, &act, &oldact) == -1) {
            dcdebug("Failed to set signal handler for fastHash\n");
            close(fd);
            return false;   // Better luck with the slow hash.
        }

    uint64_t lastRead = GET_TICK();
    unsigned long mmap_flags = static_cast<bool>(SETTING(HASH_BUFFER_PRIVATE))? MAP_PRIVATE : MAP_SHARED;
#ifdef MAP_POPULATE
    if (static_cast<bool>(SETTING(HASH_BUFFER_POPULATE)))
        mmap_flags |= MAP_POPULATE;
#endif
#ifdef MAP_NORESERVE
    if (static_cast<bool>(SETTING(HASH_BUFFER_NORESERVE)))
        mmap_flags |= MAP_NORESERVE;
#endif
    while (pos < size && !stop) {
            size_read = std::min(size - pos, BUF_SIZE);
            buf = mmap(0, size_read, PROT_READ, mmap_flags, fd, pos);
            if(buf == MAP_FAILED) {
            dcdebug("Error calling mmap for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
            break;
            }

                        if (sigsetjmp(sb_env, 1)) {
                            dcdebug("Caught SIGBUS for file %s\n", filename.c_str());
                            break;
                        }

        if (madvise(buf, size_read, MADV_SEQUENTIAL | MADV_WILLNEED) == -1) {
            dcdebug("Error calling madvise for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
            break;
        }

            if(SETTING(MAX_HASH_SPEED) > 0) {
            uint64_t now = GET_TICK();
            uint64_t minTime = size_read * 1000LL / (SETTING(MAX_HASH_SPEED) * 1024LL * 1024LL);
                if(lastRead + minTime> now) {
                uint64_t diff = now - lastRead;
                    Thread::sleep(minTime - diff);
                }
                lastRead = lastRead + minTime;
            } else {
                lastRead = GET_TICK();
            }

        tth.update(buf, size_read);
        if(xcrc32)
            (*xcrc32)(buf, size_read);

        {
            Lock l(cs);
            currentSize = max(static_cast<uint64_t>(currentSize - size_read), static_cast<uint64_t>(0));
        }

        if (munmap(buf, size_read) == -1) {
            dcdebug("Error calling munmap for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
            break;
        }

                buf = NULL;
        pos += size_read;

        instantPause();

        if (pos == size) {
            ok = true;
                }
    }

    if (buf != NULL && buf != MAP_FAILED && munmap(buf, size_read) == -1) {
        dcdebug("Error calling munmap for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
    }

    close(fd);

        if (sigaction(SIGBUS, &oldact, NULL) == -1) {
            dcdebug("Failed to reset old signal handler for SIGBUS\n");
        }

    return ok;
}

#endif // !_WIN32
int HashManager::Hasher::run() {
    setThreadPriority(Thread::IDLE);

    uint8_t* buf = NULL;
    bool virtualBuf = true;

    string fname;
    bool last = false;
    for(;;) {
        s.wait();
        if(stop)
            break;
        if(rebuild) {
            HashManager::getInstance()->doRebuild();
            rebuild = false;
            LogManager::getInstance()->message(_("Hash database rebuilt"));
            continue;
        }
        {
            Lock l(cs);
            if(!w.empty()) {
                currentFile = fname = w.begin()->first;
                currentSize = w.begin()->second;
                w.erase(w.begin());
                last = w.empty();
            } else {
                last = true;
                fname.clear();
            }
        }
        running = true;

        if(!fname.empty()) {
            int64_t size = File::getSize(fname);
            int64_t sizeLeft = size;
#ifdef _WIN32
            if(buf == NULL) {
                virtualBuf = true;
                buf = (uint8_t*)VirtualAlloc(NULL, 2*BUF_SIZE, MEM_COMMIT, PAGE_READWRITE);
            }
#else
            static const int64_t BUF_BYTES = (SETTING(HASH_BUFFER_SIZE_MB) >= 1)? SETTING(HASH_BUFFER_SIZE_MB)*1024*1024 : 0x800000;
            static const int64_t BUF_SIZE = BUF_BYTES - (BUF_BYTES % getpagesize());
#endif

            if(buf == NULL) {
                virtualBuf = false;
                buf = new uint8_t[BUF_SIZE];
            }
            try {
                File f(fname, File::READ, File::OPEN);
                int64_t bs = max(TigerTree::calcBlockSize(f.getSize(), 10), MIN_BLOCK_SIZE);
                uint64_t start = GET_TICK();
                uint32_t timestamp = f.getLastModified();
                TigerTree slowTTH(bs);
                TigerTree* tth = &slowTTH;

                CRC32Filter crc32;
                SFVReader sfv(fname);
                CRC32Filter* xcrc32 = 0;
                if(sfv.hasCRC())
                xcrc32 = &crc32;

                size_t n = 0;
                TigerTree fastTTH(bs);
                tth = &fastTTH;

                LogManager::getInstance()->message(str(F_("Hashing file: %1% (Size: %2%)")
                % Util::addBrackets(fname) % Util::formatBytes(size)));
#ifdef _WIN32
                if(!virtualBuf || !BOOLSETTING(FAST_HASH) || !fastHash(fname, buf, fastTTH, size, xcrc32)) {
#else
                if(!BOOLSETTING(FAST_HASH) || !fastHash(fname, 0, fastTTH, size, xcrc32)) {
#endif
                        tth = &slowTTH;
                        crc32 = CRC32Filter();
                        uint64_t lastRead = GET_TICK();

                        do {
                            size_t bufSize = BUF_SIZE;
                            if(SETTING(MAX_HASH_SPEED)> 0) {
                                uint64_t now = GET_TICK();
                                uint64_t minTime = n * 1000LL / (SETTING(MAX_HASH_SPEED) * 1024LL * 1024LL);
                                if(lastRead + minTime> now) {
                                    Thread::sleep(minTime - (now - lastRead));
                                }
                                lastRead = lastRead + minTime;
                            } else {
                                lastRead = GET_TICK();
                            }
                            n = f.read(buf, bufSize);
                            tth->update(buf, n);
                            if(xcrc32)
                                (*xcrc32)(buf, n);

                            {
                                Lock l(cs);
                                currentSize = max(static_cast<uint64_t>(currentSize - n), static_cast<uint64_t>(0));
                            }
                            sizeLeft -= n;

                        instantPause();
                        }while (n> 0 && !stop);
                    } else {
                        sizeLeft = 0;
                    }

                    f.close();
                    tth->finalize();
                    uint64_t end = GET_TICK();
                    int64_t speed = 0;
                    if(end> start) {
                        speed = size * _LL(1000) / (end - start);
                    }
                    if(xcrc32 && xcrc32->getValue() != sfv.getCRC()) {
                        LogManager::getInstance()->message(str(F_("%1% not shared; calculated CRC32 does not match the one found in SFV file.") % Util::addBrackets(fname)));
                    } else {
                    HashManager::getInstance()->hashDone(fname, timestamp, *tth, speed, size);
                    }
                } catch(const FileException& e) {
                    LogManager::getInstance()->message(str(F_("Error hashing %1%: %2%") % Util::addBrackets(fname) % e.getError()));
                }
            }
            {
                Lock l(cs);
                currentFile.clear();
                currentSize = 0;
            }
            running = false;
            if(buf != NULL && (last || stop)) {
                if(virtualBuf) {
#ifdef _WIN32
                    VirtualFree(buf, 0, MEM_RELEASE);
#endif
                } else {
                    delete [] buf;
                }
                buf = NULL;
            }
        }
        return 0;
    }

HashManager::HashPauser::HashPauser() {
    resume = !HashManager::getInstance()->pauseHashing();
}

HashManager::HashPauser::~HashPauser() {
    if(resume)
        HashManager::getInstance()->resumeHashing();
}

bool HashManager::pauseHashing() {
    Lock l(cs);
    return hasher.pause();
}

void HashManager::resumeHashing() {
    Lock l(cs);
    hasher.resume();
}

bool HashManager::isHashingPaused() const {
    Lock l(cs);
    return hasher.isPaused();
}

} // namespace dcpp
