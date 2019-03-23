/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
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

#include "entry.hh"

class DialogEntry : public Entry
{
public:
    DialogEntry(const EntryType type = EntryType::NONE, const std::string &ui = "", GtkWindow* parent = NULL);
    virtual ~DialogEntry();

    GtkWidget *getContainer();
    gint run();
    gint getResponseID() const;

private:
    GtkWindow* parent;
    gint responseID;
};
