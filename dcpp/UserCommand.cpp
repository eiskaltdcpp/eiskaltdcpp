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

#include "UserCommand.h"

#include "StringTokenizer.h"

namespace dcpp {

bool UserCommand::adc(const string& h) {
    return h.compare(0, 6, "adc://") == 0 || h.compare(0, 7, "adcs://") == 0;
}

const StringList& UserCommand::getDisplayName() const {
    return displayName;
}

void UserCommand::setDisplayName() {
    string name_ = name;
    Util::replace("//", "\t", name_);
    StringTokenizer<string> t(name_, '/');
    for(StringList::const_iterator i = t.getTokens().begin(), iend = t.getTokens().end(); i != iend; ++i) {
        displayName.push_back(*i);
        Util::replace("\t", "/", displayName.back());
    }
}

} // namespace dcpp
