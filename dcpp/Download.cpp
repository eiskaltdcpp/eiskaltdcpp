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

#include "stdinc.h"

#include "Download.h"

#include "UserConnection.h"
#include "QueueItem.h"
#include "HashManager.h"

namespace dcpp {

Download::Download(UserConnection& conn, QueueItem& qi, const string& path, bool supportsTrees) noexcept : Transfer(conn, path, qi.getTTH()),
    tempTarget(qi.getTempTarget()), file(0), treeValid(false)
{
    conn.setDownload(this);

    QueueItem::SourceConstIter source = qi.getSource(getUser());

    if(qi.isSet(QueueItem::FLAG_PARTIAL_LIST)) {
        setType(TYPE_PARTIAL_LIST);
    } else if(qi.isSet(QueueItem::FLAG_USER_LIST)) {
        setType(TYPE_FULL_LIST);
    }

    if(getType() == TYPE_FILE && qi.getSize() != -1) {
        if(HashManager::getInstance()->getTree(getTTH(), getTigerTree())) {
            setTreeValid(true);
            setSegment(qi.getNextSegment(getTigerTree().getBlockSize(), conn.getChunkSize(),conn.getSpeed(), source->getPartialSource()));
        } else if(supportsTrees && conn.isSet(UserConnection::FLAG_SUPPORTS_TTHL) && !qi.getSource(conn.getUser())->isSet(QueueItem::Source::FLAG_NO_TREE) && qi.getSize() > HashManager::MIN_BLOCK_SIZE) {
            // Get the tree unless the file is small (for small files, we'd probably only get the root anyway)
            setType(TYPE_TREE);
            getTigerTree().setFileSize(qi.getSize());
            setSegment(Segment(0, -1));
        } else {
            // Use the root as tree to get some sort of validation at least...
            getTigerTree() = TigerTree(qi.getSize(), qi.getSize(), getTTH());
            setTreeValid(true);
            setSegment(qi.getNextSegment(getTigerTree().getBlockSize(), 0, 0, source->getPartialSource()));
        }

        if(getSegment().getOverlapped()) {
            setFlag(FLAG_OVERLAP);

            // set overlapped flag to original segment
            for(DownloadList::const_iterator i = qi.getDownloads().begin(); i != qi.getDownloads().end(); ++i) {
                if((*i)->getSegment().contains(getSegment())) {
                    (*i)->setOverlapped(true);
                    break;
                }
            }
        }
    }
}

Download::~Download() {
    getUserConnection().setDownload(0);
}

AdcCommand Download::getCommand(bool zlib) {
    AdcCommand cmd(AdcCommand::CMD_GET);

    cmd.addParam(Transfer::names[getType()]);

    if(getType() == TYPE_PARTIAL_LIST) {
        cmd.addParam(Util::toAdcFile(getPath()));
    } else if(getType() == TYPE_FULL_LIST) {
        if(isSet(Download::FLAG_XML_BZ_LIST)) {
            cmd.addParam(USER_LIST_NAME_BZ);
        } else {
            cmd.addParam(USER_LIST_NAME);
        }
    } else {
        cmd.addParam("TTH/" + getTTH().toBase32());
    }

    cmd.addParam(Util::toString(getStartPos()));
    cmd.addParam(Util::toString(getSize()));

    if(zlib && BOOLSETTING(COMPRESS_TRANSFERS)) {
        cmd.addParam("ZL1");
    }

    return cmd;
}

void Download::getParams(const UserConnection& aSource, StringMap& params) {
    Transfer::getParams(aSource, params);
    params["target"] = getPath();
}

} // namespace dcpp
