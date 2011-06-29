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

#include "WindowManager.h"

#include "WindowInfo.h"
#include "SimpleXML.h"
#include "ClientManager.h"
#include "QueueManager.h"

namespace dcpp {

static const unsigned MAX_RECENTS_DEFAULT = 10;

const string& WindowManager::hub() {
	static const string hub("Hub");
	return hub;
}

WindowManager::WindowManager() {
	SettingsManager::getInstance()->addListener(this);
}

WindowManager::~WindowManager() throw() {
	SettingsManager::getInstance()->removeListener(this);
}

void WindowManager::autoOpen(bool skipHubs) {
	Lock l(cs);
	for(WindowInfoList::const_iterator i = list.begin(), iend = list.end(); i != iend; ++i) {
		const string& id = i->getId();
		if(skipHubs && id == hub())
			continue;
		fire(WindowManagerListener::Window(), id, i->getParams());
	}
}

void WindowManager::lock() {
	cs.enter();
}

void WindowManager::unlock() {
	cs.leave();
}

void WindowManager::add(const string& id, const StringMap& params) {
	list.push_back(WindowInfo(id, params));
}

void WindowManager::clear() {
	list.clear();
}

void WindowManager::addRecent(const string& id, const StringMap& params) {
	Lock l(cs);
	addRecent_(id, params, true);
}

void WindowManager::addRecent_(const string& id, const StringMap& params, bool top) {
	unsigned max;
	{
		MaxRecentItems::const_iterator i = maxRecentItems.find(id);
		if(i == maxRecentItems.end()) {
			maxRecentItems[id] = max = MAX_RECENTS_DEFAULT;
		} else {
			max = i->second;
		}
	}
	if(max == 0)
		return;

	WindowInfo info(id, params);

	if(recent.find(id) == recent.end()) {
		recent[id] = WindowInfoList(1, info);
		return;
	}

	WindowInfoList& infoList = recent[id];
	if(top) {
		WindowInfoList::iterator i = std::find(infoList.begin(), infoList.end(), info);
		if(i == infoList.end()) {
			infoList.insert(infoList.begin(), info);
			if(infoList.size() > max)
				infoList.erase(infoList.end() - 1);
		} else {
			infoList.erase(i);
			infoList.insert(infoList.begin(), info);
		}
	} else if(infoList.size() < max)
		infoList.push_back(info);
}

void WindowManager::updateRecent(const string& id, const StringMap& params) {
	Lock l(cs);
	RecentList::iterator ri = recent.find(id);
	if(ri != recent.end()) {
		WindowInfo info(id, params);
		WindowInfoList::iterator i = std::find(ri->second.begin(), ri->second.end(), info);
		if(i != ri->second.end())
			i->setParams(params);
	}
}

void WindowManager::setMaxRecentItems(const string& id, unsigned max) {
	Lock l(cs);
	maxRecentItems[id] = max;

	RecentList::iterator i = recent.find(id);
	if(i != recent.end()) {
		if(max == 0) {
			recent.erase(i);
		} else {
			while(i->second.size() > max)
				i->second.erase(i->second.end() - 1);
		}
	}
}

unsigned WindowManager::getMaxRecentItems(const string& id) const {
	Lock l(cs);
	MaxRecentItems::const_iterator i = maxRecentItems.find(id);
	if(i == maxRecentItems.end())
		return MAX_RECENTS_DEFAULT;
	return i->second;
}

void WindowManager::prepareSave() const {
	Lock l(cs);
	prepareSave(list);
	for(RecentList::const_iterator i = recent.begin(), iend = recent.end(); i != iend; ++i)
		prepareSave(i->second);
}

void WindowManager::prepareSave(const WindowInfoList& infoList) const {
	for(WindowInfoList::const_iterator wi = infoList.begin(), wiend = infoList.end(); wi != wiend; ++wi) {
		StringMap::const_iterator i = wi->getParams().find(WindowInfo::cid);
		if(i != wi->getParams().end())
			ClientManager::getInstance()->saveUser(CID(i->second));

		i = wi->getParams().find(WindowInfo::fileList);
		if(i != wi->getParams().end() && !i->second.empty())
			QueueManager::getInstance()->noDeleteFileList(i->second);
	}
}

void WindowManager::parseTags(SimpleXML& xml, handler_type handler) {
	xml.stepIn();

	while(xml.findChild("Window")) {
		const string& id = xml.getChildAttrib("Id");
		if(id.empty())
			continue;

		StringMap params;
		xml.stepIn();
		while(xml.findChild("Param")) {
			const string& id_ = xml.getChildAttrib("Id");
			if(id_.empty())
				continue;
			params[id_] = xml.getChildData();
		}
		xml.stepOut();

		(this->*handler)(id, params);
	}

	xml.stepOut();
}

void WindowManager::addTag(SimpleXML& xml, const WindowInfo& info) const {
	xml.addTag("Window");
	xml.addChildAttrib("Id", info.getId());

	if(!info.getParams().empty()) {
		xml.stepIn();
		for(StringMap::const_iterator i = info.getParams().begin(), iend = info.getParams().end(); i != iend; ++i) {
			xml.addTag("Param", i->second);
			xml.addChildAttrib("Id", i->first);
		}
		xml.stepOut();
	}
}

void WindowManager::on(SettingsManagerListener::Load, SimpleXML& xml) throw() {
	Lock l(cs);
	clear();

	xml.resetCurrentChild();
	if(xml.findChild("Windows"))
		parseTags(xml, &WindowManager::add);

	if(xml.findChild("Recent")) {
		xml.stepIn();
		while(xml.findChild("Configuration")) {
			const string& id = xml.getChildAttrib("Id");
			if(id.empty())
				continue;
			setMaxRecentItems(id, xml.getIntChildAttrib("MaxItems"));
		}
		xml.stepOut();
		parseTags(xml, &WindowManager::addRecent_);
	}
}

void WindowManager::on(SettingsManagerListener::Save, SimpleXML& xml) throw() {
	Lock l(cs);

	xml.addTag("Windows");
	xml.stepIn();
	for(WindowInfoList::const_iterator i = list.begin(), iend = list.end(); i != iend; ++i)
		addTag(xml, *i);
	xml.stepOut();

	xml.addTag("Recent");
	xml.stepIn();
	for(MaxRecentItems::const_iterator i = maxRecentItems.begin(), iend = maxRecentItems.end(); i != iend; ++i) {
		xml.addTag("Configuration");
		xml.addChildAttrib("Id", i->first);
		xml.addChildAttrib("MaxItems", i->second);
	}
	for(RecentList::const_iterator ri = recent.begin(), riend = recent.end(); ri != riend; ++ri) {
		const WindowInfoList& infoList = ri->second;
		for(WindowInfoList::const_iterator i = infoList.begin(), iend = infoList.end(); i != iend; ++i)
			addTag(xml, *i);
	}
	xml.stepOut();
}

} // namespace dcpp
