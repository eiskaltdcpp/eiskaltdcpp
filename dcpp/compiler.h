/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2020 EiskaltDC++ developers
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
 */

#pragma once

#if defined(__GNUC__) && !defined(__clang__)
#if (__GNUC__ < 5) || (__GNUC__ == 5 && __GNUC_MINOR__ < 2)
#error GCC 5.2 is required
#endif

#elif defined(__clang__)
#if (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 5)
#error Clang 3.5 is required
#endif

#elif defined(_MSC_VER)
#if _MSC_VER < 1800 || _MSC_FULL_VER < 180021114
#error Visual Studio 2013 with the Nov 2013 CTP is required
#endif

//disable the deprecated warnings for the CRT functions.
#define _CRT_SECURE_NO_DEPRECATE 1
#define _ATL_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1

#define strtoll _strtoi64

#else

#error No supported compiler found

#endif

#if defined(_MSC_VER)
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%I64d"
#define U64_FMT "%I64u"

#elif defined(SIZEOF_LONG) && SIZEOF_LONG == 8
#define _LL(x) x##l
#define _ULL(x) x##ul
#define I64_FMT "%ld"
#define U64_FMT "%lu"
#else
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%lld"
#define U64_FMT "%llu"
#endif

#ifndef _REENTRANT
# define _REENTRANT 1
#endif
