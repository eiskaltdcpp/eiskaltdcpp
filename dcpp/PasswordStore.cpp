/*
 * Copyright (C) 2026 EiskaltDC++ developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "stdinc.h"
#include "PasswordStore.h"

namespace dcpp {

bool PasswordStore::isAvailable() {
    return false;
}

bool PasswordStore::getHubPassword(const std::string&, const std::string&, std::string&) {
    return false;
}

bool PasswordStore::setHubPassword(const std::string&, const std::string&, const std::string&) {
    return false;
}

bool PasswordStore::deleteHubPassword(const std::string&, const std::string&) {
    return false;
}

} // namespace dcpp
