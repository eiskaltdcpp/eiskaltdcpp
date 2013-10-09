/*
 * Copyright (C) 2001-2013 Jacek Sieka, arnetheduck on gmail point com
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

#include "FileReader.h"

#include "debug.h"
#include "File.h"
#include "Text.h"
#include "Util.h"
#include "TimerManager.h"

namespace dcpp {

using std::make_pair;
using std::swap;

namespace {
static const size_t READ_FAILED = static_cast<size_t>(-1);
}

size_t FileReader::read(const string& file, const DataCallback& callback) {
        size_t ret = READ_FAILED;

        if(direct) {
                dcdebug("Reading [overlapped] %s\n", file.c_str());
                ret = readDirect(file, callback);
        }

        if(ret == READ_FAILED) {
                dcdebug("Reading [mapped] %s\n", file.c_str());
                ret = readMapped(file, callback);

                if(ret == READ_FAILED) {
                        dcdebug("Reading [full] %s\n", file.c_str());
                        ret = readCached(file, callback);
                }
        }

        return ret;
}


/** Read entire file, never returns READ_FAILED */
size_t FileReader::readCached(const string& file, const DataCallback& callback) {
        buffer.resize(getBlockSize(0));

        auto buf = &buffer[0];
        File f(file, File::READ, File::OPEN | File::SHARED);

        size_t total = 0;
        size_t n = buffer.size();
        bool go = true;
        while(f.read(buf, n) > 0 && go) {
                go = callback(buf, n);
                total += n;
                n = buffer.size();
        }

        return total;
}

size_t FileReader::getBlockSize(size_t alignment) {
        auto block = blockSize < DEFAULT_BLOCK_SIZE ? DEFAULT_BLOCK_SIZE : blockSize;
        if(alignment > 0) {
                block = ((block + alignment - 1) / alignment) * alignment;
        }

        return block;
}

void* FileReader::align(void *buf, size_t alignment) {
        return alignment == 0 ? buf
                : reinterpret_cast<void*>(((reinterpret_cast<size_t>(buf) + alignment - 1) / alignment) * alignment);
}

#ifdef _WIN32

struct Handle : boost::noncopyable {
        Handle(HANDLE h) : h(h) { }
        ~Handle() { ::CloseHandle(h); }

        operator HANDLE() { return h; }

        HANDLE h;
};

size_t FileReader::readDirect(const string& file, const DataCallback& callback) {
        DWORD sector = 0, y;

        auto tfile = Text::toT(file);

        if (!::GetDiskFreeSpace(Util::getFilePath(tfile).c_str(), &y, &sector, &y, &y)) {
                dcdebug("Failed to get sector size: %s\n", Util::translateError(::GetLastError()).c_str());
                return READ_FAILED;
        }

        auto tmp = ::CreateFile(tfile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED | FILE_FLAG_POSIX_SEMANTICS, nullptr);

        if (tmp == INVALID_HANDLE_VALUE) {
                dcdebug("Failed to open unbuffered file: %s\n", Util::translateError(::GetLastError()).c_str());
                return READ_FAILED;
        }

        Handle h(tmp);

        DWORD bufSize = static_cast<DWORD>(getBlockSize(sector));
        buffer.resize(bufSize * 2 + sector);

        auto buf = align(&buffer[0], sector);

        DWORD hn = 0;
        DWORD rn = 0;
        uint8_t* hbuf = static_cast<uint8_t*>(buf) + bufSize;
        uint8_t* rbuf = static_cast<uint8_t*>(buf);
        OVERLAPPED over = { 0 };

        // Read the first block
        auto res = ::ReadFile(h, hbuf, bufSize, NULL, &over);
        auto err = ::GetLastError();

        if(!res && err != ERROR_IO_PENDING) {
                if(err != ERROR_HANDLE_EOF) {
                        dcdebug("First overlapped read failed: %s\n", Util::translateError(::GetLastError()).c_str());
                        return READ_FAILED;
                }

                return 0;
        }

        // Finish the read and see how it went
        if(!GetOverlappedResult(h, &over, &hn, TRUE)) {
                err = ::GetLastError();
                if(err != ERROR_HANDLE_EOF) {
                        dcdebug("First overlapped read failed: %s\n", Util::translateError(::GetLastError()).c_str());
                        return READ_FAILED;
                }
        }
        over.Offset = hn;

        bool go = true;
        for (; hn == bufSize && go;) {
                // Start a new overlapped read
                res = ::ReadFile(h, rbuf, bufSize, NULL, &over);
                auto err = ::GetLastError();

                // Process the previously read data
                go = callback(hbuf, hn);

                if (!res && err != ERROR_IO_PENDING) {
                        if(err != ERROR_HANDLE_EOF) {
                                throw FileException(Util::translateError(err));
                        }

                        rn = 0;
                } else {
                        // Finish the new read
                        if (!GetOverlappedResult(h, &over, &rn, TRUE)) {
                                err = ::GetLastError();
                                if(err != ERROR_HANDLE_EOF) {
                                        throw FileException(Util::translateError(err));
                                }

                                rn = 0;
                        }
                }

                *((uint64_t*)&over.Offset) += rn;

                swap(rbuf, hbuf);
                swap(rn, hn);
        }

        if(hn != 0) {
                // Process leftovers
                callback(hbuf, hn);
        }

        return *((uint64_t*)&over.Offset);
}

