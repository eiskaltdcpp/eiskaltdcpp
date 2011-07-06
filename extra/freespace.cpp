/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

//function for use fsusage
#ifdef WIN32
    #include <io.h>
    #include <dcpp/stdinc.h>
    #include <dcpp/Text.h>
#else //WIN32
    extern "C" {
    #include "fsusage.h"
    }
#endif //WIN32
#include "freespace.h"

bool FreeSpace::FreeDiscSpace ( std::string path,  unsigned long long * res, unsigned long long * res2) {
        if ( !res ) {
            return false;
        }

#ifdef WIN32
        ULARGE_INTEGER lpFreeBytesAvailableToCaller; // receives the number of bytes on
                                               // disk available to the caller
        ULARGE_INTEGER lpTotalNumberOfBytes;    // receives the number of bytes on disk
        ULARGE_INTEGER lpTotalNumberOfFreeBytes; // receives the free bytes on disk

        if ( GetDiskFreeSpaceExW( (const WCHAR*)dcpp::Text::utf8ToWide(path).c_str(), &lpFreeBytesAvailableToCaller,
                                &lpTotalNumberOfBytes,
                                &lpTotalNumberOfFreeBytes ) == true ) {
                *res = lpTotalNumberOfFreeBytes.QuadPart;
                *res2 = lpTotalNumberOfBytes.QuadPart;
                return true;
        } else {
            return false;
        }
#else //WIN32
        struct fs_usage fsp;
        if ( get_fs_usage(path.c_str(),path.c_str(),&fsp) == 0 ) {
                *res = fsp.fsu_bavail*fsp.fsu_blocksize;
                *res2 =fsp.fsu_blocks*fsp.fsu_blocksize;
                return true;
        } else {
                return false;
        }
#endif //WIN32
}
