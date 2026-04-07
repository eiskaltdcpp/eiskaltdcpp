/*
 * Copyright © 2009-2010 freedcpp, https://github.com/eiskaltdcpp/freedcpp
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

/**
 * Message and tag type enumerations for the GTK frontend.
 *
 * These are extracted from message.hh to make them available
 * in the widget-independent model layer. The original message.hh
 * can be updated to include this header if needed.
 */

namespace gtk_msg {

/// Chat message types
enum TypeMsg
{
    MSG_GENERAL,
    MSG_PRIVATE,
    MSG_MYOWN,
    MSG_SYSTEM,
    MSG_STATUS,
    MSG_FAVORITE,
    MSG_OPERATOR,
    MSG_UNKNOWN,

    MSG_COUNT  ///< Sentinel for iteration
};

/// Text tag types used for styling chat messages
enum TypeTag
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
};

/// Returns the number of distinct message types (excluding MSG_COUNT)
inline int msgTypeCount() { return static_cast<int>(MSG_COUNT); }

/// Returns the number of distinct tag types
inline int tagTypeCount() { return static_cast<int>(TAG_LAST); }

} // namespace gtk_msg
