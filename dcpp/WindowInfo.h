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

#ifndef DCPLUSPLUS_DCPP_WINDOW_INFO_H
#define DCPLUSPLUS_DCPP_WINDOW_INFO_H

#include "forward.h"
#include "Util.h"

namespace dcpp {

class WindowInfo {
public:
	explicit WindowInfo(const string& id_, const StringMap& params_);

	GETSET(string, id, Id);
	GETSET(StringMap, params, Params);

	bool operator==(const WindowInfo& rhs) const;

	/// special param used for displaying; ignored for identification.
	static const string title;

	/// special param for hub addresses.
	static const string address;

	/// special param that indicates a CID for an user whose information shall be saved on exit.
	static const string cid;

	/// special param for paths to file lists that must not be deleted on exit.
	static const string fileList;
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_WINDOW_INFO_H)
