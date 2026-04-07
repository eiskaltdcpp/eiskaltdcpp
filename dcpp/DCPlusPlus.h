/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#include <functional>
#include <memory>
#include <string>

namespace dcpp {

class DCContext;
using std::string;

/// Create a DCContext, run startup(), and return ownership to the caller.
/// Also registers the returned context as the process-wide active context
/// (readable via getContext()) so that existing code keeps working during
/// the transition away from the global accessor.
[[nodiscard]] extern std::unique_ptr<DCContext> startup(void (*f)(void*, const string&), void* p);

/// Returns the active DCContext for this process.  This is a NON-OWNING
/// convenience pointer — the DCContext is owned by whoever called startup()
/// or by the test harness that called setContext().  Returns nullptr before
/// startup / after the owner resets.
[[nodiscard]] extern DCContext* getContext();

/// Set the process-wide DCContext pointer (non-owning).
/// Intended for test harnesses that create their own DCContext.
extern void setContext(DCContext* ctx);

} // namespace dcpp
