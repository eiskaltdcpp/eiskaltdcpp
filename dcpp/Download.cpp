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

#include "stdinc.h"

#include "Download.h"

#include "UserConnection.h"
#include "QueueItem.h"
#include "HashManager.h"
#include "SettingsManager.h"
#include "MerkleCheckOutputStream.h"
#include "MerkleTreeOutputStream.h"
#include "File.h"
#include "FilteredFile.h"
#include "ZUtils.h"

namespace dcpp {

Download::Download(UserConnection& conn, QueueItem& qi) noexcept : Transfer(conn, qi.getTarget(), qi.getTTH()),
    tempTarget(qi.getTempTarget()), treeValid(false)
{
    conn.setDownload(this);

    auto source = qi.getSource(getUser());

    if(qi.isSet(QueueItem::FLAG_PARTIAL_LIST)) {
        setType(TYPE_PARTIAL_LIST);
    } else if(qi.isSet(QueueItem::FLAG_USER_LIST)) {
        setType(TYPE_FULL_LIST);
    }

    if(qi.getSize() != -1) {
        if(HashManager::getInstance()->getTree(getTTH(), tt)) {
            setTreeValid(true);
            setSegment(qi.getNextSegment(getTigerTree().getBlockSize(), conn.getChunkSize(),conn.getSpeed(), source->getPartialSource()));
        } else if(conn.supportsTrees()&& !qi.getSource(conn.getUser())->isSet(QueueItem::Source::FLAG_NO_TREE) && qi.getSize() > HashManager::MIN_BLOCK_SIZE) {
            // Get the tree unless the file is small (for small files, we'd probably only get the root anyway)
            setType(TYPE_TREE);
            tt.setFileSize(qi.getSize());
            setSegment(Segment(0, -1));
        } else {
            // Use the root as tree to get some sort of validation at least...
            tt = TigerTree(qi.getSize(), qi.getSize(), getTTH());
            setTreeValid(true);
            setSegment(qi.getNextSegment(getTigerTree().getBlockSize(), 0, 0, source->getPartialSource()));
        }

        if(getSegment().getOverlapped()) {
            setFlag(FLAG_OVERLAP);

            // set overlapped flag to original segment
            for(auto& i : qi.getDownloads()) {
                if(i->getSegment().contains(getSegment())) {
                    i->setOverlapped(true);
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

string Download::getTargetFileName() const {
    return Util::getFileName(getPath());
}

const string& Download::getDownloadTarget() const {
    return (getTempTarget().empty() ? getPath() : getTempTarget());
}

void Download::open(int64_t bytes, bool z) {
    if(getType() == Transfer::TYPE_FILE) {
        auto target = getDownloadTarget();
        auto fullSize = tt.getFileSize();

        if(getSegment().getStart() > 0) {
            if(File::getSize(target) != fullSize) {
                // When trying the download the next time, the resume pos will be reset
                throw Exception(_("Target file is missing or wrong size"));
            }
        } else {
            File::ensureDirectory(target);
        }

        unique_ptr<File> f(new File(target, File::WRITE, File::OPEN | File::CREATE | File::SHARED));

        if(f->getSize() != fullSize) {
            f->setSize(fullSize);
        }

        f->setPos(getSegment().getStart());
        output = move(f);
        tempTarget = target;
    } else if(getType() == Transfer::TYPE_FULL_LIST) {
        auto target = getPath();
        File::ensureDirectory(target);

        if(isSet(Download::FLAG_XML_BZ_LIST)) {
            target += ".xml.bz2";
        } else {
            target += ".xml";
        }

        output.reset(new File(target, File::WRITE, File::OPEN | File::TRUNCATE | File::CREATE));
        tempTarget = target;
    } else if(getType() == Transfer::TYPE_PARTIAL_LIST) {
        output.reset(new StringRefOutputStream(pfs));
    } else if(getType() == Transfer::TYPE_TREE) {
        output.reset(new MerkleTreeOutputStream<TigerTree>(tt));
    }

    if((getType() == Transfer::TYPE_FILE || getType() == Transfer::TYPE_FULL_LIST) && SETTING(BUFFER_SIZE) > 0 ) {
        output.reset(new BufferedOutputStream<true>(output.release()));
    }

    if(getType() == Transfer::TYPE_FILE) {
        typedef MerkleCheckOutputStream<TigerTree, true> MerkleStream;

        output.reset(new MerkleStream(tt, output.release(), getStartPos()));
        setFlag(Download::FLAG_TTH_CHECK);
    }

    // Check that we don't get too many bytes
    output.reset(new LimitedOutputStream<true>(output.release(), bytes));

    if(z) {
        setFlag(Download::FLAG_ZDOWNLOAD);
        output.reset(new FilteredOutputStream<UnZFilter, true>(output.release()));
    }
}

void Download::close()
{
        output.reset();
}

} // namespace dcpp
