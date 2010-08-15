/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 *
 * This program uses the MiniUPnP client library by Thomas Bernard
 * http://miniupnp.free.fr http://miniupnp.tuxfamily.org
 */

#ifndef UPNPC_HH
#define UPNPC_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/UPnP.h>

class UPnPc :
	public dcpp::UPnP
{
	public:
		UPnPc() {}
	private:
		bool init();

		bool add(const unsigned short port, const dcpp::UPnP::Protocol protocol, const std::string& description);
		bool remove(const unsigned short port, const dcpp::UPnP::Protocol protocol);

		std::string getExternalIP();
};

#else
class UPnPc;
#endif
