/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2019 EiskaltDC++ developers
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

#include "NonCopyable.h"

#include "debug.h"

namespace dcpp {

class ISingleton {
public:
    ISingleton() {}

    virtual void release() = 0;
};

template<typename T>
class Singleton : private NonCopyable {
public:
    Singleton() { }
    virtual ~Singleton() { }

    static T* getInstance() {
        dcassert(instance);
        return instance;
    }

    static void newInstance() {
        if(instance)
            delete instance;

        instance = new T();
    }

    static void deleteInstance() {
        if(instance)
            delete instance;
        instance = NULL;
    }

    virtual void release() {
        deleteInstance();
    }

protected:
    static T* instance;
};

template<class T> T* Singleton<T>::instance = NULL;

} // namespace dcpp
