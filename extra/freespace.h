/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <string>
struct FreeSpace {
        static bool FreeDiscSpace (const std::string &path, unsigned long long * res, unsigned long long * res2);
};

