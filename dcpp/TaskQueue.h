/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_TASK_H
#define DCPLUSPLUS_DCPP_TASK_H

#include "CriticalSection.h"

namespace dcpp {

struct Task {
	virtual ~Task() { };
};
struct StringTask : public Task {
	StringTask(const string& str_) : str(str_) { }
	string str;
};

class TaskQueue {
public:
	typedef pair<int, Task*> Pair;
	typedef vector<Pair> List;
	typedef List::iterator Iter;

	TaskQueue() {
	}

	~TaskQueue() {
		clear();
	}

	void add(int type, Task* data) { Lock l(cs); tasks.push_back(make_pair(type, data)); }
	void get(List& list) { Lock l(cs); swap(tasks, list); }
	void clear() {
		List tmp;
		get(tmp);
		for(Iter i = tmp.begin(); i != tmp.end(); ++i) {
			delete i->second;
		}
	}
private:

	TaskQueue(const TaskQueue&);
	TaskQueue& operator=(const TaskQueue&);

	CriticalSection cs;
	List tasks;
};

} // namespace dcpp

#endif
