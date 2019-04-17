/*
 * Copyright (C) 2001-2019 Jacek Sieka, arnetheduck on gmail point com
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

namespace dcpp {

/* helper class to run a functor when the object goes out of scope.
the additional #defines generate a unique object name thanks to the __COUNTER__ macro.
for example, the call:
        ScopedFunctor([] { printf("hello"); });
will print "hello" when the current scope ends. */

template<typename F>
struct ScopedFunctor {
    explicit ScopedFunctor(F f_) : f(f_) { }
    ~ScopedFunctor() { f(); }
private:
    F f;
};

template<typename F>
ScopedFunctor<F> makeScopedFunctor(F f) { return ScopedFunctor<F>(f); }

#define ScopedFunctor_gen(ScopedFunctor_name, ScopedFunctor_counter) ScopedFunctor_name##ScopedFunctor_counter
#define ScopedFunctor_(ScopedFunctor_f, ScopedFunctor_counter) \
    auto ScopedFunctor_gen(ScopedFunctor_object, ScopedFunctor_counter) = makeScopedFunctor(ScopedFunctor_f)
#define ScopedFunctor(ScopedFunctor_f) ScopedFunctor_(ScopedFunctor_f, __COUNTER__)

} // namespace dcpp

