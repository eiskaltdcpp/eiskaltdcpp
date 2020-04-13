/*
 * Copyright © 2009-2010 freedcpp, https://github.com/eiskaltdcpp/freedcpp
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#pragma once

class Msg
{
public:
    typedef enum
    {
        GENERAL,
        PRIVATE,
        MYOWN,
        SYSTEM,
        STATUS,
        FAVORITE,
        OPERATOR,
        UNKNOWN
    } TypeMsg;
};

class Tag
{
public:
    typedef enum
    {
        TAG_FIRST = 0,
        TAG_GENERAL = TAG_FIRST,
        TAG_PRIVATE,
        TAG_MYOWN,
        TAG_SYSTEM,
        TAG_STATUS,
        TAG_TIMESTAMP,
        /*-*/
        TAG_MYNICK,
        TAG_NICK,
        TAG_OPERATOR,
        TAG_FAVORITE,
        TAG_URL,
        TAG_LAST
    } TypeTag;
};

