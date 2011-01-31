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
            default: dcassert(0);
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
        SimpleXMLReader(&t).parse(aXML.c_str(), aXML.size(), false);

    if(root.children.size() != 1) {
        throw SimpleXMLException("Invalid XML file, missing or multiple root tags");
    }

    current = &root;
    resetCurrentChild();
}

} // namespace dcpp
