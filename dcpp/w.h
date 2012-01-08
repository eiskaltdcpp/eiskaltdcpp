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

#ifndef DCPLUSPLUS_DCPP_W_H_
#define DCPLUSPLUS_DCPP_W_H_

#ifdef _WIN32

#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0502
#endif

#ifndef _WIN32_IE
# define _WIN32_IE	0x0501
#endif

#ifndef WINVER
# define WINVER 0x501
#endif

#ifndef STRICT
#define STRICT 1
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include <winsock2.h>

#include <windows.h>
#include <tchar.h>

#endif

#endif /* W_H_ */
