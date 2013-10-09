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

#include "stdinc.h"

#include "UserCommand.h"
#include "StringTokenizer.h"
#include "Util.h"

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
    for(auto& i: t.getTokens()) {
        displayName.push_back(i);
        Util::replace("\t", "/", displayName.back());
    }
}

} // namespace dcpp
