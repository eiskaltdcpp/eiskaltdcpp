/*
 * Copyright (C) 2019 Boris Pek <tehnick-8@yandex.ru>
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

template<typename T, bool flag> struct ReferenceSelector {
    typedef T ResultType;
};
template<typename T> struct ReferenceSelector<T,true> {
    typedef const T& ResultType;
};

template<typename T> class IsOfClassType {
public:
    template<typename U> static char check(int U::*);
    template<typename U> static float check(...);
public:
    enum { Result = sizeof(check<T>(0)) };
};

template<typename T> struct TypeTraits {
    typedef IsOfClassType<T> ClassType;
    typedef ReferenceSelector<T, ((ClassType::Result == 1) || (sizeof(T) > sizeof(char*)) ) > Selector;
    typedef typename Selector::ResultType ParameterType;
};

#define GETSET(type, name, name2) \
    private: type name; \
    public: TypeTraits<type>::ParameterType get##name2() const { return name; } \
    void set##name2(TypeTraits<type>::ParameterType a##name2) { name = a##name2; }

