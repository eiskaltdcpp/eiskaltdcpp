/*
* Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <memory>
#include <string>
#include "forward.h"
#include "noexcept.h"
#include "Transfer.h"
#include "MerkleTree.h"
#include "Flags.h"
#include "Streams.h"
#include "GetSet.h"

namespace dcpp {

using std::string;
using std::unique_ptr;

/**
 * Comes as an argument in the DownloadManagerListener functions.
 * Use it to retrieve information about the ongoing transfer.
 */
class Download : public Transfer, public Flags {
public:
    enum {
        FLAG_ZDOWNLOAD = 1 << 1,
        FLAG_TREE_TRIED = 1 << 2,
        FLAG_TTH_CHECK = 1 << 3,
        FLAG_XML_BZ_LIST = 1 << 4,
        FLAG_OVERLAP    = 0x100
    };

    Download(UserConnection& conn, QueueItem& qi) noexcept;

    virtual void getParams(const UserConnection& aSource, StringMap& params);

    virtual ~Download();

    /** @return Target filename without path. */
    string getTargetFileName() const;

    /** Open the target output for writing */
    void open(int64_t bytes, bool z);

    /** Release the target output */
    void close();

    /** @internal */
    TigerTree& getTigerTree() { return tt; }
    const string& getPFS() const { return pfs; }
    /** @internal */
    AdcCommand getCommand(bool zlib);

    const unique_ptr<OutputStream>& getOutput() const { return output; }

    GETSET(string, tempTarget, TempTarget);
    GETSET(bool, treeValid, TreeValid);
private:

    const string& getDownloadTarget() const;

    TigerTree tt;
    string pfs;
    unique_ptr<OutputStream> output;
};

} // namespace dcpp
