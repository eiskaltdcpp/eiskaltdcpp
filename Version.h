/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef VERSION_H
#define VERSION_H

const char * const EISKALTDCPP_VERSION         = "2.1.0";
const char * const EISKALTDCPP_WND_TITLE       = "EiskaltDC++";

#ifndef DCPP_REVISION
const char * const EISKALTDCPP_VERSION_SFX     = "stable";
#else
const char * const EISKALTDCPP_VERSION_SFX     = DCPP_REVISION;
#endif

#endif // VERSION_H
