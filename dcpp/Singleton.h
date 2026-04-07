/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2019 EiskaltDC++ developers
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
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

#include "debug.h"

namespace dcpp {

/// Abstract interface for releasing singleton-like objects via dynamic_cast.
/// Used by the Qt GUI's ArenaWidgetManager. Will be removed with A4.
class ISingleton {
public:
    virtual ~ISingleton() = default;
    virtual void release() = 0;
};

/**
 * CRTP singleton base — will be removed entirely once DCContext
 * owns all managers (milestone A4). Do not add new singletons.
 */
template<typename T>
class Singleton {
public:
    Singleton() = default;
    virtual ~Singleton() = default;

    // Non-copyable, non-movable
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    [[nodiscard]] static T* getInstance() {
        dcassert(instance);
        return instance;
    }

    static void newInstance() {
        delete instance;
        instance = new T();
    }

    static void deleteInstance() {
        delete instance;
        instance = nullptr;
    }

    virtual void release() {
        deleteInstance();
    }

protected:
    static T* instance;
};

template<class T> T* Singleton<T>::instance = nullptr;

} // namespace dcpp
