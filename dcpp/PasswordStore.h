/*
 * Copyright (C) 2026 EiskaltDC++ developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <string>

namespace dcpp {

class PasswordStore {
public:
    static bool isAvailable();
    static bool getHubPassword(const std::string& server, const std::string& nick, std::string& password);
    static bool setHubPassword(const std::string& server, const std::string& nick, const std::string& password);
    static bool deleteHubPassword(const std::string& server, const std::string& nick);
};

} // namespace dcpp
