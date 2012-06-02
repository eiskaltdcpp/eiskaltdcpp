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

#pragma once

// for compilers that don't support noexcept, use an exception specifier

#if defined(__clang__)

#if __clang_major__ < 3

#ifndef noexcept
#define noexcept throw()
#endif

#endif

#elif defined(__GNUC__)
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6) // GCC 4.6 is the first GCC to implement noexcept.

#ifndef noexcept
#define noexcept throw()
#endif

#endif
#elif defined(_MSC_VER)

#ifndef noexcept
#define noexcept throw()
#endif

#endif
