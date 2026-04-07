/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2020 EiskaltDC++ developers
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

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "DCContext.h"

namespace dcpp {

/// Non-owning pointer to the active DCContext.  Set by startup() or
/// setContext(); read by getContext().  NOT a singleton — the DCContext
/// lifecycle is owned by the application (the unique_ptr returned from
/// startup()).
static DCContext* g_activeContext = nullptr;

std::unique_ptr<DCContext> startup(void (*f)(void*, const string&), void* p) {
    auto ctx = std::make_unique<DCContext>();

    // Register BEFORE startup() so that manager constructors that call
    // getContext() (for listener wiring) see the new DCContext.
    g_activeContext = ctx.get();

    // Adapt the legacy C callback to DCContext's std::function progress API.
    DCContext::ProgressFn progress;
    if (f) {
        progress = [f, p](const std::string& msg) { f(p, msg); };
    }

    ctx->startup(std::move(progress));
    return ctx;  // caller owns
}

DCContext* getContext() {
    return g_activeContext;
}

void setContext(DCContext* ctx) {
    g_activeContext = ctx;
}

} // namespace dcpp
