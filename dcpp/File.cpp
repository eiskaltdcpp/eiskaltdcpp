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

#include "File.h"

namespace dcpp {

#ifdef _WIN32
File::File(const string& aFileName, int access, int mode) throw(FileException) {
    dcassert(access == WRITE || access == READ || access == (READ | WRITE));

    int m = 0;
    if(mode & OPEN) {
        if(mode & CREATE) {
            m = (mode & TRUNCATE) ? CREATE_ALWAYS : OPEN_ALWAYS;
        } else {
            m = (mode & TRUNCATE) ? TRUNCATE_EXISTING : OPEN_EXISTING;
        }
    } else {
        if(mode & CREATE) {
            m = (mode & TRUNCATE) ? CREATE_ALWAYS : CREATE_NEW;
        } else {
            dcassert(0);
        }
    }
    DWORD shared = FILE_SHARE_READ | (mode & SHARED ? FILE_SHARE_WRITE : 0);

    h = ::CreateFile(Text::toT(aFileName).c_str(), access, shared, NULL, m, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if(h == INVALID_HANDLE_VALUE) {
        throw FileException(Util::translateError(GetLastError()));
    }
}

uint32_t File::getLastModified() throw() {
    FILETIME f = {0};
    ::GetFileTime(h, NULL, NULL, &f);
    return convertTime(&f);
}

uint32_t File::convertTime(FILETIME* f) {
    SYSTEMTIME s = { 1970, 1, 0, 1, 0, 0, 0, 0 };
    FILETIME f2 = {0};
    if(::SystemTimeToFileTime(&s, &f2)) {
        ULARGE_INTEGER a,b;
        a.LowPart =f->dwLowDateTime;
        a.HighPart=f->dwHighDateTime;
        b.LowPart =f2.dwLowDateTime;
        b.HighPart=f2.dwHighDateTime;
        return (a.QuadPart - b.QuadPart) / (10000000LL); // 100ns > s
    }
    return 0;
}

bool File::isOpen() throw() {
    return h != INVALID_HANDLE_VALUE;
}

void File::close() throw() {
    if(isOpen()) {
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
    }
}

int64_t File::getSize() throw() {
    DWORD x;
    DWORD l = ::GetFileSize(h, &x);

    if( (l == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR))
        return -1;

    return (int64_t)l | ((int64_t)x)<<32;
}
int64_t File::getPos() throw() {
    LONG x = 0;
    DWORD l = ::SetFilePointer(h, 0, &x, FILE_CURRENT);

    return (int64_t)l | ((int64_t)x)<<32;
}

void File::setSize(int64_t newSize) throw(FileException) {
    int64_t pos = getPos();
    setPos(newSize);
    setEOF();
    setPos(pos);
}
void File::setPos(int64_t pos) throw() {
    LONG x = (LONG) (pos>>32);
    ::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_BEGIN);
}
void File::setEndPos(int64_t pos) throw() {
    LONG x = (LONG) (pos>>32);
    ::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_END);
}

void File::movePos(int64_t pos) throw() {
    LONG x = (LONG) (pos>>32);
    ::SetFilePointer(h, (DWORD)(pos & 0xffffffff), &x, FILE_CURRENT);
}

size_t File::read(void* buf, size_t& len) throw(FileException) {
    DWORD x;
    if(!::ReadFile(h, buf, (DWORD)len, &x, NULL)) {
        throw(FileException(Util::translateError(GetLastError())));
    }
    len = x;
    return x;
}

size_t File::write(const void* buf, size_t len) throw(FileException) {
    DWORD x;
    if(!::WriteFile(h, buf, (DWORD)len, &x, NULL)) {
        throw FileException(Util::translateError(GetLastError()));
    }
    dcassert(x == len);
    return x;
}
void File::setEOF() throw(FileException) {
    dcassert(isOpen());
    if(!SetEndOfFile(h)) {
        throw FileException(Util::translateError(GetLastError()));
    }
}

size_t File::flush() throw(FileException) {
    if(isOpen() && !FlushFileBuffers(h))
        throw FileException(Util::translateError(GetLastError()));
    return 0;
}

void File::renameFile(const string& source, const string& target) throw(FileException) {
    if(!::MoveFile(Text::toT(source).c_str(), Text::toT(target).c_str())) {
        // Can't move, try copy/delete...
        copyFile(source, target);
        deleteFile(source);
    }
}

