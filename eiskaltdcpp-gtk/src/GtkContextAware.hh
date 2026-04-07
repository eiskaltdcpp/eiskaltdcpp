/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

namespace dcpp { class DCContext; }

/**
 * Mixin that gives GTK classes access to the dcpp::DCContext.
 *
 * Pure constructor-injection — no global state.  Each class that
 * needs DCContext receives it in its constructor and passes it
 * to the GtkContextAware base.
 */
class GtkContextAware {
public:
    [[nodiscard]] dcpp::DCContext& dcCtx() const noexcept { return dcCtx_; }

protected:
    explicit GtkContextAware(dcpp::DCContext& ctx) noexcept : dcCtx_(ctx) {}
    ~GtkContextAware() = default;

private:
    dcpp::DCContext& dcCtx_;
};
