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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"

#include "WindowInfo.h"

namespace dcpp {

const string WindowInfo::title = "Title";

const string WindowInfo::address = "Address";
const string WindowInfo::cid = "CID";
const string WindowInfo::fileList = "FileList";

WindowInfo::WindowInfo(const string& id_, const StringMap& params_) :
id(id_),
params(params_)
{
}

bool WindowInfo::operator==(const WindowInfo& rhs) const {
	if(id != rhs.id)
		return false;

	if(params.size() != rhs.params.size())
		return false;

	// compare each param, except "Title" which is not used for identification.
	for(StringMap::const_iterator i = params.begin(), iend = params.end(); i != iend; ++i) {
		if(i->first == title)
			continue;
		StringMap::const_iterator ri = rhs.params.find(i->first);
		if(ri == rhs.params.end())
			return false;
		if(i->second != ri->second)
			return false;
	}

	return true;
}

} // namespace dcpp
