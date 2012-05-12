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

#include "forward.h"
#include "noexcept.h"

namespace dcpp {

class QueueManagerListener {
public:
    virtual ~QueueManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> Added;
    typedef X<1> Finished;
    typedef X<2> Removed;
    typedef X<3> Moved;
    typedef X<4> SourcesUpdated;
    typedef X<5> StatusUpdated;
    typedef X<6> SearchStringUpdated;
    typedef X<7> PartialList;

    typedef X<8> RecheckStarted;
    typedef X<9> RecheckNoFile;
    typedef X<10> RecheckFileTooSmall;
    typedef X<11> RecheckDownloadsRunning;
    typedef X<12> RecheckNoTree;
    typedef X<13> RecheckAlreadyFinished;
    typedef X<14> RecheckDone;
    typedef X<15> FileMoved;

    typedef X<16> CRCFailed;
    typedef X<17> CRCChecked;

    virtual void on(Added, QueueItem*) noexcept { }
    virtual void on(Finished, QueueItem*, const string&, int64_t) noexcept { }
    virtual void on(Removed, QueueItem*) noexcept { }
    virtual void on(Moved, QueueItem*, const string&) noexcept { }
    virtual void on(SourcesUpdated, QueueItem*) noexcept { }
    virtual void on(StatusUpdated, QueueItem*) noexcept { }
    virtual void on(SearchStringUpdated, QueueItem*) noexcept { }
    virtual void on(PartialList, const HintedUser&, const string&) noexcept { }

    virtual void on(RecheckStarted, const string&) noexcept { }
    virtual void on(RecheckNoFile, const string&) noexcept { }
    virtual void on(RecheckFileTooSmall, const string&) noexcept { }
    virtual void on(RecheckDownloadsRunning, const string&) noexcept { }
    virtual void on(RecheckNoTree, const string&) noexcept { }
    virtual void on(RecheckAlreadyFinished, const string&) noexcept { }
    virtual void on(RecheckDone, const string&) noexcept { }
    virtual void on(FileMoved, const string&) noexcept { }

    virtual void on(CRCFailed, Download*, const string&) noexcept { }
    virtual void on(CRCChecked, Download*) noexcept { }
};

} // namespace dcpp
