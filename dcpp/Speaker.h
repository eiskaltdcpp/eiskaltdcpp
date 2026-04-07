/*
 * Copyright (C) 2001-2019 Jacek Sieka, arnetheduck on gmail point com
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

#include <algorithm>
#include <cstdio>
#include <exception>
#include <ranges>
#include <typeinfo>
#include <vector>

#include "CriticalSection.h"

namespace dcpp {

template<typename Listener>
class Speaker {
public:
    Speaker() noexcept = default;
    virtual ~Speaker() = default;

    /// Fire an event to all registered listeners (snapshot under lock).
    template<typename... Args>
    void fire(Args&&... args) noexcept {
        try {
        Lock l(listenerCS);
        auto snapshot = listeners;  // snapshot so listeners can self-remove
        for (auto* listener : snapshot) {
            try {
                listener->on(std::forward<Args>(args)...);
            } catch(const std::exception& e) {
                fprintf(stderr, "[Speaker::fire<%s>] exception from listener %p (%s): %s\n",
                        typeid(Listener).name(), static_cast<void*>(listener),
                        typeid(*listener).name(), e.what());
            } catch(...) {
                fprintf(stderr, "[Speaker::fire<%s>] unknown exception from listener %p (%s)\n",
                        typeid(Listener).name(), static_cast<void*>(listener),
                        typeid(*listener).name());
            }
        }
        } catch(const std::exception& e) {
            fprintf(stderr, "[Speaker::fire<%s>] exception during snapshot/iteration: %s\n",
                    typeid(Listener).name(), e.what());
        } catch(...) {
            fprintf(stderr, "[Speaker::fire<%s>] unknown exception during snapshot/iteration\n",
                    typeid(Listener).name());
        }
    }

    void addListener(Listener* aListener) {
        Lock l(listenerCS);
        if (std::ranges::find(listeners, aListener) == listeners.end())
            listeners.push_back(aListener);
    }

    void removeListener(Listener* aListener) {
        Lock l(listenerCS);
        std::erase(listeners, aListener);
    }

    void removeListeners() {
        Lock l(listenerCS);
        listeners.clear();
    }

protected:
    std::vector<Listener*> listeners;
    CriticalSection listenerCS;
};

} // namespace dcpp
