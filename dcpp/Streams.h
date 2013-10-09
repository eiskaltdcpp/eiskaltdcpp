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

#include <algorithm>
#include <boost/noncopyable.hpp>

#include "typedefs.h"
#include "format.h"
#include "SettingsManager.h"
#include "Exception.h"

namespace dcpp {

using std::min;

STANDARD_EXCEPTION(FileException);

/**
 * A simple output stream. Intended to be used for nesting streams one inside the other.
 */
class OutputStream : boost::noncopyable {
public:
    OutputStream() { }
    virtual ~OutputStream() { }

    /**
     * @return The actual number of bytes written. len bytes will always be
     *         consumed, but fewer or more bytes may actually be written,
     *         for example if the stream is being compressed.
     */
    virtual size_t write(const void* buf, size_t len) = 0;
    /**
     * This must be called before destroying the object to make sure all data
     * is properly written (we don't want destructors that throw exceptions
     * and the last flush might actually throw). Note that some implementations
     * might not need it...
     */
    virtual size_t flush() = 0;

    /**
     * @return True if stream is at expected end
     */
    virtual bool eof() { return false; }

    size_t write(const string& str) { return write(str.c_str(), str.size()); }
};

class InputStream : boost::noncopyable {
public:
    InputStream() { }
    virtual ~InputStream() { }
    /**
     * Call this function until it returns 0 to get all bytes.
     * @return The number of bytes read. len reflects the number of bytes
     *         actually read from the stream source in this call.
     */
    virtual size_t read(void* buf, size_t& len) = 0;
};

class MemoryInputStream : public InputStream {
public:
    MemoryInputStream(const uint8_t* src, size_t len) : pos(0), size(len), buf(new uint8_t[len]) {
        memcpy(buf, src, len);
    }
    MemoryInputStream(const string& src) : pos(0), size(src.size()), buf(new uint8_t[src.size()]) {
        memcpy(buf, src.data(), src.size());
    }

        virtual ~MemoryInputStream() {
        delete[] buf;
    }

    virtual size_t read(void* tgt, size_t& len) {
        len = min(len, size - pos);
        memcpy(tgt, buf+pos, len);
        pos += len;
        return len;
    }

    size_t getSize() { return size; }

private:
    size_t pos;
    size_t size;
    uint8_t* buf;
};

class IOStream : public InputStream, public OutputStream {
};

/** Count how many bytes have been read. */
template<bool managed>
class CountedInputStream : public InputStream {
public:
    CountedInputStream(InputStream* is) : s(is), readBytes(0) { }
    virtual ~CountedInputStream() { if(managed) delete s; }

    size_t read(void* buf, size_t& len) {
        auto ret = s->read(buf, len);
        readBytes += len;
        return ret;
    }

    uint64_t getReadBytes() const { return readBytes; }

private:
    InputStream* s;
    uint64_t readBytes;
};

template<bool managed>
class LimitedInputStream : public InputStream {
public:
    LimitedInputStream(InputStream* is, uint64_t aMaxBytes) : s(is), maxBytes(aMaxBytes) {  }
    virtual ~LimitedInputStream() { if(managed) delete s; }

    size_t read(void* buf, size_t& len) {
        len = (size_t)min(maxBytes, (uint64_t)len);
        if(len == 0)
                return 0;
        size_t x = s->read(buf, len);
        maxBytes -= x;
        return x;
    }
private:
    InputStream* s;
    uint64_t maxBytes;
};

/** Limits the number of bytes that are requested to be written (not the number actually written!) */
template<bool managed>
class LimitedOutputStream : public OutputStream {
public:
    LimitedOutputStream(OutputStream* os, uint64_t aMaxBytes) : s(os), maxBytes(aMaxBytes) {        }
    virtual ~LimitedOutputStream() { if(managed) delete s; }

    virtual size_t write(const void* buf, size_t len) {
        if(maxBytes < len) {
            throw FileException(dgettext("libeiskaltdcpp", "More bytes written than requested"));
        }
        maxBytes -= len;
        return s->write(buf, len);
    }

    virtual size_t flush() {
        return s->flush();
    }

    virtual bool eof() { return maxBytes == 0; }
private:
    OutputStream* s;
    uint64_t maxBytes;
};

template<bool managed>
class BufferedOutputStream : public OutputStream {
public:
    using OutputStream::write;

    BufferedOutputStream(OutputStream* aStream, size_t aBufSize = SETTING(BUFFER_SIZE) * 1024) : s(aStream), pos(0), buf(aBufSize) { }
    virtual ~BufferedOutputStream() {
        try {
            // We must do this in order not to lose bytes when a download
            // is disconnected prematurely
            flush();
        } catch(const Exception&) { }
        if(managed) delete s;
    }

    virtual size_t flush() {
        if(pos > 0)
            s->write(&buf[0], pos);
        pos = 0;
        s->flush();
        return 0;
    }

    virtual size_t write(const void* wbuf, size_t len) {
        uint8_t* b = (uint8_t*)wbuf;
        size_t l2 = len;
        size_t bufSize = buf.size();
        while(len > 0) {
            if(pos == 0 && len >= bufSize) {
                s->write(b, len);
                break;
            } else {
                size_t n = min(bufSize - pos, len);
                memcpy(&buf[pos], b, n);
                b += n;
                pos += n;
                len -= n;
                if(pos == bufSize) {
                    s->write(&buf[0], bufSize);
                    pos = 0;
                }
            }
        }
        return l2;
    }
private:
    OutputStream* s;
    size_t pos;
    ByteVector buf;
};

class StringOutputStream : public OutputStream {
public:
    StringOutputStream() { }
    virtual ~StringOutputStream() { }
    using OutputStream::write;

    virtual size_t flush() { return 0; }
    virtual size_t write(const void* buf, size_t len) {
        str.append(reinterpret_cast<const char*>(buf), len);
        return len;
    }

    string getString() { return move(str); }
    string& stringRef() { return str; }

private:
        string str;
};

class StringRefOutputStream : public OutputStream {
public:
    StringRefOutputStream(string& out) : str(out) { }
    virtual ~StringRefOutputStream() { }
    using OutputStream::write;

    virtual size_t flush() { return 0; }
    virtual size_t write(const void* buf, size_t len) {
        str.append(reinterpret_cast<const char*>(buf), len);
        return len;
    }

private:
    string& str;
};

} // namespace dcpp
