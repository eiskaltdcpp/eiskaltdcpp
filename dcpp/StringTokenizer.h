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
#include <vector>

namespace dcpp {

using std::string;
using std::vector;

template<class T>
class StringTokenizer
{
private:
    vector<T> tokens;
public:
    StringTokenizer(const T& aString, const typename T::value_type aToken) {
        string::size_type i = 0;
        string::size_type j = 0;
        while( (i=aString.find(aToken, j)) != string::npos ) {
            tokens.push_back(aString.substr(j, i-j));
            j = i + 1;
        }
        if(j < aString.size())
            tokens.push_back(aString.substr(j, aString.size()-j));
    }

    StringTokenizer(const T& aString, const typename T::value_type* aToken) {
        string::size_type i = 0;
        string::size_type j = 0;
        size_t l = strlen(aToken);
        while( (i=aString.find(aToken, j)) != string::npos ) {
            tokens.push_back(aString.substr(j, i-j));
            j = i + l;
        }
        if(j < aString.size())
            tokens.push_back(aString.substr(j, aString.size()-j));
    }

    vector<T>& getTokens() { return tokens; }

    ~StringTokenizer() { }
};

} // namespace dcpp
