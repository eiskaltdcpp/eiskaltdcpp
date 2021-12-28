/*
 * Copyright (C) 2001-2019 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2020 Boris Pek <tehnick-8@yandex.ru>
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

#include <string>

#ifdef BUILDING_DCPP

#define dgettext(x, String) String
#define dngettext(x, String1, String2, N) String1

#define PACKAGE "libeiskaltdcpp"
#define LOCALEDIR dcpp::Util::getPath(Util::PATH_LOCALE).c_str()
#define _(String) String
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#define F_(String) dcpp::dcpp_fmt(dgettext(PACKAGE, String))
#define FN_(String1,String2, N) dcpp::dcpp_fmt(dngettext(PACKAGE, String1, String2, N))

#endif

namespace dcpp {

using std::string;

class formated_string : public std::string
{
public:
    explicit formated_string(const string& s) noexcept : string(s) { }

    formated_string& operator%(const string& x) {
        const string &&counter_string = "%" + std::to_string(++counter_) + "%";
        auto pos = this->find(counter_string);
        while(pos != string::npos) {
            this->replace(pos, counter_string.size(), x);
            pos = this->find(counter_string, pos + x.size());
        }
        return *this;
    }

    formated_string& operator%(const char* x) {
        return operator%(string(x));
    }

    formated_string& operator%(char* x) {
        return operator%(string(x));
    }

    template<typename T>
    formated_string& operator%(T x) {
        return operator%(std::to_string(x));
    }

private:
    uint8_t counter_ = 0;
};

inline formated_string dcpp_fmt(const std::string& t) {
    return formated_string(t);
}

inline formated_string dcpp_fmt(const char* t) {
    return dcpp_fmt(std::string(t));
}

inline std::string str(const formated_string& t) {
    return std::string(t);
}

} // namespace dcpp

using dcpp::str;
