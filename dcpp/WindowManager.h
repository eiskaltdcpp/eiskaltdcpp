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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DCPLUSPLUS_DCPP_WINDOW_MANAGER_H
#define DCPLUSPLUS_DCPP_WINDOW_MANAGER_H

#include "forward.h"
#include "SettingsManager.h"
#include "WindowManagerListener.h"

namespace dcpp {

class WindowManager :
	public Singleton<WindowManager>,
	public Speaker<WindowManagerListener>,
	private SettingsManagerListener
{
	typedef std::unordered_map<string, unsigned> MaxRecentItems;
public:
	typedef std::vector<WindowInfo> WindowInfoList;
	typedef std::unordered_map<string, WindowInfoList> RecentList;

	void autoOpen(bool skipHubs);

	void lock();
	void unlock();

	void add(const string& id, const StringMap& params);
	void clear();

	/// adds the referenced window if it doesn't exist, and moves it to the top of the stack.
	void addRecent(const string& id, const StringMap& params);
	/// updates the title of the referenced window without changing its position in the stack.
	void updateRecent(const string& id, const StringMap& params);
	const RecentList& getRecent() const { return recent; }
	void setMaxRecentItems(const string& id, unsigned max);
	unsigned getMaxRecentItems(const string& id) const;

	/// inform other classes about users and file lists that are of interest to us.
	void prepareSave() const;

	/// special id that designates a hub window.
	static const string& hub();

private:
	friend class Singleton<WindowManager>;

	mutable CriticalSection cs;
	WindowInfoList list;
	RecentList recent;
	MaxRecentItems maxRecentItems;

	WindowManager();
	virtual ~WindowManager() throw();

	inline void addRecent_(const string& id, const StringMap& params) { addRecent_(id, params, false); }
	void addRecent_(const string& id, const StringMap& params, bool top);

	void prepareSave(const WindowInfoList& infoList) const;

	typedef void (WindowManager::*handler_type)(const std::string&, const StringMap&);
	void parseTags(SimpleXML& xml, handler_type handler);
	void addTag(SimpleXML& xml, const WindowInfo& info) const;

	virtual void on(SettingsManagerListener::Load, SimpleXML& xml) throw();
	virtual void on(SettingsManagerListener::Save, SimpleXML& xml) throw();
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_WINDOW_MANAGER_H)
