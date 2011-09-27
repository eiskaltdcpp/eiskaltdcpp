/*
* Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef UPLOAD_H_
#define UPLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "Flags.h"

namespace dcpp {

class Upload : public Transfer, public Flags {
public:
    enum Flags {
        FLAG_ZUPLOAD = 1 << 0,
        FLAG_PENDING_KICK = 1 << 1
    };

    Upload(UserConnection& conn, const string& path, const TTHValue& tth);
    virtual ~Upload();

    virtual void getParams(const UserConnection& aSource, StringMap& params);

    GETSET(int64_t, fileSize, FileSize);
    GETSET(InputStream*, stream, Stream);
};

} // namespace dcpp

#endif /*UPLOAD_H_*/
