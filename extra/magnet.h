/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include "dcpp/typedefs.h"
#include <string>

namespace dcpp {

struct magnet {
        static bool parseUri(const std::string& uri, StringMap& params);
};

} // namespace dcpp