void File::copyFile(const string& src, const string& target) throw(FileException) {
    if(!::CopyFile(Text::toT(src).c_str(), Text::toT(target).c_str(), FALSE)) {
        throw FileException(Util::translateError(GetLastError()));
    }
}

void File::deleteFile(const string& aFileName) throw()
{
    ::DeleteFile(Text::toT(aFileName).c_str());
}

int64_t File::getSize(const string& aFileName) throw() {
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    hFind = FindFirstFile(Text::toT(aFileName).c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        return -1;
    } else {
        FindClose(hFind);
        return ((int64_t)fd.nFileSizeHigh << 32 | (int64_t)fd.nFileSizeLow);
    }
}

void File::ensureDirectory(const string& aFile) throw() {
    // Skip the first dir...
    tstring file;
    Text::toT(aFile, file);
    tstring::size_type start = file.find_first_of(_T("\\/"));
    if(start == string::npos)
        return;
    start++;
    while( (start = file.find_first_of(_T("\\/"), start)) != string::npos) {
        ::CreateDirectory(file.substr(0, start+1).c_str(), NULL);
        start++;
    }
}

bool File::isAbsolute(const string& path) throw() {
    return path.size() > 2 && (path[1] == ':' || path[0] == '/' || path[0] == '\\');
}

#else // !_WIN32

