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

#ifndef DCPLUSPLUS_DCPP_QUEUE_MANAGER_LISTENER_H
#define DCPLUSPLUS_DCPP_QUEUE_MANAGER_LISTENER_H

#include "forward.h"

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

    virtual void on(Added, QueueItem*) throw() { }
    virtual void on(Finished, QueueItem*, const string&, int64_t) throw() { }
    virtual void on(Removed, QueueItem*) throw() { }
    virtual void on(Moved, QueueItem*, const string&) throw() { }
    virtual void on(SourcesUpdated, QueueItem*) throw() { }
    virtual void on(StatusUpdated, QueueItem*) throw() { }
    virtual void on(SearchStringUpdated, QueueItem*) throw() { }
    virtual void on(PartialList, const HintedUser&, const string&) throw() { }

    virtual void on(RecheckStarted, const string&) throw() { }
    virtual void on(RecheckNoFile, const string&) throw() { }
    virtual void on(RecheckFileTooSmall, const string&) throw() { }
    virtual void on(RecheckDownloadsRunning, const string&) throw() { }
    virtual void on(RecheckNoTree, const string&) throw() { }
    virtual void on(RecheckAlreadyFinished, const string&) throw() { }
    virtual void on(RecheckDone, const string&) throw() { }
    virtual void on(FileMoved, const string&) throw() { }

    virtual void on(CRCFailed, Download*, const string&) throw() { }
    virtual void on(CRCChecked, Download*) throw() { }
};

} // namespace dcpp

#endif // !defined(QUEUE_MANAGER_LISTENER_H)
