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

#ifndef DCPLUSPLUS_DCPP_FILE_H
#define DCPLUSPLUS_DCPP_FILE_H

#include "SettingsManager.h"

#include "Util.h"
#include "Text.h"
#include "Streams.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#endif

//#ifdef _WIN32
//#include "../zlib/zlib.h"
//#else
#include <zlib.h>
//#endif

namespace dcpp {

class File : public IOStream {
public:
    enum {
        OPEN = 0x01,
        CREATE = 0x02,
        TRUNCATE = 0x04,
        SHARED = 0x08
    };

#ifdef _WIN32
    enum {
        READ = GENERIC_READ,
        WRITE = GENERIC_WRITE,
        RW = READ | WRITE
    };

    static uint32_t convertTime(FILETIME* f);

#else // !_WIN32

    enum {
        READ = 0x01,
        WRITE = 0x02,
        RW = READ | WRITE
    };

    // some ftruncate implementations can't extend files like SetEndOfFile,
    // not sure if the client code needs this...
    int extendFile(int64_t len) throw();

#endif // !_WIN32

    File(const string& aFileName, int access, int mode) throw(FileException);

    bool isOpen() throw();
    virtual void close() throw();
    virtual int64_t getSize() throw();
    virtual void setSize(int64_t newSize) throw(FileException);

    virtual int64_t getPos() throw();
    virtual void setPos(int64_t pos) throw();
    virtual void setEndPos(int64_t pos) throw();
    virtual void movePos(int64_t pos) throw();
    virtual void setEOF() throw(FileException);

    virtual size_t read(void* buf, size_t& len) throw(FileException);
    virtual size_t write(const void* buf, size_t len) throw(FileException);
    virtual size_t flush() throw(FileException);

    uint32_t getLastModified() throw();

    static void copyFile(const string& src, const string& target) throw(FileException);
    static void renameFile(const string& source, const string& target) throw(FileException);
    static void deleteFile(const string& aFileName) throw();

    static int64_t getSize(const string& aFileName) throw();

    static void ensureDirectory(const string& aFile) throw();
    static bool isAbsolute(const string& path) throw();

    virtual ~File() throw() { File::close(); }

    string read(size_t len) throw(FileException);
    string read() throw(FileException);
    void write(const string& aString) throw(FileException) { write((void*)aString.data(), aString.size()); }
    static StringList findFiles(const string& path, const string& pattern);

protected:
#ifdef _WIN32
    HANDLE h;
#else
    int h;
#endif
private:
    File(const File&);
    File& operator=(const File&);
};

class FileFindIter {
public:
        /** End iterator constructor */
        FileFindIter();
        /** Begin iterator constructor, path in utf-8 */
        FileFindIter(const string& path);

        ~FileFindIter();

        FileFindIter& operator++();
        bool operator!=(const FileFindIter& rhs) const;

        struct DirData
#ifdef _WIN32
                : public WIN32_FIND_DATA
#endif
        {
                DirData();

                string getFileName();
                bool isDirectory();
                bool isHidden();
                bool isLink();
                int64_t getSize();
                uint32_t getLastWriteTime();
#ifndef _WIN32
                dirent* ent;
                string base;
#endif
        };

        DirData& operator*() { return data; }
        DirData* operator->() { return &data; }

private:
#ifdef _WIN32
        HANDLE handle;
#else
        DIR* dir;
#endif

        DirData data;
};

} // namespace dcpp

#endif // !defined(FILE_H)
