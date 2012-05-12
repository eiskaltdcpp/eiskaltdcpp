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

#ifndef DCPLUSPLUS_DCPP_FILTERED_FILE_H
#define DCPLUSPLUS_DCPP_FILTERED_FILE_H

#include <boost/scoped_array.hpp>

#include "Streams.h"

#include "Exception.h"

namespace dcpp {

template<bool managed>
class CountOutputStream : public OutputStream {
public:
    using OutputStream::write;
    CountOutputStream(OutputStream* aStream) : s(aStream), count(0) { }
    virtual ~CountOutputStream() { if(managed) delete s; }

        size_t flush() {
        size_t n = s->flush();
        count += n;
        return n;
    }
        size_t write(const void* buf, size_t len) {
        size_t n = s->write(buf, len);
        count += n;
        return n;
    }

    int64_t getCount() const { return count; }
private:
    OutputStream* s;
    int64_t count;
};

template<class Filter, bool managed>
class CalcOutputStream : public OutputStream {
public:
    using OutputStream::write;

    CalcOutputStream(OutputStream* aStream) : s(aStream) { }
    virtual ~CalcOutputStream() { if(managed) delete s; }

        size_t flush() {
        return s->flush();
    }

        size_t write(const void* buf, size_t len) {
        filter(buf, len);
        return s->write(buf, len);
    }

    const Filter& getFilter() const { return filter; }
    Filter& getFilter() { return filter; }
private:
    OutputStream* s;
    Filter filter;
};

template<class Filter, bool managed>
class CalcInputStream : public InputStream {
public:
    CalcInputStream(InputStream* aStream) : s(aStream) { }
    virtual ~CalcInputStream() { if(managed) delete s; }

        size_t read(void* buf, size_t& len) {
        size_t x = s->read(buf, len);
        filter(buf, x);
        return x;
    }

    const Filter& getFilter() const { return filter; }
private:
    InputStream* s;
    Filter filter;
};

template<class Filter, bool manage>
class FilteredOutputStream : public OutputStream {
public:
    using OutputStream::write;

    FilteredOutputStream(OutputStream* aFile) : f(aFile), buf(new uint8_t[BUF_SIZE]), flushed(false), more(true) { }
    virtual ~FilteredOutputStream() { if(manage) delete f; }

        size_t flush() {
        if(flushed)
            return 0;

        flushed = true;
        size_t written = 0;

        for(;;) {
            size_t n = BUF_SIZE;
            size_t zero = 0;
            more = filter(NULL, zero, &buf[0], n);

            written += f->write(&buf[0], n);

            if(!more)
                break;
        }
        return written + f->flush();
    }

        size_t write(const void* wbuf, size_t len) {
        if(flushed)
            throw Exception("No filtered writes after flush");

        uint8_t* wb = (uint8_t*)wbuf;
        size_t written = 0;
        while(len > 0) {
            size_t n = BUF_SIZE;
            size_t m = len;

            more = filter(wb, m, &buf[0], n);
            wb += m;
            len -= m;

            written += f->write(&buf[0], n);

            if(!more) {
                if(len > 0) {
                    throw Exception("Garbage data after end of stream");
                }
                return written;
            }
        }
        return written;
    }

    virtual bool eof() { return !more; }

private:
    static const size_t BUF_SIZE = 64*1024;

    OutputStream* f;
    Filter filter;

    boost::scoped_array<uint8_t> buf;
    bool flushed;
    bool more;
};

template<class Filter, bool managed>
class FilteredInputStream : public InputStream {
public:
    FilteredInputStream(InputStream* aFile) : f(aFile), buf(new uint8_t[BUF_SIZE]), pos(0), valid(0), more(true) { }
        virtual ~FilteredInputStream() { if(managed) delete f; }

    /**
    * Read data through filter, keep calling until len returns 0.
    * @param rbuf Data buffer
    * @param len Buffer size on entry, bytes actually read on exit
    * @return Length of data in buffer
    */
    size_t read(void* rbuf, size_t& len) {
        uint8_t* rb = (uint8_t*)rbuf;

        size_t totalRead = 0;
        size_t totalProduced = 0;

        while(more && totalProduced < len) {
            size_t curRead = BUF_SIZE;
            if(valid == 0) {
                dcassert(pos == 0);
                valid = f->read(&buf[0], curRead);
                totalRead += curRead;
            }

            size_t n = len - totalProduced;
            size_t m = valid - pos;
            more = filter(&buf[pos], m, rb, n);
            pos += m;
            if(pos == valid) {
                valid = pos = 0;
            }
            totalProduced += n;
            rb += n;
        }
        len = totalRead;
        return totalProduced;
    }

private:
    static const size_t BUF_SIZE = 64*1024;

    InputStream* f;
    Filter filter;
    boost::scoped_array<uint8_t> buf;
    size_t pos;
    size_t valid;
    bool more;
};

} // namespace dcpp

#endif // !defined(FILTERED_FILE_H)
