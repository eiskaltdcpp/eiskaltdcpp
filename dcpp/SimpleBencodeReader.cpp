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
#include "SimpleBencodeReader.h"

#include <cstdlib>

#include "Exception.h"
#include "debug.h"

namespace dcpp {

int64_t SimpleBencodeReader::readInt(const char **data)
{
        char *endp = 0;
        auto ret = strtoll(*data, &endp, 10);
        *data = endp;
        return ret;
}

string SimpleBencodeReader::readString(const char **data, const char *end)
{
        auto len = readInt(data);
        if(len < 0 || *data + len + 1 > end) {
                throw Exception("String too large");
        }

        if(**data != ':') {
                throw Exception("Expected :");
        }

        auto start = *data + 1;
        *data += len + 1;
        return string(start, start + len);
}

void SimpleBencodeReader::decode(const char **data, const char *end) {
        if (**data == 'l') {
                cb.startList();
                ++(*data);
                while (**data != 'e') {
                        decode(data, end);
                }

                ++(*data);
                cb.endList();
                return;
        } else if (**data == 'd') {
                ++(*data);
                while (**data != 'e') {
                        cb.startDictEntry(readString(data, end));
                        decode(data, end);
                        cb.endDictEntry();
                }
                ++(*data);

                return;
        } else if (**data == 'i') {
                ++(*data);
                cb.intValue(readInt(data));

                if (**data != 'e')
                        throw Exception("Expected 'e'");

                ++(*data);

                return;
        } else if (**data >= '0' && **data <= '9') {
                cb.stringValue(readString(data, end));
                return;
        } else {
                dcdebug("Unexpected %c after %c", **data, (*data)[-1]);
                throw Exception("Unexpected token");
        }
}

void SimpleBencodeReader::parse(const string &data) {
        auto c = data.c_str();
        decode(&c, c + data.size());
}

}