size_t FileReader::readMapped(const string& file, const DataCallback& callback) {
        /** @todo mapped reads can fail on Windows by throwing an exception that may only be caught by
        SEH. MinGW doesn't have that, thus making this method of reading prone to unrecoverable
        failures. disabling this for now should be fine as DC++ always tries overlapped reads first
        (at the moment this file reader is only used in places where overlapped reads make the most
        sense).
        more info:
        <http://msdn.microsoft.com/en-us/library/aa366801(VS.85).aspx>
        <http://stackoverflow.com/q/7244645> */
#if 1
        return READ_FAILED;
#else

        auto tfile = Text::toT(file);

        auto tmp = ::CreateFile(tfile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

        if (tmp == INVALID_HANDLE_VALUE) {
                dcdebug("Failed to open unbuffered file: %s\n", Util::translateError(::GetLastError()).c_str());
                return READ_FAILED;
        }

        Handle h(tmp);

        LARGE_INTEGER size = { 0 };
        if(!::GetFileSizeEx(h, &size)) {
                dcdebug("Couldn't get file size: %s\n", Util::translateError(::GetLastError()).c_str());
                return READ_FAILED;
        }

        if(!(tmp = ::CreateFileMapping(h, NULL, PAGE_READONLY, 0, 0, NULL))) {
                dcdebug("Couldn't create file mapping: %s\n", Util::translateError(::GetLastError()).c_str());
                return READ_FAILED;
        }

        Handle hmap(tmp);

        SYSTEM_INFO si = { 0 };
        ::GetSystemInfo(&si);

        auto blockSize = getBlockSize(si.dwPageSize);

        LARGE_INTEGER total = { 0 };
        bool go = true;
        while(size.QuadPart > 0 && go) {
                auto n = min(size.QuadPart, (int64_t)blockSize);
                auto p = ::MapViewOfFile(hmap, FILE_MAP_READ, total.HighPart, total.LowPart, static_cast<DWORD>(n));
                if(!p) {
                        throw FileException(Util::translateError(::GetLastError()));
                }

                go = callback(p, n);

                if(!::UnmapViewOfFile(p)) {
                        throw FileException(Util::translateError(::GetLastError()));
                }

                size.QuadPart -= n;
                total.QuadPart += n;
        }

        return total.QuadPart;
#endif
}

#else

#include <sys/mman.h> // mmap, munmap, madvise
#include <signal.h>  // for handling read errors from previous trio
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


size_t FileReader::readDirect(const string& file, const DataCallback& callback) {
        return READ_FAILED;
}

static const int64_t BUF_SIZE = 0x1000000 - (0x1000000 % getpagesize());
static sigjmp_buf sb_env;

static void sigbus_handler(int signum, siginfo_t* info, void* context) {
        // Jump back to the readMapped which will return error. Apparently truncating
        // a file in Solaris sets si_code to BUS_OBJERR
        if (signum == SIGBUS && (info->si_code == BUS_ADRERR || info->si_code == BUS_OBJERR))
                siglongjmp(sb_env, 1);
}

size_t FileReader::readMapped(const string& filename, const DataCallback& callback) {
        int fd = open(Text::fromUtf8(filename).c_str(), O_RDONLY);
        if(fd == -1) {
                dcdebug("Error opening file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
                return READ_FAILED;
        }

        struct stat statbuf;
        if (fstat(fd, &statbuf) == -1) {
                dcdebug("Error opening file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
                close(fd);
                return READ_FAILED;
        }

        int64_t pos = 0;
        auto size = statbuf.st_size;

        // Prepare and setup a signal handler in case of SIGBUS during mmapped file reads.
        // SIGBUS can be sent when the file is truncated or in case of read errors.
        struct sigaction act, oldact;
        sigset_t signalset;

        sigemptyset(&signalset);

        act.sa_handler = NULL;
        act.sa_sigaction = sigbus_handler;
        act.sa_mask = signalset;
        act.sa_flags = SA_SIGINFO | SA_RESETHAND;

        if (sigaction(SIGBUS, &act, &oldact) == -1) {
                dcdebug("Failed to set signal handler for fastHash\n");
                close(fd);
                return READ_FAILED;     // Better luck with the slow hash.
        }

        void* buf = NULL;
        int64_t size_read = 0;

        uint64_t lastRead = GET_TICK();
        while (pos < size) {
                size_read = std::min(size - pos, BUF_SIZE);
                buf = mmap(0, size_read, PROT_READ, MAP_SHARED, fd, pos);
                if (buf == MAP_FAILED) {
                        dcdebug("Error calling mmap for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
                        break;
                }

                if (sigsetjmp(sb_env, 1)) {
                        dcdebug("Caught SIGBUS for file %s\n", filename.c_str());
                        break;
                }

                if (posix_madvise(buf, size_read, POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED) == -1) {
                        dcdebug("Error calling madvise for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
                        break;
                }

                if(!callback(buf, size_read)) {
                        break;
                }

                if (munmap(buf, size_read) == -1) {
                        dcdebug("Error calling munmap for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
                        break;
                }

                buf = NULL;
                pos += size_read;
        }

        if (buf != NULL && buf != MAP_FAILED && munmap(buf, size_read) == -1) {
                dcdebug("Error calling munmap for file %s: %s\n", filename.c_str(), Util::translateError(errno).c_str());
        }

        ::close(fd);

        if (sigaction(SIGBUS, &oldact, NULL) == -1) {
                dcdebug("Failed to reset old signal handler for SIGBUS\n");
        }

        return pos == size ? pos : READ_FAILED;
}

#endif
}
