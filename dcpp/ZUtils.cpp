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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdinc.h"
#include "ZUtils.h"

#include "Exception.h"
#include "File.h"
#include "format.h"

namespace dcpp {

using std::max;

const double ZFilter::MIN_COMPRESSION_LEVEL = 0.95;

ZFilter::ZFilter() : totalIn(0), totalOut(0), compressing(true) {
    memset(&zs, 0, sizeof(zs));

    if(deflateInit(&zs, 3) != Z_OK) {
        throw Exception(_("Error during compression"));
    }
}

ZFilter::~ZFilter() {
    deflateEnd(&zs);
}

bool ZFilter::operator()(const void* in, size_t& insize, void* out, size_t& outsize) {
    if(outsize == 0)
        return false;

    zs.next_in = (Bytef*)in;
    zs.next_out = (Bytef*)out;

    // Check if there's any use compressing; if not, save some cpu...
    if(compressing && insize > 0 && outsize > 16 && (totalIn > (64 * 1024)) &&
            (static_cast<double>(totalOut) / totalIn) > MIN_COMPRESSION_LEVEL)
    {
        zs.avail_in = 0;
        zs.avail_out = outsize;

        // Starting with zlib 1.2.9, the deflateParams API has changed.
        auto err = ::deflateParams(&zs, 0, Z_DEFAULT_STRATEGY);
#if ZLIB_VERNUM >= 0x1290
        if(err == Z_STREAM_ERROR) {
#else
        if(err != Z_OK) {
#endif
            throw Exception(_("Error during compression"));
        }

        zs.avail_in = insize;
        compressing = false;
        dcdebug("ZFilter: Dynamically disabled compression\n");

        // Check if we ate all space already...
#if ZLIB_VERNUM >= 0x1290
        if(err == Z_BUF_ERROR) {
#else
        if(zs.avail_out == 0) {
#endif
            outsize = outsize - zs.avail_out;
            insize = insize - zs.avail_in;
            totalOut += outsize;
            totalIn += insize;
            return true;
        }
    } else {
        zs.avail_in = insize;
        zs.avail_out = outsize;
    }

    if(insize == 0) {
        int err = ::deflate(&zs, Z_FINISH);
        if(err != Z_OK && err != Z_STREAM_END)
            throw Exception(_("Error during compression"));

        outsize = outsize - zs.avail_out;
        insize = insize - zs.avail_in;
        totalOut += outsize;
        totalIn += insize;
        return err == Z_OK;
    } else {
        int err = ::deflate(&zs, Z_NO_FLUSH);
        if(err != Z_OK)
            throw Exception(_("Error during compression"));

        outsize = outsize - zs.avail_out;
        insize = insize - zs.avail_in;
        totalOut += outsize;
        totalIn += insize;
        return true;
    }
}

UnZFilter::UnZFilter() {
    memset(&zs, 0, sizeof(zs));

    if(inflateInit(&zs) != Z_OK)
        throw Exception(_("Error during decompression"));
}

UnZFilter::~UnZFilter() {
    inflateEnd(&zs);
}

bool UnZFilter::operator()(const void* in, size_t& insize, void* out, size_t& outsize) {
    if(outsize == 0)
        return 0;

    zs.avail_in = insize;
    zs.next_in = (Bytef*)in;
    zs.avail_out = outsize;
    zs.next_out = (Bytef*)out;

    int err = ::inflate(&zs, Z_NO_FLUSH);

    // see zlib/contrib/minizip/unzip.c, Z_BUF_ERROR means we should have padded
    // with a dummy byte if at end of stream - since we don't do this it's not a real
    // error
    if(!(err == Z_OK || err == Z_STREAM_END || (err == Z_BUF_ERROR && in == NULL)))
        throw Exception(_("Error during decompression"));

    outsize = outsize - zs.avail_out;
    insize = insize - zs.avail_in;
    return err == Z_OK;
}

void GZ::decompress(const string& source, const string& target) {
    auto gz = gzopen(source.c_str(), "rb");
    if(!gz) {
        throw Exception(_("Error during decompression"));
    }
    File f(target, File::WRITE, File::CREATE | File::TRUNCATE);

    const size_t BUF_SIZE = 64 * 1024;
    const int BUF_SIZE_INT = static_cast<int>(BUF_SIZE);
    ByteVector buf(BUF_SIZE);

    while(true) {
        auto read = gzread(gz, &buf[0], BUF_SIZE);
        if(read > 0) {
            f.write(&buf[0], read);
        }
        if(read < BUF_SIZE_INT) {
            break;
        }
    }

    gzclose(gz);
}

} // namespace dcpp
