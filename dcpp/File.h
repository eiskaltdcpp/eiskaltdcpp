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

#pragma once

#include "Streams.h"

#ifdef _WIN32
#include "w.h"
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#endif

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
    int extendFile(int64_t len) noexcept;

#endif // !_WIN32

    File(const string& aFileName, int access, int mode);

    bool isOpen() noexcept;
    virtual void close() noexcept;
    virtual int64_t getSize() noexcept;
    virtual void setSize(int64_t newSize);

    virtual int64_t getPos() noexcept;
    virtual void setPos(int64_t pos) noexcept;
    virtual void setEndPos(int64_t pos) noexcept;
    virtual void movePos(int64_t pos) noexcept;
    virtual void setEOF();

    virtual size_t read(void* buf, size_t& len);
    virtual size_t write(const void* buf, size_t len);
    virtual size_t flush();

    uint32_t getLastModified() noexcept;

    static void copyFile(const string& src, const string& target);
    static void renameFile(const string& source, const string& target);
    static void deleteFile(const string& aFileName) noexcept;

    static int64_t getSize(const string& aFileName) noexcept;

    static void ensureDirectory(const string& aFile) noexcept;
    static bool isAbsolute(const string& path) noexcept;

    virtual ~File() { close(); }

    string read(size_t len);
    string read();
    void write(const string& aString) { write((void*)aString.data(), aString.size()); }
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
            : public WIN32_FIND_DATAW
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

#ifdef _WIN32
    // on Windows, prefer _wfopen over fopen.
    FILE* dcpp_fopen(const char* filename, const char* mode);
#else
#define dcpp_fopen fopen
#endif

} // namespace dcpp
