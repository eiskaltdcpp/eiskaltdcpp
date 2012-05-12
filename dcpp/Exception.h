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

#include <string>
#include "debug.h"

namespace dcpp {

using std::string;

class Exception : public std::exception
{
public:
    Exception() { }
    Exception(const string& aError) : error(aError) { dcdrun(if(!error.empty())) dcdebug("Thrown: %s\n", error.c_str()); }

    virtual const char* what() const throw() { return getError().c_str(); }

    virtual ~Exception() throw() { }
    virtual const string& getError() const { return error; }
protected:
    string error;
};

#ifdef _DEBUG

#define STANDARD_EXCEPTION(name) class name : public Exception { \
public:\
        name() : Exception(#name) { } \
        name(const string& aError) : Exception(#name ": " + aError) { } \
        virtual ~name() throw() { } \
}

#else // _DEBUG

#define STANDARD_EXCEPTION(name) class name : public Exception { \
public:\
        name() : Exception() { } \
        name(const string& aError) : Exception(aError) { } \
        virtual ~name() throw() { } \
}
#endif

} // namespace dcpp
