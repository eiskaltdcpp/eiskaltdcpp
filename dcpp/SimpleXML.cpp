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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SimpleXML.h"

namespace dcpp {

const string SimpleXML::utf8Header = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\r\n";

string& SimpleXML::escape(string& aString, bool aAttrib, bool aLoading /* = false */, const string &encoding /* = "UTF-8" */) {
	string::size_type i = 0;
	const char* chars = aAttrib ? "<&>'\"" : "<&>";

	if(aLoading) {
		while((i = aString.find('&', i)) != string::npos) {
			if(aString.compare(i+1, 3, "lt;") == 0) {
				aString.replace(i, 4, 1, '<');
			} else if(aString.compare(i+1, 4, "amp;") == 0) {
				aString.replace(i, 5, 1, '&');
			} else if(aString.compare(i+1, 3, "gt;") == 0) {
				aString.replace(i, 4, 1, '>');
			} else if(aAttrib) {
				if(aString.compare(i+1, 5, "apos;") == 0) {
					aString.replace(i, 6, 1, '\'');
				} else if(aString.compare(i+1, 5, "quot;") == 0) {
					aString.replace(i, 6, 1, '"');
				}
			}
			i++;
		}
		i = 0;
		if( (i = aString.find('\n')) != string::npos) {
			if(i > 0 && aString[i-1] != '\r') {
				// This is a unix \n thing...convert it...
				i = 0;
				while( (i = aString.find('\n', i) ) != string::npos) {
					if(aString[i-1] != '\r')
						aString.insert(i, 1, '\r');

					i+=2;
				}
			}
		}
		aString = Text::toUtf8(aString, encoding);
	} else {
		while( (i = aString.find_first_of(chars, i)) != string::npos) {
			switch(aString[i]) {
			case '<': aString.replace(i, 1, "&lt;"); i+=4; break;
			case '&': aString.replace(i, 1, "&amp;"); i+=5; break;
			case '>': aString.replace(i, 1, "&gt;"); i+=4; break;
			case '\'': aString.replace(i, 1, "&apos;"); i+=6; break;
			case '"': aString.replace(i, 1, "&quot;"); i+=6; break;
			default: dcasserta(0);
			}
		}
		// No need to convert back to acp since our utf8Header denotes we
		// should store it as utf8.
	}
	return aString;
}

void SimpleXML::Tag::appendAttribString(string& tmp) {
	for(StringPairIter i = attribs.begin(); i!= attribs.end(); ++i) {
		tmp.append(i->first);
		tmp.append("=\"", 2);
		if(needsEscape(i->second, true)) {
			string tmp2(i->second);
			escape(tmp2, true);
			tmp.append(tmp2);
		} else {
			tmp.append(i->second);
		}
		tmp.append("\" ", 2);
	}
	tmp.erase(tmp.size()-1);
}

/**
 * The same as the version above, but writes to a file instead...yes, this could be made
 * with streams and only one code set but streams are slow...the file f should be a buffered
 * file, otherwise things will be very slow (I assume write is not expensive and call it a lot
 */
void SimpleXML::Tag::toXML(int indent, OutputStream* f) {
	if(children.empty() && data.empty()) {
		string tmp;
		tmp.reserve(indent + name.length() + 30);
		tmp.append(indent, '\t');
		tmp.append(1, '<');
		tmp.append(name);
		tmp.append(1, ' ');
		appendAttribString(tmp);
		tmp.append("/>\r\n", 4);
		f->write(tmp);
	} else {
		string tmp;
		tmp.append(indent, '\t');
		tmp.append(1, '<');
		tmp.append(name);
		tmp.append(1, ' ');
		appendAttribString(tmp);
		if(children.empty()) {
			tmp.append(1, '>');
			if(needsEscape(data, false)) {
				string tmp2(data);
				escape(tmp2, false);
				tmp.append(tmp2);
			} else {
				tmp.append(data);
			}
		} else {
			tmp.append(">\r\n", 3);
			f->write(tmp);
			tmp.clear();
			for(Iter i = children.begin(); i!=children.end(); ++i) {
				(*i)->toXML(indent + 1, f);
			}
			tmp.append(indent, '\t');
		}
		tmp.append("</", 2);
		tmp.append(name);
		tmp.append(">\r\n", 3);
		f->write(tmp);
	}
}

bool SimpleXML::findChild(const string& aName) throw() {
	dcassert(current != NULL);

	if(found && currentChild != current->children.end())
		currentChild++;

	while(currentChild!=current->children.end()) {
		if((*currentChild)->name == aName) {
			found = true;
			return true;
		} else
			currentChild++;
	}
	return false;
}

string::size_type SimpleXMLReader::loadAttribs(const string& name, const string& tmp, string::size_type start) throw(SimpleXMLException) {
	string::size_type i = start;
	string::size_type j;

	for(;;) {
		if((j = tmp.find_first_of("= \"'/>", i)) == string::npos) {
			throw SimpleXMLException("Missing '=' in " + name);
		}
		if(tmp[j] != '=') {
			throw SimpleXMLException("Missing '=' in " + name);
		}

		if(tmp[j+1] != '"' && tmp[j+1] != '\'') {
			throw SimpleXMLException("Invalid character after '=' in " + name);
		}

		string::size_type x = j + 2;
		string::size_type y;
		if((y = tmp.find(tmp[j+1], x)) == string::npos) {
			throw SimpleXMLException("Missing '" + string(1, tmp[j+1]) + "' in " + name);
		}
		// Ok, we have an attribute...
		attribs.push_back(make_pair(tmp.substr(i, j-i), tmp.substr(x, y-x)));
		SimpleXML::escape(attribs.back().second, true, true, encoding);

		i = tmp.find_first_not_of(' ', y + 1);
		if(tmp[i] == '/' || tmp[i] == '>') {
			return i;
		}
	}
}

string::size_type SimpleXMLReader::fromXML(const string& tmp, const string& n, string::size_type start, int depth) throw(SimpleXMLException) {
	string::size_type i = start;
	string::size_type j;

	bool hasChildren = false;

	for(;;) {
		if((j = tmp.find('<', i)) == string::npos) {
			if(depth > 0) {
				throw SimpleXMLException("Missing end tag in " + n);
			}
			return tmp.size();
		} else if(depth > maxNesting) {
			throw SimpleXMLException("Too many nested tags (depth >" + Util::toString(maxNesting) + ")");
		}

		// Check that we have at least 3 more characters as the shortest valid xml tag is <a/>...
		if((j + 3) > tmp.size()) {
			throw SimpleXMLException("Missing end tag in " + n);
		}

		i = j + 1;

		if(tmp[i] == '?') {
			// <? processing instruction ?>, check encoding...
			if((j = tmp.find("?>", i)) == string::npos) {
				throw SimpleXMLException("Missing '?>' in " + n);
			}

			string str = tmp.substr(i, j - i);
			if((i = str.find("encoding=\"")) != string::npos) {
				string::size_type k = str.find('\"', i + 10);
				if(k != string::npos && k < j) {
					encoding = str.substr(i + 10, k - i - 10);
				}
 			}

			i = j + 2;
			continue;
		}

		if(tmp[i] == '!' && tmp[i+1] == '-' && tmp[i+2] == '-') {
			// <!-- comment -->, ignore...
			if((j = tmp.find("-->", i)) == string::npos) {
				throw SimpleXMLException("Missing '-->' in " + n);
			}
			i = j + 3;
			continue;
		}

		// Check if we reached the end tag
		if(tmp[i] == '/') {
			i++;

			if( (tmp.compare(i, n.length(), n) == 0) &&
				(tmp[i + n.length()] == '>') )
			{
				if(!hasChildren) {
					data = tmp.substr(start, i - start - 2);
					SimpleXML::escape(data, false, true, encoding);
				} else {
					data.clear();
				}
				return i + n.length() + 1;
			} else {
				throw SimpleXMLException("Missing end tag in " + n);
			}
		}

		// Alright, we have a real tag for sure...now get the name of it.
		if((j = tmp.find_first_of(" />", i)) == string::npos) {
			throw SimpleXMLException("Missing '>' in " + n);
		}

		string name = tmp.substr(i, j-i);
		hasChildren = true;
		if(tmp[j] == ' ') {
			if((j = tmp.find_first_not_of(' ', j+1)) == string::npos) {
				throw SimpleXMLException("Missing '>' in " + name);
			}
		}

		attribs.clear();
		if(tmp[j] != '/' && tmp[j] != '>') {
			// We have attribs...
			j = loadAttribs(name, tmp, j);
		}

		if(tmp[j] == '>') {
			// This is a real tag with data etc...
			cb->startTag(name, attribs, false);
			j = fromXML(tmp, name, j+1, depth+1);
			cb->endTag(name, data);
		} else {
			// A simple tag (<xxx/>
			cb->startTag(name, attribs, true);
			j++;
		}
		i = j;
	}
}

void SimpleXML::addTag(const string& aName, const string& aData /* = "" */) throw(SimpleXMLException) {
	if(aName.empty()) {
		throw SimpleXMLException("Empty tag names not allowed");
	}

	if(current == &root && !current->children.empty()) {
		throw SimpleXMLException("Only one root tag allowed");
	} else {
		current->children.push_back(new Tag(aName, aData, current));
		currentChild = current->children.end() - 1;
	}
}

void SimpleXML::addAttrib(const string& aName, const string& aData) throw(SimpleXMLException) {
	if(current == &root)
		throw SimpleXMLException("No tag is currently selected");

	current->attribs.push_back(make_pair(aName, aData));
}

void SimpleXML::addChildAttrib(const string& aName, const string& aData) throw(SimpleXMLException) {
	checkChildSelected();

	(*currentChild)->attribs.push_back(make_pair(aName, aData));
}

void SimpleXML::fromXML(const string& aXML) throw(SimpleXMLException) {
	if(!root.children.empty()) {
		delete root.children[0];
		root.children.clear();
	}

	TagReader t(&root);
	SimpleXMLReader(&t).fromXML(aXML);

	if(root.children.size() != 1) {
		throw SimpleXMLException("Invalid XML file, missing or multiple root tags");
	}

	current = &root;
	resetCurrentChild();
}

} // namespace dcpp
