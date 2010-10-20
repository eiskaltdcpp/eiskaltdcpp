/*
* Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_DOWNLOAD_H_
#define DCPLUSPLUS_DCPP_DOWNLOAD_H_

#include "forward.h"
#include "Transfer.h"
#include "MerkleTree.h"
#include "Flags.h"
#include "Streams.h"

namespace dcpp {

/**
 * Comes as an argument in the DownloadManagerListener functions.
 * Use it to retrieve information about the ongoing transfer.
 */
class Download : public Transfer, public Flags {
public:
    enum {
        FLAG_ZDOWNLOAD = 1 << 1,
        FLAG_CALC_CRC32 = 1 << 2,
        FLAG_CRC32_OK = 1 << 3,
        FLAG_TREE_TRIED = 1 << 5,
        FLAG_TTH_CHECK = 1 << 6,
        FLAG_XML_BZ_LIST = 1 << 7
    };

    Download(UserConnection& conn, QueueItem& qi, const string& path, bool supportsTrees) throw();

    virtual void getParams(const UserConnection& aSource, StringMap& params);

    virtual ~Download();

    /** @return Target filename without path. */
    string getTargetFileName() {
        return Util::getFileName(getPath());
    }

    /** @internal */
    const string& getDownloadTarget() {
        return (getTempTarget().empty() ? getPath() : getTempTarget());
    }

    /** @internal */
    TigerTree& getTigerTree() { return tt; }
    string& getPFS() { return pfs; }
    /** @internal */
    AdcCommand getCommand(bool zlib);

    GETSET(string, tempTarget, TempTarget);
    GETSET(OutputStream*, file, File);
    GETSET(bool, treeValid, TreeValid);
private:
    Download(const Download&);
    Download& operator=(const Download&);

    TigerTree tt;
    string pfs;
};

} // namespace dcpp

#endif /*DOWNLOAD_H_*/