File::File(const string& aFileName, int access, int mode) throw(FileException) {
    dcassert(access == WRITE || access == READ || access == (READ | WRITE));

    int m = 0;
    if(access == READ)
        m |= O_RDONLY;
    else if(access == WRITE)
        m |= O_WRONLY;
    else
        m |= O_RDWR;

    if(mode & CREATE) {
        m |= O_CREAT;
    }
    if(mode & TRUNCATE) {
        m |= O_TRUNC;
    }

    string filename = Text::fromUtf8(aFileName);

    struct stat s;
    if(lstat(filename.c_str(), &s) != -1) {
        if(!S_ISREG(s.st_mode) && !S_ISLNK(s.st_mode))
            throw FileException("Invalid file type");
    }

    h = open(filename.c_str(), m, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if(h == -1)
        throw FileException(Util::translateError(errno));
}

uint32_t File::getLastModified() throw() {
    struct stat s;
    if (::fstat(h, &s) == -1)
        return 0;

    return (uint32_t)s.st_mtime;
}

bool File::isOpen() throw() {
    return h != -1;
}

void File::close() throw() {
    if(h != -1) {
        ::close(h);
        h = -1;
    }
}

int64_t File::getSize() throw() {
    struct stat s;
    if(::fstat(h, &s) == -1)
        return -1;

    return (int64_t)s.st_size;
}

int64_t File::getPos() throw() {
    return (int64_t)lseek(h, 0, SEEK_CUR);
}

void File::setPos(int64_t pos) throw() {
    lseek(h, (off_t)pos, SEEK_SET);
}

void File::setEndPos(int64_t pos) throw() {
    lseek(h, (off_t)pos, SEEK_END);
}

void File::movePos(int64_t pos) throw() {
    lseek(h, (off_t)pos, SEEK_CUR);
}

size_t File::read(void* buf, size_t& len) throw(FileException) {
    ssize_t result = ::read(h, buf, len);
    if (result == -1) {
        throw FileException(Util::translateError(errno));
    }
    len = result;
    return (size_t)result;
}

size_t File::write(const void* buf, size_t len) throw(FileException) {
    ssize_t result;
    char* pointer = (char*)buf;
    ssize_t left = len;

    while (left > 0) {
        result = ::write(h, pointer, left);
        if (result == -1) {
            if (errno != EINTR) {
                throw FileException(Util::translateError(errno));
            }
        } else {
            pointer += result;
            left -= result;
        }
    }
    return len;
}

// some ftruncate implementations can't extend files like SetEndOfFile,
// not sure if the client code needs this...
int File::extendFile(int64_t len) throw() {
    char zero;

    if( (lseek(h, (off_t)len, SEEK_SET) != -1) && (::write(h, &zero,1) != -1) ) {
                ftruncate(h,(off_t)len);
        return 1;
    }
    return -1;
}

void File::setEOF() throw(FileException) {
    int64_t pos;
    int64_t eof;
    int ret;

    pos = (int64_t)lseek(h, 0, SEEK_CUR);
    eof = (int64_t)lseek(h, 0, SEEK_END);
    if (eof < pos)
        ret = extendFile(pos);
    else
        ret = ftruncate(h, (off_t)pos);
    lseek(h, (off_t)pos, SEEK_SET);
    if (ret == -1)
        throw FileException(Util::translateError(errno));
}

void File::setSize(int64_t newSize) throw(FileException) {
    int64_t pos = getPos();
    setPos(newSize);
    setEOF();
    setPos(pos);
}

size_t File::flush() throw(FileException) {
    if(isOpen() && fsync(h) == -1)
        throw FileException(Util::translateError(errno));
    return 0;
}

/**
 * ::rename seems to have problems when source and target is on different partitions
 * from "man 2 rename":
 * EXDEV oldpath and newpath are not on the same mounted filesystem. (Linux permits a
 * filesystem to be mounted at multiple points, but rename(2) does not
 * work across different mount points, even if the same filesystem is mounted on both.)
*/
void File::renameFile(const string& source, const string& target) throw(FileException) {
    int ret = ::rename(Text::fromUtf8(source).c_str(), Text::fromUtf8(target).c_str());
    if(ret != 0 && errno == EXDEV) {
        copyFile(source, target);
        deleteFile(source);
    } else if(ret != 0)
        throw FileException(source + Util::translateError(errno));
}

// This doesn't assume all bytes are written in one write call, it is a bit safer
void File::copyFile(const string& source, const string& target) throw(FileException) {
    const size_t BUF_SIZE = 64 * 1024;
    boost::scoped_array<char> buffer(new char[BUF_SIZE]);
    size_t count = BUF_SIZE;
    File src(source, File::READ, 0);
    File dst(target, File::WRITE, File::CREATE | File::TRUNCATE);

    while(src.read(&buffer[0], count) > 0) {
        char* p = &buffer[0];
        while(count > 0) {
            size_t ret = dst.write(p, count);
            p += ret;
            count -= ret;
        }
        count = BUF_SIZE;
    }
}

void File::deleteFile(const string& aFileName) throw() {
    ::unlink(Text::fromUtf8(aFileName).c_str());
}

int64_t File::getSize(const string& aFileName) throw() {
    struct stat s;
    if(stat(Text::fromUtf8(aFileName).c_str(), &s) == -1)
        return -1;

    return s.st_size;
}

void File::ensureDirectory(const string& aFile) throw() {
    string file = Text::fromUtf8(aFile);
    string::size_type start = 0;
    while( (start = file.find_first_of('/', start)) != string::npos) {
        mkdir(file.substr(0, start+1).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        start++;
    }
}

bool File::isAbsolute(const string& path) throw() {
    return path.size() > 1 && path[0] == '/';
}

#endif // !_WIN32

string File::read(size_t len) throw(FileException) {
    string s(len, 0);
    size_t x = read(&s[0], len);
    if(x != len)
        s.resize(x);
    return s;
}

string File::read() throw(FileException) {
    setPos(0);
    int64_t sz = getSize();
    if(sz == -1)
        return Util::emptyString;
    return read((uint32_t)sz);
}

StringList File::findFiles(const string& path, const string& pattern) {
    StringList ret;

#ifdef _WIN32
    WIN32_FIND_DATA data;
    HANDLE hFind;

    hFind = ::FindFirstFile(Text::toT(path + pattern).c_str(), &data);
    if(hFind != INVALID_HANDLE_VALUE) {
        do {
            const char* extra = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "\\" : "";
            ret.push_back(path + Text::fromT(data.cFileName) + extra);
        } while(::FindNextFile(hFind, &data));

        ::FindClose(hFind);
    }
#else
    DIR* dir = opendir(Text::fromUtf8(path).c_str());
    if (dir) {
        while (struct dirent* ent = readdir(dir)) {
            if (fnmatch(pattern.c_str(), ent->d_name, 0) == 0) {

            //NOTE: dcplusplus revision 2126 compilation fixes for OpenSolaris
            //struct stat s;
            //stat(ent->d_name, &s);
            //const char* extra = (s.st_mode & S_IFDIR) ? "/" : "";
            //ret.push_back(path + Text::toUtf8(ent->d_name) + extra);

            //NOTE: freedcpp, fix error file path
            struct stat s;
            if (stat((Text::fromUtf8(path) + PATH_SEPARATOR + ent->d_name).c_str(), &s) != -1)
            {
                                        const char* extra = S_ISDIR(s.st_mode) ? "/" : "";
                ret.push_back(path + Text::toUtf8(ent->d_name) + extra);
            }
        }
                }
        closedir(dir);
    }
#endif

    return ret;
}

#ifdef _WIN32

FileFindIter::FileFindIter() : handle(INVALID_HANDLE_VALUE) { }

FileFindIter::FileFindIter(const string& path) : handle(INVALID_HANDLE_VALUE) {
        handle = ::FindFirstFile(Text::toT(path).c_str(), &data);
}

FileFindIter::~FileFindIter() {
        if(handle != INVALID_HANDLE_VALUE) {
                ::FindClose(handle);
        }
}

FileFindIter& FileFindIter::operator++() {
        if(!::FindNextFile(handle, &data)) {
                ::FindClose(handle);
                handle = INVALID_HANDLE_VALUE;
        }
        return *this;
}

bool FileFindIter::operator!=(const FileFindIter& rhs) const { return handle != rhs.handle; }

FileFindIter::DirData::DirData() { }

string FileFindIter::DirData::getFileName() {
        return Text::fromT(cFileName);
}

bool FileFindIter::DirData::isDirectory() {
        return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
}

bool FileFindIter::DirData::isHidden() {
        return ((dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (cFileName[0] == L'.'));
}

bool FileFindIter::DirData::isLink() {
        return false;
}

int64_t FileFindIter::DirData::getSize() {
        return (int64_t)nFileSizeLow | ((int64_t)nFileSizeHigh)<<32;
}

uint32_t FileFindIter::DirData::getLastWriteTime() {
        return File::convertTime(&ftLastWriteTime);
}

#else // !_WIN32

FileFindIter::FileFindIter() {
        dir = NULL;
        data.ent = NULL;
}

FileFindIter::FileFindIter(const string& path) {

//      string filename = Text::fromUtf8(path);
//      dir = opendir(filename.c_str());
        string base = Text::fromUtf8(path);//NOTE: freedcpp
        dir = opendir(base.c_str());//NOTE: freedcpp
        if (!dir)
                return;
//      data.base = filename;
        data.base = base;//NOTE: freedcpp
        data.ent = readdir(dir);
        if (!data.ent) {
                closedir(dir);
                dir = NULL;
        }
}

FileFindIter::~FileFindIter() {
        if (dir) closedir(dir);
}

FileFindIter& FileFindIter::operator++() {
        if (!dir)
                return *this;
        data.ent = readdir(dir);
        if (!data.ent) {
                closedir(dir);
                dir = NULL;
        }
        return *this;
}

bool FileFindIter::operator!=(const FileFindIter& rhs) const {
        // good enough to to say if it's null
        return dir != rhs.dir;
}

FileFindIter::DirData::DirData() : ent(NULL) {}

string FileFindIter::DirData::getFileName() {
        if (!ent) return Util::emptyString;
        return Text::toUtf8(ent->d_name);
}

bool FileFindIter::DirData::isDirectory() {
        struct stat inode;
        if (!ent) return false;
        if (stat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return false;
        return S_ISDIR(inode.st_mode);
}

bool FileFindIter::DirData::isHidden() {
        if (!ent) return false;
        return (ent->d_name[0] == '.');//NOTE: freedcpp, see core 0.750
}

bool FileFindIter::DirData::isLink() {
        struct stat inode;
        if (!ent) return false;
        if (lstat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return false;
        return S_ISLNK(inode.st_mode);
}

int64_t FileFindIter::DirData::getSize() {
        struct stat inode;
        if (!ent) return false;
        if (stat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return 0;
        return inode.st_size;
}

uint32_t FileFindIter::DirData::getLastWriteTime() {
        struct stat inode;
        if (!ent) return false;
        if (stat((base + PATH_SEPARATOR + ent->d_name).c_str(), &inode) == -1) return 0;
        return inode.st_mtime;
}

#endif // !_WIN32

} // namespace dcpp
